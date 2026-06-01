
#include "stdafx.h"
#include "CustomChoTroi.h"
#include "CSProtocol.h"
#include "CashShop.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "GMHolyItem.h"
#include "ItemLevel.h"
#include "ItemManager.h"
#include "Log.h"
#include "MapServerManager.h"
#include "MasterSkillTree.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "Trade.h"
#include "Util.h"

#if (CUSTOM_CHOTROI)
BCustomChoTroi gBCustomChoTroi;
char *TypeCoin[7] = {"NULL", "WC", "WP", "GP", "B", "S", "C"};

static int ChoTroiParseCurrencyKind(const char *kind) {
  if (kind == 0) {
    return 0;
  }

  if (_stricmp(kind, "Coin") == 0) {
    return eChoTroiCurrencyKindCoin;
  }

  if (_stricmp(kind, "ItemBank") == 0) {
    return eChoTroiCurrencyKindItemBank;
  }

  if (_stricmp(kind, "Item") == 0) {
    return eChoTroiCurrencyKindItem;
  }

  if (_stricmp(kind, "Zen") == 0) {
    return eChoTroiCurrencyKindZen;
  }

  return 0;
}

static int ChoTroiParseCoinType(const char *coinType) {
  if (coinType == 0) {
    return eChoTroiCoinTypeNone;
  }

  if (_stricmp(coinType, "WC") == 0) {
    return eChoTroiCoinTypeWC;
  }

  if (_stricmp(coinType, "WP") == 0) {
    return eChoTroiCoinTypeWP;
  }

  if (_stricmp(coinType, "GP") == 0) {
    return eChoTroiCoinTypeGP;
  }

  return eChoTroiCoinTypeNone;
}

static void ChoTroiSendClientCacheState(int aIndex, DWORD thaoTac) {
  if (!OBJMAX_RANGE(aIndex)) {
    return;
  }

  XULY_CGPACKET pMsg;
  pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
  pMsg.ThaoTac = thaoTac;
  DataSend(aIndex, (BYTE *)&pMsg, pMsg.header.size);
}

static LPOBJ ChoTroiFindOnlineSeller(const char *name) {
  if (name == 0 || name[0] == 0) {
    return 0;
  }

  char sellerName[MARKET_NAME_LEN] = {0};
  memcpy(sellerName, name, MARKET_NAME_LEN - 1);

  return gObjFind(sellerName);
}

// === Helper: Count jewel in inventory by item index ===
static int ChoTroi_CountJewel(int aIndex, int itemIndex) {
  LPOBJ lpObj = &gObj[aIndex];
  int count = 0;
  for (int n = INVENTORY_WEAR_SIZE; n < INVENTORY_FULL_SIZE; n++) {
    if (lpObj->Inventory[n].IsItem() &&
        lpObj->Inventory[n].m_Index == itemIndex) {
      count++;
    }
  }
  return count;
}

// === Helper: Remove jewel from inventory ===
static void ChoTroi_RemoveJewel(int aIndex, int itemIndex, int removeCount) {
  int removed = 0;
  for (int n = INVENTORY_WEAR_SIZE;
       n < INVENTORY_FULL_SIZE && removed < removeCount; n++) {
    if (gObj[aIndex].Inventory[n].IsItem() &&
        gObj[aIndex].Inventory[n].m_Index == itemIndex) {
      gItemManager.InventoryDelItem(aIndex, n);
      gItemManager.GCItemDeleteSend(aIndex, n, 1);
      removed++;
    }
  }
}

// === Helper: Add jewel to inventory ===
static void ChoTroi_AddJewel(int aIndex, int itemIndex, int addCount) {
  for (int i = 0; i < addCount; i++) {
    CItem item;
    item.m_Index = itemIndex;
    item.m_Level = 0;
    item.m_Option1 = 0;
    item.m_Option2 = 0;
    item.m_Option3 = 0;
    item.m_NewOption = 0;
    item.m_Durability = 1;
    BYTE slot = gItemManager.InventoryInsertItem(aIndex, item);
    if (slot != 0xFF) {
      gItemManager.GCItemModifySend(aIndex, slot);
    }
  }
}

static int ChoTroi_FindHolyBankSlot(int aIndex, int itemIndex) {
#ifdef INVENTORY_HOLY_JOH
  if (!OBJECT_RANGE(aIndex) || itemIndex < 0) {
    return -1;
  }

  LPOBJ lpObj = &gObj[aIndex];
  if (lpObj->LoadHolyInventory == 0) {
    return -1;
  }

  for (int n = 0; n < HOLY_INVENTORY_SIZE; n++) {
    if (lpObj->HolyInventory[n].bItemindex == itemIndex) {
      return n;
    }
  }
#endif
  return -1;
}

static __int64 ChoTroi_CountHolyBankItem(int aIndex, int itemIndex) {
#ifdef INVENTORY_HOLY_JOH
  int slot = ChoTroi_FindHolyBankSlot(aIndex, itemIndex);
  if (slot < 0) {
    return 0;
  }

  return gObj[aIndex].HolyInventory[slot].bItemCount;
#else
  return 0;
#endif
}

static bool ChoTroi_AddHolyBankItem(int aIndex, int itemIndex, int addCount) {
#ifdef INVENTORY_HOLY_JOH
  if (addCount <= 0) {
    return false;
  }

  int slot = ChoTroi_FindHolyBankSlot(aIndex, itemIndex);
  if (slot < 0) {
    return false;
  }

  gObj[aIndex].HolyInventory[slot].bItemCount += addCount;
  GMHolyItem->GCInventoryListSend(aIndex);
  GMHolyItem->DGHolyInventorySave(aIndex);
  return true;
#else
  return false;
#endif
}

static bool ChoTroi_RemoveHolyBankItem(int aIndex, int itemIndex,
                                       int removeCount) {
#ifdef INVENTORY_HOLY_JOH
  if (removeCount <= 0) {
    return false;
  }

  int slot = ChoTroi_FindHolyBankSlot(aIndex, itemIndex);
  if (slot < 0) {
    return false;
  }

  if (gObj[aIndex].HolyInventory[slot].bItemCount < removeCount) {
    return false;
  }

  gObj[aIndex].HolyInventory[slot].bItemCount -= removeCount;
  GMHolyItem->GCInventoryListSend(aIndex);
  GMHolyItem->DGHolyInventorySave(aIndex);
  return true;
#else
  return false;
#endif
}

static __int64 ChoTroi_GetCurrencyValue(int aIndex,
                                        const CHOTROI_CURRENCY_INFO *currency) {
  if (!OBJECT_RANGE(aIndex) || currency == 0) {
    return 0;
  }

  LPOBJ lpObj = &gObj[aIndex];

  switch (currency->Kind) {
  case eChoTroiCurrencyKindCoin:
    if (currency->CoinType == eChoTroiCoinTypeWC) {
      return lpObj->Coin1;
    }
    if (currency->CoinType == eChoTroiCoinTypeWP) {
      return lpObj->Coin2;
    }
    if (currency->CoinType == eChoTroiCoinTypeGP) {
      return lpObj->Coin3;
    }
    break;
  case eChoTroiCurrencyKindItemBank:
    return ChoTroi_CountHolyBankItem(aIndex, currency->ItemIndex);
  case eChoTroiCurrencyKindItem:
    return ChoTroi_CountJewel(aIndex, currency->ItemIndex);
  case eChoTroiCurrencyKindZen:
    return lpObj->Money;
  default:
    break;
  }

  return 0;
}

