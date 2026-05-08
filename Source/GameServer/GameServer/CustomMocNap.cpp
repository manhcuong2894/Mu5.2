#include "StdAfx.h"
#include "CustomMocNap.h"
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
#include "Log.h"
#include <limits.h>
#if(CUSTOM_MOCNAP)
gBMocNap gMocNap;

gBMocNap::gBMocNap()
{
	this->Enable = 0;
	this->Firework = 0;
	this->Notice = 0;
	this->RateNapThe = 0;
	this->RateApplyTo = 0;
	this->AutoRewardTick = 0;
}

gBMocNap::~gBMocNap()
{
}

static int CalcMocNapRateCoin(int giaTriNap, int rateNapThe)
{
	if (giaTriNap <= 0 || rateNapThe <= 0)
	{
		return 0;
	}

	__int64 value = (__int64)giaTriNap * (__int64)rateNapThe;
	return (value > INT_MAX) ? INT_MAX : (int)value;
}

static void ApplyMocNapRate(DATA_CBMOCNAP* infoData, int rateNapThe, int rateApplyTo)
{
	if (infoData == 0 || rateNapThe <= 0)
	{
		return;
	}

	int coinValue = CalcMocNapRateCoin(infoData->GiaTriNap, rateNapThe);
	switch (rateApplyTo)
	{
	case 1:
		infoData->WC = coinValue;
		break;
	case 2:
		infoData->WP = coinValue;
		break;
	case 3:
		infoData->WC = coinValue;
		infoData->WP = coinValue;
		break;
	case 4:
		infoData->GP = coinValue;
		break;
	case 5:
		infoData->WC = coinValue;
		infoData->GP = coinValue;
		break;
	case 6:
		infoData->WP = coinValue;
		infoData->GP = coinValue;
		break;
	case 7:
		infoData->WC = coinValue;
		infoData->WP = coinValue;
		infoData->GP = coinValue;
		break;
	}
}

void gBMocNap::CalcAutoRewardCoin(int amount, int* wc, int* wp, int* gp)
{
	if (wc == 0 || wp == 0 || gp == 0)
	{
		return;
	}

	*wc = 0;
	*wp = 0;
	*gp = 0;

	if (amount <= 0 || this->RateNapThe <= 0)
	{
		return;
	}

	int coinValue = CalcMocNapRateCoin(amount, this->RateNapThe);
	switch (this->RateApplyTo)
	{
	case 1:
		*wc = coinValue;
		break;
	case 2:
		*wp = coinValue;
		break;
	case 3:
		*wc = coinValue;
		*wp = coinValue;
		break;
	case 4:
		*gp = coinValue;
		break;
	case 5:
		*wc = coinValue;
		*gp = coinValue;
		break;
	case 6:
		*wp = coinValue;
		*gp = coinValue;
		break;
	case 7:
		*wc = coinValue;
		*wp = coinValue;
		*gp = coinValue;
		break;
	default:
		*wc = coinValue;
		*wp = coinValue;
		*gp = coinValue;
		break;
	}
}

static void SyncMocNapAccountOnline(const char* account, int tongNap, int nhanMocNap, int skipIndex)
{
	if (account == 0 || account[0] == 0)
	{
		return;
	}

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		if (gObj[n].Type != OBJECT_USER || strcmp(gObj[n].Account, account) != 0)
		{
			continue;
		}

		if (tongNap >= 0)
		{
			gObj[n].TongNap = tongNap;
		}

		if (nhanMocNap >= 0 && gObj[n].NhanMocNap < nhanMocNap)
		{
			gObj[n].NhanMocNap = nhanMocNap;
		}

		if (n != skipIndex)
		{
			gMocNap.UserSendClientInfo(n);
		}
	}
}

static bool IsMocNapRewardItemForClass(const DATA_CBITEMMOCNAP& item, int Class)
{
	if (Class < 0 || Class >= MAX_CLASS)
	{
		return false;
	}

	return (item.Class[Class] != 0);
}

static void SendMocNapRewardInventoryFullNotice(LPOBJ lpObj, char* message)
{
	if (lpObj == 0 || message == 0)
	{
		return;
	}

	gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, message);
	gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, message);
}

