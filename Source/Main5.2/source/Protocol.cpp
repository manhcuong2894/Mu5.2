#include "stdafx.h"
#include "Protocol.h"
#include "WSclient.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "SkillManager.h"
#include "CharacterManager.h"
#include "CBInterface.h"
#if (CB_GETMIXRATE)
#include "CB_GetMixRate.h"
#endif
#include "CustomEventTime.h"
#include "CB_DangKyInGame.h"
#include "H_BotMix.h"
#include "H_RankingDmgBoss.h"
#include "H_ViewCharInfo.h"
#include "CustomChoTroi.h"
#include "TextClient.h"
#include "APICB.h"

#if(CUSTOM_CHANGEITEM)
	namespace {
		const int MAX_CHANGEITEM_INFO_WIDTH = 285;

		int GetChangeItemInfoTextWidth(const std::string& text) {
			if (text.empty()) {
				return 0;
			}

			SIZE textSize = { 0 };
			HFONT fontBack = g_pRenderText->GetFont();
			g_pRenderText->SetFont(g_hFontBold);

			if (g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), text.c_str(), (int)text.length(), &textSize) == FALSE) {
				g_pRenderText->SetFont(fontBack);
				return ((int)text.length() * 6);
			}

			g_pRenderText->SetFont(fontBack);
			return (int)((float)textSize.cx / g_fScreenRate_x);
		}

		void AppendWrappedChangeItemPart(std::string& result, const std::string& part) {
			if (part.empty()) {
				return;
			}

			if (result.empty()) {
				result = part;
				return;
			}

			size_t lineBreak = result.find_last_of('\n');
			std::string currentLine = (lineBreak == std::string::npos) ? result : result.substr(lineBreak + 1);
			std::string candidateLine = currentLine + ", " + part;

			if (GetChangeItemInfoTextWidth(candidateLine) > MAX_CHANGEITEM_INFO_WIDTH) {
				result += "\n";
				result += part;
				return;
			}

			result += ", ";
			result += part;
		}

		void BuildChangeItemRequiredInfo(INFO_CHANGEITEM_CLIENT* recv, char* text, size_t size) {
			if (recv == 0 || text == 0 || size == 0) {
				return;
			}

			std::string result;

			auto appendPart = [&result](const std::string& part) {
				AppendWrappedChangeItemPart(result, part);
			};

			if (recv->RequiredLevel > 0) {
				char buff[120];
				sprintf_s(buff, gTextClient.txtClient_ChangeItem[4], recv->RequiredLevel);
				appendPart(buff);
			}

			if (recv->RequiredSkill > 0) {
				appendPart(gTextClient.txtClient_ChangeItem[5]);
			}

			if (recv->RequiredLuck > 0) {
				appendPart(gTextClient.txtClient_ChangeItem[6]);
			}

			if (recv->RequiredOption > 0) {
				char buff[120];
				sprintf_s(buff, gTextClient.txtClient_ChangeItem[7], recv->RequiredOption);
				appendPart(buff);
			}

			if (recv->RequiredExc > 0) {
				appendPart(gTextClient.txtClient_ChangeItem[8]);
			}

			strncpy_s(text, size, result.c_str(), _TRUNCATE);
		}
	}
#endif


