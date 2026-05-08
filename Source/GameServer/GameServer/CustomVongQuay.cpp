
#include "stdafx.h"
#include "CustomVongQuay.h"
#include "CashShop.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "Guild.h"
#include "ItemBagManager.h"
#include "ItemLevel.h"
#include "ItemOptionRate.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Monster.h"
#include "Move.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "Party.h"
#include "RandomManager.h"
#include "ServerInfo.h"
#include "Util.h"

CCustomVongQuay gCustomVongQuay;

namespace {
const size_t kMaxVongQuayItems = 16;
const size_t kMaxTichLuyRewards = 32;

void LoadVongQuayItemFromXml(pugi::xml_node Item,
                             DATA_VONGQUAYITEM *ListItemInfo) {
  ListItemInfo->SizeBMD = Item.attribute("SizeBMD").as_float();
  ListItemInfo->PosX = Item.attribute("PosX").as_float();
  ListItemInfo->PosY = Item.attribute("PosY").as_float();
  ListItemInfo->IndexItem = SafeGetItem(GET_ITEM(
      Item.attribute("Type").as_int(), Item.attribute("Index").as_int()));
  ListItemInfo->LvItem = Item.attribute("LvItem").as_int();
  ListItemInfo->Dur = Item.attribute("Dur").as_int();
  ListItemInfo->Skill = Item.attribute("Skill").as_int();
  ListItemInfo->Luck = Item.attribute("Luck").as_int();
  ListItemInfo->Opt = Item.attribute("Opt").as_int();
  ListItemInfo->Exc = Item.attribute("Exc").as_int();
  ListItemInfo->Anc = Item.attribute("Anc").as_int();
  ListItemInfo->SK[0] = Item.attribute("SK1").as_int();
  ListItemInfo->SK[1] = Item.attribute("SK2").as_int();
  ListItemInfo->SK[2] = Item.attribute("SK3").as_int();
  ListItemInfo->SK[3] = Item.attribute("SK4").as_int();
  ListItemInfo->SK[4] = Item.attribute("SK5").as_int();
  ListItemInfo->SKBonus = Item.attribute("SKBonus").as_int();
  ListItemInfo->HSD = Item.attribute("HSD").as_int();
  ListItemInfo->Rate = Item.attribute("Rate").as_int();
  ListItemInfo->Star = Item.attribute("Star").as_int();
  ListItemInfo->Quantity = Item.attribute("Quantity").as_int(1);
}

void CreateVongQuayRewardItem(LPOBJ lpObj, const DATA_VONGQUAYITEM &item) {
  BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {
      item.SK[0], item.SK[1], item.SK[2], item.SK[3], item.SK[4]};
  DWORD expireTime = 0;

  if (item.HSD > 0) {
    expireTime = (DWORD)time(NULL) + item.HSD * 60;
  }

  GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y,
                   item.IndexItem, item.LvItem, item.Dur, item.Skill, item.Luck,
                   item.Opt, -1, item.Exc, item.Anc, 0, 0, ItemSocketOption,
                   item.SKBonus, expireTime);
}

bool HasVongQuayRewardInventorySpace(LPOBJ lpObj,
                                     const DATA_VONGQUAYITEM &item) {
  if (gItemManager.CheckItemInventorySpace(lpObj, item.IndexItem) != 0) {
    return true;
  }

  return false;
}

bool HasAllVongQuayRewardInventorySpace(
    LPOBJ lpObj, const std::vector<DATA_VONGQUAYITEM> &items) {
  for (std::vector<DATA_VONGQUAYITEM>::const_iterator it = items.begin();
       it != items.end(); it++) {
    if (HasVongQuayRewardInventorySpace(lpObj, *it) == false) {
      return false;
    }
  }

  return true;
}

void SendVongQuayState(int aIndex, DWORD startRoll, DWORD indexWin) {
  if (OBJECT_RANGE(aIndex) == 0) {
    return;
  }

  XULY_CGPACKET_VONGQUAY pMsg;
  pMsg.header.set(0xD3, 0x8C, sizeof(pMsg));
  pMsg.StartRoll = startRoll;
  pMsg.IndexWin = indexWin;
  DataSend(aIndex, (BYTE *)&pMsg, pMsg.header.size);
}

void SendVongQuayCancel(int aIndex) { SendVongQuayState(aIndex, 0, (DWORD)-1); }

void SendVongQuayInventoryFullNotice(int aIndex, char *message) {
  gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, message);
}

