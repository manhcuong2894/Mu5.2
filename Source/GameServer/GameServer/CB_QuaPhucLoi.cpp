#include "StdAfx.h"
#include "CB_QuaPhucLoi.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "Map.h"
#include "MemScript.h"
#include "Notice.h"
#include "Util.h"
#include "ItemOptionRate.h"
#include "ObjectManager.h"
#include "Guild.h"
#include "Move.h"
#include "Monster.h"
#include "ItemBagManager.h"
#include "Party.h"
#include "CashShop.h"
#include "MapServerManager.h"
#include "ServerInfo.h"
#include "GameMain.h"
#include "Log.h"
#if(CB_CUSTOMGIFTCODE)
#include "CB_GiftCode.h"
#endif
#if(CB_QUAPHUCLOI)
CB_QuaPhucLoi gCB_QuaPhucLoi;


CB_QuaPhucLoi::CB_QuaPhucLoi()
{
}


CB_QuaPhucLoi::~CB_QuaPhucLoi()
{
}
void CB_QuaPhucLoi::LoadFileXML(char* FilePath)
{
	memset(&this->EnableB, 0, sizeof(this->EnableB));
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}

	//--
	pugi::xml_node oCustomQuaPhucLoi = file.child("CustomQuaPhucLoi");
	this->Enable = oCustomQuaPhucLoi.attribute("Enable").as_int();

	//= Mess Load
	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = oCustomQuaPhucLoi.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO>(info.Index, info));
	}

	this->List_QuaNapDau.clear();
	pugi::xml_node DataQuaNapDau = oCustomQuaPhucLoi.child("QuaNapDau");
	this->EnableB[eQuaNapDau]  = DataQuaNapDau.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaNapDau = DataQuaNapDau.child("List"); ListQuaNapDau; ListQuaNapDau = ListQuaNapDau.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaNapDau;

		infoQuaNapDau.DieuKienNhan = ListQuaNapDau.attribute("DieuKienNhan").as_int();
		infoQuaNapDau.GiftIndex = ListQuaNapDau.attribute("GiftIndex").as_int();

		this->List_QuaNapDau.push_back(infoQuaNapDau);
	}

	this->List_QuaNapNgay.clear();
	pugi::xml_node DataQuaNapNgay = oCustomQuaPhucLoi.child("QuaNapNgay");
	this->EnableB[eQuaNapNgay] = DataQuaNapNgay.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaNapNgay = DataQuaNapNgay.child("List"); ListQuaNapNgay; ListQuaNapNgay = ListQuaNapNgay.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaNapNgay;

		infoQuaNapNgay.DieuKienNhan = ListQuaNapNgay.attribute("DieuKienNhan").as_int();
		infoQuaNapNgay.GiftIndex = ListQuaNapNgay.attribute("GiftIndex").as_int();

		this->List_QuaNapNgay.push_back(infoQuaNapNgay);
	}

	this->List_QuaNapThang.clear();
	pugi::xml_node DataQuaNapThang = oCustomQuaPhucLoi.child("QuaNapThang");
	this->EnableB[eQuaNapThang] = DataQuaNapThang.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaNapThang = DataQuaNapThang.child("List"); ListQuaNapThang; ListQuaNapThang = ListQuaNapThang.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaNapThang;

		infoQuaNapThang.DieuKienNhan = ListQuaNapThang.attribute("DieuKienNhan").as_int();
		infoQuaNapThang.GiftIndex = ListQuaNapThang.attribute("GiftIndex").as_int();

		this->List_QuaNapThang.push_back(infoQuaNapThang);
	}

	this->List_QuaNapTichLuy.clear();
	pugi::xml_node DataQuaNapTichLuy = oCustomQuaPhucLoi.child("QuaNapTichLuy");
	this->EnableB[eQuaNapTichLuy] = DataQuaNapTichLuy.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaNapTichLuy = DataQuaNapTichLuy.child("List"); ListQuaNapTichLuy; ListQuaNapTichLuy = ListQuaNapTichLuy.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaNapTichLuy;

		infoQuaNapTichLuy.DieuKienNhan = ListQuaNapTichLuy.attribute("DieuKienNhan").as_int();
		infoQuaNapTichLuy.GiftIndex = ListQuaNapTichLuy.attribute("GiftIndex").as_int();

		this->List_QuaNapTichLuy.push_back(infoQuaNapTichLuy);
	}

	this->List_QuaTieuPhiNgay.clear();
	pugi::xml_node DataQuaTieuPhiNgay = oCustomQuaPhucLoi.child("QuaTieuPhiNgay");
	this->EnableB[eQuaTieuPhiNgay] = DataQuaTieuPhiNgay.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaTieuPhiNgay = DataQuaTieuPhiNgay.child("List"); ListQuaTieuPhiNgay; ListQuaTieuPhiNgay = ListQuaTieuPhiNgay.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaTieuPhiNgay;

		infoQuaTieuPhiNgay.DieuKienNhan = ListQuaTieuPhiNgay.attribute("DieuKienNhan").as_int();
		infoQuaTieuPhiNgay.GiftIndex = ListQuaTieuPhiNgay.attribute("GiftIndex").as_int();

		this->List_QuaTieuPhiNgay.push_back(infoQuaTieuPhiNgay);
	}

	this->List_QuaTieuPhiThang.clear();
	pugi::xml_node DataQuaTieuPhiThang = oCustomQuaPhucLoi.child("QuaTieuPhiThang");
	this->EnableB[eQuaTieuPhiThang] = DataQuaTieuPhiThang.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaTieuPhiThang = DataQuaTieuPhiThang.child("List"); ListQuaTieuPhiThang; ListQuaTieuPhiThang = ListQuaTieuPhiThang.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaTieuPhiThang;

		infoQuaTieuPhiThang.DieuKienNhan = ListQuaTieuPhiThang.attribute("DieuKienNhan").as_int();
		infoQuaTieuPhiThang.GiftIndex = ListQuaTieuPhiThang.attribute("GiftIndex").as_int();

		this->List_QuaTieuPhiThang.push_back(infoQuaTieuPhiThang);
	}

	this->List_QuaTieuPhiTichLuy.clear();
	pugi::xml_node DataQuaTieuPhiTichLuy = oCustomQuaPhucLoi.child("QuaTieuPhiTichLuy");
	this->EnableB[eQuaTieuPhiTichLuy] = DataQuaTieuPhiTichLuy.attribute("Enable").as_int();
	for (pugi::xml_node ListQuaTieuPhiTichLuy = DataQuaTieuPhiTichLuy.child("List"); ListQuaTieuPhiTichLuy; ListQuaTieuPhiTichLuy = ListQuaTieuPhiTichLuy.next_sibling())
	{
		DATA_QUAPHUCLOI_LIST infoQuaTieuPhiTichLuy;

		infoQuaTieuPhiTichLuy.DieuKienNhan = ListQuaTieuPhiTichLuy.attribute("DieuKienNhan").as_int();
		infoQuaTieuPhiTichLuy.GiftIndex = ListQuaTieuPhiTichLuy.attribute("GiftIndex").as_int();

		this->List_QuaTieuPhiTichLuy.push_back(infoQuaTieuPhiTichLuy);
	}

	this->m_DataGift.clear();
	pugi::xml_node DataItemGift = oCustomQuaPhucLoi.child("DataItemGift");
	for (pugi::xml_node DataGift = DataItemGift.child("Data"); DataGift; DataGift = DataGift.next_sibling())
	{
		int GiftIndex = DataGift.attribute("GiftIndex").as_int();
		DATA_GIFT infoGift;
		//==Coin Nhan
		pugi::xml_node CoinNhan = DataGift.child("CoinNhan");
		infoGift.GiftIndex = GiftIndex;
		infoGift.ListItemNhan.clear();
		pugi::xml_node ItemNhan = DataGift.child("ItemNhan");
		for (pugi::xml_node Item = ItemNhan.child("Item"); Item; Item = Item.next_sibling())
		{
			DATA_CBITEM_LIST ListItemInfo;
			ListItemInfo.SizeBMD = Item.attribute("SizeBMD").as_float(1.0f);
			if (ListItemInfo.SizeBMD <= 0.0f)
			{
				ListItemInfo.SizeBMD = 1.0f;
			}
			ListItemInfo.Count = Item.attribute("Count").as_int();
			ListItemInfo.IndexItem = Item.attribute("IndexItem").as_int();
			ListItemInfo.LvItem = Item.attribute("LvItem").as_int();
			ListItemInfo.Dur = Item.attribute("Dur").as_int();
			ListItemInfo.Skill = Item.attribute("Skill").as_int();
			ListItemInfo.Luck = Item.attribute("Luck").as_int();
			ListItemInfo.Opt = Item.attribute("Opt").as_int();
			ListItemInfo.Exc = Item.attribute("Exc").as_int();
			ListItemInfo.Anc = Item.attribute("Anc").as_int();

			ListItemInfo.SK[0] = Item.attribute("SK1").as_int();
			ListItemInfo.SK[1] = Item.attribute("SK2").as_int();
			ListItemInfo.SK[2] = Item.attribute("SK3").as_int();
			ListItemInfo.SK[3] = Item.attribute("SK4").as_int();
			ListItemInfo.SK[4] = Item.attribute("SK5").as_int();

			ListItemInfo.SKBonus = Item.attribute("SKBonus").as_int();
			ListItemInfo.HSD = Item.attribute("HSD").as_int();

			ListItemInfo.Class[0] = Item.attribute("DW").as_int();
			ListItemInfo.Class[1] = Item.attribute("DK").as_int();
			ListItemInfo.Class[2] = Item.attribute("ELF").as_int();
			ListItemInfo.Class[3] = Item.attribute("MG").as_int();
			ListItemInfo.Class[4] = Item.attribute("DL").as_int();
			ListItemInfo.Class[5] = Item.attribute("SUM").as_int();
			ListItemInfo.Class[6] = Item.attribute("RF").as_int();
			infoGift.ListItemNhan.push_back(ListItemInfo);
		}
		this->m_DataGift.insert(std::pair<int, DATA_GIFT>(GiftIndex, infoGift));
	}
	LogAdd(LOG_BLUE, "[QuaPhucLoi] Load OK !");
}
char* CB_QuaPhucLoi::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO>::iterator it = this->m_MessageInfoBP.find(index);

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