static bool ChoTroi_DebitCurrency(int aIndex,
                                  const CHOTROI_CURRENCY_INFO *currency,
                                  int value) {
  if (!OBJECT_RANGE(aIndex) || currency == 0 || value <= 0) {
    return false;
  }

  LPOBJ lpObj = &gObj[aIndex];

  switch (currency->Kind) {
  case eChoTroiCurrencyKindCoin:
    if (currency->CoinType == eChoTroiCoinTypeWC) {
      GDSetCoinSend(aIndex, -value, 0, 0, "ChoTroi - WCoinC");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    if (currency->CoinType == eChoTroiCoinTypeWP) {
      GDSetCoinSend(aIndex, 0, -value, 0, "ChoTroi - WCoinP");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    if (currency->CoinType == eChoTroiCoinTypeGP) {
      GDSetCoinSend(aIndex, 0, 0, -value, "ChoTroi - GobinP");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    break;
  case eChoTroiCurrencyKindItemBank:
    return ChoTroi_RemoveHolyBankItem(aIndex, currency->ItemIndex, value);
  case eChoTroiCurrencyKindItem:
    ChoTroi_RemoveJewel(aIndex, currency->ItemIndex, value);
    return true;
  case eChoTroiCurrencyKindZen:
    if (lpObj->Money < (DWORD)value) {
      return false;
    }
    lpObj->Money -= value;
    GCMoneySend(aIndex, lpObj->Money);
    return true;
  default:
    break;
  }

  return false;
}

static bool ChoTroi_CreditCurrency(int aIndex,
                                   const CHOTROI_CURRENCY_INFO *currency,
                                   int value) {
  if (!OBJECT_RANGE(aIndex) || currency == 0 || value <= 0) {
    return false;
  }

  LPOBJ lpObj = &gObj[aIndex];

  switch (currency->Kind) {
  case eChoTroiCurrencyKindCoin:
    if (currency->CoinType == eChoTroiCoinTypeWC) {
      GDSetCoinSend(aIndex, value, 0, 0, "ChoTroi + WCoinC");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    if (currency->CoinType == eChoTroiCoinTypeWP) {
      GDSetCoinSend(aIndex, 0, value, 0, "ChoTroi + WCoinP");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    if (currency->CoinType == eChoTroiCoinTypeGP) {
      GDSetCoinSend(aIndex, 0, 0, value, "ChoTroi + GobinP");
      gCashShop.CGCashShopPointRecv(aIndex);
      return true;
    }
    break;
  case eChoTroiCurrencyKindItemBank:
    return ChoTroi_AddHolyBankItem(aIndex, currency->ItemIndex, value);
  case eChoTroiCurrencyKindItem:
    ChoTroi_AddJewel(aIndex, currency->ItemIndex, value);
    return true;
  case eChoTroiCurrencyKindZen:
    lpObj->Money += value;
    GCMoneySend(aIndex, lpObj->Money);
    return true;
  default:
    break;
  }

  return false;
}

void BCustomChoTroi::Load(char *FilePath) {
  this->m_DataVAT.clear();
  this->m_DataItemBlock.clear();
  this->Enable = 0;
  this->TypeHSD = 0;

  this->OnCointType = 0;
  this->m_CurrencyInfo.clear();
  this->m_CurrencyOrder.clear();
  pugi::xml_document file;
  pugi::xml_parse_result res = file.load_file(FilePath);
  if (res.status != pugi::status_ok) {
    ErrorMessageBox("File %s load fail. Error: %s", FilePath,
                    res.description());
    return;
  }
  pugi::xml_node oCustomChoTroi = file.child("CustomChoTroi");
  this->Enable = oCustomChoTroi.attribute("Enable").as_int();
  this->TypeHSD = oCustomChoTroi.attribute("TypeHSD").as_int();

  pugi::xml_node oCurrencyList = oCustomChoTroi.child("CurrencyList");
  for (pugi::xml_node currency = oCurrencyList.child("Currency"); currency;
       currency = currency.next_sibling("Currency")) {
    CHOTROI_CURRENCY_INFO info;

    info.Id = currency.attribute("Id").as_int();
    info.Enable = currency.attribute("Enable").as_int();
    strncpy_s(info.Name, currency.attribute("Name").as_string(),
              sizeof(info.Name));
    info.Kind =
        ChoTroiParseCurrencyKind(currency.attribute("Kind").as_string());
    info.CoinType =
        ChoTroiParseCoinType(currency.attribute("CoinType").as_string());
    info.ItemIndex = currency.attribute("ItemIndex").as_int(-1);
    info.TaxRate = currency.attribute("TaxRate").as_int(10);

    for (pugi::xml_node tax = currency.child("Tax"); tax;
         tax = tax.next_sibling("Tax")) {
      info.TaxRateByTypeItem[tax.attribute("TypeItem").as_int()] =
          tax.attribute("TaxRate").as_int(info.TaxRate);
    }

    if (info.Id <= 0 || info.Id > 31 || info.Name[0] == 0 ||
        info.Kind == 0) {
      LogAdd(LOG_RED,
             "[CustomChoTroi] Skip invalid currency Id=%d Name=%s Kind=%d",
             info.Id, info.Name, info.Kind);
      continue;
    }

    this->m_CurrencyInfo[info.Id] = info;

    if (info.Enable) {
      this->m_CurrencyOrder.push_back(info.Id);
      this->OnCointType |= this->GetCurrencyBit(info.Id);
    }
  }

  if (this->m_CurrencyInfo.empty()) {
    struct LEGACY_CURRENCY {
      int Id;
      const char *Name;
      int Kind;
      int CoinType;
      int ItemIndex;
      const char *FlagName;
    };

    LEGACY_CURRENCY legacy[] = {
        {eMarketPriceWC, "WC", eChoTroiCurrencyKindCoin, eChoTroiCoinTypeWC,
         -1, "OnWC"},
        {eMarketPriceWP, "WP", eChoTroiCurrencyKindCoin, eChoTroiCoinTypeWP,
         -1, "OnWP"},
        {eMarketPriceGP, "GP", eChoTroiCurrencyKindCoin, eChoTroiCoinTypeGP,
         -1, "OnGP"},
        {eMarketPriceB, "Bless", eChoTroiCurrencyKindItemBank,
         eChoTroiCoinTypeNone, 7181, "OnB"},
        {eMarketPriceS, "Soul", eChoTroiCurrencyKindItemBank,
         eChoTroiCoinTypeNone, 7182, "OnS"},
        {eMarketPriceC, "Chaos", eChoTroiCurrencyKindItemBank,
         eChoTroiCoinTypeNone, 6159, "OnC"},
    };

    for (int n = 0; n < (int)(sizeof(legacy) / sizeof(legacy[0])); n++) {
      CHOTROI_CURRENCY_INFO info;
      info.Id = legacy[n].Id;
      info.Enable = oCustomChoTroi.attribute(legacy[n].FlagName).as_int();
      strncpy_s(info.Name, legacy[n].Name, sizeof(info.Name));
      info.Kind = legacy[n].Kind;
      info.CoinType = legacy[n].CoinType;
      info.ItemIndex = legacy[n].ItemIndex;
      info.TaxRate = 10;
      this->m_CurrencyInfo[info.Id] = info;

      if (info.Enable) {
        this->m_CurrencyOrder.push_back(info.Id);
        this->OnCointType |= this->GetCurrencyBit(info.Id);
      }
    }
  }

  this->m_MessageInfoBP.clear();
  pugi::xml_node Message = oCustomChoTroi.child("MessageInfo");
  for (pugi::xml_node msg = Message.child("Message"); msg;
       msg = msg.next_sibling()) {
    CUSTOM_CHOTROI_MESSAGE info;

    info.Index = msg.attribute("Index").as_int();

    strncpy_s(info.Message, msg.attribute("Text").as_string(),
              sizeof(info.Message));

    this->m_MessageInfoBP.insert(
        std::pair<int, CUSTOM_CHOTROI_MESSAGE>(info.Index, info));
  }

  //=== LOad COnfig VAT
  pugi::xml_node oConfigVAT = oCustomChoTroi.child("ConfigVAT");
  for (pugi::xml_node ConfigVAT = oConfigVAT.child("Type"); ConfigVAT;
       ConfigVAT = ConfigVAT.next_sibling()) {
    VAT_DATA_INFO info = {0};
    info.TypeItem = ConfigVAT.attribute("TypeItem").as_int();
    info.WCRate = ConfigVAT.attribute("WCRate").as_int();
    info.WPRate = ConfigVAT.attribute("WPRate").as_int();
    info.GPRate = ConfigVAT.attribute("GPRate").as_int();
    info.BRate = ConfigVAT.attribute("BRate").as_int();
    info.SRate = ConfigVAT.attribute("SRate").as_int();
    info.CRate = ConfigVAT.attribute("CRate").as_int();
    this->m_DataVAT.insert(std::pair<int, VAT_DATA_INFO>(info.TypeItem, info));
  }
  //===== Load COnfig Item Block
  pugi::xml_node oConfigItemBlock = oCustomChoTroi.child("ConfigItemBlock");
  for (pugi::xml_node ConfigItemBlock = oConfigItemBlock.child("Item");
       ConfigItemBlock; ConfigItemBlock = ConfigItemBlock.next_sibling()) {
    DATA_ITEMBLOCK info = {0};
    info.IndexMin = ConfigItemBlock.attribute("IndexMin").as_int();
    info.IndexMax = ConfigItemBlock.attribute("IndexMax").as_int();
    info.LvMin = ConfigItemBlock.attribute("LvMin").as_int();
    info.LvMax = ConfigItemBlock.attribute("LvMax").as_int();
    info.Luck = ConfigItemBlock.attribute("Luck").as_int();
    info.Skill = ConfigItemBlock.attribute("Skill").as_int();
    info.Opt = ConfigItemBlock.attribute("Opt").as_int();
    info.ExcOpt = ConfigItemBlock.attribute("ExcOpt").as_int();
    this->m_DataItemBlock.push_back(info);
  }
  LogAdd(LOG_BLUE, "[CustomChoTroi] Load VAT %d, Load ItemBlock %d",
         this->m_DataVAT.size(), this->m_DataItemBlock.size());
}

int BCustomChoTroi::GetCurrencyBit(int CurrencyId) {
  if (CurrencyId <= 0 || CurrencyId > 31) {
    return 0;
  }

  return (1 << (CurrencyId - 1));
}

CHOTROI_CURRENCY_INFO *BCustomChoTroi::GetCurrencyInfo(int CurrencyId) {
  std::map<int, CHOTROI_CURRENCY_INFO>::iterator it =
      this->m_CurrencyInfo.find(CurrencyId);

  if (it == this->m_CurrencyInfo.end()) {
    return 0;
  }

  return &it->second;
}

bool BCustomChoTroi::IsCurrencyEnabled(int CurrencyId) {
  CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(CurrencyId);
  if (currency == 0 || currency->Enable == 0) {
    return false;
  }

  return ((this->OnCointType & this->GetCurrencyBit(CurrencyId)) != 0);
}

const char *BCustomChoTroi::GetCurrencyName(int CurrencyId) {
  CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(CurrencyId);
  if (currency == 0 || currency->Name[0] == 0) {
    return "NULL";
  }

  return currency->Name;
}

void BCustomChoTroi::SendCurrencyList(int aIndex) {
  if (!this->Enable || !gObjIsConnected(aIndex)) {
    return;
  }

  BYTE send[1024];
  PMSG_CHOTROI_CURRENCY_LIST pMsg;
  pMsg.header.set(0xD3, 0x22, 0);

  int size = sizeof(pMsg);
  pMsg.Count = 0;

  for (std::vector<int>::iterator it = this->m_CurrencyOrder.begin();
       it != this->m_CurrencyOrder.end(); it++) {
    CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(*it);
    if (currency == 0 || currency->Enable == 0) {
      continue;
    }

    if ((size + (int)sizeof(PMSG_CHOTROI_CURRENCY_DATA)) >=
        (int)sizeof(send)) {
      break;
    }

    PMSG_CHOTROI_CURRENCY_DATA info;
    memset(&info, 0, sizeof(info));
    info.Id = currency->Id;
    strncpy_s(info.Name, currency->Name, sizeof(info.Name));
    info.Kind = currency->Kind;
    info.CoinType = currency->CoinType;
    info.ItemIndex = currency->ItemIndex;

    memcpy(&send[size], &info, sizeof(info));
    size += sizeof(info);
    pMsg.Count++;
  }

  pMsg.header.size[0] = SET_NUMBERHB(size);
  pMsg.header.size[1] = SET_NUMBERLB(size);
  memcpy(send, &pMsg, sizeof(pMsg));
  DataSend(aIndex, send, size);
}

char *BCustomChoTroi::GetMessage(int index) // OK
{
  std::map<int, CUSTOM_CHOTROI_MESSAGE>::iterator it =
      this->m_MessageInfoBP.find(index);

  if (it == this->m_MessageInfoBP.end()) {
    char Error[256];
    wsprintf(Error, "Could not find message %d!", index);
    return Error;
  } else {
    return it->second.Message;
  }
}
int BCustomChoTroi::GetRateTaxTypeItem(int TypeItem, int TypeCoin) // OK
{
  CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(TypeCoin);
  if (currency != 0) {
    std::map<int, int>::iterator tax = currency->TaxRateByTypeItem.find(TypeItem);
    if (tax != currency->TaxRateByTypeItem.end()) {
      return tax->second;
    }

    tax = currency->TaxRateByTypeItem.find(0);
    if (tax != currency->TaxRateByTypeItem.end()) {
      return tax->second;
    }

    return currency->TaxRate;
  }

  if (TypeItem > 10 || TypeItem < 0)
    TypeItem = 0;
  std::map<int, VAT_DATA_INFO>::iterator it = this->m_DataVAT.find(TypeItem);
  if (it == this->m_DataVAT.end()) {
    return 10;
  } else {
    if (TypeCoin == eMarketPriceWC)
      return it->second.WCRate;
    else if (TypeCoin == eMarketPriceWP)
      return it->second.WPRate;
    else if (TypeCoin == eMarketPriceGP)
      return it->second.GPRate;
    else if (TypeCoin == eMarketPriceB)
      return it->second.BRate;
    else if (TypeCoin == eMarketPriceS)
      return it->second.SRate;
    else if (TypeCoin == eMarketPriceC)
      return it->second.CRate;
  }
  return 10;
}
bool BCustomChoTroi::CheckItemBlockSend(CItem *lpItem) {
  // LogAdd(LOG_RED, "Check Item %d", lpItem->m_Index);
  for (std::vector<DATA_ITEMBLOCK>::iterator it = this->m_DataItemBlock.begin();
       it != this->m_DataItemBlock.end(); it++) {
    if (it->IndexMin != -1 && lpItem->m_Index < it->IndexMin) {
      // LogAdd(LOG_RED, "Check Item 1");
      continue;
    }
    if (it->IndexMax != -1 && lpItem->m_Index > it->IndexMax) {
      // LogAdd(LOG_RED, "Check Item 2");
      continue;
    }
    if (it->LvMin != -1 && lpItem->m_Level >= it->LvMin) {
      // LogAdd(LOG_RED, "Check Item 3");
      continue;
    }
    if (it->LvMax != -1 && lpItem->m_Level <= it->LvMax) {
      // LogAdd(LOG_RED, "Check Item 4");
      continue;
    }
    if (it->Skill != -1 && !lpItem->m_Option1) {
      // LogAdd(LOG_RED, "Check Item 5");
      continue;
    }
    if (it->Luck != -1 && !lpItem->m_Option2) {
      //	LogAdd(LOG_RED, "Check Item 6");
      continue;
    }
    if (it->Opt != -1 && lpItem->m_Option3 != it->Opt) {
      //	LogAdd(LOG_RED, "Check Item 7");
      continue;
    }
    if (it->ExcOpt != -1 && lpItem->m_NewOption != it->ExcOpt) {
      //	LogAdd(LOG_RED, "Check Item 8");
      continue;
    }
    return 1;
  }

  return 0;
}
void BCustomChoTroi::RollbackCacheItem(int aIndex, bool sendClientState,
                                       bool saveCharacter,
                                       const char *reason) {
  if (!OBJMAX_RANGE(aIndex)) {
    return;
  }

  LPOBJ lpObj = &gObj[aIndex];

  if (lpObj->Type != OBJECT_USER || !this->Enable) {
    return;
  }
  const bool hasChoTroiCache =
      (lpObj->StatusCacheItem == CACHE_ITEMCHOTROI ||
       lpObj->CH_IndexItem[0] != -1 || lpObj->CH_InfoItem[0].IsItem());

  if (!hasChoTroiCache) {
    if (sendClientState) {
      ChoTroiSendClientCacheState(lpObj->Index, 0);
    }
    return;
  }

  bool restoredItem = false;
  BYTE slotRecv = 0xFF;

  if (lpObj->CH_IndexItem[0] != -1 && lpObj->CH_InfoItem[0].IsItem()) {
    const int sourceSlot = lpObj->CH_IndexItem[0];

    if (INVENTORY_BASE_RANGE(sourceSlot) != 0 &&
        lpObj->Inventory[sourceSlot].IsItem() == 0) {
      slotRecv =
          gItemManager.InventoryAddItem(aIndex, lpObj->CH_InfoItem[0], sourceSlot);
    }

    if (slotRecv == 0xFF) {
      slotRecv =
          gItemManager.InventoryInsertItem(aIndex, lpObj->CH_InfoItem[0]);
    }

    if (slotRecv != 0xFF) {
      restoredItem = true;

      if (sendClientState) {
        gItemManager.GCItemModifySend(aIndex, slotRecv);
      }
    } else {
      LogAdd(LOG_RED,
             "[ChoTroiRollback] Cannot restore item Account:%s Name:%s "
             "SourceSlot:%d Index:%d Serial:%u Reason:%s",
             lpObj->Account, lpObj->Name, sourceSlot,
             lpObj->CH_InfoItem[0].m_Index, lpObj->CH_InfoItem[0].m_Serial,
             (reason == 0) ? "" : reason);
      if (sendClientState) {
        ChoTroiSendClientCacheState(lpObj->Index, 0);
      }
      return;
    }
  }

  if (restoredItem && saveCharacter) {
    GDCharacterInfoSaveSend(aIndex);
  }

  LogAdd(LOG_RED,
         "[ChoTroiRollback] Account:%s Name:%s Restore:%d Slot:%d Reason:%s",
         lpObj->Account, lpObj->Name, restoredItem ? 1 : 0, slotRecv,
         (reason == 0) ? "" : reason);

  if (lpObj->StatusCacheItem == CACHE_ITEMCHOTROI) {
    lpObj->StatusCacheItem = -1;
  }
  lpObj->CH_IndexItem[0] = -1;
  lpObj->CH_InfoItem[0].Clear();

  //====
  if (sendClientState) {
    ChoTroiSendClientCacheState(lpObj->Index, 0); // Clear Item Cache O Client
  }
  return;
}

void BCustomChoTroi::RollBack(int aIndex, XULY_CGPACKET *lpMsg) {
  this->RollbackCacheItem(aIndex, true, true, "ClientRollback");
  return;
}
void BCustomChoTroi::ClientSendItemRaoBan(int aIndex,
                                          PMSG_REQ_MARKET_ITEM_MOVE *lpMsg) {

  if (!OBJMAX_RANGE(aIndex)) {
    return;
  }

  LPOBJ lpObj = &gObj[aIndex];

  if (lpObj->Type != OBJECT_USER || !this->Enable) {
    return;
  }

  if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0) {
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }

  int sourceSlot = lpMsg->Source;

  if (sourceSlot < 0 ||
      sourceSlot >= (INVENTORY_EXT4_SIZE - INVENTORY_WEAR_SIZE)) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }

  sourceSlot += INVENTORY_WEAR_SIZE;

  if (gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[sourceSlot],
                                        0) == 0) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }

  if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj)) {
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }
  if (!lpObj->Inventory[sourceSlot].IsItem()) {
    // Vi tri nay khong co Item do
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }

  //=== Kiem Tra Item List Block
  if (this->CheckItemBlockSend(&lpObj->Inventory[sourceSlot])) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }
  if (lpObj->StatusCacheItem != -1) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }

  if (lpObj->CH_IndexItem[0] == -1) //
  {
    lpObj->CH_IndexItem[0] = sourceSlot;
    lpObj->CH_InfoItem[0] = lpObj->Inventory[sourceSlot];
    lpObj->StatusCacheItem = CACHE_ITEMCHOTROI;
    //=== Send Del item
    gItemManager.InventoryDelItem(aIndex, sourceSlot);
    gItemManager.GCItemDeleteSend(aIndex, sourceSlot, 1);
    //====
    XULY_CGPACKET pMsg;
    pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
    pMsg.ThaoTac = 1; // Show Item Cache
    DataSend(lpObj->Index, (BYTE *)&pMsg, pMsg.header.size);
    return;
  } else {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(3)); //
    ChoTroiSendClientCacheState(aIndex, 0);
    return;
  }
}