bool HasEnoughVongQuayCoin(LPOBJ lpObj, const DATA_VONGQUAY &data) {
  return (data.WC <= 0 || lpObj->Coin1 >= data.WC) &&
         (data.WP <= 0 || lpObj->Coin2 >= data.WP) &&
         (data.GP <= 0 || lpObj->Coin3 >= data.GP);
}

void SendMissingVongQuayCoinNotice(int aIndex, char *message,
                                   const char *coinName, int requiredCoin,
                                   int currentCoin) {
  gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, message, coinName,
                       requiredCoin, currentCoin);
}

void SendMissingVongQuayCoinNotices(LPOBJ lpObj, const DATA_VONGQUAY &data,
                                    char *message) {
  if (data.WC > 0 && lpObj->Coin1 < data.WC) {
    SendMissingVongQuayCoinNotice(lpObj->Index, message, "WCoinC", data.WC,
                                  lpObj->Coin1);
  }

  if (data.WP > 0 && lpObj->Coin2 < data.WP) {
    SendMissingVongQuayCoinNotice(lpObj->Index, message, "WCoinP", data.WP,
                                  lpObj->Coin2);
  }

  if (data.GP > 0 && lpObj->Coin3 < data.GP) {
    SendMissingVongQuayCoinNotice(lpObj->Index, message, "GoblinPoint", data.GP,
                                  lpObj->Coin3);
  }
}
} // namespace

CCustomVongQuay::CCustomVongQuay() { this->Init(); }

CCustomVongQuay::~CCustomVongQuay() {}

void CCustomVongQuay::Init() {
  this->Enable = 0;
  this->Firework = 0;
  this->Notice = 0;
  this->ResetTichLuyTime = 0;
}

void CCustomVongQuay::LoadFileXML(char *FilePath) {
  pugi::xml_document file;
  pugi::xml_parse_result res = file.load_file(FilePath);
  if (res.status != pugi::status_ok) {
    ErrorMessageBox("File %s load fail. Error: %s", FilePath,
                    res.description());
    return;
  }
  //--
  //--
  pugi::xml_node oCustomVongQuay = file.child("CustomVongQuay");
  this->Enable = oCustomVongQuay.attribute("Enable").as_int();
  this->Firework = oCustomVongQuay.attribute("Firework").as_int();
  this->Notice = oCustomVongQuay.attribute("Notice").as_int();
  this->ResetTichLuyTime =
      oCustomVongQuay.child("ThoiGianResetTichLuy").attribute("Time").as_int();

  if (this->ResetTichLuyTime < 0 || this->ResetTichLuyTime > 2) {
    this->ResetTichLuyTime = 0;
  }
  //= Mess Load
  this->m_MessageInfoBP.clear();
  pugi::xml_node Message = oCustomVongQuay.child("MessageInfo");
  for (pugi::xml_node msg = Message.child("Message"); msg;
       msg = msg.next_sibling()) {
    MESSAGE_INFO_VONGQUAY info;

    info.Index = msg.attribute("Index").as_int();

    strcpy_s(info.Message, msg.attribute("Text").as_string());

    this->m_MessageInfoBP.insert(
        std::pair<int, MESSAGE_INFO_VONGQUAY>(info.Index, info));
  }
  //====Load Data Moc Nap
  this->m_DataVongQuay.clear();
  pugi::xml_node ConfigVongQuay = oCustomVongQuay.child("ConfigVongQuay");
  int IndexMocNap = 1;
  for (pugi::xml_node VongQuay = ConfigVongQuay.child("VongQuay"); VongQuay;
       VongQuay = VongQuay.next_sibling()) {
    DATA_VONGQUAY infoData = {0};
    infoData.IndexVongQuay = IndexMocNap++;

    infoData.IndexItemYC = -1;
    infoData.WC = VongQuay.attribute("WC").as_int();
    infoData.WP = VongQuay.attribute("WP").as_int();
    infoData.GP = VongQuay.attribute("GP").as_int();
    if (infoData.WC < 0) {
      infoData.WC = 0;
    }
    if (infoData.WP < 0) {
      infoData.WP = 0;
    }
    if (infoData.GP < 0) {
      infoData.GP = 0;
    }
    infoData.Count = 0;
    strncpy_s(infoData.NameVongQuay, VongQuay.attribute("Name").as_string(),
              sizeof(infoData.NameVongQuay));
    //===ItemNhan
    infoData.ListItemNhan.clear();
    pugi::xml_node ItemNhan = VongQuay.child("ItemNhan");

    for (pugi::xml_node Item = ItemNhan.child("Item"); Item;
         Item = Item.next_sibling()) {
      if (infoData.ListItemNhan.size() >= kMaxVongQuayItems)
        break;

      DATA_VONGQUAYITEM ListItemInfo;
      LoadVongQuayItemFromXml(Item, &ListItemInfo);
      // LogAdd(LOG_CAM, "[VongQuay] [%d] index %d size ",
      // ListItemInfo.IndexItem, infoData.ListItemNhan.size());
      infoData.ListItemNhan.push_back(ListItemInfo);
    }
    this->m_DataVongQuay.insert(
        std::pair<int, DATA_VONGQUAY>(infoData.IndexVongQuay, infoData));
  }

  this->m_DataTichLuy.clear();
  pugi::xml_node TichLuyVongQuay = oCustomVongQuay.child("TichLuyVongQuay");

  for (pugi::xml_node Reward = TichLuyVongQuay.first_child(); Reward;
       Reward = Reward.next_sibling()) {
    if (strcmp(Reward.name(), "Reward") != 0 &&
        strcmp(Reward.name(), "Item") != 0) {
      continue;
    }

    if (this->m_DataTichLuy.size() >= kMaxTichLuyRewards) {
      break;
    }

    DATA_VONGQUAY_TICHLUY infoReward = {0};
    infoReward.RequiredSpin = Reward.attribute("RequiredSpin").as_int();

    if (infoReward.RequiredSpin <= 0) {
      infoReward.RequiredSpin = Reward.attribute("Spin").as_int();
    }

    if (infoReward.RequiredSpin <= 0) {
      continue;
    }

    LoadVongQuayItemFromXml(Reward, &infoReward.Item);
    this->m_DataTichLuy.push_back(infoReward);
  }

  LogAdd(LOG_BLUE, "[VongQuay] [%d] Size %d ", this->Enable,
         this->m_DataVongQuay.size());
}

