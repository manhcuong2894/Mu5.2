#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "MemScript.h"
#include "CustomRanking.h"
#include "ObjectManager.h"
#include "ItemManager.h"
#include "Notice.h"

CCustomRanking gCustomRanking;

void CCustomRanking::Load(char* path)
{

	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	for (int n = 0; n < MAX_RANK; n++)
	{
		this->r_Data[n];
	}

	this->m_count = 0;

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					strcpy_s(this->r_Data[index].Name, lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col1, lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col2, lpMemScript->GetAsString());

					this->m_count++;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomRanking::GCReqRankingCount(int Index, PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	if (gServerInfo.m_CustomRankingSwitch == 0)
	{
		return;
	}

	PMSG_CUSTOM_RANKING_COUNT_SEND pMsg;

	pMsg.header.set(0xF3, 0xE7, sizeof(pMsg));

	pMsg.count = (this->m_count > MAX_RANK) ? MAX_RANK : this->m_count;

	memset(pMsg.RankNames, 0, sizeof(pMsg.RankNames));
	for(int i = 0; i < pMsg.count; i++)
	{
		memcpy(pMsg.RankNames[i], this->r_Data[i].Name, sizeof(pMsg.RankNames[i]));
	}

	DataSend(Index, (LPBYTE)&pMsg, sizeof(pMsg));
#endif
}

void CCustomRanking::GCReqRanking(int Index, PMSG_CUSTOM_RANKING_RECV* lpMsg)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	if (gObjIsConnected(Index) != false)
	{
		if (gServerInfo.m_CustomRankingSwitch == 0)
		{
			return;
		}

		if (this->m_count <= 0)
		{
			return;
		}

		if (lpMsg->type < 0 || lpMsg->type >= this->m_count)
		{
			return;
		}

		if (this->m_count == 0)
		{
			return;
		}

		SDHP_CUSTOM_RANKING_SEND pMsg;

		pMsg.header.set(0xF4, sizeof(pMsg));

		pMsg.index = Index;

		pMsg.type = lpMsg->type;

		gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.header.size);
	}
#endif
}

void CCustomRanking::GDCustomRankingRecv(BYTE* ReceiveBuffer)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	SDHP_CUSTOM_RANKING_RECV* Data = (SDHP_CUSTOM_RANKING_RECV*)ReceiveBuffer;

	BYTE send[4096];

	PMSG_CUSTOM_RANKING_SEND pMsg;

	pMsg.header.set(0xF3, 0xE6, 0);

	pMsg.RankIndex = Data->type;

	memcpy(pMsg.rankname, this->r_Data[Data->type].Name, sizeof(pMsg.rankname));

	memcpy(pMsg.col1, this->r_Data[Data->type].Col1, sizeof(pMsg.col1));

	memcpy(pMsg.col2, this->r_Data[Data->type].Col2, sizeof(pMsg.col2));

	int size = sizeof(pMsg);

	int offset = sizeof(SDHP_CUSTOM_RANKING_RECV);

	pMsg.count = 0;

	for (int n = 0; n < Data->count; n++)
	{
		CUSTOM_RANKING_DATA* Data2 = (CUSTOM_RANKING_DATA*)(ReceiveBuffer + offset);

		if ((size + sizeof(CUSTOM_RANKING_DATA)) < sizeof(send))
		{
			memcpy(&send[size], Data2, sizeof(CUSTOM_RANKING_DATA));
			size += sizeof(CUSTOM_RANKING_DATA);
			pMsg.count++;
		}

		offset += (sizeof(CUSTOM_RANKING_DATA));
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Data->index, send, size);
#endif
}
void CCustomRanking::CGetInfoCharTop(REQUESTINFO_CHARTOP * lpMsg, int aIndex)
{
	if (lpMsg == NULL) return;
	if (gObjIsConnected(aIndex) == false) return;
	
	char szTargetName[11] = {0};
	memcpy(szTargetName, lpMsg->NameChar, sizeof(lpMsg->NameChar));
	szTargetName[10] = 0;
	
	LPOBJ lpTarget = gObjFind(szTargetName);
	if (lpTarget != NULL)
	{
		DATA_VIEWTOPRANKING localMsg;
		memset(&localMsg, 0, sizeof(localMsg));
		localMsg.aIndex = aIndex;
		memcpy(localMsg.NameChar, szTargetName, sizeof(localMsg.NameChar));
		localMsg.Class = lpTarget->Class;
		
		for (int n = 0; n < 12; n++)
		{
			if (lpTarget->Inventory[n].IsItem())
			{
				gItemManager.DBItemByteConvert(localMsg.Item[n], &lpTarget->Inventory[n]);
			}
			else
			{
				memset(localMsg.Item[n], 0xFF, 16);
			}
		}
		
		this->RecvInfoCharTop(&localMsg);
		return;
	}

	REQUESTINFO_CHARTOP pMsg;
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.header.set(0xD3, 0x40, sizeof(pMsg));
	pMsg.aIndex = aIndex;
	memcpy(pMsg.NameChar, lpMsg->NameChar, sizeof(pMsg.NameChar));
	pMsg.NameChar[10] = 0; // Ensure trailing null
	
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.header.size);
}

#define EQUIPMENT_LENGTH 17
struct DATA_VIEWTOPRANKING_TO_CLIENT
{
	PSWMSG_HEAD header; // GameServer usually uses PWMSG_HEAD for word size
	char NameChar[11];
	int aIndex;
	DWORD Equipment[EQUIPMENT_LENGTH];
};

void CCustomRanking::RecvInfoCharTop(DATA_VIEWTOPRANKING * lpMsg)
{
	if (lpMsg == NULL) return;
	int aIndex = lpMsg->aIndex;
	if (gObjIsConnected(aIndex) == false) return;

	DATA_VIEWTOPRANKING_TO_CLIENT pMsg;
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.header.set(0xD3, 0x40, sizeof(pMsg));
	memcpy(pMsg.NameChar, lpMsg->NameChar, sizeof(pMsg.NameChar));
	pMsg.aIndex = aIndex;

	CItem lpItemTest;
	for (int n = 0; n < EQUIPMENT_LENGTH; n++)
	{
		if (n < 12) {
			lpItemTest.Clear();
			gItemManager.ConvertItemByte(&lpItemTest, lpMsg->Item[n]);
			lpItemTest.m_IsValidItem = true;
			if (lpItemTest.IsItem() && lpItemTest.m_Index != 0xFFFF)
			{
				DWORD packedData = 0;
				packedData |= ((DWORD)(lpItemTest.m_Index & 0x1FFF)) << 19;
				packedData |= (lpItemTest.m_Level & 0x0F) << 15;
				packedData |= (lpItemTest.m_NewOption & 0x3F) << 9;
				packedData |= (lpItemTest.m_SetOption & 0x03) << 7;
				pMsg.Equipment[n] = packedData;
			}
			else
			{
				pMsg.Equipment[n] = 0xFFFFFFFF;
			}
		} else {
			pMsg.Equipment[n] = 0xFFFFFFFF;
		}
	}
	
	//LogAdd(LOG_RED, "[F8DEBUG] Equipment Sent - Name:%s, C:%d, Wing[7]: %02X %02X %02X %02X => PK:%08X", pMsg.NameChar, lpMsg->Class, lpMsg->Item[7][0], lpMsg->Item[7][1], lpMsg->Item[7][2], lpMsg->Item[7][3], pMsg.Equipment[7]);

	DataSend(aIndex, (BYTE*)&pMsg, sizeof(pMsg));
}