void BCustomChoTroi::CGReqItemSell(PMSG_REQ_MARKET_SELL *lpMsg, int aIndex) {
  if (lpMsg->ItemPrice < 0) {
    return;
  }

  int ItemPriceType = lpMsg->ItemPriceType;
  int ItemPrice = lpMsg->ItemPrice;
  if (!OBJECT_RANGE(aIndex)) {
    return;
  }

  if (!this->IsCurrencyEnabled(ItemPriceType)) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(11)); //
    return;
  }

  LPOBJ lpUser = &gObj[aIndex];
  if (lpUser->Interface.type == INTERFACE_CHAOS_BOX ||
      lpUser->Interface.type == INTERFACE_TRADE ||
      lpUser->Interface.type == INTERFACE_PARTY ||
      lpUser->Interface.type == INTERFACE_WAREHOUSE ||
      lpUser->Interface.type == INTERFACE_PERSONAL_SHOP ||
      lpUser->Interface.type == INTERFACE_CASH_SHOP ||
      lpUser->Interface.type == INTERFACE_TRAINER ||
      lpUser->Interface.use != 0 || lpUser->State == OBJECT_DELCMD ||
      lpUser->DieRegen != 0 || lpUser->Teleport != 0 ||
      lpUser->PShopOpen != 0 || lpUser->ChaosLock != 0 ||
      lpUser->SkillSummonPartyTime != 0) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(0)); //
    return;
  }

  if (lpUser->CH_IndexItem[0] == -1) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); //
    return;
  }
  if (!this->IsCurrencyEnabled(ItemPriceType) || ItemPrice < 1) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(0)); //
    return;
  }
  SDHP_REQ_MARKET_SELL pMsg;

  pMsg.h.set(0xEE, 0x01, sizeof(pMsg));

  pMsg.Result = 1;

  pMsg.aIndex = aIndex;

  pMsg.PriceType = ItemPriceType;

  pMsg.Price = ItemPrice;

  int itemDay = lpMsg->ItemDay;
  if (itemDay < 1) {
    itemDay = 1;
  } else if (itemDay > 30) {
    itemDay = 30;
  }

  pMsg.ItemDay = itemDay; // Ngay ban

  pMsg.Pass = lpMsg->Pass;
  // pMsg.Account[10] = 0;

  memcpy(pMsg.Account, lpUser->Account, sizeof(pMsg.Account) - 1);

