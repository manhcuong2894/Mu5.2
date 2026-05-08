#include "StdAfx.h"
#include "H_BotMix.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzOpenglUtil.h"
#include "TextClient.h"

namespace
{
	int GetMaxScrollPage(int totalItems, int itemsPerPage)
	{
		if (totalItems <= itemsPerPage || itemsPerPage <= 0)
		{
			return 0;
		}

		return (totalItems - 1) / itemsPerPage;
	}

	void HandleScrollWheel(SEASON3B::CNewUIScrollBar* scrollBar, float x, float y, float width, float height)
	{
		if (!scrollBar || !SEASON3B::CheckMouseIn(x, y, width, height))
		{
			return;
		}

		if (MouseWheel < 0)
		{
			MouseWheel = 0;
			scrollBar->SetCurPos(scrollBar->GetCurPos() + 1);
		}
		else if (MouseWheel > 0)
		{
			MouseWheel = 0;
			scrollBar->SetCurPos(scrollBar->GetCurPos() - 1);
		}
	}
}


CB_BotMix* gCB_BotMix;

CB_BotMix::CB_BotMix()
{
	ListTraderMixScrollBar = nullptr;
	ListItemScrollBar = nullptr;
	MaxListItemInPage = 8;
	StartXWindow = 0;
	LastClickTick = 0;
	LastClickMixIndex = -1;
	LastLeftButtonState = false;
	PendingMixIndex = -1;
	LastRequestTick = 0;
	CB_BotMix::ClearData();
}

