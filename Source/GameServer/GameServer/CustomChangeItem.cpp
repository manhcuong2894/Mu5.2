#include "stdafx.h"
#include "CustomChangeItem.h"
#include "Monster.h"
#include "ItemManager.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "EffectManager.h"
#include "MasterSkillTree.h"
#include "ServerInfo.h"
#include "MemScript.h"
#include "Util.h"
#include "CashShop.h"
#include "Message.h"
#include "Notice.h"
#include "Trade.h"
#include "DSProtocol.h"
#include "Log.h"
#include "ItemLevel.h"
#include "GameMain.h"
#include "CSProtocol.h"
#include "MonsterSetBase.h"
#include "MapServerManager.h"

#if(CUSTOM_CHANGEITEM)

CustomChangeItem gCustomChangeItem;

CustomChangeItem::CustomChangeItem()
{
	this->Enable = 0;
	this->mNPCData = new NPC_DATA_CHANGEITEM;
}

CustomChangeItem::~CustomChangeItem()
{

}

void CustomChangeItem::SetNPC()
{
	if (gCustomChangeItem.Enable)
	{
		MONSTER_SET_BASE_INFO info;
		memset(&info, 0, sizeof(info));

		info.Type = 0;
		info.MonsterClass = gCustomChangeItem.mNPCData->NPCClass;
		info.Map = gCustomChangeItem.mNPCData->NPCMap;
		info.Dis = 0;
		info.Dir = gCustomChangeItem.mNPCData->NPCDir;
		info.X = gCustomChangeItem.mNPCData->NPCX;
		info.Y = gCustomChangeItem.mNPCData->NPCY;

		gMonsterSetBase.SetInfo(info);
	}
}
void CustomChangeItem::LoadConfig(char* FilePath)
{
	this->m_GroupDoiItemData.clear();
	this->Enable = 0;
	this->mNPCData->Clear();

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oDungLuyenItem = file.child("DoiItem");
	this->Enable = oDungLuyenItem.attribute("Enable").as_int();

	//= Mess Load
	pugi::xml_node Message = oDungLuyenItem.child("Message");
	for (pugi::xml_node msg = Message.child("Msg"); msg; msg = msg.next_sibling())
	{
		CHANGEITEM_MSG info;

		info.Index = msg.attribute("Index").as_int();

		strncpy_s(info.Message, msg.attribute("Text").as_string(), sizeof(info.Message));

		this->m_MessageInfoBP.insert(std::pair<int, CHANGEITEM_MSG>(info.Index, info));
	}

	//= Config NPC
	pugi::xml_node EventConfigNPC = oDungLuyenItem.child("NPC");
	this->mNPCData->NPCClass = EventConfigNPC.attribute("NPCClass").as_int();
	this->mNPCData->NPCMap = EventConfigNPC.attribute("NPCMap").as_int();
	this->mNPCData->NPCX = EventConfigNPC.attribute("NPCX").as_int();
	this->mNPCData->NPCY = EventConfigNPC.attribute("NPCY").as_int();
	this->mNPCData->NPCDir = EventConfigNPC.attribute("NPCDir").as_int();
	//=== Load Data
	int mIndex = 0;
	for (pugi::xml_node GroupDungLuyen = oDungLuyenItem.child("GroupDoiItem"); GroupDungLuyen; GroupDungLuyen = GroupDungLuyen.next_sibling())
	{
		GROUPDOIITEM_DATA infoGroup = {};
		infoGroup.Index = mIndex++;
		infoGroup.Notice = GroupDungLuyen.attribute("Notice").as_int();
		strncpy_s(infoGroup.Name, GroupDungLuyen.attribute("Name").as_string(), sizeof(infoGroup.Name));

		pugi::xml_attribute attrWC = GroupDungLuyen.attribute("WC");
		infoGroup.WC = (!attrWC.empty()) ? attrWC.as_int() : GroupDungLuyen.attribute("Wcoin").as_int();
		infoGroup.WP = GroupDungLuyen.attribute("WP").as_int();
		infoGroup.GP = GroupDungLuyen.attribute("GP").as_int();

		//Config ItemChinh
		pugi::xml_node ItemChinh = GroupDungLuyen.child("ItemChinh");
		infoGroup.ItemChinh.IndexItem = ItemChinh.attribute("IndexItem").as_int();
		infoGroup.ItemChinh.Level = ItemChinh.attribute("Level").as_int();
		infoGroup.ItemChinh.Skill = ItemChinh.attribute("Skill").as_int();
		infoGroup.ItemChinh.Luck = ItemChinh.attribute("Luck").as_int();
		infoGroup.ItemChinh.Option = ItemChinh.attribute("Option").as_int();
		infoGroup.ItemChinh.Exc = ItemChinh.attribute("Exc").as_int();

		//Config KetQua
		pugi::xml_node ItemKetQua = GroupDungLuyen.child("ItemKetQua");
		const char* ResultItemAttr[12] = { "IndexItem", "IndexItem1", "IndexItem2", "IndexItem3", "IndexItem4", "IndexItem5", "IndexItem6", "IndexItem7", "IndexItem8", "IndexItem9", "IndexItem10", "IndexItem11" };

		for (int n = 0; n < 12; n++)
		{
			infoGroup.ItemKetQua.IndexItem[n] = ItemKetQua.attribute(ResultItemAttr[n]).as_int(-1);
		}

		this->m_GroupDoiItemData.insert(std::pair<int, GROUPDOIITEM_DATA>(infoGroup.Index, infoGroup));

	}

	LogAdd(LOG_BLUE, "[DoiItem] Load OK Enable = %d, Size %d", this->Enable, this->m_GroupDoiItemData.size());
}