#if (MARKET_NAME_DEV)

  // pMsg.Name[10] = 0;

  memcpy(pMsg.Name, lpUser->Name, sizeof(pMsg.Name));

#endif

  pMsg.ItemPos = 0;

  gItemManager.DBItemByteConvert((LPBYTE)pMsg.ItemData,
                                 &lpUser->CH_InfoItem[0]);

  if (lpUser->CH_InfoItem[0].m_IsPeriodicItem) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    // LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
    return;
  }

  if (lpUser->CH_InfoItem[0].m_LoadPeriodicItem) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    // LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
    return;
  }

  if (lpUser->CH_InfoItem[0].m_PeriodicItemTime) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    // LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
    return;
  }
  if (lpUser->CH_InfoItem[0].m_JewelOfHarmonyOption) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(4)); //
    // LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
    return;
  }
  // --

  //==Phan Loai Item
  int mTypeItem = lpUser->CH_InfoItem[0].m_Slot;
  if (lpUser->CH_InfoItem[0].m_Slot == 0 ||
      lpUser->CH_InfoItem[0].m_Slot == 1) {
    mTypeItem = 1;
  }
  if (lpUser->CH_InfoItem[0].m_Slot == 12) {
    mTypeItem = 8;
  }
  if (lpUser->CH_InfoItem[0].m_Slot == 13 ||
      lpUser->CH_InfoItem[0].m_Slot == 14) {
    mTypeItem = 9;
  }
  pMsg.TypeItem = mTypeItem;
  // gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"%d",mTypeItem);
  // Set Delay
  lpUser->ClickClientSend = GetTickCount();
  gNotice.GCNoticeSendToAll(
      0, 0, 0, 0, 0, 0, this->GetMessage(12), lpUser->Name,
      gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index,
                             lpUser->CH_InfoItem[0].m_Level),
      lpUser->CH_InfoItem[0].m_Level);
  gLog.Output(
      LOG_TRADE,
      "[ChoTroi][%s][%s] Rao Ban item (%s | Index: %04d Gia: %d [%d] Level: "
      "%02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, "
      "NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, "
      "SocketOption: %03d, %03d, %03d, %03d, %03d)",
      lpUser->Account, lpUser->Name,
      gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index,
                             lpUser->CH_InfoItem[0].m_Level),
      lpUser->CH_InfoItem[0].m_Index, ItemPrice, ItemPriceType,
      lpUser->CH_InfoItem[0].m_Level, lpUser->CH_InfoItem[0].m_Serial,
      lpUser->CH_InfoItem[0].m_Option1, lpUser->CH_InfoItem[0].m_Option2,
      lpUser->CH_InfoItem[0].m_Option3, lpUser->CH_InfoItem[0].m_NewOption,
      lpUser->CH_InfoItem[0].m_JewelOfHarmonyOption,
      lpUser->CH_InfoItem[0].m_ItemOptionEx,
      lpUser->CH_InfoItem[0].m_SocketOption[0],
      lpUser->CH_InfoItem[0].m_SocketOption[1],
      lpUser->CH_InfoItem[0].m_SocketOption[2],
      lpUser->CH_InfoItem[0].m_SocketOption[3],
      lpUser->CH_InfoItem[0].m_SocketOption[4]);
  gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                       this->GetMessage(9),
                       gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index,
                                              lpUser->CH_InfoItem[0].m_Level),
                       ItemPrice, this->GetCurrencyName(ItemPriceType)); //
  gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);

  gObj[aIndex].CH_IndexItem[0] = -1;
  gObj[aIndex].CH_InfoItem[0].Clear();
  gObj[aIndex].StatusCacheItem = -1;
  //====Set Clear Cache Item Client
  XULY_CGPACKET cMsg;
  cMsg.header.set(0xD3, 0x01, sizeof(cMsg));
  cMsg.ThaoTac = 2; // Show Item Cache
  DataSend(lpUser->Index, (BYTE *)&cMsg, cMsg.header.size);
}