char *CCustomVongQuay::GetMessage(int index) // OK
{
  std::map<int, MESSAGE_INFO_VONGQUAY>::iterator it =
      this->m_MessageInfoBP.find(index);

  if (it == this->m_MessageInfoBP.end()) {
    char Error[256];
    wsprintf(Error, "Could not find message %d!", index);
    return Error;
  } else {
    return it->second.Message;
  }
}

DWORD CCustomVongQuay::GetCurrentTichLuyResetKey() {
  time_t currentTime = time(NULL);
  tm currentDate;
  localtime_s(&currentDate, &currentTime);

  if (this->ResetTichLuyTime == 1) {
    currentDate.tm_hour = 0;
    currentDate.tm_min = 0;
    currentDate.tm_sec = 0;

    int daysFromMonday = (currentDate.tm_wday + 6) % 7;
    time_t mondayTime = mktime(&currentDate) - (daysFromMonday * 86400);
    tm mondayDate;
    localtime_s(&mondayDate, &mondayTime);

    return (DWORD)(((mondayDate.tm_year + 1900) * 1000) + mondayDate.tm_yday +
                   1);
  }

  if (this->ResetTichLuyTime == 2) {
    return (DWORD)(((currentDate.tm_year + 1900) * 100) + currentDate.tm_mon +
                   1);
  }

  return (DWORD)(((currentDate.tm_year + 1900) * 1000) + currentDate.tm_yday +
                 1);
}

void CCustomVongQuay::CheckTichLuyReset(int aIndex) {
  if (OBJECT_RANGE(aIndex) == 0 || gObj[aIndex].Type != OBJECT_USER ||
      gObjIsConnected(aIndex) == false) {
    return;
  }

  LPOBJ lpObj = &gObj[aIndex];
  DWORD currentResetKey = this->GetCurrentTichLuyResetKey();

  if (lpObj->MocResetTichLuyVQ == 0) {
    lpObj->MocResetTichLuyVQ = currentResetKey;
    GDVongQuayTichLuySaveSend(lpObj->Index);
    this->SendTichLuyInfo(lpObj->Index);
    return;
  }

  if (lpObj->MocResetTichLuyVQ == currentResetKey) {
    return;
  }

  lpObj->DiemTichLuyVQ = 0;
  lpObj->NhanThuongTichLuyVQ = 0;
  lpObj->MocResetTichLuyVQ = currentResetKey;
  GDVongQuayTichLuySaveSend(lpObj->Index);
  this->SendTichLuyInfo(lpObj->Index);
}