char* CustomChangeItem::GetMessage(int index) // OK
{
	std::map<int, CHANGEITEM_MSG>::iterator it = this->m_MessageInfoBP.find(index);

	if (it == this->m_MessageInfoBP.end())
	{
		char Error[256];
		wsprintf(Error, "Could not find message %d!", index);
		return Error;
	}
	else
	{
		return it->second.Message;
	}
}
void CustomChangeItem::SetStateInterface(int aIndex, int Type)
{

	if (Type == 1)
	{
		gObj[aIndex].Interface.state = 1;
		gObj[aIndex].Interface.use = 1;
		gObj[aIndex].Interface.type = INTERFACE_CHANGEITEM;
		gObj[aIndex].Transaction = 0;
		gObjInventoryTransaction(aIndex);
	}
	else
	{


		if (gObj[aIndex].CItem_StatusMix > 0) //SAVE Neu Da Mix
		{
			if (gObj[aIndex].CItem_StatusMix == 1)
			{
				gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CItem_ItemChinh);
			}
			else
			{
				for (int n = 0; n < 12; n++)
				{
					gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CItem_ItemKetQua[n]);
				}

			}
		}
		else
		{
			gObjInventoryRollback(aIndex);
		}

		//==Clear Cache
		gObj[aIndex].CItem_IndexMix = -1;
		gObj[aIndex].CItem_ItemChinh.Clear();
		for (int n = 0; n < 12; n++)
		{
			gObj[aIndex].CItem_ItemKetQua[n].Clear();
		}

		gObj[aIndex].CItem_StatusMix = -1;
		SendInfoItemCache(aIndex);
		gItemManager.GCItemListSend(aIndex);
		gObjectManager.CharacterMakePreviewCharSet(aIndex);
		gItemManager.GCItemEquipmentSend(aIndex);

		gObj[aIndex].Interface.use = 0;
		gObj[aIndex].Interface.type = INTERFACE_NONE;
		gObj[aIndex].Interface.state = 0;
		gObj[aIndex].TargetNumber = -1;

	}
}