std::vector<CB_QuaPhucLoi::DATA_QUAPHUCLOI_LIST> CB_QuaPhucLoi::GetDataList(int Type)
{
	std::vector<DATA_QUAPHUCLOI_LIST> temp;
	switch (Type)
	{
	case CB_QuaPhucLoi::eQuaNapDau: 
		temp = this->List_QuaNapDau;
		break;
	case CB_QuaPhucLoi::eQuaNapNgay:
		temp = this->List_QuaNapNgay;
		break;
	case CB_QuaPhucLoi::eQuaNapThang:
		temp = this->List_QuaNapThang;
		break;
	case CB_QuaPhucLoi::eQuaNapTichLuy:
		temp = this->List_QuaNapTichLuy;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiNgay:
		temp = this->List_QuaTieuPhiNgay;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiThang:
		temp = this->List_QuaTieuPhiThang;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
		temp = this->List_QuaTieuPhiTichLuy;
		break;
	default:
		break;
	}
	return temp;
}
void CB_QuaPhucLoi::SaveMocDaNhan(LPOBJ lpObj, int Type, int DaNhan)
{
	int temp = 0;
	switch (Type)
	{
	case CB_QuaPhucLoi::eQuaNapDau:
		lpObj->NhanQuaNapDau = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaNapNgay:
		lpObj->NhanQuaNapNgay = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaNapThang:
		lpObj->NhanQuaNapThang = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaNapTichLuy:
		lpObj->NhanQuaNapTichLuy = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiNgay:
		lpObj->NhanQuaTieuPhiNgay = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiThang:
		lpObj->NhanQuaTieuPhiThang = DaNhan;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
		lpObj->NhanQuaTieuPhiTichLuy = DaNhan;
		break;
	default:
		break;
	}

}
int CB_QuaPhucLoi::GetMocDaNhan(LPOBJ lpObj, int Type)
{
	int temp = 0;
	switch (Type)
	{
	case CB_QuaPhucLoi::eQuaNapDau:
		temp = lpObj->NhanQuaNapDau;
		break;
	case CB_QuaPhucLoi::eQuaNapNgay:
		temp = lpObj->NhanQuaNapNgay;
		break;
	case CB_QuaPhucLoi::eQuaNapThang:
		temp = lpObj->NhanQuaNapThang;
		break;
	case CB_QuaPhucLoi::eQuaNapTichLuy:
		temp = lpObj->NhanQuaNapTichLuy;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiNgay:
		temp = lpObj->NhanQuaTieuPhiNgay;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiThang:
		temp = lpObj->NhanQuaTieuPhiThang;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
		temp = lpObj->NhanQuaTieuPhiTichLuy;
		break;
	default:
		break;
	}
	return temp;
}
int CB_QuaPhucLoi::GetCoinCache(LPOBJ lpObj,int Type)
{
	int temp = 0;
	switch (Type)
	{
	case CB_QuaPhucLoi::eQuaNapDau:
		temp = lpObj->QuaNapDau;
		break;
	case CB_QuaPhucLoi::eQuaNapNgay:
		temp = lpObj->QuaNapNgay;
		break;
	case CB_QuaPhucLoi::eQuaNapThang:
		temp = lpObj->QuaNapThang;
		break;
	case CB_QuaPhucLoi::eQuaNapTichLuy:
		temp = lpObj->QuaNapTichLuy;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiNgay:
		temp = lpObj->QuaTieuPhiNgay;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiThang:
		temp = lpObj->QuaTieuPhiThang;
		break;
	case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
		temp = lpObj->QuaTieuPhiTichLuy;
		break;
	default:
		break;
	}
	return temp;
}
void CB_QuaPhucLoi::ItemByteConvert(BYTE* lpMsg, DATA_CBITEM_LIST* Data) // OK
{

	lpMsg[0] = Data->IndexItem & 0xFF;

	lpMsg[1] = 0;
	lpMsg[1] |= Data->LvItem * 8;
	lpMsg[1] |= Data->Skill * 128;
	lpMsg[1] |= Data->Luck * 4;
	lpMsg[1] |= Data->Opt & 3;

	lpMsg[2] = Data->Dur;

	lpMsg[3] = 0;
	lpMsg[3] |= (Data->IndexItem & 0x100) >> 1;
	lpMsg[3] |= ((Data->Opt > 3) ? 0x40 : 0);
	lpMsg[3] |= Data->Exc;

	lpMsg[4] = Data->Anc;

	lpMsg[5] = 0;
	lpMsg[5] |= (Data->IndexItem & 0x1E00) >> 5;
	lpMsg[5] |= ((Data->Exc & 0x80) >> 4);
	lpMsg[5] |= ((Data->HSD & 1) << 2);

	lpMsg[6] = Data->SKBonus;

	lpMsg[7] = Data->SK[0];
	lpMsg[8] = Data->SK[1];
	lpMsg[9] = Data->SK[2];
	lpMsg[10] = Data->SK[3];
	lpMsg[11] = Data->SK[4];
}
void CB_QuaPhucLoi::RecvClientNhanQua(BYTE* aRecv, int aIndex)
{
	if (!aRecv) return;

	if (!OBJECT_USER_RANGE(aIndex))
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	CGPACKET_SENDRECV_A* lpMsg = (CGPACKET_SENDRECV_A*)aRecv;


	std::vector<DATA_QUAPHUCLOI_LIST> GetListQuaPhucLoi = this->GetDataList(lpMsg->Type);


	if (GetListQuaPhucLoi.empty() || lpMsg->Index > GetListQuaPhucLoi.size())
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(1)); 
		return;
	}
	int IndexMocNhan = lpMsg->Index;
	int MocDaNhan = this->GetMocDaNhan(&gObj[aIndex], lpMsg->Type);
	int TongDieuKien = this->GetCoinCache(&gObj[aIndex], lpMsg->Type);
	LogAdd(LOG_RED, "Nhan Qua Type %d Index %d (MocDaNhan %d)", lpMsg->Type, lpMsg->Index, MocDaNhan);
	//===Kiem tra thung do
	if (gItemManager.CheckItemInventorySpace(lpObj, 4, 4) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(6));
		return;
	}
	if (IndexMocNhan > MocDaNhan)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(2), MocDaNhan+1); //Nhan MOc Nho hon truoc
		return;
	}
	if (IndexMocNhan < MocDaNhan)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(4)); //da nhan roi
		return;
	}
	if (GetListQuaPhucLoi[IndexMocNhan].DieuKienNhan > TongDieuKien)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(GetListQuaPhucLoi[IndexMocNhan].DieuKienNhan)); //Yeu cau dieu kien
		return;
	}
	//===Lay Thong Item
	std::map<int, DATA_GIFT>::iterator it = this->m_DataGift.find(GetListQuaPhucLoi[IndexMocNhan].GiftIndex);

	if (it == this->m_DataGift.end())
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, "Khong Co Thong Tin GiftIndex ");
		return;
	}
	//================ Add Item
	if (it->second.ListItemNhan.empty() == 0)
	{
		for (int n = 0; n < it->second.ListItemNhan.size(); n++)
		{
			if (it->second.ListItemNhan[n].Class[gObj[aIndex].Class] == 0) 
			{
				continue;
			}
			for (int count = 0; count < it->second.ListItemNhan[n].Count; count++)
			{
				BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { it->second.ListItemNhan[n].SK[0], it->second.ListItemNhan[n].SK[1], it->second.ListItemNhan[n].SK[2], it->second.ListItemNhan[n].SK[3], it->second.ListItemNhan[n].SK[4] };
				time_t t = time(NULL);
				localtime(&t);
				DWORD iTime = (DWORD)t + it->second.ListItemNhan[n].HSD * 60;
				if (it->second.ListItemNhan[n].HSD > 0)
				{
					GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, iTime);
				}
				else
				{
					GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, 0);
				}
			}
		}
	}
	this->SaveMocDaNhan(lpObj, lpMsg->Type, MocDaNhan + 1);

	int giftIndex = GetListQuaPhucLoi[IndexMocNhan].GiftIndex;
	int dieuKien = GetListQuaPhucLoi[IndexMocNhan].DieuKienNhan;
	int mocSau = this->GetMocDaNhan(lpObj, lpMsg->Type);

	// LOG TONG
	gLog.Output(LOG_QUAPHUCLOI, "-----------");
	gLog.Output(LOG_QUAPHUCLOI,
		"[QuaNap][CLAIM][Acc:%s][Name:%s][Idx:%d] Type=%d Moc=%d GiftIndex=%d DK=%d Tong=%d MocDaNhan=%d->%d (After=%d)",
		lpObj->Account, lpObj->Name, aIndex,
		lpMsg->Type, IndexMocNhan, giftIndex,
		dieuKien, TongDieuKien,
		MocDaNhan, MocDaNhan + 1, mocSau
	);

	// LOG ITEM THEO LIST
	for (int n = 0; n < (int)it->second.ListItemNhan.size(); n++)
	{
		auto& rw = it->second.ListItemNhan[n];

		if (rw.Class[lpObj->Class] == 0)
			continue;

		gLog.Output(LOG_QUAPHUCLOI,
			"[QuaNap][ITEM][Acc:%s][Name:%s] Type=%d Moc=%d GiftIndex=%d Item=%d Lv=%d Dur=%d Skill=%d Luck=%d Opt=%d Exc=%d Anc=%d Count=%d HSD=%d SKB=%d SK={%d,%d,%d,%d,%d}",
			lpObj->Account, lpObj->Name,
			lpMsg->Type, IndexMocNhan, giftIndex,
			rw.IndexItem, rw.LvItem, rw.Dur,
			rw.Skill, rw.Luck, rw.Opt, rw.Exc, rw.Anc,
			rw.Count, rw.HSD, rw.SKBonus,
			rw.SK[0], rw.SK[1], rw.SK[2], rw.SK[3], rw.SK[4]
		);
		
	}
	gLog.Output(LOG_QUAPHUCLOI, "-----------");
	//Reload Info
	CB_QuaPhucLoi::CGPACKET_SENDRECV_B pMsg;
	pMsg.header.set(0xD3, 0x4D, sizeof(pMsg));
	pMsg.ThaoTac = lpMsg->Type; //
	CB_QuaPhucLoi::RecvClient((LPBYTE)&pMsg, aIndex);
}
void CB_QuaPhucLoi::RecvClient(BYTE* aRecv, int aIndex)
{
	if (!aRecv) return;

	if (!OBJECT_USER_RANGE(aIndex))
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}

	CGPACKET_SENDRECV_B* lpMsg = (CGPACKET_SENDRECV_B*)aRecv;
	bool SendStatus = true;
	switch (lpMsg->ThaoTac)
	{
	case CB_QuaPhucLoi::eGiftCode:
		{
#if(CB_CUSTOMGIFTCODE)
		if (!gCB_GiftCode.Enable)
		{
			gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "Chức năng này hiện tại đang bị khóa !"); //
			return;
		}
		
#else
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "Chức năng này hiện tại đang bị khóa !"); //
		return;
