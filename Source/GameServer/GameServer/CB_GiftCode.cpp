
#include "StdAfx.h"
#include "CB_GiftCode.h"
#include "CashShop.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "Guild.h"
#include "ItemBagManager.h"
#include "ItemOptionRate.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Monster.h"
#include "Move.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "Party.h"
#include "ServerInfo.h"
#include "Util.h"

#if (CB_CUSTOMGIFTCODE)
CB_GiftCode gCB_GiftCode;

CB_GiftCode::CB_GiftCode() {}

CB_GiftCode::~CB_GiftCode() {}
void CB_GiftCode::LoadFileXML(char *FilePath) {
  pugi::xml_document file;
  pugi::xml_parse_result res = file.load_file(FilePath);
  if (res.status != pugi::status_ok) {
    ErrorMessageBox("File %s load fail. Error: %s", FilePath,
                    res.description());
    return;
  }
  pugi::xml_node oCustomGiftCode = file.child("CustomGiftCode");
  this->Enable = oCustomGiftCode.attribute("Enable").as_int();
  this->Firework = oCustomGiftCode.attribute("Firework").as_int();
  this->Notice = oCustomGiftCode.attribute("Notice").as_int();
  //= Mess Load
  this->m_MessageInfoBP.clear();
  pugi::xml_node Message = oCustomGiftCode.child("MessageInfo");
  for (pugi::xml_node msg = Message.child("Message"); msg;
       msg = msg.next_sibling()) {
    MESSAGE_INFO info;

    info.Index = msg.attribute("Index").as_int();

    strcpy_s(info.Message, msg.attribute("Text").as_string());

    this->m_MessageInfoBP.insert(
        std::pair<int, MESSAGE_INFO>(info.Index, info));
  }

  //====Load Data
  this->m_DataGiftCode.clear();
  pugi::xml_node ConfigDataGiftCode =
      oCustomGiftCode.child("ConfigDataGiftCode");
  int IndexMocQua = 1;
  for (pugi::xml_node Gift = ConfigDataGiftCode.child("Gift"); Gift;
       Gift = Gift.next_sibling()) {
    DATA_LIST infoData = {0};
    infoData.IndexMoc = IndexMocQua++;

    strncpy_s(infoData.GiftCode, sizeof(infoData.GiftCode),
              Gift.attribute("GiftCode").as_string(),
              sizeof(infoData.GiftCode));
    std::string nme(infoData.GiftCode);
    std::transform(nme.begin(), nme.end(), nme.begin(), tolower);

    //==Coin Nhan
    pugi::xml_node CoinNhan = Gift.child("CoinNhan");
    infoData.WC = CoinNhan.attribute("WC").as_int();
    infoData.WP = CoinNhan.attribute("WP").as_int();
    infoData.GP = CoinNhan.attribute("GP").as_int();
    // infoData.Ruud = CoinNhan.attribute("Ruud").as_int();
    //===ItemNhan
    infoData.ListItemNhan.clear();
    pugi::xml_node ItemNhan = Gift.child("ItemNhan");
    for (pugi::xml_node Item = ItemNhan.child("Item"); Item;
         Item = Item.next_sibling()) {
      DATA_CBITEM ListItemInfo;
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

    //==List Account
    infoData.ListAccount.clear();
    pugi::xml_node AccountCheck = Gift.child("AccountCheck");
    for (pugi::xml_node ListAcc = AccountCheck.child("List"); ListAcc;
         ListAcc = ListAcc.next_sibling()) {
      char AccountGet[11] = {0};
      strncpy_s(AccountGet, sizeof(AccountGet),
                ListAcc.attribute("Account").as_string(), sizeof(AccountGet));
      infoData.ListAccount.push_back(AccountGet);
    }
    LogAdd(LOG_RED, "%s / Item %d / Account %d", infoData.GiftCode,
           infoData.ListItemNhan.size(), infoData.ListAccount.size());
    this->m_DataGiftCode.insert(
        std::pair<std::string, DATA_LIST>(nme, infoData));
  }
  LogAdd(LOG_BLUE, "[LoadGiftCode] [%d] DataGiftCode %d", this->Enable,
         this->m_DataGiftCode.size());
}

void CB_GiftCode::ItemByteConvert(BYTE *lpMsg, DATA_CBITEM *Data) // OK
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

char *CB_GiftCode::GetMessage(int index) // OK
{
  std::map<int, MESSAGE_INFO>::iterator it = this->m_MessageInfoBP.find(index);

  if (it == this->m_MessageInfoBP.end()) {
    static char Error[256];
    wsprintf(Error, "Could not find message %d!", index);
    return Error;
  } else {
    return it->second.Message;
  }
}

void CB_GiftCode::SendListNhanThuong(
    int aIndex, const std::string &GiftCodeText) // Send List SendListNhanThuong
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
  /*if ( gObj[aIndex].IsFakeOnline != 0)
  {
          return;
  }*/
  BYTE send[4096];
  PMSG_CBLISTTHUONG_SEND pMsg = {0};
  // ---
  pMsg.header.set(0xD3, 0x4F, 0);

  int size = sizeof(pMsg);

  pMsg.count = 0;

  std::map<std::string, DATA_LIST>::iterator it =
      this->m_DataGiftCode.find(GiftCodeText);

  if (it == this->m_DataGiftCode.end()) {
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); // Khong co thong tin
    return;
  }

  pMsg.WC = it->second.WC;
  pMsg.WP = it->second.WP;
  pMsg.GP = it->second.GP;
  // pMsg.Ruud = it->second.Ruud;

  for (std::vector<DATA_CBITEM>::iterator itItem =
           it->second.ListItemNhan.begin();
       itItem != it->second.ListItemNhan.end(); itItem++) {
    if (itItem == it->second.ListItemNhan.end()) {
      break;
    }

    if (itItem->Class[gObj[aIndex].Class] ==
        0) // Neu khong phai class duoc active thi bo qua
    {
      continue;
    }
    LISTITEM_SENDINFO info;
    info.SizeBMD = itItem->SizeBMD;
    info.Count = itItem->Count;
    info.Index = itItem->IndexItem;
    info.Dur = itItem->Dur;
    CB_GiftCode::ItemByteConvert(info.Item, &*itItem);
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

  if (pMsg.count > 0) {
    pMsg.header.size[0] = SET_NUMBERHB(size);
    pMsg.header.size[1] = SET_NUMBERLB(size);
    // ---
    memcpy(send, &pMsg, sizeof(pMsg));

    DataSend(aIndex, send, size);
  }
  LogAdd(LOG_RED, "Send List Phan Thuong  %s Code %s", gObj[aIndex].Name,
         GiftCodeText.c_str());
}
void CB_GiftCode::DSRecvProtocol(BYTE *aRecv) {
  if (!aRecv)
    return;

  SEND_DS_GETSTATUS *lpMsg = (SEND_DS_GETSTATUS *)aRecv;
  // LogAdd(LOG_RED, "DSRecvProtocol %d", lpMsg->index);
  if (!OBJECT_USER_RANGE(lpMsg->index)) {
    return;
  }

  LPOBJ lpObj = &gObj[lpMsg->index];

  if (lpMsg->Status == 0) {
    gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    return;
  } else {

    if (lpMsg->Status == 2) {
      // this->SendListNhanThuong(lpObj->Index, lpMsg->giftcode);
      if (lpObj->CacheGiftCode[0] != '\0') {
        //===KIem tra Code GiftCode
        std::map<std::string, DATA_LIST>::iterator it =
            this->m_DataGiftCode.find(lpObj->CacheGiftCode);

        if (it == this->m_DataGiftCode.end()) {
          gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,
                               this->GetMessage(1)); // Khong co thong tin
          return;
        }
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
        wsprintf(tmp, this->GetMessage(5), lpObj->Name);
        if (this->Notice == 1) { // Thong Bao trong Sub
          gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, tmp);
        } else if (this->Notice == 2) { // Thong Bao Toan Sub

          GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0,
                             0, 0, tmp);
        } else if (this->Notice == 3) { // Thong Bao Toan Sub
          wsprintf(tmp2, "%s %s", gServerInfo.m_ServerName, tmp);
          GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0,
                             0, 0, tmp2);
        }
        //===============Cong Coin
        if (it->second.WC > 0 || it->second.WP > 0 || it->second.GP > 0) {
          GDSetCoinSend(lpObj->Index, it->second.WC, it->second.WP,
                        it->second.GP, "CoinGiftCode");
        }
        /*
        if (it->second.Ruud > 0) {
          gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, 0, 0, 0,
                                               it->second.Ruud);
          //-- Ruud Update
          gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,
                               "[Ruud] Add Coin %d", it->second.Ruud);
        }
        */
        //=====
        //================ Add Item
        if (it->second.ListItemNhan.empty() == 0) {
          for (int n = 0; n < it->second.ListItemNhan.size(); n++) {
            if (it->second.ListItemNhan[n].Class[lpObj->Class] ==
                0) // Neu khong phai class duoc active thi bo qua
            {
              continue;
            }
            for (int count = 0; count < it->second.ListItemNhan[n].Count;
                 count++) {
              BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {
                  it->second.ListItemNhan[n].SK[0],
                  it->second.ListItemNhan[n].SK[1],
                  it->second.ListItemNhan[n].SK[2],
                  it->second.ListItemNhan[n].SK[3],
                  it->second.ListItemNhan[n].SK[4]};
              time_t t = time(NULL);
              localtime(&t);
              DWORD iTime = (DWORD)t + it->second.ListItemNhan[n].HSD * 60;
              if (it->second.ListItemNhan[n].HSD > 0) {
                GDCreateItemSend(
                    lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y,
                    it->second.ListItemNhan[n].IndexItem,
                    it->second.ListItemNhan[n].LvItem,
                    it->second.ListItemNhan[n].Dur,
                    it->second.ListItemNhan[n].Skill,
                    it->second.ListItemNhan[n].Luck,
                    it->second.ListItemNhan[n].Opt, -1,
                    it->second.ListItemNhan[n].Exc,
                    it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption,
                    it->second.ListItemNhan[n].SKBonus, iTime);
              } else {
                GDCreateItemSend(
                    lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y,
                    it->second.ListItemNhan[n].IndexItem,
                    it->second.ListItemNhan[n].LvItem,
                    it->second.ListItemNhan[n].Dur,
                    it->second.ListItemNhan[n].Skill,
                    it->second.ListItemNhan[n].Luck,
                    it->second.ListItemNhan[n].Opt, -1,
                    it->second.ListItemNhan[n].Exc,
                    it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption,
                    it->second.ListItemNhan[n].SKBonus, 0);
              }
            }
          }
        }
        memset(lpObj->CacheGiftCode, 0, sizeof(lpObj->CacheGiftCode));
      }
    }
  }
}