//===Send Get List DS
void BCustomChoTroi::CGOpenListChoTroi(int aIndex) {
  this->GDReqItemStatus(aIndex);

  SDHP_REQ_MARKET_ITEM pMsg;

  pMsg.h.set(0xEE, 0x00, sizeof(pMsg));
  pMsg.Result = 1;
  pMsg.aIndex = aIndex;
  pMsg.PriceType = -1;
  pMsg.GetTypeItem = 0;
  memset(pMsg.Account, 0, sizeof(pMsg.Account));
  memcpy(pMsg.Account, &gObj[aIndex].Account, sizeof(pMsg.Account));
  pMsg.PageNumber = 1;
  pMsg.TypeHSD = this->TypeHSD; // GG
  gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);
}
void BCustomChoTroi::CGReqItemList(PMSG_REQ_MARKET_ITEM *lpMsg, int aIndex) {
  SDHP_REQ_MARKET_ITEM pMsg;

  pMsg.h.set(0xEE, 0x00, sizeof(pMsg));
  pMsg.Result = 1;
  pMsg.aIndex = aIndex;
  pMsg.PriceType = lpMsg->PriceType;
  pMsg.GetTypeItem = lpMsg->GetTypeItem;
  memset(pMsg.Account, 0, sizeof(pMsg.Account));
  memcpy(pMsg.Account, &gObj[aIndex].Account, sizeof(pMsg.Account));
  pMsg.PageNumber = lpMsg->PageNumber;
  pMsg.TypeHSD = this->TypeHSD; // GG
  gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);
}
//===DS Recv
void BCustomChoTroi::DGAnsItemList(CUSTOM_LOAD_COUNT *lpMsg) {
  if (!this->Enable) {
    return;
  }

  int aIndex = lpMsg->aIndex;

  if (gObjIsConnected(aIndex) == false) {
    return;
  }
  // ----
  this->m_ListDataChoTroi[aIndex].clear();

  for (int n = 0; n < lpMsg->count; n++) {
    MARKET_DATA *lpInfo =
        (MARKET_DATA *)(((BYTE *)lpMsg) + sizeof(CUSTOM_LOAD_COUNT) +
                        (sizeof(MARKET_DATA) * n));

    //	LogAdd(LOG_RED,"GCSetListChoTroi %d  %s  %x %d", lpInfo->ID,
    // lpInfo->Name, lpInfo->Item,lpInfo->Pass );
    this->m_ListDataChoTroi[aIndex].push_back(*lpInfo);
  }

  /*LogAdd(LOG_BLUE, "[ChoTroi] DGAnsItemList aIndex=%d DSCount=%d Cached=%d",
         aIndex, lpMsg->count, (int)this->m_ListDataChoTroi[aIndex].size());*/

  this->SendListUser(aIndex);
}