CB_BotMix::~CB_BotMix()
{
}
void CB_BotMix::ClearData()
{
	ListItemMix.Clear();
	m_DataListMix.clear();
	LastClickTick = 0;
	LastClickMixIndex = -1;
	LastLeftButtonState = false;
	PendingMixIndex = -1;
	LastRequestTick = 0;
	if (ListTraderMixScrollBar)
	{
		ListTraderMixScrollBar->SetCurPos(0);
		ListTraderMixScrollBar->SetMaxPos(0);
	}
	if (ListItemScrollBar)
	{
		ListItemScrollBar->SetCurPos(0);
		ListItemScrollBar->SetMaxPos(0);
	}
	gInterface->Data[eWindowListItemTrader].OnClick = false;
	gInterface->Data[eWindowListItemTrader].FirstLoad = false;
	gInterface->Data[eWindowListItemTrader].Close();
}
void CB_BotMix::DrawItemMix()
{
	if (!gInterface->Data[eWindowListItemTrader].OnShow)
	{
		return;
	}

	float WindowW = 225;
	float WindowH = 270;

	float StartX = StartXWindow - WindowW;
	float StartY = 55.0;

	char* NameCoin[] = { "Zen","WCoin","WCoinP", "GobinPoint","NULL" };
	int CountTextInfo = 0;
	gInterface->Data[eWindowListItemTrader].AllowMove = false;
	if (gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowListItemTrader, gTextClient.txtClient_BotMix[0]))
	{

		StartX = StartX + 10;
		StartY = StartY + 5;
		float WInfo = (WindowW - 20);
		float HInfo = WindowH - 100;

		if (!ListItemMix.IsLoad)
		{
			char buf[25];
			int step = GetTickCount() / 1000 % 4;
			sprintf(buf, "Loading data%%.%ds", step);
			TextDraw((HFONT)g_hFont, StartX + 1, StartY + 55, 0xffffffff, 0x0, WInfo, 0, 3, buf, "...");
			return;
		}

		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 35, 0xFF00EEDFF, 0x0, WindowW, 0, 1, gTextClient.txtClient_BotMix[1]);

		//Scroll Bar
		int DataListItem = ListItemMix.ListItem.size();
		if (!ListItemScrollBar)
		{
			ListItemScrollBar = new CNewUIScrollBar();
			ListItemScrollBar->Create((StartX + WindowW) - 25, StartY + 35 + 15, 130);
		}
		if (ListItemScrollBar)
		{
			ListItemScrollBar->SetMaxPos(GetMaxScrollPage(DataListItem, MaxListItemInPage));
			ListItemScrollBar->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
			ListItemScrollBar->Render();
			ListItemScrollBar->UpdateMouseEvent();
			ListItemScrollBar->Update();
			HandleScrollWheel(ListItemScrollBar, StartX + 15, StartY + 35 + 15, WindowW, 130);
		}

		//==List Box Item 
		float PosXBoxItemGoc = StartX + 25;
		float PosXBoxItem = StartX + 25;
		float PosYBoxItem = StartY + 55;
		float WBox = 30;
		float KhoangCach = 45;
		int CountNgang = 0;
		int CountDoc = 0;
		int ItemListPage = ListItemScrollBar->GetCurPos();
		int BBShowInfoItem = -1;

		//
		int currentRow = 0;
		int currentCol = 0;
		int CountText = 0;
		int CountList = 0;

		for (int n = (ItemListPage * MaxListItemInPage); n < DataListItem; n++)
		{
			if (CountList >= MaxListItemInPage) break;
			char itemName[256] = { 0 };
			GetItemName(ListItemMix.ListItem[n].IndexItem, ListItemMix.ListItem[n].MinLevel, itemName);
			std::ostringstream ossNameItem;
			ossNameItem << itemName;
			if (ListItemMix.ListItem[n].MaxLevel > ListItemMix.ListItem[n].MinLevel && ListItemMix.ListItem[n].MinLevel > 0) ossNameItem << "Lv " << ListItemMix.ListItem[n].MinLevel << "~" << ListItemMix.ListItem[n].MaxLevel;
			if (ListItemMix.ListItem[n].MaxOpt > ListItemMix.ListItem[n].MinOpt && ListItemMix.ListItem[n].MinOpt > 0) ossNameItem << ", Opt " << ListItemMix.ListItem[n].MinOpt << "~" << ListItemMix.ListItem[n].MaxOpt;
			if (ListItemMix.ListItem[n].Luck) ossNameItem << ", +Luck";
			if (ListItemMix.ListItem[n].Skill) ossNameItem << ", +Skill";
			if (ListItemMix.ListItem[n].NeedExc) ossNameItem << ", +Exl";

			const std::string itemLine = ossNameItem.str();
			TextDraw(g_hFont, StartX + 5, PosYBoxItem + (14 * CountText++), 0xFFFFFFFF, 0x0, WindowH, 0, 1, "x1 %s", itemLine.c_str());
			CountList++;

		}

		//===Coin
		float PosYCoinNhan = StartY + 175;
		CountTextInfo++;
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, gTextClient.txtClient_BotMix[2]);
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, "%s : %s", NameCoin[ListItemMix.TypeCoin], gInterface->NumberFormat(ListItemMix.Coin));

		if (ListItemMix.KeepOption || ListItemMix.KeepLevel)
		{
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, gTextClient.txtClient_BotMix[3]);
			std::ostringstream ossTextKeep;
			if (ListItemMix.KeepLevel) ossTextKeep << gTextClient.txtClient_BotMix[4];
			if (ListItemMix.KeepOption) ossTextKeep << gTextClient.txtClient_BotMix[5];
			const std::string keepText = ossTextKeep.str();
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, keepText.c_str());
		}
		if (ListItemMix.KeepItemMixFail)
		{
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, gTextClient.txtClient_BotMix[6]);
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, gTextClient.txtClient_BotMix[7]);
		}
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, gTextClient.txtClient_BotMix[8]);
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, "%d%%", ListItemMix.Rate);
	}
}
bool CB_BotMix::DrawWindow(int X, int Y)
{
	StartXWindow = X;
	if (this->m_DataListMix.empty()) return 0;

	float StartX = X + 9;
	float StartY = Y + 35;
	float W = 172;
	float H = 190;

	RenderImage(CNewUITrade::IMAGE_TRADE_BACK, StartX, StartY, W, H);

	TextDraw(g_hFontBold, StartX, StartY, 0xFFFFFFFF, 0x0, W, 0, 3, gTextClient.txtClient_BotMix[9]);
	gInterface->DrawInfoBox(StartX + 3, StartY + 15, W - 13, 130, 0x00000096, 0, 0);

	////===ScrollBar
	float ScrollBarX = StartX + 3;
	float ScrollBarY = StartY + 17;
	float ScrollBarW = 160;
	float ScrollBarH = 130;
	const int ScrollBarMaxPos = GetMaxScrollPage(static_cast<int>(this->m_DataListMix.size()), MaxListItemInPage);
	if (!ListTraderMixScrollBar)
	{
		ListTraderMixScrollBar = new CNewUIScrollBar();
		ListTraderMixScrollBar->Create(ScrollBarX, ScrollBarY, ScrollBarH);
	}
	if (ListTraderMixScrollBar)
	{
		ListTraderMixScrollBar->SetMaxPos(ScrollBarMaxPos);
		ListTraderMixScrollBar->SetPos(ScrollBarX + (ScrollBarW - 5), ScrollBarY);
		ListTraderMixScrollBar->Render();
		ListTraderMixScrollBar->UpdateMouseEvent();
		ListTraderMixScrollBar->Update();
		HandleScrollWheel(ListTraderMixScrollBar, ScrollBarX, ScrollBarY, ScrollBarW, ScrollBarH);
	}
	int CountText = 0;
	DWORD Color = 0x0;
	int mPageGet = ListTraderMixScrollBar->GetCurPos();
	int CountList = 0;
	int MaxListInPage = 8;
	const bool isLeftButtonDown = ((GetKeyState(VK_LBUTTON) & 0x8000) != 0);
	for (int n = (mPageGet * MaxListInPage); n < this->m_DataListMix.size(); n++)
	{
		if (CountList >= MaxListInPage) break;
		if (SEASON3B::CheckMouseIn(StartX + 5, StartY + 20 + (14 * CountText), 140, 14))
		{
			Color = 0x87CEFAA5;
			if (isLeftButtonDown && !LastLeftButtonState)
			{
				const DWORD currentTick = GetTickCount();
				if (LastClickMixIndex == this->m_DataListMix[n].MixIndex && (currentTick - LastClickTick) <= 500)
				{
					this->OpenListMix(this->m_DataListMix[n].MixIndex);
					LastClickTick = 0;
					LastClickMixIndex = -1;
				}
				else
				{
					LastClickTick = currentTick;
					LastClickMixIndex = this->m_DataListMix[n].MixIndex;
				}
			}
		}
		else
		{
			Color = 0x0;
		}
		TextDraw(g_hFont, StartX + 5, StartY + 20 + (14 * CountText++), 0xFFFFFFFF, Color, 140, 0, 1, "%s [%d]", this->m_DataListMix[n].NameMix, this->m_DataListMix[n].MixIndex);
		CountList++;
	}
	TextDraw((HFONT)g_hFont, StartX, StartY + 160, 0x14FFC0FF, 0x0, W, 0, 1, gTextClient.txtClient_BotMix[10]);
	TextDraw((HFONT)g_hFont, StartX, StartY + 175, 0xA9FFC0FF, 0x0, W, 0, 1, gTextClient.txtClient_BotMix[11]);
	LastLeftButtonState = isLeftButtonDown;
	//===
	CB_BotMix::DrawItemMix();
	return true;
}
void CB_BotMix::OpenListMix(int MixIndex)
{
	if (MixIndex < 0)
	{
		return;
	}

	if (PendingMixIndex == MixIndex && !ListItemMix.IsLoad && (GetTickCount() - LastRequestTick) < 1000)
	{
		return;
	}

	ListItemMix.Clear();
	PendingMixIndex = MixIndex;
	LastRequestTick = GetTickCount();
	if (ListItemScrollBar)
	{
		ListItemScrollBar->SetCurPos(0);
		ListItemScrollBar->SetMaxPos(0);
	}
	gInterface->Data[eWindowListItemTrader].FirstLoad = false;
	gInterface->Data[eWindowListItemTrader].OnShow = 1;
	XULY_CGPACKET pMsg;
	pMsg.header.set(0xD3, 0x2E, sizeof(pMsg));
	pMsg.ThaoTac = MixIndex; //
	DataSend((LPBYTE)&pMsg, pMsg.header.size);
}
bool CB_BotMix::HasMixList() const
{
	return !this->m_DataListMix.empty();
}

