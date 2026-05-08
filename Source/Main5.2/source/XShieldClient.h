#pragma once

#if (ANTIHACK_GGNEW)

class CXShieldClient
{
public:
	CXShieldClient();
	~CXShieldClient();

	void Init(const char* defaultIpAddress);
	void Shutdown();

private:
	static DWORD WINAPI ThreadProc(LPVOID lpParam);

	void LoadConfig(const char* defaultIpAddress);
	void Run();
	bool ConnectServer();
	void Disconnect();
	bool SendClientInfo();
	bool SendConnectionStatus();
	bool SendAll(const BYTE* data, int size);
	bool PumpRecv();
	void BuildCurrentAccount(char* account, int accountSize, char* character, int characterSize);

	bool m_Enable;
	char m_IpAddress[64];
	WORD m_Port;
	char m_HackVersion[8];
	DWORD m_ClientFileCRC;
	SOCKET m_Socket;
	HANDLE m_Thread;
	volatile LONG m_Running;
};

extern CXShieldClient gXShieldClient;

#endif
