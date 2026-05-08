#pragma once
#include "Protocol.h"

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIScrollBar.h"
#include "NewUITextBox.h"

using namespace SEASON3B;

class CB_BotMix
{

public:

	struct LIST_MIX_TRADER
	{
		char NameMix[60];
		int MixIndex;
	};

	//==Packet
	enum  TYPE_SEND_RECV
	{
		eSendRecvListDataMix,
		eSendRecvListItemMix,
	};
	struct DATA_ITEMSEND
	{
		short IndexItem;
		int MinLevel;
		int MaxLevel;
		int MinOpt;
		int MaxOpt;
		int Luck;
		int Skill;
		int NeedExc;
		int Durability;
	};

	struct LISTITEMTRADER_CLIENT
	{
		int TypeCoin;
		int Coin;
		int Rate;
		int KeepLevel;
		int KeepOption;
		int KeepItemMixFail;
		std::vector<DATA_ITEMSEND> ListItem;
		bool IsLoad;
		void Clear()
		{
			TypeCoin = 0;
			Coin = 0;
			Rate = 0;
			KeepLevel = 0;
			KeepOption = 0;
			KeepItemMixFail = 0;
			ListItem.clear();
			IsLoad = false;
		}
	};

	struct PMSG_CUSTOM_LIST
	{
		PSWMSG_HEAD header;
		int TypeSend;
		int count;

		int TypeCoin;
		int Coin;
		int Rate;
		int KeepLevel;
		int KeepOption;
		int KeepItemMixFail;
	};
	struct XULY_CGPACKET
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD ThaoTac;
	};

	CB_BotMix();
	~CB_BotMix();
	void ClearData();
	bool DrawWindow(int X, int Y);
	CNewUIScrollBar* ListTraderMixScrollBar;
	CNewUIScrollBar* ListItemScrollBar;
	void CB_BotMix::DrawItemMix();
	LISTITEMTRADER_CLIENT ListItemMix;
	int MaxListItemInPage;

	std::vector<LIST_MIX_TRADER> m_DataListMix;
	void RecvProtocol(BYTE* lpMsg);
	void OpenListMix(int MixIndex);
	bool HasMixList() const;
	bool IsInputLockActive() const;
	bool IsItemWindowVisible() const;
	bool IsMouseOnWindow(int X, int Y) const;

	int StartXWindow;
	DWORD LastClickTick;
	int LastClickMixIndex;
	bool LastLeftButtonState;
	int PendingMixIndex;
	DWORD LastRequestTick;
};

extern CB_BotMix* gCB_BotMix;