void CCustomVongQuay::UserSendClientInfo(
    int aIndex) // Send Danh Sach Moc Nap Ve Client
{

  if (gObj[aIndex].Type != OBJECT_USER) {
    return;
  }

  if (gObjIsConnected(aIndex) == false) {
    return;
  }
  // if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 ||
  // gObj[aIndex].IsFakeOnline != 0)
  if (gObj[aIndex].IsBot >= 1) {
    return;
  }
  this->CheckTichLuyReset(aIndex);
  BYTE send[4096];
  PMSG_VONGQUAY_SEND pMsg = {0};
  // ---
  pMsg.header.set(0xD3, 0x8A, 0);

  int size = sizeof(pMsg);

  pMsg.count = 0;

  for (std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.begin();
       it != this->m_DataVongQuay.end(); it++) {
    if (it == this->m_DataVongQuay.end()) {
      break;
    }
    ListVongQuaySend info;
    info.IndexVongQuay = it->second.IndexVongQuay;
    ;
    memset(info.Name, 0, sizeof(info.Name));
    memcpy(info.Name, it->second.NameVongQuay, sizeof(info.Name));
    if ((size + sizeof(info) >= 4096)) {
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
  this->SendTichLuyInfo(aIndex);
  // LogAdd(LOG_RED, "SendINfo List Vong Quay %s", gObj[aIndex].Name);
  // SendListNhanThuong(gObj[aIndex].Index, 1);
}

void VongQuay_ItemByteConvert(BYTE *lpMsg, DATA_VONGQUAYITEM *Data) // OK
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

void CCustomVongQuay::SendListNhanThuong(
    int aIndex, int VongQuaySo) // Send List SendListNhanThuong
{
  if (!this->Enable) {
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
    return;
  }

  if (gObj[aIndex].Type != OBJECT_USER) {
    return;
  }

  if (gObjIsConnected(aIndex) == false) {
    return;
  }
  // if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 ||
  // gObj[aIndex].IsFakeOnline != 0)
  if (gObj[aIndex].IsBot >= 1) {
    return;
  }
  this->CheckTichLuyReset(aIndex);
  BYTE send[4096];
  PMSG_YCVONGQUAY_SEND pMsg = {0};
  // ---
  pMsg.header.set(0xD3, 0x8B, 0);

  int size = sizeof(pMsg);

  pMsg.count = 0;

  std::map<int, DATA_VONGQUAY>::iterator it =
      this->m_DataVongQuay.find(VongQuaySo);

  if (it == this->m_DataVongQuay.end()) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); // Khong co thong tin cua moc nap
    return;
  }
  pMsg.IndexYC = -1;
  pMsg.CountItem = 0;
  pMsg.WCYC = it->second.WC;
  pMsg.WPYC = it->second.WP;
  pMsg.GPYC = it->second.GP;

  for (std::vector<DATA_VONGQUAYITEM>::iterator itItem =
           it->second.ListItemNhan.begin();
       itItem != it->second.ListItemNhan.end(); itItem++) {
    if (itItem == it->second.ListItemNhan.end()) {
      break;
    }

    LISTITEMVONGQUAY_SENDINFO info;
    info.SizeBMD = itItem->SizeBMD;
    info.PosX = itItem->PosX;
    info.PosY = itItem->PosY;
    info.Index = itItem->IndexItem;
    info.Dur = itItem->Dur;
    VongQuay_ItemByteConvert(info.Item, &*itItem);
    time_t t = time(NULL);
    localtime(&t);
    DWORD iTime = (DWORD)t + itItem->HSD * 60;
    if ((itItem->HSD) > 0) {
      info.PeriodTime = iTime;
    } else {
      info.PeriodTime = itItem->HSD;
    }
    if ((size + sizeof(info) >= 4096)) {
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
  this->SendTichLuyInfo(aIndex);
  // LogAdd(LOG_RED, "Send List Item Vong Quay %s", gObj[aIndex].Name);
}

void CCustomVongQuay::SendTichLuyInfo(int aIndex) {
  if (gObj[aIndex].Type != OBJECT_USER) {
    return;
  }

  if (gObjIsConnected(aIndex) == false) {
    return;
  }

  if (gObj[aIndex].IsBot >= 1) {
    return;
  }
  this->CheckTichLuyReset(aIndex);

  BYTE send[4096];
  PMSG_VONGQUAY_TICHLUY_SEND pMsg = {0};
  pMsg.header.set(0xD3, 0x8D, 0);

  int size = sizeof(pMsg);
  pMsg.count = 0;
  pMsg.DiemTichLuy = gObj[aIndex].DiemTichLuyVQ;
  pMsg.NhanThuongMask = gObj[aIndex].NhanThuongTichLuyVQ;
  pMsg.ResetTichLuyTime = this->ResetTichLuyTime;

  for (std::vector<DATA_VONGQUAY_TICHLUY>::iterator it =
           this->m_DataTichLuy.begin();
       it != this->m_DataTichLuy.end(); it++) {
    LISTVONGQUAY_TICHLUY_SENDINFO info = {0};
    info.RequiredSpin = it->RequiredSpin;
    info.SizeBMD = it->Item.SizeBMD;
    info.PosX = it->Item.PosX;
    info.PosY = it->Item.PosY;
    info.Index = it->Item.IndexItem;
    info.Dur = it->Item.Dur;
    VongQuay_ItemByteConvert(info.Item, &it->Item);

    if (it->Item.HSD > 0) {
      info.PeriodTime = (DWORD)time(NULL) + it->Item.HSD * 60;
    } else {
      info.PeriodTime = it->Item.HSD;
    }

    if ((size + sizeof(info) >= 4096)) {
      break;
    }

    pMsg.count++;
    memcpy(&send[size], &info, sizeof(info));
    size += sizeof(info);
  }

  pMsg.header.size[0] = SET_NUMBERHB(size);
  pMsg.header.size[1] = SET_NUMBERLB(size);
  memcpy(send, &pMsg, sizeof(pMsg));

  DataSend(aIndex, send, size);
}

void CCustomVongQuay::MakeItem(int aIndex, int type) {
  LPOBJ lpObj = &gObj[aIndex];
  this->CheckTichLuyReset(aIndex);

  if (type != 1) {
    goto Next;
  }
  if (lpObj->CusVongQuay > 0 && GetTickCount() - lpObj->CusVongQuay > 5000)
  Next: {
    lpObj->CusVongQuay = 0;

    std::map<int, DATA_VONGQUAY>::iterator it =
        this->m_DataVongQuay.find(lpObj->CusSoVongQuay);

    if (it == this->m_DataVongQuay.end() || it->second.ListItemNhan.empty()) {
      SendVongQuayCancel(lpObj->Index);
      return;
    }

    WORD iIndex = 0;

    CRandomManager RandomMng;

    for (int n = 0; n < it->second.ListItemNhan.size(); n++) {
      RandomMng.AddElement(n, it->second.ListItemNhan[n].Rate);
    }

    RandomMng.GetRandomElement(&iIndex);

    if (HasVongQuayRewardInventorySpace(
            lpObj, it->second.ListItemNhan[iIndex]) == false) {
      SendVongQuayCancel(lpObj->Index);
      SendVongQuayInventoryFullNotice(lpObj->Index, this->GetMessage(6));
      this->SendTichLuyInfo(lpObj->Index);
      return;
    }

    CreateVongQuayRewardItem(lpObj, it->second.ListItemNhan[iIndex]);
    lpObj->DiemTichLuyVQ++;
    GDVongQuayTichLuySaveSend(lpObj->Index);

    //==Send Effect
    if (this->Firework == 1) {
      GCServerCommandSend(lpObj->Index, 0, lpObj->X, lpObj->Y);
    } else if (this->Firework == 2) {
      GCServerCommandSend(lpObj->Index, 2, lpObj->X, lpObj->Y);
    } else if (this->Firework == 3) {
      GCServerCommandSend(lpObj->Index, 58, SET_NUMBERHB(lpObj->Index),
                          SET_NUMBERLB(lpObj->Index));
    }
    char tmp[255];
    char tmp2[255];
    wsprintf(tmp, this->GetMessage(5), lpObj->Name,
             gItemLevel.GetItemName(it->second.ListItemNhan[iIndex].IndexItem,
                                    it->second.ListItemNhan[iIndex].LvItem));
    if (this->Notice == 1) { // Thong Bao trong Sub
      gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, tmp);
    } else if (this->Notice == 2) { // Thong Bao Toan Sub

      GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0,
                         0, tmp);
    } else if (this->Notice == 3) { // Thong Bao Toan Sub
      wsprintf(tmp2, "%s %s", gServerInfo.m_ServerName, tmp);
      GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0,
                         0, tmp2);
    }

    SendVongQuayState(lpObj->Index, 0, iIndex);
    this->SendTichLuyInfo(lpObj->Index);
  }
}

