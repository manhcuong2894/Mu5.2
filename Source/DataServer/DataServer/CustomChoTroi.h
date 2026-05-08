#pragma once

#include "stdafx.h"
#include "DataServerProtocol.h"


#if (CUSTOM_CHOTROI)

#define MARKET_ITEM_BUFFER 16
#define MARKET_ITEM_MAX 100
#define MARKET_NAME_LEN 11

#define MARKET_DEBUG 1
#define MARKET_PRICE_DEV 1
#define MARKET_FILTER_DEV 1
#define MARKET_NAME_DEV 1
#define MARKET_PAGE_DEV 1

#define MARET_MUEMU 1

#define PMHC_BYTE 0xC1
#define PMHC_WORD 0xC2

// Bỏ PWMSG_HEAD2 và PBMSG_HEAD2 do SRCMainGS DataServer đã có sẵn chuẩn
// Thêm lại CUSTOM_LOAD_COUNT vì DataServer cần struct này để gửi list
struct CBCUSTOM_LOAD_COUNT {
  PSWMSG_HEAD header;

  int aIndex;
  int count;
};

enum {
  eMarketPriceWC = 1,
  eMarketPriceWP = 2,
  eMarketPriceGP = 3,
  eMarketPriceB = 4,
  eMarketPriceS = 5,
  eMarketPriceC = 6,
};

struct MARKET_DATA {
  int ID;
  char Name[MARKET_NAME_LEN];
  BYTE Item[MARKET_ITEM_BUFFER];
  int PriceType;
  int Price;
  int TypeItem;
  int TimeItemRaoBan;
  int Pass;
};

#if (MARKET_FILTER_DEV)
struct MARKET_FILTER {
  int FilterType;
  int FilterLevel;
  int FilterLuck;
  int FilterExl;
  int FilterAnc;
};
#endif

// -> DATA SERVER PROTOCOL

struct SDHP_REQ_MARKET_ITEM {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  int PriceType;
  int GetTypeItem;
  char Account[11];
  int PageNumber;
  int TypeHSD;
};

struct SDHP_ANS_MARKET_ITEM {
  PSWMSG_HEAD h;
  int Result;
  int aIndex;
  MARKET_DATA Data[MARKET_ITEM_MAX];
};

struct SDHP_REQ_MARKET_SELL {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  char Account[11];
  char Name[11];
  int PriceType;
  int Price;
  int ItemPos;
  BYTE ItemData[MARKET_ITEM_BUFFER];
  int TypeItem;
  int ItemDay;
  int Pass;
};

struct SDHP_ANS_MARKET_SELL {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  int ItemPos;
};

struct SDHP_REQ_MARKET_BUY {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  int ID;
};

struct SDHP_ANS_MARKET_BUY {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  int ID;
};

#if (MARKET_PRICE_DEV)
struct SDHP_REQ_MARKET_STATUS {
  PSBMSG_HEAD h;
  int Result;
  int aIndex;
  char Account[11];
};

struct SDHP_ANS_MARKET_STATUS {
  BYTE ItemData[MARKET_ITEM_BUFFER];
  char Account[11];
  int PriceType;
  int PriceValue;
  int TypeItem;
};
#endif

class CChoTroi {
public:
  CChoTroi();
  ~CChoTroi();

  void Init();
  void Load();

  bool Connect();
  void CreateTable();
  void Protocol(BYTE protoNum, BYTE *aRecv, int uIndex);

  void GDReqItemList(SDHP_REQ_MARKET_ITEM *lpMsg, int uIndex);
  void GDReqItemSell(SDHP_REQ_MARKET_SELL *lpMsg, int uIndex);
  void GDReqBuyItem(SDHP_REQ_MARKET_BUY *lpMsg, int uIndex);
#if (MARKET_PRICE_DEV)
  void GDReqItemStatus(SDHP_REQ_MARKET_STATUS *lpMsg, int uIndex);
#endif
#if (MARKET_PRICE_DEV)
  void GDReqItemListPage(SDHP_REQ_MARKET_ITEM *lpMsg, int uIndex);
#endif
};

extern CChoTroi g_CustomChoTroi;

#endif