void CB_GiftCode::RecvProtocol(BYTE *aRecv, int aIndex) {
  if (!aRecv)
    return;

  if (!OBJECT_USER_RANGE(aIndex)) {
    return;
  }

  CGPACKET_SENDRECV *lpMsg = (CGPACKET_SENDRECV *)aRecv;

  LogAdd(LOG_RED, "[GiftCode] Player %s send Packet. ThaoTac: %d, Code: %s",
         gObj[aIndex].Name, lpMsg->ThaoTac, lpMsg->GiftCode);

  if (this->Enable == 0) {
    LPOBJ lpObj = &gObj[aIndex];
    memset(lpObj->CacheGiftCode, 0, sizeof(lpObj->CacheGiftCode));
    gNotice.GCNoticeSend(
        aIndex, 1, 0, 0, 0, 0, 0,
        this->GetMessage(0)); // check xem co bat tinh nang khong
    LogAdd(LOG_RED, "[GiftCode] Server XML config Enable = 0 -> Reject!");
    return;
  }

  LPOBJ lpObj = &gObj[aIndex];
  memset(lpObj->CacheGiftCode, 0, sizeof(lpObj->CacheGiftCode));

  LogAdd(LOG_RED, "RecvProtocol %d  [%s]", lpMsg->ThaoTac, lpMsg->GiftCode);

  //===KIem tra Code GiftCode
  std::string StringGiftCode(lpMsg->GiftCode);
  std::transform(StringGiftCode.begin(), StringGiftCode.end(),
                 StringGiftCode.begin(), tolower);

  std::map<std::string, DATA_LIST>::iterator it =
      this->m_DataGiftCode.find(StringGiftCode);

  if (it == this->m_DataGiftCode.end()) {
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); // Khong co thong tin
    return;
  }
  std::string StringAcc(lpObj->Account);
  std::transform(StringAcc.begin(), StringAcc.end(), StringAcc.begin(),
                 tolower);

  if (!it->second.ListAccount.empty()) {
    bool CheckAccount = false;
    for (int i = 0; i < it->second.ListAccount.size(); i++) {
      if (areEqual(it->second.ListAccount[i].c_str(), StringAcc.c_str())) {
        CheckAccount = true;
        break;
      }
    }
    if (!CheckAccount) {
      gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(3));
      return;
    }
  }

  switch (lpMsg->ThaoTac) {
  case CB_GiftCode::eGetInfoCode: {
    //===Check Status
    SEND_DS_GETSTATUS pMsg;

    pMsg.h.set(0xD3, 0x6A, sizeof(pMsg));

    pMsg.index = aIndex;

    pMsg.Status = 0; // Get Ko Save

    pMsg.IsAccountCheck = (!it->second.ListAccount.empty()) ? 1 : 0;

    memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

    memcpy(pMsg.name, lpObj->Name, sizeof(pMsg.name));

    memcpy(pMsg.giftcode, strdup(StringGiftCode.c_str()),
           sizeof(pMsg.giftcode));

    gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);
  } break;
  case CB_GiftCode::eAccepGift: {
    //===Kiem tra thung do
    if (gItemManager.CheckItemInventorySpace(lpObj, 4, 4) == 0) {
      gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(6));
      return;
    }

    //===Save
    SEND_DS_GETSTATUS pMsg;

    pMsg.h.set(0xD3, 0x6A, sizeof(pMsg));

    pMsg.index = aIndex;

    pMsg.Status = 1; // Save

    pMsg.IsAccountCheck = (!it->second.ListAccount.empty()) ? 1 : 0;

    memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

    memcpy(pMsg.name, lpObj->Name, sizeof(pMsg.name));

    memset(pMsg.giftcode, 0, sizeof(pMsg.giftcode));
    strcpy_s(pMsg.giftcode, sizeof(pMsg.giftcode), StringGiftCode.c_str());

    strcpy_s(lpObj->CacheGiftCode, sizeof(lpObj->CacheGiftCode),
             StringGiftCode.c_str());

    LogAdd(LOG_RED, "[%s][%s] eAccepGift [%s] ~  [%s]", lpObj->Account,
           lpObj->Name, pMsg.giftcode, lpObj->CacheGiftCode);

    gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);

  } break;
  default:
    break;
  }
}
#endif