#endif
		}
		break;
	case CB_QuaPhucLoi::eQuaNapDau:
	case CB_QuaPhucLoi::eQuaNapNgay:
	case CB_QuaPhucLoi::eQuaNapThang:
	case CB_QuaPhucLoi::eQuaNapTichLuy:
	case CB_QuaPhucLoi::eQuaTieuPhiNgay:
	case CB_QuaPhucLoi::eQuaTieuPhiThang:
	case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
	{
		if (!this->Enable || !this->EnableB[lpMsg->ThaoTac])
		{
			gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
			return;
		}
		BYTE send[4096];
		PMSG_CBLISTTHUONG_SEND pMsg = { 0 };
		// ---
		pMsg.header.set(0xD3, 0x4E, 0);

		int size = sizeof(pMsg);

		pMsg.count = 0;
		pMsg.MocDaNhan = this->GetMocDaNhan(&gObj[aIndex], lpMsg->ThaoTac);
		pMsg.CoinCache = this->GetCoinCache(&gObj[aIndex],lpMsg->ThaoTac);

		std::vector<DATA_QUAPHUCLOI_LIST> GetListQuaPhucLoi = this->GetDataList(lpMsg->ThaoTac);


		if (GetListQuaPhucLoi.empty())
		{
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(1));
			return;
		}

		for (std::vector<DATA_QUAPHUCLOI_LIST>::iterator itItem = GetListQuaPhucLoi.begin(); itItem != GetListQuaPhucLoi.end(); itItem++)
		{
			
			DATALIST_QUAPL info = { 0 };
			info.DieuKienNhan = itItem->DieuKienNhan;
			//===Lay Thong Item
			std::map<int, DATA_GIFT>::iterator it = this->m_DataGift.find(itItem->GiftIndex);

			if (it == this->m_DataGift.end())
			{
				gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, "Khong Co Thong Tin GiftIndex "); 
				return;
			}
			int nList = 0;
			for (std::vector<DATA_CBITEM_LIST>::iterator itItem = it->second.ListItemNhan.begin(); itItem != it->second.ListItemNhan.end(); itItem++)
			{
				if (itItem == it->second.ListItemNhan.end() || nList > 5)
				{
					break;
				}

				if (itItem->Class[gObj[aIndex].Class] == 0) //Neu khong phai class duoc active thi bo qua
				{
					continue;
				}

				info.mListItem[nList].SizeBMD = itItem->SizeBMD;
				info.mListItem[nList].Count = itItem->Count;
				info.mListItem[nList].Index = itItem->IndexItem;
				info.mListItem[nList].Dur = itItem->Dur;
				this->ItemByteConvert(info.mListItem[nList].Item, &*itItem);
				time_t t = time(NULL);
				localtime(&t);
				DWORD iTime = (DWORD)t + itItem->HSD * 60;
				if ((itItem->HSD) > 0)
				{
					info.mListItem[nList].PeriodTime = iTime;
				}
				else
				{
					info.mListItem[nList].PeriodTime = itItem->HSD;
				}
				nList++;

			}
			if ((size + sizeof(info) >= 4096))
			{
				break;
			}
			pMsg.count++;
			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);
		}
		if (pMsg.count > 0)
		{
			pMsg.header.size[0] = SET_NUMBERHB(size);
			pMsg.header.size[1] = SET_NUMBERLB(size);
			// ---
			memcpy(send, &pMsg, sizeof(pMsg));

			DataSend(aIndex, send, size);
		}


	}
	break;
	default:
		SendStatus = false;
		break;
	}
	if (SendStatus)
	{
		CB_QuaPhucLoi::CGPACKET_SENDRECV_B pMsg;
		pMsg.header.set(0xD3, 0x4D, sizeof(pMsg));
		pMsg.ThaoTac = lpMsg->ThaoTac;	//Index Cua So NPC cong huong
		pMsg.server_time = time(NULL);
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		gObj[aIndex].CacheTimeGetInfo = GetTickCount()+1000;
	}
}
void CB_QuaPhucLoi::SetCoinTieuPhi(int aIndex, int CoinTieuPhi) // OK
{
	if (!OBJECT_USER_RANGE(aIndex))
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_SET_COIN_TIEU_PHI pMsg;

	pMsg.h.set(0xD3, 0x6B, sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.CoinTieuPhi = CoinTieuPhi;

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
}
void CB_QuaPhucLoi::GDGetInfoQuaPhucLoi(int aIndex) // OK
{
	if (!OBJECT_USER_RANGE(aIndex))
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_SET_COIN_TIEU_PHI pMsg;

	pMsg.h.set(0xD3, 0x6C, sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.CoinTieuPhi = 0;

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	//LogAdd(LOG_RED, "GDGetInfoQuaPhucLoi %s", gObj[aIndex].Name);
}
void CB_QuaPhucLoi::DSGetInfoQuaPhucLoi(BYTE* Recv)
{
	SDHP_SEND_GET_INFO_QPL* lpMsg = (SDHP_SEND_GET_INFO_QPL*)Recv;


	if (!OBJECT_USER_RANGE(lpMsg->index))
	{
		return;
	}
	if (gObj[lpMsg->index].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(lpMsg->index) == false)
	{
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];
	lpObj->QuaNapDau = lpMsg->QuaNapDau;
	lpObj->QuaNapNgay = lpMsg->QuaNapNgay;
	lpObj->QuaNapThang = lpMsg->QuaNapThang;
	lpObj->QuaNapTichLuy = lpMsg->QuaNapTichLuy;
	lpObj->QuaTieuPhiNgay = lpMsg->QuaTieuPhiNgay;
	lpObj->QuaTieuPhiThang = lpMsg->QuaTieuPhiThang;
	lpObj->QuaTieuPhiTichLuy = lpMsg->QuaTieuPhiTichLuy;

	//LogAdd(LOG_RED, "DSGetInfoQuaPhucLoi %s %d,%d,%d,%d,%d,%d,%d", lpObj->Name,
	//	lpObj->QuaNapDau,
	//	lpObj->QuaNapNgay,
	//	lpObj->QuaNapThang,
	//	lpObj->QuaNapTichLuy,
	//	lpObj->QuaTieuPhiNgay,
	//	lpObj->QuaTieuPhiThang,
	//	lpObj->QuaTieuPhiTichLuy);
}

void CB_QuaPhucLoi::GDResetQuaPhucLoi(int aIndex, int Type) // OK
{
	if (!OBJECT_USER_RANGE(aIndex))
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_SET_COIN_TIEU_PHI pMsg;

	pMsg.h.set(0xD3, 0x6E, sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.CoinTieuPhi = Type;

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);
	//==Reload
	if (Type == 0)
	{
		lpObj->QuaNapNgay = 0;
		lpObj->NhanQuaNapNgay = 0;
		lpObj->QuaTieuPhiNgay = 0;
		lpObj->NhanQuaTieuPhiNgay = 0;
	}
	else if (Type == 1)
	{
		lpObj->QuaNapThang = 0;
		lpObj->NhanQuaNapThang = 0;
		lpObj->QuaTieuPhiThang = 0;
		lpObj->NhanQuaTieuPhiThang = 0;
	}
	gObj[aIndex].CacheTimeGetInfo = GetTickCount() + 1000;
	//LogAdd(LOG_RED, "GDGetInfoQuaPhucLoi %s", gObj[aIndex].Name);
}
#endif