static bool CheckMocNapRewardInventorySpace(LPOBJ lpObj, DATA_CBMOCNAP* lpMocNap)
{
	if (lpObj == 0 || lpMocNap == 0)
	{
		return false;
	}

	BYTE InventoryMap[INVENTORY_FULL_SIZE];
	memcpy(InventoryMap, lpObj->InventoryMap, sizeof(InventoryMap));

	int MaxY = (gItemManager.GetInventoryMaxValue(lpObj) - INVENTORY_WEAR_SIZE) / 8;
	for (std::vector<DATA_CBITEMMOCNAP>::iterator itItem = lpMocNap->ListItemNhan.begin(); itItem != lpMocNap->ListItemNhan.end(); itItem++)
	{
		if (IsMocNapRewardItemForClass(*itItem, lpObj->Class) == false || itItem->Count <= 0)
		{
			continue;
		}

		ITEM_INFO ItemInfo;
		if (gItemManager.GetInfo(itItem->IndexItem, &ItemInfo) == 0)
		{
			return false;
		}

		for (int count = 0; count < itItem->Count; count++)
		{
			if (gItemManager.CheckItemInventorySpace(InventoryMap, MaxY, ItemInfo.Width, ItemInfo.Height) == false)
			{
				return false;
			}
		}
	}

	return true;
}

void gBMocNap::LoadFileXML(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	//--
	pugi::xml_node oCustomMocNap = file.child("CustomMocNap");
	this->Enable = oCustomMocNap.attribute("Enable").as_int();
	this->Firework = oCustomMocNap.attribute("Firework").as_int();
	this->Notice = oCustomMocNap.attribute("Notice").as_int();
	//= Mess Load
	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = oCustomMocNap.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_MocNap info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO_MocNap>(info.Index, info));
	}
	//====Load Data Moc Nap
	this->m_DataMocNap.clear();
	pugi::xml_node ConfigMocNap = oCustomMocNap.child("ConfigMocNap");
	this->RateNapThe = ConfigMocNap.attribute("RateNapThe").as_int(0);
	this->RateApplyTo = ConfigMocNap.attribute("RateApplyTo").as_int(0);
	if (this->RateApplyTo < 1 || this->RateApplyTo > 7)
	{
		this->RateApplyTo = 7;
	}
	int IndexMocNap = 1;
	for (pugi::xml_node MocNap = ConfigMocNap.child("MocNap"); MocNap; MocNap = MocNap.next_sibling())
	{
		DATA_CBMOCNAP infoData = { 0 };
		infoData.IndexMocNap = IndexMocNap++;
		infoData.GiaTriNap = MocNap.attribute("GiaTriNap").as_int();
		//==Coin Nhan
		pugi::xml_node CoinNhan = MocNap.child("CoinNhan");
		infoData.WC = CoinNhan.attribute("WC").as_int();
		infoData.WP = CoinNhan.attribute("WP").as_int();
		infoData.GP = CoinNhan.attribute("GP").as_int();
		infoData.Ruud = CoinNhan.attribute("Ruud").as_int();
		//===ItemNhan
		infoData.ListItemNhan.clear();
		pugi::xml_node ItemNhan = MocNap.child("ItemNhan");
		for (pugi::xml_node Item = ItemNhan.child("Item"); Item; Item = Item.next_sibling())
		{
			DATA_CBITEMMOCNAP ListItemInfo = { 0 };
			ListItemInfo.SizeBMD = Item.attribute("SizeBMD").as_float();
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
			infoData.ListItemNhan.push_back(ListItemInfo);
		}
		this->m_DataMocNap.insert(std::pair<int, DATA_CBMOCNAP>(infoData.IndexMocNap, infoData));
	}
	//===Cap Nhat Lai Danh Sach List Nap
	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Connected == OBJECT_ONLINE)
		{
			if ((gObj[n].Type != OBJECT_USER))
			{
				continue;
			}
			UserSendClientInfo(n);
		}
	}
	LogAdd(LOG_BLUE, "[LoadMocNap] [%d] Size %d RateNapThe %d RateApplyTo %d", this->Enable, this->m_DataMocNap.size(), this->RateNapThe, this->RateApplyTo);
}

void gBMocNap::MainProc()
{
	if (this->Enable == 0 || this->RateNapThe <= 0)
	{
		return;
	}

	if ((GetTickCount() - this->AutoRewardTick) < 30000)
	{
		return;
	}

	this->AutoRewardTick = GetTickCount();

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		if (gObj[n].Type != OBJECT_USER || gObj[n].IsBot >= 1)
		{
			continue;
		}

		GDMocNapAutoRewardSend(n);
	}
}