void BCustomChoTroi::SendListUser(int Index) {
  if (!this->Enable) {
    return;
  }

  if (gObjIsConnected(Index) == false) {
    return;
  }
  // ----
  this->SendCurrencyList(Index);

  BYTE send[8192];

  BCSEV_COUNTLIST pMsg;

  pMsg.header.set(0xD3, 0x20, 0);

  int size = sizeof(pMsg);

  pMsg.Count = 0;

  pMsg.Type = this->OnCointType;

  MARKET_DATA info;
  int skipCount = 0;

  for (int i = 0; i < this->m_ListDataChoTroi[Index].size(); i++) {

    info.ID = this->m_ListDataChoTroi[Index][i].ID;

    memcpy(info.Name, this->m_ListDataChoTroi[Index][i].Name,
           sizeof(info.Name));
    //	LogAdd(LOG_CAM, "%s", info.Name);
    CItem item;

    if (gItemManager.ConvertItemByte(
            &item, this->m_ListDataChoTroi[Index][i].Item) == 0) {
      skipCount++;
      if (skipCount <= 3) {
        LogAdd(LOG_RED,
               "[ChoTroi] SendListUser skip ID=%d Item=%02X %02X %02X %02X "
               "%02X %02X %02X %02X",
               this->m_ListDataChoTroi[Index][i].ID,
               this->m_ListDataChoTroi[Index][i].Item[0],
               this->m_ListDataChoTroi[Index][i].Item[1],
               this->m_ListDataChoTroi[Index][i].Item[2],
               this->m_ListDataChoTroi[Index][i].Item[3],
               this->m_ListDataChoTroi[Index][i].Item[4],
               this->m_ListDataChoTroi[Index][i].Item[5],
               this->m_ListDataChoTroi[Index][i].Item[6],
               this->m_ListDataChoTroi[Index][i].Item[7]);
      }
      continue;
    }
    gItemManager.ItemByteConvert(info.Item, item); // Set Info Item
    // memcpy(info.Item, this->m_ListDataChoTroi[i].Item, sizeof(info.Item) -
    // 1);
    info.PriceType = this->m_ListDataChoTroi[Index][i].PriceType;
    info.Price = this->m_ListDataChoTroi[Index][i].Price;
    info.TypeItem = this->m_ListDataChoTroi[Index][i].TypeItem;
    info.TimeItemRaoBan = this->m_ListDataChoTroi[Index][i].TimeItemRaoBan;
    info.Pass = this->m_ListDataChoTroi[Index][i].Pass;
    // LogAdd(LOG_RED, "SendList Model Skin OK !!");
    pMsg.Count++;
    memcpy(&send[size], &info, sizeof(info));
    size += sizeof(info);
  }

  pMsg.header.size[0] = SET_NUMBERHB(size);
  pMsg.header.size[1] = SET_NUMBERLB(size);
  // ---
  memcpy(send, &pMsg, sizeof(pMsg));

  DataSend(Index, send, size);

  /*LogAdd(LOG_BLUE, "[ChoTroi] SendListUser aIndex=%d Cached=%d Sent=%d Skip=%d",
         Index, (int)this->m_ListDataChoTroi[Index].size(), pMsg.Count,
         skipCount);*/
}

bool BCustomChoTroi::GetCheckMoney(int aIndex, int PriceType, int PriceValue) {
  if (!OBJECT_RANGE(aIndex)) {
    return false;
  }
  LPOBJ lpUser = &gObj[aIndex];
  CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(PriceType);
  if (currency == 0 || !this->IsCurrencyEnabled(PriceType)) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(11));
    gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0,
                        this->GetMessage(11));
    return false;
  }

  __int64 currentValue = ChoTroi_GetCurrencyValue(aIndex, currency);

  if (PriceValue <= currentValue) {
    return true;
  }

  gNotice.GCNoticeSend(
      aIndex, eMessageBox, 0, 0, 0, 0, 0,
      this->GetMessage(2),
      currency->Name, PriceValue, (int)currentValue);
  gNotice.GCNoticeSend(
      aIndex, 1, 0, 0, 0, 0, 0,
      this->GetMessage(2),
      currency->Name, PriceValue, (int)currentValue);
  /*LogAdd(LOG_BLUE,
         "[ChoTroi] Buy denied insufficient currency Account=%s Name=%s Need=%d %s Have=%d",
         lpUser->Account, lpUser->Name, PriceValue, currencyName, currentValue);*/

  return false;
}

