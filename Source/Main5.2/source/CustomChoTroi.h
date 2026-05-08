#pragma once
#include "CBInterface.h"
#include "WSclient.h"
#define MARKET_NAME_LEN 11
#define MARKET_ITEM_BUFFER 16
#define MARKET_ITEM_MAX 100

class CustomChoTroi {
public:
  struct PMSG_REQ_MARKET_SELL {
    PSBMSG_HEAD h;
    int Result;
    int ItemPos;
    int ItemPriceType;
    int ItemPrice;
    int ItemDay; // HSD Them
    int Pass;
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
  //===Get List Cho Troi
  struct PMSG_REQ_MARKET_ITEM // Client Send GS
  {
    PSBMSG_HEAD h;
    int Result;
    int PriceType;
    int PageNumber;
    int GetTypeItem;
  };

  struct PMSG_REQ_MARKET_BUY {
    PSBMSG_HEAD h;
    int Result;
    int ID;
    int Pass;
  };

  struct BCSEV_COUNTLIST {
    PSWMSG_HEAD header;
    int Count;
    int Type;
  };

  CustomChoTroi();
  virtual ~CustomChoTroi();
  void CustomChoTroi::BDrawWindowChoTroi();
  void CustomChoTroi::GetOpenChoTroiWinDow();
  void CustomChoTroi::CloseWindow(bool sendRollback = true);
  bool CustomChoTroi::HandleEscapeKey();
  bool CustomChoTroi::IsWindowActive() const;
  bool CustomChoTroi::IsBuyPassWindowActive() const;
  void GCSetListChoTroi(BYTE *Recv, int Size = 0);
  void SetShowItemCache(BYTE *Recv);
  void UpdateInputFocus();
  SEASON3B::CNewUIRadioGroupButton m_TabBtn;
  int m_iNumCurOpenTab;

  std::map<int, char *> mListItemFind;
  std::vector<MARKET_DATA> m_DataChoTroi;
  int OnCointType;
  ITEM *ItemCacheSelect;
  DWORD ItemCacheTime;
  bool ItemCacheShow;
  bool SendItemRaoBan(ITEM *ItemSell, int Slot, bool KeyClick);
};

extern CustomChoTroi gCusChoTroi;