char* gBMocNap::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_MocNap>::iterator it = this->m_MessageInfoBP.find(index);

	if (it == this->m_MessageInfoBP.end())
	{
		static char Error[256];
		wsprintf(Error, "Could not find message %d!", index);
		return Error;
	}
	else
	{
		return it->second.Message;
	}
}
void gBMocNap::UserSendClientInfo(int aIndex) //Send Danh Sach Moc Nap Ve Client
{
	if (OBJECT_RANGE(aIndex) == 0)
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
	//if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	if (gObj[aIndex].IsBot >= 1)
	{
		return;
	}
	BYTE send[4096];
	PMSG_CBMOCNAP_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x9A, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.NhanMocNap = gObj[aIndex].NhanMocNap;
	pMsg.TongNap = gObj[aIndex].TongNap;
	pMsg.RateNapThe = this->RateNapThe;
	pMsg.RateApplyTo = this->RateApplyTo;

	for (std::map<int, DATA_CBMOCNAP>::iterator it = this->m_DataMocNap.begin(); it != this->m_DataMocNap.end(); it++)
	{
		if (it == this->m_DataMocNap.end())
		{
			break;
		}
		ListMocNapSend info = { 0 };
		info.IndexMocNap = it->second.IndexMocNap;
		info.GiaTriNap = it->second.GiaTriNap;
		if ((size + sizeof(info) >= 4096))
		{
			break;
		}
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "SendINfo List Moc Nap %s", gObj[aIndex].Name);
}