void CCustomVongQuay::ClaimTichLuyReward(int aIndex, int rewardIndex) {
  if (!this->Enable) {
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0));
    return;
  }

  if (gObj[aIndex].Type != OBJECT_USER) {
    return;
  }

  if (gObjIsConnected(aIndex) == false) {
    return;
  }

  if (gObj[aIndex].IsBot >= 1) {
    return;
  }

  if (rewardIndex < 0 || rewardIndex >= (int)this->m_DataTichLuy.size() ||
      rewardIndex >= (int)kMaxTichLuyRewards) {
    return;
  }

  LPOBJ lpObj = &gObj[aIndex];
  this->CheckTichLuyReset(aIndex);
  DWORD claimBit = (1u << rewardIndex);
  DATA_VONGQUAY_TICHLUY &reward = this->m_DataTichLuy[rewardIndex];

  if ((lpObj->NhanThuongTichLuyVQ & claimBit) != 0) {
    gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(9));
    return;
  }

  if (lpObj->DiemTichLuyVQ < (DWORD)reward.RequiredSpin) {
    gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(10));
    return;
  }

  if (HasVongQuayRewardInventorySpace(lpObj, reward.Item) == false) {
    SendVongQuayInventoryFullNotice(lpObj->Index, this->GetMessage(6));
    return;
  }

  CreateVongQuayRewardItem(lpObj, reward.Item);
  lpObj->NhanThuongTichLuyVQ |= claimBit;
  GDVongQuayTichLuySaveSend(lpObj->Index);
  this->SendTichLuyInfo(lpObj->Index);
  gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(11));
}

