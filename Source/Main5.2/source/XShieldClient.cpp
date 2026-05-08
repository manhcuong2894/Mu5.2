#include "stdafx.h"
#include "XShieldClient.h"

#if (ANTIHACK_GGNEW)

#include "Util.h"
#include "CGMProtect.h"
#include "ZzzCharacter.h"
#include "ZzzScene.h"
#include "./Utilities/Log/ErrorReport.h"
#include "..\..\..\Util\CCRC32.H"

extern CErrorReport g_ErrorReport;
extern char LogInID[MAX_ID_SIZE + 1];

namespace
{
	struct PBMSG_HEAD_XSHIELD
	{
		void set(BYTE head, BYTE size)
		{
			this->type = 0xC1;
			this->size = size;
			this->key = (BYTE)(GetTickCount() & 0xFF);
			this->head = head;
		}

		BYTE type;
		BYTE size;
		BYTE key;
		BYTE head;
	};

#pragma pack(push, 1)
	struct SDHP_CLIENT_INFO_RECV_XSHIELD
	{
		PBMSG_HEAD_XSHIELD header;
		DWORD IsReconnect;
		DWORD ClientFileCRC;
		char HackVersion[8];
		char HardwareId[36];
	};

	struct SDHP_CONNECTION_STATUS_RECV_XSHIELD
	{
		PBMSG_HEAD_XSHIELD header;
		char account[11];
		char name[11];
	};
#pragma pack(pop)

	void CopyLimited(char* dest, int destSize, const char* source, int sourceMaxLen)
	{
		if (dest == 0 || destSize <= 0)
		{
			return;
		}

		ZeroMemory(dest, destSize);

		if (source == 0)
		{
			return;
		}

		int len = 0;

		while (len < sourceMaxLen && source[len] != 0)
		{
			len++;
		}

		if (len >= destSize)
		{
			len = destSize - 1;
		}

		if (len > 0)
		{
			memcpy(dest, source, len);
		}
	}

	void PacketEncryptDataXShield(BYTE* lpMsg, int size, BYTE key)
	{
		for (int n = 0; n < size; n++)
		{
			lpMsg[n] = (lpMsg[n] ^ 0xA0) - key;
		}
	}

	void PacketDecryptDataXShield(BYTE* lpMsg, int size, BYTE key)
	{
		for (int n = 0; n < size; n++)
		{
			lpMsg[n] = (lpMsg[n] + key) ^ 0xA0;
		}
	}

	bool EncryptPacketXShield(BYTE* data, int size)
	{
		if (data == 0 || size <= 0)
		{
			return false;
		}

		if (data[0] == 0xC1)
		{
			if (size < 4)
			{
				return false;
			}

			PacketEncryptDataXShield(&data[3], (size - 3), data[2]);
			return true;
		}

		if (data[0] == 0xC2)
		{
			if (size < 5)
			{
				return false;
			}

			PacketEncryptDataXShield(&data[4], (size - 4), data[3]);
			return true;
		}

		return false;
	}