void gBMocNap::DGMocNapAutoRewardRecv(SDHP_MOCNAP_AUTO_REWARD_RECV* lpMsg)
{
	if (lpMsg == 0 || lpMsg->result == 0 || lpMsg->AutoRewardAmount <= 0)
	{
		return;
	}

	if (OBJECT_RANGE(lpMsg->index) == 0 || gObjIsConnectedGP(lpMsg->index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if (strcmp(lpObj->Account, lpMsg->account) != 0 || strcmp(lpObj->Name, lpMsg->name) != 0)
	{
		return;
	}

	int addWC = 0;
	int addWP = 0;
	int addGP = 0;
	this->CalcAutoRewardCoin(lpMsg->AutoRewardAmount, &addWC, &addWP, &addGP);

	if (addWC <= 0 && addWP <= 0 && addGP <= 0)
	{
		return;
	}

	lpObj->TongNap = lpMsg->TongNap;
	SyncMocNapAccountOnline(lpObj->Account, lpObj->TongNap, -1, lpObj->Index);
	GDSetCoinSend(lpObj->Index, addWC, addWP, addGP, "AutoMocNap");
	UserSendClientInfo(lpObj->Index);

	gLog.Output(LOG_MocNap, "[AutoMocNap] %s - %s Nap:%d TongNap:%d Reward WC:%d WP:%d GP:%d Rate:%d Apply:%d",
		lpObj->Account, lpObj->Name, lpMsg->AutoRewardAmount, lpMsg->TongNap, addWC, addWP, addGP, this->RateNapThe, this->RateApplyTo);
}

static void SendMocNapPaymentCreateResult(int aIndex, int result, int amount, QWORD orderCode, const char* message)
{
	if (OBJECT_RANGE(aIndex) == 0 || gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_MOCNAP_PAYMENT_CREATE_SEND pMsg;
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.header.set(0xD3, 0x9D, sizeof(pMsg));
	pMsg.Result = result;
	pMsg.Amount = amount;
	pMsg.OrderCode = orderCode;
	if (message != 0)
	{
		strncpy_s(pMsg.Message, sizeof(pMsg.Message), message, _TRUNCATE);
	}

	DataSend(aIndex, (BYTE*)&pMsg, sizeof(pMsg));
}

void gBMocNap::CreatePaymentLink(int aIndex, int amount)
{
	if (OBJECT_RANGE(aIndex) == 0 || gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (this->Enable == 0)
	{
		SendMocNapPaymentCreateResult(aIndex, 0, amount, 0, "Moc nap chua duoc bat");
		return;
	}

	if (amount <= 0)
	{
		SendMocNapPaymentCreateResult(aIndex, 0, amount, 0, "So tien nap khong hop le");
		return;
	}

	GDMocNapPaymentCreateSend(aIndex, amount);
}

void gBMocNap::DGMocNapPaymentCreateRecv(SDHP_MOCNAP_PAYMENT_CREATE_RECV* lpMsg)
{
	if (lpMsg == 0)
	{
		return;
	}

	if (OBJECT_RANGE(lpMsg->index) == 0 || gObjIsConnectedGP(lpMsg->index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];
	if (strcmp(lpObj->Account, lpMsg->account) != 0 || strcmp(lpObj->Name, lpMsg->name) != 0)
	{
		return;
	}

	PMSG_MOCNAP_PAYMENT_CREATE_SEND pMsg;
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.header.set(0xD3, 0x9D, sizeof(pMsg));
	pMsg.Result = lpMsg->Result;
	pMsg.Amount = lpMsg->Amount;
	pMsg.OrderCode = lpMsg->OrderCode;
	memcpy(pMsg.Message, lpMsg->Message, sizeof(pMsg.Message));
	memcpy(pMsg.CheckoutUrl, lpMsg->CheckoutUrl, sizeof(pMsg.CheckoutUrl));
	memcpy(pMsg.QrCode, lpMsg->QrCode, sizeof(pMsg.QrCode));
	memcpy(pMsg.BankBin, lpMsg->BankBin, sizeof(pMsg.BankBin));
	memcpy(pMsg.AccountNumber, lpMsg->AccountNumber, sizeof(pMsg.AccountNumber));
	memcpy(pMsg.AccountName, lpMsg->AccountName, sizeof(pMsg.AccountName));
	memcpy(pMsg.Description, lpMsg->Description, sizeof(pMsg.Description));

	DataSend(lpMsg->index, (BYTE*)&pMsg, sizeof(pMsg));
}

void CBMONAP_ItemByteConvert(BYTE* lpMsg, DATA_CBITEMMOCNAP* Data) // OK
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

void gBMocNap::SendListNhanThuong(int aIndex, int MocNap) //Send List SendListNhanThuong
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	if (!this->Enable)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
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

	//if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	if (gObj[aIndex].IsBot >= 1)
	{
		return;
	}

	BYTE send[4096];

	PMSG_CBLISTTHUONG_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x9B, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	std::map<int, DATA_CBMOCNAP>::iterator it = this->m_DataMocNap.find(MocNap);

	if (it == this->m_DataMocNap.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	pMsg.WC = it->second.WC;
	pMsg.WP = it->second.WP;
	pMsg.GP = it->second.GP;
	pMsg.Ruud = it->second.Ruud;

	for (std::vector<DATA_CBITEMMOCNAP>::iterator itItem = it->second.ListItemNhan.begin(); itItem != it->second.ListItemNhan.end(); itItem++)
	{
		if (itItem == it->second.ListItemNhan.end())
		{
			break;
		}

		if (IsMocNapRewardItemForClass(*itItem, gObj[aIndex].Class) == false) //Neu khong phai class duoc active thi bo qua
		{
			continue;
		}
		LISTITEMMOCNAP_SENDINFO info = { 0 };
		info.SizeBMD = itItem->SizeBMD;
		info.Count = itItem->Count;
		info.Index = itItem->IndexItem;
		info.Dur = itItem->Dur;
		CBMONAP_ItemByteConvert(info.Item, &*itItem);
		time_t t = time(NULL);
		localtime(&t);
		DWORD iTime = (DWORD)t + itItem->HSD * 60;
		if ((itItem->HSD) > 0)
		{
			info.PeriodTime = iTime;
		}
		else
		{
			info.PeriodTime = itItem->HSD;
		}
		if ((size + sizeof(info) >= 4096))
		{
			break;
		}
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	LogAdd(LOG_RED, "Send List Phan Thuong Moc Nap %s", gObj[aIndex].Name);
}
void gBMocNap::NhanThuongMocNap(int aIndex, int MocNap)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	if (!this->Enable)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
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
	//if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	if (gObj[aIndex].IsBot >= 1)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//===KIem tra trang thai co duoc add item khong
	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 || lpObj->Transaction == 1)
	{
		return;
	}

	/*if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}*/
	//===================================================

	//===Lay Thong Tin Moc Nap
	std::map<int, DATA_CBMOCNAP>::iterator it = this->m_DataMocNap.find(MocNap);

	if (it == this->m_DataMocNap.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	if (it->second.GiaTriNap > lpObj->TongNap)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(it->second.GiaTriNap)); //Yeu cau gia tri nap
		return;
	}
	if (it->second.IndexMocNap <= lpObj->NhanMocNap)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4)); //da nhan roi
		return;
	}
	//===Kiem tra thung do
	if (CheckMocNapRewardInventorySpace(lpObj, &it->second) == false)
	{
		SendMocNapRewardInventoryFullNotice(lpObj, this->GetMessage(6));
		return;
	}
	if ((MocNap - lpObj->NhanMocNap) > 1)
	{
		int nextMocNap = lpObj->NhanMocNap + 1;
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), nextMocNap); //Nhan MOc Nho hon truoc
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(2), nextMocNap);
		return;
	}


	//===Set Va thong bao
	lpObj->NhanMocNap = MocNap; //Save Moc Nhan
	SyncMocNapAccountOnline(lpObj->Account, lpObj->TongNap, lpObj->NhanMocNap, lpObj->Index);
	//==Send Effect
	if (this->Firework == 1)
	{
		GCServerCommandSend(lpObj->Index, 0, lpObj->X, lpObj->Y);
	}
	else if (this->Firework == 2)
	{
		GCServerCommandSend(lpObj->Index, 2, lpObj->X, lpObj->Y);
	}
	else if (this->Firework == 3)
	{
		GCServerCommandSend(lpObj->Index, 58, SET_NUMBERHB(lpObj->Index), SET_NUMBERLB(lpObj->Index));
	}
	char tmp[255];
	char tmp2[255];
	wsprintf(tmp, this->GetMessage(5), lpObj->Name, NumberFormat(it->second.GiaTriNap));
	if (this->Notice == 1) { //Thong Bao trong Sub
		gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, tmp);
	}
	else if (this->Notice == 2)
	{ //Thong Bao Toan Sub

		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
	}
	else if (this->Notice == 3)
	{ //Thong Bao Toan Sub
		wsprintf(tmp2, "%s %s", gServerInfo.m_ServerName, tmp);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp2);
	}
	//===============Cong Coin
	if (it->second.WC > 0 || it->second.WP > 0 || it->second.GP > 0)
	{
		GDSetCoinSend(lpObj->Index, it->second.WC, it->second.WP, it->second.GP, "CoinMocNap");
	}
		//if (it->second.Ruud > 0)
		//{
		//	gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, 0, 0, 0, it->second.Ruud);
		//	//-- Ruud Update
		//	gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "[Ruud] Add Coin %d", it->second.Ruud);

		//}
		//=====
		//================ Add Item
		if (it->second.ListItemNhan.empty() == 0)
		{
			for (int n = 0; n < it->second.ListItemNhan.size(); n++)
			{
				if (IsMocNapRewardItemForClass(it->second.ListItemNhan[n], gObj[aIndex].Class) == false) //Neu khong phai class duoc active thi bo qua
				{
					continue;
				}
				for (int count = 0; count < it->second.ListItemNhan[n].Count; count++)
				{
					BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { (BYTE)it->second.ListItemNhan[n].SK[0], (BYTE)it->second.ListItemNhan[n].SK[1], (BYTE)it->second.ListItemNhan[n].SK[2], (BYTE)it->second.ListItemNhan[n].SK[3], (BYTE)it->second.ListItemNhan[n].SK[4] };
					time_t t = time(NULL);
					localtime(&t);
					DWORD iTime = (DWORD)t + it->second.ListItemNhan[n].HSD * 60;
					if (it->second.ListItemNhan[n].HSD > 0)
					{
						GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, iTime);
						gLog.Output(LOG_MocNap, "%s - %s: nhận mốc:%d có giá trị là %s", lpObj->Account, lpObj->Name, it->second.IndexMocNap, NumberFormat(it->second.GiaTriNap));

					}
					else
					{
						GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, 0);
						gLog.Output(LOG_MocNap, "%s - %s: nhận mốc: %d có giá trị là %s - %s+%d Dur:%d Skill:%d Luck:%d Opt:%d Exc:%d Anc:%d",
							lpObj->Account,
							lpObj->Name,
							it->second.IndexMocNap,
							NumberFormat(it->second.GiaTriNap),
							gItemManager.GetItemName(it->second.ListItemNhan[n].IndexItem),
							it->second.ListItemNhan[n].LvItem,
							it->second.ListItemNhan[n].Dur,
							it->second.ListItemNhan[n].Skill,
							it->second.ListItemNhan[n].Luck,
							it->second.ListItemNhan[n].Opt,
							it->second.ListItemNhan[n].Exc,
							it->second.ListItemNhan[n].Anc);
					}
				}
			}
		}
	UserSendClientInfo(aIndex);//Update lai thong tin list
	GDCharacterInfoSaveSend(aIndex);

}
#endif