void CCustomVongQuay::ActionVongQuay(int aIndex, int MocNap, int solan) {
  if (!this->Enable) {
    SendVongQuayCancel(aIndex);
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
    return;
  }

  if (gObj[aIndex].Type != OBJECT_USER) {
    return;
  }

  if (gObjIsConnected(aIndex) == false) {
    return;
  }
  // if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 ||
  // gObj[aIndex].IsFakeOnline != 0)
  if (gObj[aIndex].IsBot >= 1) {
    return;
  }
  LPOBJ lpObj = &gObj[aIndex];
  this->CheckTichLuyReset(aIndex);

  if (solan < 1) {
    solan = 1;
  }

  // if ((GetTickCount() - lpObj->ClickClientSend) < 2000)
  //{
  //	gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,
  // this->GetMessage(8)); 	return;
  // }

  //===KIem tra trang thai co duoc add item khong
  if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 ||
      lpObj->Transaction == 1) {
    SendVongQuayCancel(aIndex);
    return;
  }

  if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj)) {
    SendVongQuayCancel(aIndex);
    return;
  }
  //===================================================

  //===Lay Thong Tin Moc Nap
  std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.find(MocNap);
  lpObj->CusSoVongQuay = MocNap;
  if (it == this->m_DataVongQuay.end()) {
    SendVongQuayCancel(aIndex);
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); // Khong co thong tin cua moc nap
    return;
  }

  if (!HasEnoughVongQuayCoin(lpObj, it->second)) {
    SendVongQuayCancel(aIndex);
    SendMissingVongQuayCoinNotices(lpObj, it->second, this->GetMessage(8));
    return;
  }

  //===Kiem tra thung do
  if (HasAllVongQuayRewardInventorySpace(lpObj, it->second.ListItemNhan) ==
      false) {
    SendVongQuayCancel(aIndex);
    SendVongQuayInventoryFullNotice(lpObj->Index, this->GetMessage(6));
    return;
  }

  //===============Cong Coin
  if (it->second.WC > 0 || it->second.WP > 0 || it->second.GP > 0) {
    GDSetCoinSend(lpObj->Index, -it->second.WC, -it->second.WP, -it->second.GP,
                  "Tru Coin Vong Quay");
  }

  SendVongQuayState(lpObj->Index, 1, (DWORD)-1);
  if (solan == 1) {
    lpObj->CusVongQuay = GetTickCount();
  } else {
    this->MakeItem(lpObj->Index, 2);
  }
  // lpObj->ClickClientSend = GetTickCount();
}