void CustomChangeItem::SendInfoItemCache(int aIndex)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	//if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || !this->Enable)
	if (lpObj->Type != OBJECT_USER || !this->Enable)
	{
		return;

	}

	if (lpObj->Interface.type != INTERFACE_CHANGEITEM)
	{

		return;
	}

	std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.find(lpObj->CItem_IndexMix);

	if (it == this->m_GroupDoiItemData.end() && !this->m_GroupDoiItemData.empty())
	{
		it = this->m_GroupDoiItemData.begin();
	}

	BYTE send[8192];

	INFO_CHANGEITEM_CLIENT pMsg = {};

	pMsg.header.set(0xD3, 0x6B, 0);

	int size = sizeof(pMsg);

	pMsg.ActiveMix = 1;

	if (!lpObj->CItem_ItemChinh.IsItem()
		|| !lpObj->CItem_ItemChinh.IsItem())
		//|| !lpObj->DL_ItemKetQua.IsItem()

	{

		pMsg.ActiveMix = 0;
	}

	gItemManager.ItemByteConvert(pMsg.ItemChinh, lpObj->CItem_ItemChinh); // Set Info Item
	for (int n = 0; n < 12; n++)
	{
		gItemManager.ItemByteConvert(pMsg.ItemKetQua[n], lpObj->CItem_ItemKetQua[n]); // Set Info Item
	}

	if (it != this->m_GroupDoiItemData.end())
	{
		pMsg.WC = it->second.WC;
		pMsg.WP = it->second.WP;
		pMsg.GP = it->second.GP;
		pMsg.RequiredLevel = it->second.ItemChinh.Level;
		pMsg.RequiredSkill = it->second.ItemChinh.Skill;
		pMsg.RequiredLuck = it->second.ItemChinh.Luck;
		pMsg.RequiredOption = it->second.ItemChinh.Option;
		pMsg.RequiredExc = it->second.ItemChinh.Exc;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
}