	void XShieldLog(const char* format, ...)
	{
		char buffer[1024] = { 0 };
		va_list args;
		va_start(args, format);
		vsprintf_s(buffer, sizeof(buffer), format, args);
		va_end(args);

		OutputDebugStringA(buffer);

		HANDLE file = CreateFileA(".\\XShieldClient.log", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (file != INVALID_HANDLE_VALUE)
		{
			DWORD written = 0;
			WriteFile(file, buffer, (DWORD)strlen(buffer), &written, 0);
			CloseHandle(file);
		}
	}
}

CXShieldClient gXShieldClient;

CXShieldClient::CXShieldClient()
{
	this->m_Enable = false;
	ZeroMemory(this->m_IpAddress, sizeof(this->m_IpAddress));
	this->m_Port = 55111;
	ZeroMemory(this->m_HackVersion, sizeof(this->m_HackVersion));
	this->m_ClientFileCRC = 0;
	this->m_Socket = INVALID_SOCKET;
	this->m_Thread = 0;
	this->m_Running = 0;
}

CXShieldClient::~CXShieldClient()
{
	this->Shutdown();
}

void CXShieldClient::Init(const char* defaultIpAddress)
{
	if (this->m_Thread != 0)
	{
		return;
	}

	this->LoadConfig(defaultIpAddress);

	if (this->m_Enable == false)
	{
		XShieldLog("[XShieldClient] disabled by MainConnect.ini\r\n");
		return;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		XShieldLog("[XShieldClient] WSAStartup failed: %d\r\n", WSAGetLastError());
		return;
	}

	InterlockedExchange(&this->m_Running, 1);
	this->m_Thread = CreateThread(0, 0, CXShieldClient::ThreadProc, this, 0, 0);

	if (this->m_Thread == 0)
	{
		InterlockedExchange(&this->m_Running, 0);
		WSACleanup();
		XShieldLog("[XShieldClient] CreateThread failed: %d\r\n", GetLastError());
	}
}

void CXShieldClient::Shutdown()
{
	if (this->m_Thread == 0)
	{
		return;
	}

	InterlockedExchange(&this->m_Running, 0);
	this->Disconnect();

	if (WaitForSingleObject(this->m_Thread, 3000) == WAIT_TIMEOUT)
	{
		TerminateThread(this->m_Thread, 0);
	}

	CloseHandle(this->m_Thread);
	this->m_Thread = 0;
	WSACleanup();
}

void CXShieldClient::LoadConfig(const char* defaultIpAddress)
{
	this->m_Enable = GMProtect->IsXShieldEnabled();
	CopyLimited(this->m_IpAddress, sizeof(this->m_IpAddress), defaultIpAddress ? defaultIpAddress : "", sizeof(this->m_IpAddress) - 1);
	this->m_Port = GMProtect->GetXShieldAntiHackPort();
	CopyLimited(this->m_HackVersion, sizeof(this->m_HackVersion), GMProtect->GetXShieldHackVersion(), sizeof(this->m_HackVersion) - 1);

	if (this->m_HackVersion[0] == 0)
	{
		CopyLimited(this->m_HackVersion, sizeof(this->m_HackVersion), "PREMIUM", 7);
	}

	CCRC32 CRC32;

	if (CRC32.FileCRC(".\\Main.exe", &this->m_ClientFileCRC, 1024) == 0)
	{
		this->m_ClientFileCRC = 0;
	}

	XShieldLog("[XShieldClient] Antihack Enable=%d IP=%s Port=%d Version=%s MainCRC=%08X\r\n",
		this->m_Enable ? 1 : 0,
		this->m_IpAddress,
		this->m_Port,
		this->m_HackVersion,
		this->m_ClientFileCRC);
}

DWORD WINAPI CXShieldClient::ThreadProc(LPVOID lpParam)
{
	((CXShieldClient*)lpParam)->Run();
	return 0;
}

void CXShieldClient::Run()
{
	while (this->m_Running != 0)
	{
		if (this->ConnectServer() == false)
		{
			Sleep(5000);
			continue;
		}

		this->SendClientInfo();
		this->SendConnectionStatus();

		DWORD lastStatusTick = GetTickCount();

		while (this->m_Running != 0 && this->m_Socket != INVALID_SOCKET)
		{
			this->PumpRecv();

			DWORD now = GetTickCount();

			if ((now - lastStatusTick) >= 5000)
			{
				if (this->SendConnectionStatus() == false)
				{
					this->Disconnect();
					break;
				}

				lastStatusTick = now;
			}

			Sleep(50);
		}

		this->Disconnect();
	}
}

bool CXShieldClient::ConnectServer()
{
	if (this->m_IpAddress[0] == 0)
	{
		return false;
	}

	this->m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->m_Socket == INVALID_SOCKET)
	{
		XShieldLog("[XShieldClient] socket failed: %d\r\n", WSAGetLastError());
		return false;
	}

	DWORD timeout = 3000;
	setsockopt(this->m_Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	setsockopt(this->m_Socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

	sockaddr_in addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->m_Port);
	addr.sin_addr.s_addr = inet_addr(this->m_IpAddress);

	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(this->m_IpAddress);

		if (host == 0 || host->h_addr_list == 0 || host->h_addr_list[0] == 0)
		{
			XShieldLog("[XShieldClient] resolve failed: %s\r\n", this->m_IpAddress);
			this->Disconnect();
			return false;
		}

		memcpy(&addr.sin_addr, host->h_addr_list[0], sizeof(addr.sin_addr));
	}