void BCustomChoTroi::CGReqItemBuy(PMSG_REQ_MARKET_BUY *lpMsg, int aIndex) {
  if (!OBJECT_RANGE(aIndex)) {
    return;
  }
  int ID = lpMsg->ID;
  LPOBJ lpUser = &gObj[aIndex];
  if (lpUser->Interface.type == INTERFACE_CHAOS_BOX ||
      lpUser->Interface.type == INTERFACE_TRADE ||
      lpUser->Interface.type == INTERFACE_PARTY ||
      lpUser->Interface.type == INTERFACE_WAREHOUSE ||
      lpUser->Interface.type == INTERFACE_PERSONAL_SHOP ||
      lpUser->Interface.type == INTERFACE_CASH_SHOP ||
      lpUser->Interface.type == INTERFACE_TRAINER ||
      lpUser->Interface.use != 0 || lpUser->State == OBJECT_DELCMD ||
      lpUser->DieRegen != 0 || lpUser->Teleport != 0 ||
      lpUser->PShopOpen != 0 || lpUser->ChaosLock != 0 ||
      lpUser->SkillSummonPartyTime != 0) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(0)); //
    return;
  }
  // Delay Khi Click
  if ((GetTickCount() - lpUser->ClickClientSend) < 2000) {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(5)); //
    return;
  }
  // LogAdd(LOG_RED, "%d",lpMsg->Pass);

  SDHP_REQ_MARKET_BUY pMsg;

  pMsg.h.set(0xEE, 0x02, sizeof(pMsg));

  pMsg.aIndex = aIndex;

  pMsg.ID = ID;

  pMsg.Result = 0;

  if (!lpMsg->Result)
    return;
  for (int i = 0; i < this->m_ListDataChoTroi[aIndex].size(); i++) {
    if (ID == this->m_ListDataChoTroi[aIndex][i].ID) {
      CItem item;

      if (gItemManager.ConvertItemByte(
              &item, this->m_ListDataChoTroi[aIndex][i].Item) == 0) {
        gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                             this->GetMessage(0)); //
        return;
      }
      if (gItemManager.GetInventoryEmptySlotCount(lpUser) < 1) {
        gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                             this->GetMessage(6)); //
        return;
      }

      if (lpMsg->Result == 1) {
        if (!this->GetCheckMoney(aIndex,
                                 this->m_ListDataChoTroi[aIndex][i].PriceType,
                                 this->m_ListDataChoTroi[aIndex][i].Price)) {
          return;
        }
        if (this->m_ListDataChoTroi[aIndex][i].Pass >= 0 &&
            this->m_ListDataChoTroi[aIndex][i].Pass != lpMsg->Pass) {
          gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                                this->GetMessage(13)); //
          gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0,
                                this->GetMessage(13)); //
          return;
        }
      }

      pMsg.Result = lpMsg->Result;
      break;
    }
  }

  // Set Delay
  lpUser->ClickClientSend = GetTickCount();

  gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);
}

void BCustomChoTroi::DGAnsItemBuy(SDHP_ANS_MARKET_BUY *lpMsg) {
  int aIndex = lpMsg->aIndex;

  if (!OBJECT_RANGE(aIndex)) {
    return;
  }
  LPOBJ lpUser = &gObj[aIndex];
  // LogAdd(LOG_RED, "DGAnsItemBuy %d", lpMsg->Result);
  if (lpMsg->Result == 1 || lpMsg->Result == 2) {
    CItem item;
    int Count = 0;
    for (int i = 0; i < this->m_ListDataChoTroi[aIndex].size(); i++) {
      if (lpMsg->ID == this->m_ListDataChoTroi[aIndex][i].ID) {
        gItemManager.ConvertItemByte(&item,
                                     this->m_ListDataChoTroi[aIndex][i].Item);
        if (lpMsg->Result == 1) // MUA
        {
          CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(
              this->m_ListDataChoTroi[aIndex][i].PriceType);
          if (!ChoTroi_DebitCurrency(aIndex, currency,
                                     this->m_ListDataChoTroi[aIndex][i].Price)) {
            LogAdd(LOG_RED,
                   "[ChoTroi] Debit currency failed Buyer=%s PriceType=%d "
                   "Price=%d",
                   lpUser->Name, this->m_ListDataChoTroi[aIndex][i].PriceType,
                   this->m_ListDataChoTroi[aIndex][i].Price);
            return;
          }

          gNotice.GCNoticeSend(
              aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(7),
              gItemLevel.GetItemName(item.m_Index, item.m_Level)); //
          int ListID = this->m_ListDataChoTroi[aIndex][i].ID;
          int ItemPrice = this->m_ListDataChoTroi[aIndex][i].Price;
          int ItemPriceType = this->m_ListDataChoTroi[aIndex][i].PriceType;
          gLog.Output(
              LOG_TRADE,
              "[ChoTroi][%s][%s] Vua mua Item [%d] (%s | Index: %04d Gia: %d "
              "[%d] Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, "
              "Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, "
              "ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",
              lpUser->Account, lpUser->Name, ListID,
              gItemLevel.GetItemName(item.m_Index, item.m_Level), item.m_Index,
              ItemPrice, ItemPriceType, item.m_Level, item.m_Serial,
              item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption,
              item.m_JewelOfHarmonyOption, item.m_ItemOptionEx,
              item.m_SocketOption[0], item.m_SocketOption[1],
              item.m_SocketOption[2], item.m_SocketOption[3],
              item.m_SocketOption[4]);

          LPOBJ lpSeller =
              ChoTroiFindOnlineSeller(this->m_ListDataChoTroi[aIndex][i].Name);
          if (lpSeller != 0) {
            LogAdd(LOG_BLUE, "[ChoTroi] Request seller payout Name=%s ID=%d",
                   lpSeller->Name, ListID);
            this->GDReqItemStatus(lpSeller->Index);
          } else {
            LogAdd(LOG_BLUE, "[ChoTroi] Seller offline payout pending Name=%s ID=%d",
                   this->m_ListDataChoTroi[aIndex][i].Name, ListID);
          }

        } else // Thu hoi
        {
          gNotice.GCNoticeSend(
              aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(8),
              gItemLevel.GetItemName(item.m_Index, item.m_Level)); //
          int ListID = this->m_ListDataChoTroi[aIndex][i].ID;
          int ItemPrice = this->m_ListDataChoTroi[aIndex][i].Price;
          int ItemPriceType = this->m_ListDataChoTroi[aIndex][i].PriceType;
          gLog.Output(LOG_TRADE,
                      "[ChoTroi][%s][%s] Vua thu hoi Item [%d] (%s | Index: "
                      "%04d Gia: %d [%d] Level: %02d, Serial: %08X, Option1: "
                      "%01d, Option2: %01d, Option3: %01d, NewOption: %03d, "
                      "JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, "
                      "SocketOption: %03d, %03d, %03d, %03d, %03d)",
                      lpUser->Account, lpUser->Name, ListID,
                      gItemLevel.GetItemName(item.m_Index, item.m_Level),
                      item.m_Index, ItemPrice, ItemPriceType, item.m_Level,
                      item.m_Serial, item.m_Option1, item.m_Option2,
                      item.m_Option3, item.m_NewOption,
                      item.m_JewelOfHarmonyOption, item.m_ItemOptionEx,
                      item.m_SocketOption[0], item.m_SocketOption[1],
                      item.m_SocketOption[2], item.m_SocketOption[3],
                      item.m_SocketOption[4]);
        }

        BYTE btItemPos = gItemManager.InventoryInsertItem(lpMsg->aIndex, item);
        gItemManager.GCItemModifySend(lpMsg->aIndex, btItemPos);
        this->m_ListDataChoTroi[aIndex].erase(
            this->m_ListDataChoTroi[aIndex].begin() + Count);
        this->SendListUser(aIndex);

        break;
      }
      Count++;
    }
  }

  else {
    gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0,
                         this->GetMessage(1)); //
    return;
  }

  // PMSG_ANS_MARKET_BUY pMsg;
  //
  // pMsg.h.set((LPBYTE)& pMsg, 0xEC, 0x01, sizeof(pMsg));
  //
  // pMsg.Result = 1;
  //
  // DataSend(aIndex, (LPBYTE)& pMsg, sizeof(pMsg));
}