void CustomChangeItem::ProcItemSend(int aIndex, int SlotItem)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	//if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || !this->Enable)
	if (lpObj->Type != OBJECT_USER || !this->Enable)
	{
		return;

	}

	if (lpObj->Interface.type != INTERFACE_CHANGEITEM)
	{

		return;
	}

	if (gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[SlotItem], 0) == 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}
	if (!lpObj->Inventory[SlotItem].IsItem())
	{
		//Vi tri nay khong co Item do
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}
	if (gObj[aIndex].CItem_StatusMix > 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, "Vui Long Dong Dung Luyen Va Mo Lai!!");// That Bai
		return;
	}
	//==Cache Item Chinh
	if (!lpObj->CItem_ItemChinh.IsItem())
	{
		for (std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.begin(); it != this->m_GroupDoiItemData.end(); it++)
		{
			if (it == this->m_GroupDoiItemData.end())
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
				return;
			}
			if (lpObj->Inventory[SlotItem].m_Index == it->second.ItemChinh.IndexItem)

			{
				if ((it->second.ItemChinh.Level != -1 && it->second.ItemChinh.Level > lpObj->Inventory[SlotItem].m_Level)
					|| (it->second.ItemChinh.Skill != -1 && it->second.ItemChinh.Skill > lpObj->Inventory[SlotItem].m_Option1)
					|| (it->second.ItemChinh.Luck != -1 && it->second.ItemChinh.Luck > lpObj->Inventory[SlotItem].m_Option2)
					|| (it->second.ItemChinh.Option != -1 && it->second.ItemChinh.Option > lpObj->Inventory[SlotItem].m_Option3)
					|| it->second.ItemChinh.Exc != 0 && !lpObj->Inventory[SlotItem].IsExcItem())
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
					return;
				}


				lpObj->CItem_IndexMix = it->second.Index;
				lpObj->CItem_ItemChinh = lpObj->Inventory[SlotItem];
				//=== Set Item Ket Qua

                for (int n = 0; n < 12; n++)
                {
                    if (it->second.ItemKetQua.IndexItem[n] < 0)
                    {
                        lpObj->CItem_ItemKetQua[n].Clear();
                        continue;
                    }

                    lpObj->CItem_ItemKetQua[n] = lpObj->Inventory[SlotItem];
                    lpObj->CItem_ItemKetQua[n].m_Index = it->second.ItemKetQua.IndexItem[n];
                    lpObj->CItem_ItemKetQua[n].m_JewelOfHarmonyOption = 0;
                    lpObj->CItem_ItemKetQua[n].m_ItemOptionEx = 0;
                }

				//=== Send Del item
				gItemManager.InventoryDelItem(aIndex, SlotItem);
				gItemManager.GCItemDeleteSend(aIndex, SlotItem, 1);
				this->SendInfoItemCache(aIndex);
				//====
				return;

			}
		}

	}
}
void CustomChangeItem::BackItem(int aIndex, int BackSlot)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || !this->Enable)
	{
		return;
	}

	std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.find(lpObj->CItem_IndexMix);

	if (it == this->m_GroupDoiItemData.end())
	{
		return;
	}

	if (lpObj->Interface.type != INTERFACE_CHANGEITEM)
	{
		return;
	}

	if (BackSlot >= 1 && (lpObj->Coin1 < it->second.WC || lpObj->Coin2 < it->second.WP || lpObj->Coin3 < it->second.GP))
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2));//
		return;
	}

	BYTE SlotRecv = 0xFF;

	if (BackSlot == 0)
	{
		if (lpObj->CItem_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->CItem_ItemChinh);
			lpObj->CItem_ItemChinh.Clear();
			for (int n = 0; n < 12; n++)
			{
				lpObj->CItem_ItemKetQua[n].Clear();
			}
		}
	}
	else if (BackSlot >= 1 && BackSlot <= 12)
	{
		int ResultIndex = (BackSlot - 1);

		if (lpObj->CItem_ItemKetQua[ResultIndex].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->CItem_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->CItem_ItemKetQua[ResultIndex]);
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s]", lpObj->Name, gItemLevel.GetItemName(lpObj->CItem_ItemChinh.m_Index, lpObj->CItem_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->CItem_ItemKetQua[ResultIndex].m_Index, lpObj->CItem_ItemKetQua[ResultIndex].m_Level));
			lpObj->CItem_ItemChinh.Clear();
			for (int n = 0; n < 12; n++)
			{
				lpObj->CItem_ItemKetQua[n].Clear();
			}
		}

		gObjInventoryCommit(aIndex);
	}
	else
	{
		return;
	}

	bool isExchangeSuccess = (BackSlot >= 1 && SlotRecv != 0xFF);

	if (isExchangeSuccess)
	{
		GDSetCoinSend(lpObj->Index, -it->second.WC, -it->second.WP, -it->second.GP, "Coin");
	}

	char tmp[255];

	if (isExchangeSuccess && it->second.Notice == 1)
	{
		gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, this->GetMessage(4), lpObj->Name, it->second.Name);
	}
	else if (isExchangeSuccess && it->second.Notice == 2)
	{
		wsprintf(tmp, this->GetMessage(4), lpObj->Name, it->second.Name);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
	}
	else if (isExchangeSuccess && it->second.Notice == 3)
	{
		wsprintf(tmp, this->GetMessage(4), lpObj->Name, it->second.Name);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
	}

	if (SlotRecv != 0xFF)
	{
		gItemManager.GCItemModifySend(aIndex, SlotRecv);
		this->SendInfoItemCache(aIndex);
	}
}

bool CustomChangeItem::Dialog(LPOBJ lpObj, LPOBJ lpNpc)
{
	if (!this->Enable || lpObj->Interface.type != INTERFACE_NONE)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0));
		return false;
	}

	if (lpNpc->Class == this->mNPCData->NPCClass &&
		lpNpc->Map == this->mNPCData->NPCMap &&
		lpNpc->X == this->mNPCData->NPCX &&
		lpNpc->Y == this->mNPCData->NPCY)
	{
		//lpObj->DL_StatusMix = TRUE;
		//====Open Window
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x6A, sizeof(pMsg));
		pMsg.ThaoTac = 111;	//Open NPC
		DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
		SetStateInterface(lpObj->Index, 1);
		this->SendInfoItemCache(lpObj->Index);
		return true;
	}

	return false;
}











#endif