	if (connect(this->m_Socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		XShieldLog("[XShieldClient] connect %s:%d failed: %d\r\n", this->m_IpAddress, this->m_Port, WSAGetLastError());
		this->Disconnect();
		return false;
	}

	u_long nonBlocking = 1;
	ioctlsocket(this->m_Socket, FIONBIO, &nonBlocking);

	XShieldLog("[XShieldClient] connected %s:%d\r\n", this->m_IpAddress, this->m_Port);
	return true;
}

void CXShieldClient::Disconnect()
{
	if (this->m_Socket != INVALID_SOCKET)
	{
		closesocket(this->m_Socket);
		this->m_Socket = INVALID_SOCKET;
	}
}

bool CXShieldClient::SendClientInfo()
{
	SDHP_CLIENT_INFO_RECV_XSHIELD pMsg;
	ZeroMemory(&pMsg, sizeof(pMsg));

	pMsg.header.set(0x00, sizeof(pMsg));
	pMsg.IsReconnect = 0;
	pMsg.ClientFileCRC = this->m_ClientFileCRC;
	CopyLimited(pMsg.HackVersion, sizeof(pMsg.HackVersion), this->m_HackVersion, sizeof(pMsg.HackVersion) - 1);
	create_hwid_system(pMsg.HardwareId);

	bool result = this->SendAll((BYTE*)&pMsg, sizeof(pMsg));
	XShieldLog("[XShieldClient] SendClientInfo result=%d HWID=%.*s\r\n", result ? 1 : 0, (int)sizeof(pMsg.HardwareId), pMsg.HardwareId);
	return result;
}

bool CXShieldClient::SendConnectionStatus()
{
	SDHP_CONNECTION_STATUS_RECV_XSHIELD pMsg;
	ZeroMemory(&pMsg, sizeof(pMsg));

	pMsg.header.set(0x01, sizeof(pMsg));
	this->BuildCurrentAccount(pMsg.account, sizeof(pMsg.account), pMsg.name, sizeof(pMsg.name));

	bool result = this->SendAll((BYTE*)&pMsg, sizeof(pMsg));
	XShieldLog("[XShieldClient] SendStatus result=%d Account=%s Character=%s\r\n", result ? 1 : 0, pMsg.account, pMsg.name);
	return result;
}

bool CXShieldClient::SendAll(const BYTE* data, int size)
{
	BYTE sendBuffer[2048];

	if (data == 0 || size <= 0 || size > sizeof(sendBuffer))
	{
		XShieldLog("[XShieldClient] invalid packet size: %d\r\n", size);
		return false;
	}

	memcpy(sendBuffer, data, size);

	if (EncryptPacketXShield(sendBuffer, size) == false)
	{
		XShieldLog("[XShieldClient] packet encrypt failed: %02X size=%d\r\n", sendBuffer[0], size);
		return false;
	}

	int sent = 0;

	while (sent < size)
	{
		int result = send(this->m_Socket, (const char*)sendBuffer + sent, size - sent, 0);

		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK)
			{
				Sleep(10);
				continue;
			}

			XShieldLog("[XShieldClient] send failed: %d\r\n", error);
			return false;
		}

		if (result == 0)
		{
			XShieldLog("[XShieldClient] server closed connection\r\n");
			return false;
		}

		sent += result;
	}

	return true;
}

bool CXShieldClient::PumpRecv()
{
	BYTE buffer[1024];

	for (;;)
	{
		int result = recv(this->m_Socket, (char*)buffer, sizeof(buffer), 0);

		if (result > 0)
		{
			if (result >= 4 && buffer[0] == 0xC1 && buffer[1] >= 4 && buffer[1] <= result)
			{
				BYTE packet[1024];
				memcpy(packet, buffer, result);
				PacketDecryptDataXShield(&packet[3], (packet[1] - 3), packet[2]);
				XShieldLog("[XShieldClient] Recv C1 Head=%02X Size=%d Result=%d\r\n", packet[3], packet[1], (packet[1] > 4) ? packet[4] : -1);
			}

			continue;
		}

		if (result == 0)
		{
			XShieldLog("[XShieldClient] server closed connection\r\n");
			this->Disconnect();
			return false;
		}

		int error = WSAGetLastError();

		if (error == WSAEWOULDBLOCK || error == WSAETIMEDOUT)
		{
			return true;
		}

		XShieldLog("[XShieldClient] recv failed: %d\r\n", error);
		this->Disconnect();
		return false;
	}
}

void CXShieldClient::BuildCurrentAccount(char* account, int accountSize, char* character, int characterSize)
{
	CopyLimited(account, accountSize, LogInID, MAX_ID_SIZE);
	ZeroMemory(character, characterSize);

	if (SceneFlag == MAIN_SCENE && Hero != 0 && Hero->ID[0] != 0)
	{
		CopyLimited(character, characterSize, Hero->ID, MAX_ID_SIZE);
	}
}

#endif
