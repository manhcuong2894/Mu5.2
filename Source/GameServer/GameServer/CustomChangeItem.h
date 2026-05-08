#pragma once
#include "StdAfx.h"
#include "Protocol.h"
#include "DSProtocol.h"
#include "User.h"
//#include "OfflineMode.h"
#include "ItemManager.h"

#if(CUSTOM_CHANGEITEM)
#define eMessageBox				255

struct CHANGEITEM_MSG
{
	int Index;
	char Message[256];
};
struct NPC_DATA_CHANGEITEM
{
	int NPCClass;
	int NPCMap;
	int NPCX;
	int NPCY;
	int NPCDir;
	void Clear()
	{
		NPCClass = -1;
		NPCMap = 0;
		NPCX = 0;
		NPCY = 0;
		NPCDir = 0;
	}
};
struct DATA_ITEM_CHINH
{
	int IndexItem;
	int Level;
	int Skill;
	int Luck;
	int Option;
	int Exc;
};


struct DATA_ITEM_KQ
{
	int IndexItem[12];
};

struct GROUPDOIITEM_DATA
{
	int Index;
	int Notice;
	char Name[90];
	int WC;
	int WP;
	int GP;
	DATA_ITEM_CHINH ItemChinh;
	std::vector<DATA_ITEM_KQ> vItemKetQua;
	DATA_ITEM_KQ ItemKetQua;

};
struct INFO_CHANGEITEM_CLIENT
{
	PSWMSG_HEAD header;
	BYTE ActiveMix;
	BYTE ItemChinh[16];
	BYTE vItemKetQua[3][16];
	BYTE ItemKetQua[12][16];
	int WC;
	int WP;
	int GP;
	int RequiredLevel;
	int RequiredSkill;
	int RequiredLuck;
	int RequiredOption;
	int RequiredExc;
};

struct PMSG_CHANGEITEM_ITEM_RECV
{
	PSBMSG_HEAD h;
	BYTE sFlag;
	BYTE tFlag;
	BYTE Source;
	BYTE Target;
};

class CustomChangeItem
{
public:
	CustomChangeItem();
	virtual ~CustomChangeItem();
	int Enable;
	NPC_DATA_CHANGEITEM* mNPCData;
	void LoadConfig(char* FilePath);
	bool CustomChangeItem::Dialog(LPOBJ lpObj, LPOBJ lpNpc);
	std::map<int, GROUPDOIITEM_DATA> m_GroupDoiItemData;

	void CustomChangeItem::SetStateInterface(int aIndex, int Type = 0);
	void CustomChangeItem::ProcItemSend(int aIndex, int SlotItem);
	void CustomChangeItem::SendInfoItemCache(int aIndex);
	void CustomChangeItem::BackItem(int aIndex, int BackSlot);

	void SetNPC();
private:
	//===Mess
	std::map<int, CHANGEITEM_MSG> m_MessageInfoBP;
	char* GetMessage(int index);

};

extern CustomChangeItem gCustomChangeItem;



#endif