bool CB_BotMix::IsInputLockActive() const
{
	return (this->HasMixList() || this->IsItemWindowVisible());
}

bool CB_BotMix::IsItemWindowVisible() const
{
	return (gInterface->Data[eWindowListItemTrader].OnShow != 0);
}

bool CB_BotMix::IsMouseOnWindow(int X, int Y) const
{
	if (this->HasMixList() && SEASON3B::CheckMouseIn(static_cast<float>(X + 9), static_cast<float>(Y + 35), 172.0f, 190.0f))
	{
		return true;
	}

	if (this->IsItemWindowVisible())
	{
		float windowX = static_cast<float>(X - 225);
		float windowY = 55.0f;

		if (gInterface->Data[eWindowListItemTrader].FirstLoad)
		{
			windowX = gInterface->Data[eWindowListItemTrader].X;
			windowY = gInterface->Data[eWindowListItemTrader].Y;
		}

		if (SEASON3B::CheckMouseIn(windowX, windowY, 225.0f, 270.0f))
		{
			return true;
		}
	}

	return false;
}

void CB_BotMix::RecvProtocol(BYTE* lpMsg)
{
	if (!lpMsg) return;
	PMSG_CUSTOM_LIST* mRecv = (PMSG_CUSTOM_LIST*)lpMsg;

	//gInterface->DrawMessage(1, "Recv Type %d", mRecv->TypeSend);
	switch (mRecv->TypeSend)
	{
	case CB_BotMix::eSendRecvListDataMix:
	{
		this->m_DataListMix.clear();
		if (ListTraderMixScrollBar)
		{
			ListTraderMixScrollBar->SetCurPos(0);
		}
		for (int i = 0; i < mRecv->count; i++)
		{
			LIST_MIX_TRADER lpInfo = *(LIST_MIX_TRADER*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_LIST) + (sizeof(LIST_MIX_TRADER) * i));
			this->m_DataListMix.push_back(lpInfo);
		}
	}
	break;
	case CB_BotMix::eSendRecvListItemMix:
	{
		this->ListItemMix.Clear();
		PendingMixIndex = -1;
		LastRequestTick = 0;
		this->ListItemMix.TypeCoin = mRecv->TypeCoin;
		this->ListItemMix.Coin = mRecv->Coin;
		this->ListItemMix.Rate = mRecv->Rate;
		this->ListItemMix.KeepLevel = mRecv->KeepLevel;
		this->ListItemMix.KeepOption = mRecv->KeepOption;
		this->ListItemMix.KeepItemMixFail = mRecv->KeepItemMixFail;
		for (int i = 0; i < mRecv->count; i++)
		{
			DATA_ITEMSEND lpInfo = *(DATA_ITEMSEND*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_LIST) + (sizeof(DATA_ITEMSEND) * i));
			this->ListItemMix.ListItem.push_back(lpInfo);
		}
		this->ListItemMix.IsLoad = true;
	}
	break;
	default:
		break;
	}

}