BOOL ProtocolCoreEx(BYTE head, BYTE* lpMsg, int size, int key) // OK
{
	switch (head)
	{
#if(CB_GETMIXRATE)
	case 0x88:
		if (gCB_GetMixRate) gCB_GetMixRate->GCRecvMixInfo(lpMsg, size);
		break;
#endif
	case 0xF3:
		switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
		{
		case 0xE8:
			g_CustomEventTime->GCReqEventTime((PMSG_CUSTOM_EVENTTIME_RECV*)lpMsg);
			return 1;

		}
		break;
	case 0xD3:
		switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
		{
#if(ANTIHACK_GGNEW)
		case 0xAC: //
		{
			gAPICB.Recv(lpMsg);
			return 1;
		}
		break;
#endif
#if(CB_DANGKYINGAME)
		case 0x05:
			gCB_DangKyInGame->RecvKQRegInGame((XULY_CGPACKET*)lpMsg);
			break;
#endif
#if(H_VIEWCHARINFO)
		case 0x02:
			if (gH_ViewCharInfo == NULL)
			{
				gH_ViewCharInfo = new H_ViewCharInfo;
			}
			if (gH_ViewCharInfo)
			{
				gH_ViewCharInfo->RecvProtocol(lpMsg);
			}
			return 1;
#endif
#if(H_RANKINGDMGBOSS)
		case 0x2D:
			gDmgBoss.DmgGetInfo((PMSG_RANKING_DMG_BOSS_RECV*)lpMsg);
			return 1;
#endif
#if(CUSTOM_CHOTROI)
		case 0x01:
			gCusChoTroi.SetShowItemCache(lpMsg);
			return 1;
		case 0x20:
			gCusChoTroi.GCSetListChoTroi(lpMsg, size);
			return 1;
		case 0x22:
			gCusChoTroi.GCSetCurrencyList(lpMsg, size);
			return 1;
#endif
		case 0x2E:
			if (gCB_BotMix == nullptr)
			{
				gCB_BotMix = new CB_BotMix;
			}
			if (gCB_BotMix)
			{
				gCB_BotMix->RecvProtocol(lpMsg);
			}
			return 1;
#if(CUSTOM_CHANGEITEM)
		case 0x6A:
		{
			XULY_CGPACKET* mRecv = (XULY_CGPACKET*)lpMsg;

			if (mRecv->ThaoTac != 111)
			{
				return 0;
			}

			if (GetTickCount() > gInterface->Data[eWindowChangeItem].EventTick)
			{
				gInterface->Data[eWindowChangeItem].EventTick = GetTickCount() + 300;
				gInterface->Data[eWindowChangeItem].OnShow ^= 1;
				if (gInterface->Data[eWindowChangeItem].OnShow != 0)
				{
					gInterface->CostWC = 0;
					gInterface->CostWP = 0;
					gInterface->CostGP = 0;
					memset(gInterface->requiredInfoItem, 0, sizeof(gInterface->requiredInfoItem));
				}
			}

			gInterface->SetStateDoiItem = true;
		}
		return 1;
		case 0x6B:
		{
			INFO_CHANGEITEM_CLIENT* mRecv = (INFO_CHANGEITEM_CLIENT*)lpMsg;

			memcpy(gInterface->CItem_ItemChinh, mRecv->ItemChinh, sizeof(gInterface->CItem_ItemChinh));
			memcpy(gInterface->CItem_ItemPhu, mRecv->ItemPhu, sizeof(gInterface->CItem_ItemPhu));

			for (int n = 0; n < 12; n++)
			{
				memcpy(gInterface->CItem_ItemKetQua[n], mRecv->ItemKetQua[n], sizeof(gInterface->CItem_ItemKetQua[n]));
			}

			gInterface->CostWC = mRecv->WC;
			gInterface->CostWP = mRecv->WP;
			gInterface->CostGP = mRecv->GP;
			gInterface->CItem_ActiveMix = (mRecv->ActiveMix != 0);
			gInterface->CItem_EffectsMix = false;
			memset(gInterface->requiredInfoItem, 0, sizeof(gInterface->requiredInfoItem));
			BuildChangeItemRequiredInfo(mRecv, gInterface->requiredInfoItem, sizeof(gInterface->requiredInfoItem));
		}
		return 1;
#endif
		case 0x40:
			GCRecvInfoCharTop((DATA_VIEWTOPRANKING_TO_CLIENT*)lpMsg);
			break;
		}
		break;

	}
	return false;
}





void CGReqInfoCharTop(const char* Name)
{
	REQUESTINFO_CHARTOP pMsg;
	pMsg.header.set(0xD3, 0x40, sizeof(pMsg));
	memcpy(pMsg.NameChar, Name, sizeof(pMsg.NameChar));
	DataSend((LPBYTE)&pMsg, pMsg.header.size);
}

void GCRecvInfoCharTop(DATA_VIEWTOPRANKING_TO_CLIENT* lpMsg)
{
	// Character preview removed
}