void BCustomChoTroi::GDReqItemStatus(int aIndex) {
  if (!OBJECT_RANGE(aIndex)) {
    return;
  }

  LPOBJ lpUser = &gObj[aIndex];

  SDHP_REQ_MARKET_STATUS pMsg;
  memset(&pMsg, 0, sizeof(pMsg));

  pMsg.h.set(0xEE, 0x03, sizeof(pMsg));

  pMsg.Result = 1;

  pMsg.aIndex = aIndex;

  memcpy(pMsg.Account, lpUser->Account, sizeof(pMsg.Account) - 1);

  gDataServerConnection.DataSend((BYTE *)&pMsg, pMsg.h.size);
}
void BCustomChoTroi::DGAnsItemStatus(CUSTOM_LOAD_COUNT *lpMsg) {
  if (!OBJECT_RANGE(lpMsg->aIndex) || !gObjIsConnectedGP(lpMsg->aIndex)) {
    return;
  }

  LPOBJ lpUser = &gObj[lpMsg->aIndex];
  for (int n = 0; n < lpMsg->count; n++) {
    SDHP_ANS_MARKET_STATUS *lpInfo =
        (SDHP_ANS_MARKET_STATUS *)(((BYTE *)lpMsg) + sizeof(CUSTOM_LOAD_COUNT) +
                                   (sizeof(SDHP_ANS_MARKET_STATUS) * n));
    // LogAdd(LOG_RED, "DEBUG CONg Coin -> %s Coint %d",
    // gObj[lpMsg->aIndex].Name, lpInfo->PriceValue);
    CItem item;
    if (!gItemManager.ConvertItemByte(&item, lpInfo->ItemData)) {
      continue;
    }
    int ItemPrice = lpInfo->PriceValue;
    int ItemPriceType = lpInfo->PriceType;
    CHOTROI_CURRENCY_INFO *currency = this->GetCurrencyInfo(ItemPriceType);
    if (currency == 0 || currency->Enable == 0) {
      continue;
    }

    int RateTax = this->GetRateTaxTypeItem(lpInfo->TypeItem, lpInfo->PriceType);
    int CoinAfterTax =
        lpInfo->PriceValue - ((lpInfo->PriceValue * RateTax) / 100);

    if (!ChoTroi_CreditCurrency(lpUser->Index, currency, CoinAfterTax)) {
      LogAdd(LOG_RED,
             "[ChoTroi] Credit currency failed Seller=%s PriceType=%d "
             "Value=%d",
             lpUser->Name, ItemPriceType, CoinAfterTax);
      continue;
    }

    gNotice.GCNoticeSend(
        lpUser->Index, eMessageBox, 0, 0, 0, 0, 0,
        "[Cho Troi] Da ban %s: +%d %s (Thue %d%%)",
        gItemLevel.GetItemName(item.m_Index, item.m_Level), CoinAfterTax,
        this->GetCurrencyName(ItemPriceType), RateTax);
    this->GuiThuThongBao(
        lpUser, this->GetMessage(10),
        "=====[Successfully Sold]===== \n- TAX %d%% (%s) += %d \n- %s (Price: "
        "%d [%s])",
        RateTax, this->GetCurrencyName(ItemPriceType), CoinAfterTax,
        gItemLevel.GetItemName(item.m_Index, item.m_Level), ItemPrice,
        this->GetCurrencyName(ItemPriceType));
    /*this->GuiThuThongBao(
        lpUser, this->GetMessage(10),
        "[Cho Troi] Ban vat pham [%s] voi gia %d %s.",
        gItemLevel.GetItemName(item.m_Index, item.m_Level), ItemPrice,
        TypeCoin[ItemPriceType]);*/

    gLog.Output(
        LOG_TRADE,
        "[ChoTroi][%s][%s] Ban Thanh Cong Item (%s | Index: %04d Gia: %d [%s] "
        "Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: "
        "%01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: "
        "%03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",
        lpUser->Account, lpUser->Name,
        gItemLevel.GetItemName(item.m_Index, item.m_Level), item.m_Index,
        ItemPrice, this->GetCurrencyName(ItemPriceType), item.m_Level, item.m_Serial,
        item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption,
        item.m_JewelOfHarmonyOption, item.m_ItemOptionEx,
        item.m_SocketOption[0], item.m_SocketOption[1], item.m_SocketOption[2],
        item.m_SocketOption[3], item.m_SocketOption[4]);
  }
}

void FriendMessageSend(PMSG_FRIEND_MEMO *lpMsg, int aIndex, int bIndex) {
  if (!gObjIsConnectedGP(aIndex)) {
    return;
  }

  char szName[11];

  memset(szName, 0, sizeof(szName));

  memcpy(szName, lpMsg->Name, sizeof(lpMsg->Name));

  FHP_FRIEND_MEMO_SEND pMsg;
  int bsize = lpMsg->MemoSize + sizeof(pMsg) - sizeof(pMsg.Memo);

  pMsg.h.set(0x70, bsize);
  pMsg.WindowGuid = lpMsg->WindowGuid;
  pMsg.MemoSize = lpMsg->MemoSize;
  pMsg.Number = aIndex;
  pMsg.Dir = lpMsg->Dir;
  pMsg.Action = lpMsg->Action;

  memcpy(pMsg.Subject, lpMsg->Subject, sizeof(pMsg.Subject));
  memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
  memcpy(pMsg.Equipment, gObj[bIndex].CharSet, sizeof(pMsg.Equipment));
  memset(pMsg.ToName, 0, sizeof(pMsg.ToName));
  memcpy(pMsg.ToName, gObj[aIndex].Name, sizeof(lpMsg->Name));
  memcpy(pMsg.Memo, lpMsg->Memo, lpMsg->MemoSize);

  CSDataSend((BYTE *)&pMsg, bsize);
}
void BCustomChoTroi::GuiThuThongBao(LPOBJ lpTarget, char *Subject,
                                    char *message, ...) {
  if (lpTarget == 0 || !OBJECT_RANGE(lpTarget->Index) ||
      !gObjIsConnectedGP(lpTarget->Index)) {
    return;
  }

  char buff[1000] = {0};

  va_list arg;
  va_start(arg, message);
  vsnprintf_s(buff, sizeof(buff), _TRUNCATE, message, arg);
  va_end(arg);

  if (buff[0] == 0) {
    LogAdd(LOG_RED, "[ChoTroi] Seller mail memo empty Name=%s",
           lpTarget->Name);
    return;
  }

  char sender[11] = {0};
  char target[11] = {0};
  char subject[32] = {0};

  strncpy_s(sender, sizeof(sender), "[Hệ Thống]", _TRUNCATE);
  strncpy_s(target, sizeof(target), lpTarget->Name, _TRUNCATE);
  strncpy_s(subject, sizeof(subject), Subject, _TRUNCATE);

  GEMailMessageSend(sender, target, subject, 2, 143, lpTarget->CharSet, buff);
}

#endif
