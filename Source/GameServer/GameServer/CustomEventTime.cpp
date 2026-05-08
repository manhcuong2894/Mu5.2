#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "CustomEventTime.h"
#include "MemScript.h"
#include "CEventName.h"
#include "Notice.h"

CCustomEventTime gCustomEventTime;


//void CCustomEventTime::GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
//{
//
//#if (GAMESERVER_CLIENTE_UPDATE >= 8)
//
//	if (gServerInfo.m_CustomEventTimeSwitch == 0)
//	{
//		return;
//	}
//
//	if (gObjIsConnected(Index) == false)
//	{
//		return;
//	}
//
//	BYTE send[4096];
//
//	PMSG_CUSTOM_EVENTTIME_SEND pMsg;
//
//	pMsg.header.set(0xF3, 0xE8, 0);
//
//	int size = sizeof(pMsg);
//
//	pMsg.count = 0;
//
//	CUSTOM_EVENTTIME_DATA info;
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n;
//
//		info.time = gEventName.GlobalRemainTime(n);
//		/*if (n == 0)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 1)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 2)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 3)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 4)
//			info.time = gServerDisplayer.EventCustomLottery;
//
//		if (n == 5)
//			info.time = gServerDisplayer.EventCustomQuiz;
//
//		if (n == 6)
//			info.time = gServerDisplayer.EventCustomBonus;
//
//		if (n == 7)
//			info.time = gServerDisplayer.EventMoss;
//
//		if (n == 8)
//			info.time = gServerDisplayer.EventDrop;
//
//		if (n == 9)
//			info.time = gServerDisplayer.EventKing;
//
//		if (n == 10)
//			info.time = gServerDisplayer.EventTvT;
//
//		if (n == 11)
//			info.time = -1;
//
//		if (n == 12)
//			info.time = -1;
//
//		if (n == 13)
//			info.time = -1;
//
//		if (n > 13)
//			info.time = -1;*/
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n + 20;
//
//		info.time = gEventName.InvasionRemainTime(n);
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n + 40;
//
//		info.time = gEventName.ArenaRemainTime(n);
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	pMsg.header.size[0] = SET_NUMBERHB(size);
//	pMsg.header.size[1] = SET_NUMBERLB(size);
//	// ---
//	memcpy(send, &pMsg, sizeof(pMsg));
//
//	DataSend(Index, send, size);
//
//#endif
//	return;
//}


int MaxPerPage = 14;
void CCustomEventTime::GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 2)

	if (gServerInfo.m_CustomEventTimeSwitch == 0)
	{
		return;
	}

	if (gObjIsConnected(Index) == false)
	{
		return;
	}

	int GetPage = lpMsg->Page;

	BYTE send[4096];

	PMSG_CUSTOM_EVENTTIME_SEND pMsg;

	pMsg.header.set(0xF3, 0xE8, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	memset(pMsg.EventTypeName, 0, sizeof(pMsg.EventTypeName));

	// Get dynamic EventType count from XML config
	int eventTypeCount = gEventName.GetEventTypeCount();
	if (eventTypeCount <= 0) eventTypeCount = 1;
	if (eventTypeCount > MAX_EVENT_TYPES) eventTypeCount = MAX_EVENT_TYPES;

	// Build separate index lists per EventType
	std::vector<int> typeIndices[MAX_EVENT_TYPES];

	for (int n = 0; n < (int)gEventName.m_SendClientDataEventTime.size(); n++)
	{
		int type = gEventName.m_SendClientDataEventTime[n].switch_on;
		if (type >= 0 && type < eventTypeCount)
		{
			typeIndices[type].push_back(n);
		}
	}

	// Calculate pages per EventType
	int pagesPerType[MAX_EVENT_TYPES] = { 0 };
	int totalPages = 0;
	for (int t = 0; t < eventTypeCount; t++)
	{
		if (!typeIndices[t].empty())
		{
			pagesPerType[t] = ((int)typeIndices[t].size() + MaxPerPage - 1) / MaxPerPage;
		}
		totalPages += pagesPerType[t];
	}

	// Set MaxList so client formula (MaxList/14)+1 gives correct total pages
	if (totalPages > 0)
		pMsg.MaxList = (totalPages - 1) * MaxPerPage;
	else
		pMsg.MaxList = 0;

	// Determine which EventType and sub-page for the requested page
	int targetType = -1;
	int subPage = 0;
	int pageAccum = 0;
	for (int t = 0; t < eventTypeCount; t++)
	{
		if (pagesPerType[t] == 0)
			continue;

		if (GetPage < pageAccum + pagesPerType[t])
		{
			targetType = t;
			subPage = GetPage - pageAccum;
			break;
		}
		pageAccum += pagesPerType[t];
	}

	// Set EventTypeName for the current page
	if (targetType >= 0 && targetType < eventTypeCount)
	{
		strncpy(pMsg.EventTypeName, gEventName.GetEventTypeName(targetType), sizeof(pMsg.EventTypeName) - 1);

		// Send entries from the target EventType, starting from subPage
		int startIdx = subPage * MaxPerPage;
		int endIdx = startIdx + MaxPerPage;
		if (endIdx > (int)typeIndices[targetType].size())
			endIdx = (int)typeIndices[targetType].size();

		CUSTOM_EVENTTIME_DATA info;

		for (int i = startIdx; i < endIdx; i++)
		{
			int n = typeIndices[targetType][i];

			int CatE = gEventName.m_SendClientDataEventTime[n].switch_on;
			int IndexE = gEventName.m_SendClientDataEventTime[n].m_Key;

			info.index = n;
			info.time = gEventName.GetTimeEventSwitch(CatE, IndexE);
			info.NumberGate = gEventName.m_SendClientDataEventTime[n].GetGate();

			memset(&info.NameEvent, 0, sizeof(info.NameEvent));
			strncpy(info.NameEvent, gEventName.m_SendClientDataEventTime[n].GetName(), sizeof(info.NameEvent) - 1);

			memset(&info.DesString, 0, sizeof(info.DesString));
			strncpy(info.DesString, gEventName.m_SendClientDataEventTime[n].GetDes(), sizeof(info.DesString) - 1);

			pMsg.count++;
			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Index, send, size);

#endif
	return;
}