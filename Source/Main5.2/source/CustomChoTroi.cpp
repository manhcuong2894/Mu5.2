
#include "stdafx.h"
#include "CustomChoTroi.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "NewUIBase.h"
#include "NewUISystem.h"
#include "Protocol.h"
#include "TextClient.h"
#include "Util.h"
#include "NewUIScrollBar.h"
#include "ZzzInfomation.h"
#include "_struct.h"
#include "CGMItemMng.h"

#if CUSTOM_CHOTROI

CustomChoTroi gCusChoTroi;
extern info::ITEM_ATTRIBUTE *ItemAttribute;
extern bool MouseOnWindow;
extern DWORD g_dwKeyFocusUIID;
CUIPhotoViewer m_PhotoViewChotroi;
SEASON3B::CNewUIScrollBar *WindowChoTroiScrollBar = NULL;
CUITextInputBox *CoinRaoBanChoTroi = NULL;
CUITextInputBox *NgayRaoBanChoTroi = NULL; // HSD Them
CUITextInputBox *PassChoTroi = NULL;
char szNgayRaoBan[2 + 1] = {
    '1',
    0,
};
char szGiaTriCoin[8 + 1] = {
    0,
};
char szPassChoTroi[5 + 1] = {
    0,
};

int SelectBarChoTroiNumber = 0;
char *NameGiaCoin[7] = {"NULL",
                        gTextClient.txtClient_ChoTroi[1],
                        gTextClient.txtClient_ChoTroi[2],
                        gTextClient.txtClient_ChoTroi[3],
                        "Bless",
                        "Soul",
                        "Chaos"};
char *SelectBarListChoTroi[13] = {
    gTextClient.txtClient_ChoTroi[26], gTextClient.txtClient_ChoTroi[27],
    gTextClient.txtClient_ChoTroi[28], gTextClient.txtClient_ChoTroi[29],
    gTextClient.txtClient_ChoTroi[30], gTextClient.txtClient_ChoTroi[31],
    gTextClient.txtClient_ChoTroi[32], gTextClient.txtClient_ChoTroi[33],
    gTextClient.txtClient_ChoTroi[34], gTextClient.txtClient_ChoTroi[35],
    gTextClient.txtClient_ChoTroi[36], gTextClient.txtClient_ChoTroi[37],
    gTextClient.txtClient_ChoTroi[38]};
bool SelectBarChoTroi = false;
CUITextInputBox *m_TimTuKhoaChoTroi = NULL;
CUITextInputBox *m_TimTuKhoaChoTroi_Clear = NULL;
CUITextInputBox *NhapPassChoTroi = NULL;
char szNhapPassChoTroi[5 + 1] = {
    0,
};

int MaxPerPageChoTroi = 3;
int mPageChoTroi = 0;

DWORD CacheTimeFind = 0;
int CacheSizeInputTimKiem = -1;
int CacheItemRaoBan = -1;
int TypeCoinBan = 1;
int ShowInfoItem = -1;
bool UpdateMaxPosChoTroi = false;
int LocItemTypCoin = 63;
int LocItemLvMin = 0;
int LocItemLvMax = 15;
bool LocItemCoSkill = false;
bool LocItemCoLuck = false;
bool LocItemCoOpt = false;
bool LocItemCoExc = false;
bool HoverItemSell = false;

bool ShowWindowChoOK = false;

int BuyID = -1;
static bool ChoTroiOwnsSellCacheItem = false;
static int ChoTroiSellCacheSourceSlot = -1;
static bool ChoTroiBlockEscapeUntilRelease = false;
static bool ChoTroiNhapPassNeedFocus = false;
static bool ChoTroiNhapPassManualFocus = false;
static bool ChoTroiNhapPassReplaceAll = false;
static bool ChoTroiBuyConfirmOpen = false;
static int ChoTroiPendingBuyID = -1;
static int ChoTroiPendingBuyPass = -1;
static const int CHOTROI_BUY_PASS_INPUT_WIDTH = 130;
static DWORD ChoTroiLastSellConfirmTick = 0;

static void ChoTroiConfirmBuyCallback(LPVOID lpParam);
static bool OpenChoTroiBuyConfirm(const CustomChoTroi::MARKET_DATA &marketData,
                                  int pass);

static void ClearChoTroiBuyConfirmState(bool closeMessageBox) {
  if (closeMessageBox && ChoTroiBuyConfirmOpen &&
      gInterface->Data[eWindowMessageBox].OnShow) {
    gInterface->Data[eWindowMessageBox].OnShow = 0;
    gInterface->MsgBoxCallback = 0;
  }

  ChoTroiBuyConfirmOpen = false;
  ChoTroiPendingBuyID = -1;
  ChoTroiPendingBuyPass = -1;
}

static void UpdateChoTroiBuyConfirmState() {
  if (ChoTroiBuyConfirmOpen && !gInterface->Data[eWindowMessageBox].OnShow) {
    ClearChoTroiBuyConfirmState(false);
  }
}

static void ReleaseChoTroiSellCacheItem() {
  if (ChoTroiOwnsSellCacheItem && gCusChoTroi.ItemCacheSelect != NULL) {
    g_pNewItemMng->DeleteItem(gCusChoTroi.ItemCacheSelect);
  }

  ChoTroiOwnsSellCacheItem = false;
}

static void DeleteChoTroiTempItem(ITEM *&item) {
  if (item != NULL) {
    g_pNewItemMng->DeleteItem(item);
    item = NULL;
  }
}

static int GetChoTroiMaxScrollPage(int totalItems, int itemsPerPage) {
  if (totalItems <= itemsPerPage || itemsPerPage <= 0) {
    return 0;
  }

  return (totalItems - 1) / itemsPerPage;
}

static void SetChoTroiScrollBarPosition(SEASON3B::CNewUIScrollBar *scrollBar,
                                        int x, int y) {
  if (scrollBar == NULL) {
    return;
  }

  const int currentPos = scrollBar->GetCurPos();
  scrollBar->SetPos(x, y);
  scrollBar->SetCurPos(currentPos);
}

static void HandleChoTroiScrollWheel(SEASON3B::CNewUIScrollBar *scrollBar,
                                     float x, float y, float width,
                                     float height) {
  if (scrollBar == NULL || !SEASON3B::CheckMouseIn(x, y, width, height)) {
    return;
  }

  if (MouseWheel < 0) {
    MouseWheel = 0;
    scrollBar->SetCurPos(scrollBar->GetCurPos() + 1);
  } else if (MouseWheel > 0) {
    MouseWheel = 0;
    scrollBar->SetCurPos(scrollBar->GetCurPos() - 1);
  }
}

static void RequestChoTroiList(int typeItem) {
  if (typeItem < 0 || typeItem > 12) {
    typeItem = 0;
  }

  CustomChoTroi::PMSG_REQ_MARKET_ITEM pMsg = {0};
  pMsg.h.set(0xD3, 0x20, sizeof(pMsg));
  pMsg.Result = 1;
  pMsg.PriceType = -1;
  pMsg.PageNumber = 1;
  pMsg.GetTypeItem = typeItem;
  DataSend((LPBYTE)&pMsg, pMsg.h.size);

  if (WindowChoTroiScrollBar != NULL) {
    WindowChoTroiScrollBar->SetCurPos(0);
  }

  UpdateMaxPosChoTroi = true;
}

static bool IsChoTroiCoinTypeEnabled(int coinType) {
  if (coinType < 1 || coinType > 6) {
    return false;
  }

  return (gCusChoTroi.OnCointType & (1 << (coinType - 1))) != 0;
}

static int GetFirstEnabledChoTroiCoinType() {
  for (int coinType = 1; coinType <= 6; coinType++) {
    if (IsChoTroiCoinTypeEnabled(coinType)) {
      return coinType;
    }
  }

  return 0;
}

static void SetChoTroiInputState(CUITextInputBox *input, int state) {
  if (input != NULL) {
    input->SetState(state);
  }
}

static void HideChoTroiInputBoxes() {
  SetChoTroiInputState(CoinRaoBanChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(NgayRaoBanChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(PassChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(m_TimTuKhoaChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(m_TimTuKhoaChoTroi_Clear, UISTATE_HIDE);
}

static void HideChoTroiSellInputBoxes() {
  SetChoTroiInputState(CoinRaoBanChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(NgayRaoBanChoTroi, UISTATE_HIDE);
  SetChoTroiInputState(PassChoTroi, UISTATE_HIDE);
}

static void RestoreChoTroiGameInputFocus() {
  HideChoTroiInputBoxes();
  g_dwMouseUseUIID = 0;
  g_dwKeyFocusUIID = 0;
  MouseOnWindow = false;

  if (g_pNewUIMng != NULL) {
    g_pNewUIMng->ResetActiveUIObj();
  }

  HWND hGameWindow = gwinhandle->GethWnd();
  if (hGameWindow != NULL && GetFocus() != hGameWindow) {
    SetFocus(hGameWindow);
  }
}

static void ResetChoTroiSellInputs() {
  memset(&szGiaTriCoin, 0, sizeof(szGiaTriCoin));
  memset(&szNgayRaoBan, 0, sizeof(szNgayRaoBan));
  memset(&szPassChoTroi, 0, sizeof(szPassChoTroi));
  strcpy_s(szGiaTriCoin, sizeof(szGiaTriCoin), "1");
  strcpy_s(szNgayRaoBan, sizeof(szNgayRaoBan), "1");

  if (CoinRaoBanChoTroi != NULL) {
    CoinRaoBanChoTroi->SetText("1");
  }

  if (NgayRaoBanChoTroi != NULL) {
    NgayRaoBanChoTroi->SetText("1");
  }

  if (PassChoTroi != NULL) {
    PassChoTroi->SetText("");
  }
}

static void ClearChoTroiSellCache(bool sendRollback) {
  if (sendRollback && gCusChoTroi.ItemCacheSelect != NULL) {
    XULY_CGPACKET pMsg;
    pMsg.header.set(0xD3, 0x12, sizeof(pMsg));
    pMsg.ThaoTac = 0;
    DataSend((LPBYTE)&pMsg, pMsg.header.size);
  }

  ReleaseChoTroiSellCacheItem();
  gCusChoTroi.ItemCacheSelect = NULL;
  gCusChoTroi.ItemCacheShow = false;
  gCusChoTroi.ItemCacheTime = 0;
  ChoTroiSellCacheSourceSlot = -1;
  CacheItemRaoBan = -1;
  ResetChoTroiSellInputs();
}

static void CloseChoTroiWindow(bool sendRollback) {
  HideChoTroiInputBoxes();
  gInterface->Data[eWindowChoTroi].Close();
  gInterface->Data[eWindowNhapPass].Close();
  ChoTroiNhapPassNeedFocus = false;
  ChoTroiNhapPassManualFocus = false;
  ChoTroiNhapPassReplaceAll = false;
  ClearChoTroiBuyConfirmState(true);
  memset(&szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi));
  if (NhapPassChoTroi != NULL) {
    NhapPassChoTroi->SetText("");
  }

  if (ShowWindowChoOK) {
    ShowWindowChoOK = false;
    SelectBarChoTroiNumber = 0;
    SelectBarChoTroi = false;
    WindowChoTroiScrollBar = NULL;
    m_TimTuKhoaChoTroi = NULL;
    m_TimTuKhoaChoTroi_Clear = NULL;
    m_PhotoViewChotroi.SetAutoupdatePlayer(TRUE);
    m_PhotoViewChotroi.CopyPlayer();
    BuyID = -1;
    gCusChoTroi.m_iNumCurOpenTab = 0;
    gCusChoTroi.m_TabBtn.ChangeFrame(gCusChoTroi.m_iNumCurOpenTab);
  }

  if (CacheItemRaoBan == 1 ||
      (gCusChoTroi.ItemCacheSelect != NULL && !gCusChoTroi.ItemCacheShow)) {
    ClearChoTroiSellCache(sendRollback);
  }

  RestoreChoTroiGameInputFocus();
}

static void UpdateChoTroiWindowAnchor(float &startX, float &startY,
                                      float windowW, float windowH) {
  const float gap = 5.0f;
  float targetX = startX;
  float targetY = startY;

  if (g_pNewUISystem != NULL && g_pMyInventory != NULL &&
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)) {
    const POINT &inventoryPos = g_pMyInventory->GetPos();
    float anchorX = (float)inventoryPos.x;
    targetY = (float)inventoryPos.y;

    if (g_pInvenExpansion != NULL &&
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY_EXTENSION)) {
      tagPOINT *expansionPos = g_pInvenExpansion->GetPos();
      if (expansionPos != NULL) {
        anchorX = (float)expansionPos->x;
      }
    }

    targetX = anchorX - windowW - gap;
  }

  if (targetX < 0.0f) {
    targetX = 0.0f;
  }

  if (targetY < 0.0f) {
    targetY = 0.0f;
  }

  startX = targetX;
  startY = targetY;
  gInterface->Data[eWindowChoTroi].X = targetX;
  gInterface->Data[eWindowChoTroi].Y = targetY;
  gInterface->Data[eWindowChoTroi].Width = windowW;
  gInterface->Data[eWindowChoTroi].Height = windowH;
  gInterface->Data[eWindowChoTroi].AllowMove = false;
  gInterface->Data[eWindowChoTroi].FirstLoad = true;
}

static bool IsChoTroiPendingSellCacheExpired(DWORD currentTick) {
  if (gCusChoTroi.ItemCacheSelect == NULL || gCusChoTroi.ItemCacheShow) {
    return false;
  }

  return (gCusChoTroi.ItemCacheTime == 0 ||
          (currentTick - gCusChoTroi.ItemCacheTime) > 1500);
}

static void ClearChoTroiExpiredPendingSellCache() {
  if (IsChoTroiPendingSellCacheExpired(GetTickCount())) {
    ClearChoTroiSellCache(true);
  }
}

static void HandleChoTroiInputFocus(CUITextInputBox *input, float x, float y,
                                    float width, float height) {
  if (input == NULL) {
    return;
  }

  if (SEASON3B::CheckMouseIn((int)x, (int)y, (int)width, (int)height)) {
    MouseOnWindow = true;
    g_dwMouseUseUIID = input->GetUIID();

    if (MouseLButtonPush || (GetKeyState(VK_LBUTTON) & 0x8000)) {
      input->GiveFocus(TRUE);
      if (MouseLButtonPush) {
        PlayBuffer(25, 0, 0);
      }
    }
  }
}

static void GetChoTroiNhapPassLayout(float &windowW, float &windowH,
                                     float &startX, float &startY,
                                     float &boxX, float &boxY, float &boxW,
                                     float &boxH, float &inputX,
                                     float &inputY) {
  windowW = 200.0f;
  windowH = 120.0f;
  startY = ((MAX_WIN_HEIGHT - 151) / 2) - (windowH / 2) + 50.0f;
  startX = (MAX_WIN_WIDTH / 2) - (windowW / 2);

  if (gInterface->Data[eWindowNhapPass].FirstLoad) {
    startX = gInterface->Data[eWindowNhapPass].X;
    startY = gInterface->Data[eWindowNhapPass].Y;
  }

  boxX = startX + (windowW / 2) - 70.0f;
  boxY = startY + 50.0f;
  boxW = 140.0f;
  boxH = 20.0f;
  inputX = boxX + 5.0f;
  inputY = boxY + 4.0f;
}

static void EnsureChoTroiNumberInput(CUITextInputBox *&input, HWND hWnd,
                                     int width, int height, int maxLength,
                                     const char *defaultText) {
  if (input != NULL) {
    input->SetState(UISTATE_NORMAL);
    return;
  }

  input = new CUITextInputBox;
  input->Init(hWnd, width, height, maxLength);
  input->SetBackColor(255, 0, 0, 0);
  input->SetTextColor(255, 255, 157, 0);
  input->SetFont((HFONT)g_hFont);
  input->SetState(UISTATE_NORMAL);
  input->SetOption(UIOPTION_NUMBERONLY);
  input->SetText(defaultText);
}

static void SetChoTroiBuyPassText(const char *text) {
  memset(szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi));
  if (text != NULL) {
    strncpy(szNhapPassChoTroi, text, sizeof(szNhapPassChoTroi) - 1);
  }

  if (NhapPassChoTroi != NULL) {
    NhapPassChoTroi->SetText(szNhapPassChoTroi);
  }
}

static void ConsumeChoTroiInputKey(int key) {
  SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
      key, SEASON3B::CNewKeyInput::KEY_REPEAT);
}

static void AppendChoTroiBuyPassDigit(char digit, int key) {
  if (ChoTroiNhapPassReplaceAll) {
    SetChoTroiBuyPassText("");
    ChoTroiNhapPassReplaceAll = false;
  }

  size_t len = strlen(szNhapPassChoTroi);
  if (len < sizeof(szNhapPassChoTroi) - 1) {
    szNhapPassChoTroi[len] = digit;
    szNhapPassChoTroi[len + 1] = 0;
    if (NhapPassChoTroi != NULL) {
      NhapPassChoTroi->SetText(szNhapPassChoTroi);
    }
  }

  ConsumeChoTroiInputKey(key);
}

static void UpdateChoTroiBuyPassKeyboard() {
  if (!ChoTroiNhapPassManualFocus) {
    return;
  }

  if (SEASON3B::IsPress(VK_BACK)) {
    if (ChoTroiNhapPassReplaceAll) {
      SetChoTroiBuyPassText("");
      ChoTroiNhapPassReplaceAll = false;
    } else {
      size_t len = strlen(szNhapPassChoTroi);
      if (len > 0) {
        szNhapPassChoTroi[len - 1] = 0;
        if (NhapPassChoTroi != NULL) {
          NhapPassChoTroi->SetText(szNhapPassChoTroi);
        }
      }
    }
    ConsumeChoTroiInputKey(VK_BACK);
    return;
  }

  for (int i = 0; i <= 9; i++) {
    int key = '0' + i;
    if (SEASON3B::IsPress(key)) {
      AppendChoTroiBuyPassDigit((char)key, key);
      return;
    }

    int numpadKey = VK_NUMPAD0 + i;
    if (SEASON3B::IsPress(numpadKey)) {
      AppendChoTroiBuyPassDigit((char)key, numpadKey);
      return;
    }
  }
}

static void RenderChoTroiBuyPassText(float x, float y, float width) {
  char displayText[sizeof(szNhapPassChoTroi) + 2] = {0};
  strncpy(displayText, szNhapPassChoTroi, sizeof(displayText) - 2);

  if (ChoTroiNhapPassManualFocus && ((GetTickCount() / 400) % 2) == 0) {
    size_t len = strlen(displayText);
    if (len < sizeof(displayText) - 1) {
      displayText[len] = '|';
      displayText[len + 1] = 0;
    }
  }

  SEASON3B::TextDraw((HFONT)g_hFont, x, y, 0xFFFF9DFF, 0x0, (int)width, 0, 1,
                     "%s", displayText);
}

static bool IsChoTroiItemTypeSafe(int itemType) {
  return (itemType >= 0 && itemType < MAX_ITEM);
}

static const char *ChoTroiGetItemName(int itemType) {
  if (ItemAttribute != NULL && ItemAttribute[itemType].Name[0] != 0) {
    return ItemAttribute[itemType].Name;
  }

  Script_Item *itemInfo = GMItemMng->find(itemType);
  if (itemInfo != NULL && itemInfo->Name[0] != 0) {
    return itemInfo->Name;
  }

  return "Item";
}

static void SendChoTroiBuyRequest(int id, int pass) {
  if (id < 0) {
    return;
  }

  CustomChoTroi::PMSG_REQ_MARKET_BUY pMsg;
  memset(&pMsg, 0, sizeof(pMsg));
  pMsg.h.set(0xD3, 0x21, sizeof(pMsg));
  pMsg.Result = 1;
  pMsg.ID = id;
  pMsg.Pass = pass;
  DataSend((LPBYTE)&pMsg, pMsg.h.size);
}

static const CustomChoTroi::MARKET_DATA *FindChoTroiMarketDataByID(int id) {
  for (size_t n = 0; n < gCusChoTroi.m_DataChoTroi.size(); n++) {
    if (gCusChoTroi.m_DataChoTroi[n].ID == id) {
      return &gCusChoTroi.m_DataChoTroi[n];
    }
  }

  return NULL;
}

static void ChoTroiConfirmBuyCallback(LPVOID lpParam) {
  if (ChoTroiPendingBuyID >= 0) {
    SendChoTroiBuyRequest(ChoTroiPendingBuyID, ChoTroiPendingBuyPass);
  }

  ClearChoTroiBuyConfirmState(false);
}

static bool OpenChoTroiBuyConfirm(const CustomChoTroi::MARKET_DATA &marketData,
                                  int pass) {
  if (gInterface->Data[eWindowMessageBox].OnShow) {
    return false;
  }

  char itemName[128] = {0};
  ITEM *item = g_pNewItemMng->CreateItem((BYTE *)marketData.Item);

  if (item != NULL && IsChoTroiItemTypeSafe(item->Type)) {
    strncpy_s(itemName, sizeof(itemName), ChoTroiGetItemName(item->Type),
              _TRUNCATE);
  } else {
    strncpy_s(itemName, sizeof(itemName), "Item", _TRUNCATE);
  }

  DeleteChoTroiTempItem(item);

  const char *priceName = "Coin";
  if (marketData.PriceType >= 1 && marketData.PriceType <= 6) {
    priceName = NameGiaCoin[marketData.PriceType];
  }

  char message[256] = {0};
  sprintf_s(message, sizeof(message),
            "Bạn có chắc muốn mua [%s]\nGiá: %s %s", itemName,
            gInterface->NumberFormat(marketData.Price), priceName);

  ChoTroiPendingBuyID = marketData.ID;
  ChoTroiPendingBuyPass = pass;
  ChoTroiBuyConfirmOpen = true;
  gInterface->OpenMessageBoxOkCancel(ChoTroiConfirmBuyCallback,
                                     (char *)"[Chợ Trời] Xác nhận mua", "%s",
                                     message);
  return true;
}

static bool ChoTroiCanPreviewEquip(int itemType) {
  int baseClass = gCharacterManager.GetBaseClass(Hero->Class);

  if (ItemAttribute != NULL) {
    return (ItemAttribute[itemType].RequireClass[baseClass] != 0);
  }

  Script_Item *itemInfo = GMItemMng->find(itemType);
  return (itemInfo != NULL && itemInfo->RequireClass[baseClass] != 0);
}

static int ChoTroiExtractClientItemType(const BYTE *item) {
  if (item == NULL) {
    return -1;
  }

  return 32 * (item[5] & 0xF0) + item[0] + 2 * (item[3] & 0x80);
}

static void ChoTroiDbItemToClientItem(const BYTE *dbItem, BYTE *clientItem) {
  if (dbItem == NULL || clientItem == NULL) {
    return;
  }

  memset(clientItem, 0xFF, MARKET_ITEM_BUFFER);
  clientItem[0] = dbItem[0];
  clientItem[1] = dbItem[1];
  clientItem[2] = dbItem[2];
  clientItem[3] = dbItem[7];
  clientItem[4] = dbItem[8];
  clientItem[5] = dbItem[9];
  clientItem[6] = dbItem[10];
  memcpy(&clientItem[7], &dbItem[11], 5);
}

static void ChoTroiNormalizeClientItem(BYTE *item) {
  if (item == NULL) {
    return;
  }

  if (IsChoTroiItemTypeSafe(ChoTroiExtractClientItemType(item))) {
    return;
  }

  BYTE clientItem[MARKET_ITEM_BUFFER];
  ChoTroiDbItemToClientItem(item, clientItem);

  if (IsChoTroiItemTypeSafe(ChoTroiExtractClientItemType(clientItem))) {
    memcpy(item, clientItem, MARKET_ITEM_BUFFER);
  }
}

static int ChoTroiReadInt(const BYTE *data) {
  int value = 0;
  memcpy(&value, data, sizeof(value));
  return value;
}

static int ChoTroiGetPacketSize(BYTE *recv, int size) {
  if (size > 0) {
    return size;
  }

  if (recv == NULL) {
    return 0;
  }

  if (recv[0] == 0xC1 || recv[0] == 0xC3) {
    return recv[1];
  }

  if (recv[0] == 0xC2 || recv[0] == 0xC4) {
    return MAKE_NUMBERW(recv[1], recv[2]);
  }

  return 0;
}

static bool ChoTroiTryPacketLayout(BYTE *recv, int packetSize, int countOffset,
                                   int rowSize, int *count, int *type) {
  const int dataOffset = countOffset + (int)(sizeof(int) * 2);

  if (recv == NULL || count == NULL || type == NULL) {
    return false;
  }

  if (packetSize > 0 && dataOffset > packetSize) {
    return false;
  }

  int packetCount = ChoTroiReadInt(recv + countOffset);
  if (packetCount < 0 || packetCount > MARKET_ITEM_MAX) {
    return false;
  }

  if (packetSize > 0 && dataOffset + (packetCount * rowSize) > packetSize) {
    return false;
  }

  *count = packetCount;
  *type = ChoTroiReadInt(recv + countOffset + sizeof(int));
  return true;
}

CustomChoTroi::CustomChoTroi() {
  gCusChoTroi.mListItemFind.clear();
  gCusChoTroi.m_DataChoTroi.clear();
  m_iNumCurOpenTab = 0;
  ItemCacheSelect = NULL;
  ItemCacheTime = 0;
  ItemCacheShow = 0;
}

CustomChoTroi::~CustomChoTroi() {}

void CustomChoTroi::UpdateInputFocus() {
  if (!gInterface->Data[eWindowChoTroi].OnShow) {
    if (ChoTroiBlockEscapeUntilRelease &&
        ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0)) {
      ChoTroiBlockEscapeUntilRelease = false;
    }
    return;
  }

  if (SEASON3B::IsPress(VK_ESCAPE) && HandleEscapeKey()) {
    return;
  }

  const float WindowX = 0.0f;
  const float WindowY = 20.0f;
  const float WindowW = 400.0f;
  const float WindowH = 320.0f;

  if (SEASON3B::CheckMouseIn((int)WindowX, (int)WindowY, (int)WindowW,
                             (int)WindowH)) {
    MouseOnWindow = true;
  }

  HWND pGameWindow = gwinhandle->GethWnd();

  if (gInterface->Data[eWindowNhapPass].OnShow) {
    HideChoTroiSellInputBoxes();

    float WindowWP = 0.0f;
    float WindowHP = 0.0f;
    float StartXP = 0.0f;
    float StartYP = 0.0f;
    float NhapPassBoxX = 0.0f;
    float NhapPassBoxY = 0.0f;
    float NhapPassBoxW = 0.0f;
    float NhapPassBoxH = 0.0f;
    float NhapPassX = 0.0f;
    float NhapPassY = 0.0f;
    GetChoTroiNhapPassLayout(WindowWP, WindowHP, StartXP, StartYP,
                             NhapPassBoxX, NhapPassBoxY, NhapPassBoxW,
                             NhapPassBoxH, NhapPassX, NhapPassY);

    const bool isMouseInNhapPassWindow =
        SEASON3B::CheckMouseIn((int)StartXP, (int)StartYP, (int)WindowWP,
                               (int)WindowHP);
    if (isMouseInNhapPassWindow) {
      MouseOnWindow = true;
    }

    SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);

    const bool isMouseInNhapPassInput =
        SEASON3B::CheckMouseIn((int)NhapPassBoxX, (int)NhapPassBoxY,
                               (int)NhapPassBoxW, (int)NhapPassBoxH);
    if (isMouseInNhapPassInput) {
      MouseOnWindow = true;
      if (MouseLButtonPush || (GetKeyState(VK_LBUTTON) & 0x8000)) {
        ChoTroiNhapPassManualFocus = true;
        ChoTroiNhapPassReplaceAll = true;
        if (MouseLButtonPush) {
          PlayBuffer(25, 0, 0);
        }
      }
    }

    if (ChoTroiNhapPassNeedFocus) {
      ChoTroiNhapPassManualFocus = true;
      ChoTroiNhapPassReplaceAll = true;
      ChoTroiNhapPassNeedFocus = false;
    }

    if (ChoTroiNhapPassManualFocus && GetFocus() != pGameWindow) {
      SetFocus(pGameWindow);
    }

    UpdateChoTroiBuyPassKeyboard();
    return;
  }

  SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);

  const bool isChoTroiSellTab =
      (m_iNumCurOpenTab == 1 || m_TabBtn.GetCurButtonIndex() == 1);

  if (isChoTroiSellTab && ItemCacheSelect != NULL && ItemCacheShow) {
    const float StartX = WindowX + 215.0f;
    const float StartY = WindowY + 65.0f;
    const int ChoTroiInputWidth = 70;
    const int ChoTroiInputHeight = 14;
    const float ChoTroiInputBoxX = StartX + 10.0f;
    const float ChoTroiInputBoxW = 140.0f;
    const float ChoTroiInputBoxH = 20.0f;

    float TCoinX = StartX + 80.0f;
    float TCoinY = StartY + 154.0f;
    const float TCoinBoxY = StartY + 150.0f;
    EnsureChoTroiNumberInput(CoinRaoBanChoTroi, pGameWindow,
                             ChoTroiInputWidth, ChoTroiInputHeight, 8, "1");
    CoinRaoBanChoTroi->SetPosition(TCoinX, TCoinY);
    CoinRaoBanChoTroi->DoAction();
    HandleChoTroiInputFocus(CoinRaoBanChoTroi, ChoTroiInputBoxX, TCoinBoxY,
                            ChoTroiInputBoxW, ChoTroiInputBoxH);

    float NgayX = StartX + 80.0f;
    float NgayY = StartY + 174.0f;
    const float NgayBoxY = StartY + 170.0f;
    EnsureChoTroiNumberInput(NgayRaoBanChoTroi, pGameWindow,
                             ChoTroiInputWidth, ChoTroiInputHeight, 2, "1");
    NgayRaoBanChoTroi->SetPosition(NgayX, NgayY);
    NgayRaoBanChoTroi->DoAction();
    HandleChoTroiInputFocus(NgayRaoBanChoTroi, ChoTroiInputBoxX, NgayBoxY,
                            ChoTroiInputBoxW, ChoTroiInputBoxH);

    float PassX = StartX + 80.0f;
    float PassY = StartY + 194.0f;
    const float PassBoxY = StartY + 190.0f;
    EnsureChoTroiNumberInput(PassChoTroi, pGameWindow, ChoTroiInputWidth,
                             ChoTroiInputHeight, 6, "");
    PassChoTroi->SetPosition(PassX, PassY);
    PassChoTroi->DoAction();
    HandleChoTroiInputFocus(PassChoTroi, ChoTroiInputBoxX, PassBoxY,
                            ChoTroiInputBoxW, ChoTroiInputBoxH);
  } else {
    HideChoTroiSellInputBoxes();
  }

}

void DrawInfoPhai(int X, int Y) {

  switch (gCusChoTroi.m_iNumCurOpenTab) {
  case 0: {
    //=================== Khung Char
    float WindowHoatAnhW = 160;
    float ViewCharX = X;
    float ViewCharY = Y;
    if (gInterface->DrawButton(ViewCharX + 48, ViewCharY + 95, 100, 12,
                               gTextClient.txtClient_ChoTroi[40], 55)) {
      m_PhotoViewChotroi.SetAutoupdatePlayer(TRUE);
      m_PhotoViewChotroi.CopyPlayer();
    }
    m_PhotoViewChotroi.SetPosition(ViewCharX + 15, ViewCharY - 35);
    m_PhotoViewChotroi.Render();
    m_PhotoViewChotroi.DoMouseAction();

    //=================== Khung Loc List
    float LocListItemX = X;
    float LocListItemY = Y + 120;
    float LocListH = 100;
    gInterface->DrawInfoBox(LocListItemX + 7, LocListItemY,
                            (WindowHoatAnhW - 25), LocListH, 0x00000096, 0);
    SEASON3B::TextDraw((HFONT)g_hFontBold, LocListItemX + 9, LocListItemY + 2,
                       0xFFFFFFFF, 0x00BFFF96, WindowHoatAnhW - 21, 0, 3,
                       gTextClient.txtClient_ChoTroi[21]); //
    float CheckBoxX = LocListItemX + 7;
    float CheckBoxY = LocListItemY + 13;
    //==============
    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY, 0xFFCC00C8,
                                   (LocItemTypCoin & 1) == 1 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[1])) {
      if ((LocItemTypCoin & 1) == 1) {
        LocItemTypCoin -= 1;
      } else {
        LocItemTypCoin += 1;
      }
    }

    if (gInterface->RenderCheckBox(CheckBoxX + 50, CheckBoxY, 0xFFCC00C8,
                                   (LocItemTypCoin & 2) == 2 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[2])) {
      if ((LocItemTypCoin & 2) == 2) {
        LocItemTypCoin -= 2;
      } else {
        LocItemTypCoin += 2;
      }
    }
    if (gInterface->RenderCheckBox(CheckBoxX + (50 * 2), CheckBoxY, 0xFFCC00C8,
                                   (LocItemTypCoin & 4) == 4 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[3])) {
      if ((LocItemTypCoin & 4) == 4) {
        LocItemTypCoin -= 4;
      } else {
        LocItemTypCoin += 4;
      }
    }

    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY + 15, 0xFFCC00C8,
                                   (LocItemTypCoin & 8) == 8 ? TRUE : FALSE,
                                   "Bless")) {
      if ((LocItemTypCoin & 8) == 8) {
        LocItemTypCoin -= 8;
      } else {
        LocItemTypCoin += 8;
      }
    }

    if (gInterface->RenderCheckBox(CheckBoxX + 50, CheckBoxY + 15, 0xFFCC00C8,
                                   (LocItemTypCoin & 16) == 16 ? TRUE : FALSE,
                                   "Soul")) {
      if ((LocItemTypCoin & 16) == 16) {
        LocItemTypCoin -= 16;
      } else {
        LocItemTypCoin += 16;
      }
    }
    if (gInterface->RenderCheckBox(
            CheckBoxX + (50 * 2), CheckBoxY + 15, 0xFFCC00C8,
            (LocItemTypCoin & 32) == 32 ? TRUE : FALSE, "Chaos")) {
      if ((LocItemTypCoin & 32) == 32) {
        LocItemTypCoin -= 32;
      } else {
        LocItemTypCoin += 32;
      }
    }

    //	gInterface->DrawMessage(1, "LocItemTypCoin %d",LocItemTypCoin);
    CheckBoxY += 18;
    //===========
    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 1), 0xFFCC00C8,
                                   LocItemCoSkill == 1 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[22])) {
      LocItemCoSkill ^= 1;
    }
    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 2), 0xFFCC00C8,
                                   LocItemCoLuck == 1 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[23])) {
      LocItemCoLuck ^= 1;
    }
    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 3), 0xFFCC00C8,
                                   LocItemCoOpt == 1 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[24])) {
      LocItemCoOpt ^= 1;
    }
    if (gInterface->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 4), 0xFFCC00C8,
                                   LocItemCoExc == 1 ? TRUE : FALSE,
                                   gTextClient.txtClient_ChoTroi[25])) {
      LocItemCoExc ^= 1;
    }
  } break;
  case 1: // Rao Ban
  {
    //==============
    float StartX = X;
    float StartY = Y;
    float WindowW = 180;
    float WindowH = 250;
    int GetInfoLoadItem = 0;
    const float SlotX = StartX + (WindowW / 2) - 30;
    const float SlotY = StartY + 40;
    const float SlotW = 55;
    const float SlotH = 55;

    if (gCusChoTroi.ItemCacheSelect == NULL && gCusChoTroi.ItemCacheShow) {
      ClearChoTroiSellCache(false);
    }
    ClearChoTroiExpiredPendingSellCache();

    const bool HasSellCache =
        (gCusChoTroi.ItemCacheSelect != NULL && gCusChoTroi.ItemCacheShow);

    //===Draw Button Buy
    float ButtonW = 100;
    gInterface->DrawInfoBox(SlotX - 15, SlotY - 20, SlotW, SlotH,
                            (HoverItemSell ? 0x66646496 : 0x00000096), 0);
    float CheckBoxX = StartX + (WindowW / 2) - 90;

    if (HasSellCache) {
      if (gInterface->RenderCheckBox(CheckBoxX, StartY + 110, 0xFFCC00C8,
                                     TypeCoinBan == 1 ? TRUE : FALSE,
                                     gTextClient.txtClient_ChoTroi[1])) {
        if (gCusChoTroi.OnCointType & 1)
          TypeCoinBan = 1;
      }

      if (gInterface->RenderCheckBox(CheckBoxX + 50, StartY + 110, 0xFFCC00C8,
                                     TypeCoinBan == 2 ? TRUE : FALSE,
                                     gTextClient.txtClient_ChoTroi[2])) {
        if (gCusChoTroi.OnCointType & 2)
          TypeCoinBan = 2;
      }
      if (gInterface->RenderCheckBox(CheckBoxX + (50 * 2), StartY + 110,
                                     0xFFCC00C8,
                                     TypeCoinBan == 3 ? TRUE : FALSE,
                                     gTextClient.txtClient_ChoTroi[3])) {
        if (gCusChoTroi.OnCointType & 4)
          TypeCoinBan = 3;
      }

      if (gInterface->RenderCheckBox(CheckBoxX, StartY + 130, 0xFFCC00C8,
                                     TypeCoinBan == 4 ? TRUE : FALSE, "Bless")) {
        if (gCusChoTroi.OnCointType & 8)
          TypeCoinBan = 4;
      }

      if (gInterface->RenderCheckBox(CheckBoxX + (50 * 1), StartY + 130,
                                     0xFFCC00C8,
                                     TypeCoinBan == 5 ? TRUE : FALSE, "Soul")) {
        if (gCusChoTroi.OnCointType & 16)
          TypeCoinBan = 5;
      }

      if (gInterface->RenderCheckBox(CheckBoxX + (50 * 2), StartY + 130,
                                     0xFFCC00C8,
                                     TypeCoinBan == 6 ? TRUE : FALSE, "Chaos")) {
        if (gCusChoTroi.OnCointType & 32)
          TypeCoinBan = 6;
      }

      //Ngày bán
      RenderBitmap(SEASON3B::CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
                   StartX + (WindowW / 2) - 90, StartY + 150, 140, 20, 0.0, 0.0,
                   0.82, 0.71, 1, 1, 0.0);
      SEASON3B::TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 90,
                         StartY + 154, 0xFFFFFFC8, 0x0, 70, 0, 3,
                         gTextClient.txtClient_ChoTroi[4]);
      const int ChoTroiInputWidth = 70;
      const int ChoTroiInputHeight = 14;
      const float ChoTroiInputBoxX = StartX + (WindowW / 2) - 90;
      const float ChoTroiInputBoxW = 140;
      const float ChoTroiInputBoxH = 20;
      float TCoinX = StartX + (WindowW / 2) - 30;
      float TCoinY = StartY + 154;
      const float TCoinBoxY = StartY + 100;
      if (!CoinRaoBanChoTroi) {
        CoinRaoBanChoTroi = new CUITextInputBox;
        CoinRaoBanChoTroi->Init(pGameWindow, ChoTroiInputWidth,
                                ChoTroiInputHeight, 8);
        CoinRaoBanChoTroi->SetBackColor(255, 0, 0, 0);
        CoinRaoBanChoTroi->SetTextColor(255, 255, 157, 0);
        CoinRaoBanChoTroi->SetFont((HFONT)g_hFont);
        CoinRaoBanChoTroi->SetState(UISTATE_NORMAL);
        CoinRaoBanChoTroi->SetOption(UIOPTION_NUMBERONLY);
        CoinRaoBanChoTroi->SetPosition(TCoinX, TCoinY);
        CoinRaoBanChoTroi->SetText("1");
        strcpy_s(szGiaTriCoin, sizeof(szGiaTriCoin), "1");
      } else {
        CoinRaoBanChoTroi->SetState(UISTATE_NORMAL);
        CoinRaoBanChoTroi->SetPosition(TCoinX, TCoinY);
        CoinRaoBanChoTroi->Render();
        CoinRaoBanChoTroi->DoAction();
        HandleChoTroiInputFocus(CoinRaoBanChoTroi, ChoTroiInputBoxX,
                                TCoinBoxY, ChoTroiInputBoxW,
                                ChoTroiInputBoxH);
        CoinRaoBanChoTroi->GetText(szGiaTriCoin, 8 + 1);
      }
      //HSD
      RenderBitmap(SEASON3B::CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
                   StartX + (WindowW / 2) - 90, StartY + 170, 140, 20, 0.0, 0.0,
                   0.82, 0.71, 1, 1, 0.0);
      SEASON3B::TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 90,
                         StartY + 174, 0xFFFFFFC8, 0x0, 70, 0, 3,
                         gTextClient.txtClient_ChoTroi[43]);
      float NTCoinX = StartX + (WindowW / 2) - 30;
      float NTCoinY = StartY + 174;
      const float NTCoinBoxY = StartY + 170;
      if (!NgayRaoBanChoTroi) {
        NgayRaoBanChoTroi = new CUITextInputBox;
        NgayRaoBanChoTroi->Init(pGameWindow, ChoTroiInputWidth,
                                ChoTroiInputHeight, 2);
        NgayRaoBanChoTroi->SetBackColor(255, 0, 0, 0);
        NgayRaoBanChoTroi->SetTextColor(255, 255, 157, 0);
        NgayRaoBanChoTroi->SetFont((HFONT)g_hFont);
        NgayRaoBanChoTroi->SetState(UISTATE_NORMAL);
        NgayRaoBanChoTroi->SetOption(UIOPTION_NUMBERONLY);
        NgayRaoBanChoTroi->SetPosition(NTCoinX, NTCoinY);
        NgayRaoBanChoTroi->SetText("1");
        strcpy_s(szNgayRaoBan, sizeof(szNgayRaoBan), "1");
      } else {
        NgayRaoBanChoTroi->SetState(UISTATE_NORMAL);
        NgayRaoBanChoTroi->SetPosition(NTCoinX, NTCoinY);
        NgayRaoBanChoTroi->Render();
        NgayRaoBanChoTroi->DoAction();
        HandleChoTroiInputFocus(NgayRaoBanChoTroi, ChoTroiInputBoxX,
                                NTCoinBoxY, ChoTroiInputBoxW,
                                ChoTroiInputBoxH);
        NgayRaoBanChoTroi->GetText(szNgayRaoBan, 2 + 1);
        if (atoi(szNgayRaoBan) > 30) {
          NgayRaoBanChoTroi->SetText("30");
        } else if (atoi(szNgayRaoBan) < 1) {
          NgayRaoBanChoTroi->SetText("1");
        }
      }

      //Pass
      RenderBitmap(SEASON3B::CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
                   StartX + (WindowW / 2) - 90, StartY + 190, 140, 20, 0.0, 0.0,
                   0.82, 0.71, 1, 1, 0.0);
      SEASON3B::TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 90,
                         StartY + 194, 0xFFFFFFC8, 0x0, 70, 0, 3, "Pass:");
      float NTCoinX2 = StartX + (WindowW / 2) - 30;
      float NTCoinY2 = StartY + 194;
      const float NTCoinBoxY2 = StartY + 190;
      if (!PassChoTroi) {
        PassChoTroi = new CUITextInputBox;
        PassChoTroi->Init(pGameWindow, ChoTroiInputWidth,
                          ChoTroiInputHeight, 6);
        PassChoTroi->SetBackColor(255, 0, 0, 0);
        PassChoTroi->SetTextColor(255, 255, 157, 0);
        PassChoTroi->SetFont((HFONT)g_hFont);
        PassChoTroi->SetState(UISTATE_NORMAL);
        PassChoTroi->SetOption(UIOPTION_NUMBERONLY);
        PassChoTroi->SetPosition(NTCoinX2, NTCoinY2);
        PassChoTroi->SetText("");
      } else {
        PassChoTroi->SetState(UISTATE_NORMAL);
        PassChoTroi->SetPosition(NTCoinX2, NTCoinY2);
        PassChoTroi->Render();
        PassChoTroi->DoAction();
        HandleChoTroiInputFocus(PassChoTroi, ChoTroiInputBoxX, NTCoinBoxY2,
                                ChoTroiInputBoxW, ChoTroiInputBoxH);
        PassChoTroi->GetText(szPassChoTroi, 5 + 1);
      }

      const float SellButtonX = StartX + (WindowW / 2) - (ButtonW / 2);
      const float SellButtonY = StartY + WindowH - 30;
      const float SellButtonW = 110.0f;
      const float SellButtonH = (SellButtonW * 20.0f) / 100.0f;
      bool confirmSell =
          gInterface->DrawButton(SellButtonX, SellButtonY, SellButtonW, 12,
                                 gTextClient.txtClient_ChoTroi[5]);

      if (!confirmSell &&
          SEASON3B::CheckMouseIn(SellButtonX, SellButtonY, SellButtonW,
                                 SellButtonH) &&
          SEASON3B::IsRelease(VK_LBUTTON) &&
          GetTickCount() - ChoTroiLastSellConfirmTick > 500) {
        ChoTroiLastSellConfirmTick = GetTickCount();
        PlayBuffer(25, 0, 0);
        confirmSell = true;
      }

      if (confirmSell) {
        ChoTroiLastSellConfirmTick = GetTickCount();
        if (atoi(szGiaTriCoin) == 0) {
          gInterface->DrawMessage(1, "Cho Troi: Hay nhap gia ban.");
          return;
        }
        if (!IsChoTroiCoinTypeEnabled(TypeCoinBan)) {
          gInterface->DrawMessage(1, "Cho Troi: Hay chon loai tien dang bat.");
          return;
        }
        // gInterface->DrawMessage(1, "Coint %d", atoi(szGiaTriCoin));
        //
        // gInterface->DrawMessage(1, "Coint %d", TypeCoinBan);
        CustomChoTroi::PMSG_REQ_MARKET_SELL pMsg;
        int itemDay = atoi(szNgayRaoBan);
        if (itemDay < 1) {
          itemDay = 1;
        } else if (itemDay > 30) {
          itemDay = 30;
        }
        pMsg.h.set(0xD3, 0x14, sizeof(pMsg));
        pMsg.Result = 1;
        pMsg.ItemPos = 0;
        pMsg.ItemPriceType = TypeCoinBan;
        pMsg.ItemPrice = atoi(szGiaTriCoin);
        pMsg.ItemDay = itemDay;            // HSD Them
        pMsg.Pass = (szPassChoTroi[0] == 0) ? -1 : atoi(szPassChoTroi);
        memset(&szGiaTriCoin, 0, sizeof(szGiaTriCoin));
        memset(&szNgayRaoBan, 0, sizeof(szNgayRaoBan));   // HSD Them
        memset(&szPassChoTroi, 0, sizeof(szPassChoTroi)); // HSD Them
        // gInterface->DrawMessage(1, "%d",pMsg.Pass);
        CoinRaoBanChoTroi->SetText("1");
        NgayRaoBanChoTroi->SetText("1"); // HSD Them
        PassChoTroi->SetText("");
        DataSend((LPBYTE)&pMsg, pMsg.h.size);
      }

      //======SHow INFO ITEM Rao Ban

      if (CacheItemRaoBan == -1) {
        CacheItemRaoBan = 1;
      }

      //Item trong ô Rao Bán
      g_pNewUISystem->RenderItem3DFree(
          SlotX - 10, SlotY - 45, SlotW, SlotH,
          gCusChoTroi.ItemCacheSelect->Type, gCusChoTroi.ItemCacheSelect->Level,
          gCusChoTroi.ItemCacheSelect->Option1,
          gCusChoTroi.ItemCacheSelect->ExtOption, 0, 1.4); // BMD MOdel
    } else {
      SEASON3B::TextDraw((HFONT)g_hFontBold, StartX - 13, StartY + 120, 0x0,
                         0x000000CC, 50, 0, 3, ""); // Name Item
      SEASON3B::TextDraw((HFONT)g_hFontBold, StartX - 14, StartY + 120, 0xFFFFFFFF,
                         0xFFA20096, WindowW, 0, 3,
                         gTextClient.txtClient_ChoTroi[6]); // Name Item
      SEASON3B::TextDraw((HFONT)g_hFont, StartX - 13, StartY + 120 + 15, 0xFFA200FF,
                         0x0, WindowW, 0, 3,
                         gTextClient.txtClient_ChoTroi[7]); // Name Item
      SEASON3B::TextDraw((HFONT)g_hFont, StartX - 13, StartY + 120 + 30, 0xA2FF00FF,
                         0x0, WindowW, 0, 3,
                         gTextClient.txtClient_ChoTroi[8]); // Name Item
    }
    //===
    if (SEASON3B::CheckMouseIn(SlotX, SlotY, SlotW, SlotH)) {
      HoverItemSell = true;
      //===Show
      if (HasSellCache) {
        RenderItemInfo(MouseX + 75, MouseY, gCusChoTroi.ItemCacheSelect, 0, 0,
                       false, false);
        if (SEASON3B::IsRelease(VK_RBUTTON)) {
          PlayBuffer(25, 0, 0);
          ClearChoTroiSellCache(true);
        }
      } else {
        if (SEASON3B::IsRelease(VK_LBUTTON)) {
          SEASON3B::CNewUIPickedItem *pPickedItem =
              g_pMyInventory->GetInventoryCtrl(0)->GetPickedItem();
          if (!pPickedItem)
            return;
          ITEM *ItemSell = pPickedItem->GetItem();
          int Slot = pPickedItem->GetSourceLinealPos();
          SEASON3B::CNewUIInventoryCtrl *pOwnerInventory =
              pPickedItem->GetOwnerInventory();
          if (g_pMyInventory != NULL && pOwnerInventory != NULL) {
            for (int i = 0; i < g_pMyInventory->GetInvenEnableCnt(); ++i) {
              if (g_pMyInventory->GetInventoryCtrl(i) == pOwnerInventory) {
                Slot = g_pMyInventory->GetInvenSlotIndex(ItemSell->x,
                                                         ItemSell->y, i);
                break;
              }
            }
          }
          if (gCusChoTroi.SendItemRaoBan(ItemSell, Slot, TRUE))
            pPickedItem->HidePickedItem();
        }
      }
    } else {
      HoverItemSell = false;
    }
  } break;
  default:
    break;
  }
}

void DrawWindowChoTroi() {
  UpdateChoTroiBuyConfirmState();

  if (gInterface->CheckWindow(CB_Interface::ObjWindow::MoveList) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::CashShop) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::SkillTree) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::FullMap) ||
      (gInterface->CheckWindow(CB_Interface::Inventory) &&
       gInterface->CheckWindow(CB_Interface::ExpandInventory) &&
       gInterface->CheckWindow(CB_Interface::Store)) ||
      (gInterface->CheckWindow(CB_Interface::Inventory) &&
       gInterface->CheckWindow(CB_Interface::Warehouse) &&
       gInterface->CheckWindow(CB_Interface::ExpandWarehouse))) {
    if (gInterface->Data[eWindowChoTroi].OnShow || ShowWindowChoOK ||
        CacheItemRaoBan == 1 ||
        (gCusChoTroi.ItemCacheSelect != NULL && !gCusChoTroi.ItemCacheShow) ||
        gInterface->Data[eWindowNhapPass].OnShow) {
      CloseChoTroiWindow(true);
    }
    return;
  }

  if (!gInterface->Data[eWindowChoTroi].OnShow) {
    if (ShowWindowChoOK || CacheItemRaoBan == 1 ||
        (gCusChoTroi.ItemCacheSelect != NULL && !gCusChoTroi.ItemCacheShow) ||
        gInterface->Data[eWindowNhapPass].OnShow) {
      CloseChoTroiWindow(true);
    }

    return;
  }
  if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)) {
    g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
  }
  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)) {
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
  }

  float StartX = 0;
  float StartY = 20;
  float WindowW = 410;
  float WindowH = 320;

  UpdateChoTroiWindowAnchor(StartX, StartY, WindowW, WindowH);
  gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH,
                                eWindowChoTroi,
                                gTextClient.txtClient_ChoTroi[10]);
  if (!gInterface->Data[eWindowChoTroi].OnShow) {
    CloseChoTroiWindow(true);
    return;
  }

  //=== List Item
  float ListItemX = StartX + 10;
  float ListItemY = StartY + 60;
  float ListItemWH = 65;
  float ListItemCachX = 205;
  float ListItemCachY = 15;
  int countx = 0;
  int county = 0;

  int DataListItem = gCusChoTroi.m_DataChoTroi.size();
  int MaxScrollPage = GetChoTroiMaxScrollPage(DataListItem, MaxPerPageChoTroi);
  // int DataListItem = 6;
  const int ScrollBarX = (int)(StartX + ListItemCachX);
  const int ScrollBarY = (int)(StartY + 65);
  const int ScrollBarH = (int)(WindowH - 100);

  ////===ScrollBar
  if (WindowChoTroiScrollBar == NULL) {
    WindowChoTroiScrollBar = new SEASON3B::CNewUIScrollBar();
    WindowChoTroiScrollBar->Create(ScrollBarX, ScrollBarY, ScrollBarH);
    WindowChoTroiScrollBar->SetMaxPos(MaxScrollPage);
    WindowChoTroiScrollBar->SetCurPos(0);
  }

  if (WindowChoTroiScrollBar != NULL) {
    const int currentScrollPos = WindowChoTroiScrollBar->GetCurPos();
    WindowChoTroiScrollBar->SetMaxPos(MaxScrollPage);
    WindowChoTroiScrollBar->SetCurPos(currentScrollPos);
    if (WindowChoTroiScrollBar->GetCurPos() > MaxScrollPage) {
      WindowChoTroiScrollBar->SetCurPos(MaxScrollPage);
    }
    if (MaxScrollPage <= 0) {
      WindowChoTroiScrollBar->SetCurPos(0);
    }
    UpdateMaxPosChoTroi = false;

    if (gInterface->Data[eWindowChoTroi].OnClick) {
      SetChoTroiScrollBarPosition(WindowChoTroiScrollBar, ScrollBarX,
                                  ScrollBarY);
    }

    if (MaxScrollPage > 0) {
      HandleChoTroiScrollWheel(WindowChoTroiScrollBar, ListItemX - 5,
                               StartY + 37, WindowW - 185, WindowH - 100);
      WindowChoTroiScrollBar->UpdateMouseEvent();
      WindowChoTroiScrollBar->Update();
      WindowChoTroiScrollBar->Render();
    }
  }

  //==List Item
  ShowInfoItem = -1;
  mPageChoTroi =
      (MaxScrollPage > 0 && WindowChoTroiScrollBar != NULL)
          ? WindowChoTroiScrollBar->GetCurPos()
          : 0;
  if (mPageChoTroi > MaxScrollPage) {
    mPageChoTroi = MaxScrollPage;
    if (WindowChoTroiScrollBar != NULL) {
      WindowChoTroiScrollBar->SetCurPos(MaxScrollPage);
    }
  }
  int CountList = 0;
  int SkipCreateItem = 0;
  int SkipUnsafeType = 0;
  int SkipFilter = 0;
  for (int n = (mPageChoTroi * MaxPerPageChoTroi);
       n < gCusChoTroi.m_DataChoTroi.size(); n++) {
    float KhungInfoX = ListItemX + ((ListItemWH + ListItemCachX) * countx);
    float KhungInfoY = ListItemY + ((ListItemWH + ListItemCachY) * county);
    DWORD BGList = 0x00000096;
    // int GetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36),
    // gInterface->m_DataChoTroi[n].Item);
    ITEM *CTItem = g_pNewItemMng->CreateItem(gCusChoTroi.m_DataChoTroi[n].Item);
    if (!CTItem) {
      SkipCreateItem++;
      continue;
    }
    int CTItemIndex = CTItem->Type;
    if (!IsChoTroiItemTypeSafe(CTItemIndex)) {
      SkipUnsafeType++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }

    BYTE m_Skill = (gCusChoTroi.m_DataChoTroi[n].Item[1] / 128) & 1;

    BYTE m_Luck = (gCusChoTroi.m_DataChoTroi[n].Item[1] / 4) & 1;

    BYTE m_Opt = (gCusChoTroi.m_DataChoTroi[n].Item[1] & 3) +
                 ((gCusChoTroi.m_DataChoTroi[n].Item[7] & 64) / 16);
    //==SHow Info Item

    if (SEASON3B::CheckMouseIn(KhungInfoX, KhungInfoY, ListItemWH,
                               ListItemWH) &&
        !SelectBarChoTroi) {
      ShowInfoItem = n;
      BGList = 0x66646496;
      if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) // Click
      {
        if (GetKeyState(VK_LBUTTON) & 0x8000) {
          gInterface->Data[eTIME].EventTick = GetTickCount();
          // if (m_TimTuKhoaChoTroi)m_TimTuKhoaChoTroi_Clear->GiveFocus(1);
        }
      }
    } else {
      BGList = 0x00000096;
    }
    std::map<int, char *>::iterator it =
        gCusChoTroi.mListItemFind.find(CTItemIndex);
    //=== Lọc KQ Tìm Kiếm
    if (it == gCusChoTroi.mListItemFind.end() && CacheSizeInputTimKiem != -1) {
      SkipFilter++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }
    //--Loc Coin
    if (LocItemTypCoin != 63) {
      if (!(LocItemTypCoin & 1) &&
          gCusChoTroi.m_DataChoTroi[n].PriceType == 1) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      } else if (!(LocItemTypCoin & 2) &&
                 gCusChoTroi.m_DataChoTroi[n].PriceType == 2) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      } else if (!(LocItemTypCoin & 4) &&
                 gCusChoTroi.m_DataChoTroi[n].PriceType == 3) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      }
      if (!(LocItemTypCoin & 8) &&
          gCusChoTroi.m_DataChoTroi[n].PriceType == 4) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      } else if (!(LocItemTypCoin & 16) &&
                 gCusChoTroi.m_DataChoTroi[n].PriceType == 5) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      } else if (!(LocItemTypCoin & 32) &&
                 gCusChoTroi.m_DataChoTroi[n].PriceType == 6) {
        SkipFilter++;
        DeleteChoTroiTempItem(CTItem);
        continue;
      }
    }

    //=== Loc Opt Item
    if (LocItemCoSkill && !m_Skill) {
      SkipFilter++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }
    if (LocItemCoLuck && !m_Luck) {
      SkipFilter++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }
    // g_Console.AddMessage(1, "%x", m_Opt);
    if (LocItemCoOpt && m_Opt < 5) {
      SkipFilter++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }
    if (LocItemCoExc && !CTItem->Option1) {
      SkipFilter++;
      DeleteChoTroiTempItem(CTItem);
      continue;
    }
    //==========

    gInterface->DrawBarForm(KhungInfoX + ListItemWH + 10, KhungInfoY,
                            (ListItemWH + 50), ListItemWH + 9, 0.0, 0.0, 0.0,
                            0.8);
    gInterface->DrawInfoBox(KhungInfoX, KhungInfoY, ListItemWH, ListItemWH,
                            BGList, 0);
    g_pNewUISystem->RenderItem3DFree(
        KhungInfoX, KhungInfoY - 30, ListItemWH, ListItemWH, CTItem->Type,
        CTItem->Level, CTItem->Option1, CTItem->ExtOption, 0, 1.4); // BMD MOdel
    // RenderItemInfo(350, 100, CTItem, 0, 0, false, true);
    //============
    //==Draw Time
    if (gCusChoTroi.m_DataChoTroi[n].TimeItemRaoBan > 86400) {
      SEASON3B::TextDraw(
          (HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 58, 0xFFE50096, 0x0,
          ListItemWH, 0, 3, gTextClient.txtClient_ChoTroi[41],
          ((gCusChoTroi.m_DataChoTroi[n].TimeItemRaoBan) / 86400)); //
    } else {
      SEASON3B::TextDraw(
          (HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 58, 0xFFE50096, 0x0,
          ListItemWH, 0, 3, gTextClient.txtClient_ChoTroi[42],
          ((gCusChoTroi.m_DataChoTroi[n].TimeItemRaoBan) / 3600)); //
    }

    if (gCusChoTroi.m_DataChoTroi[n].Pass >= 0) {
      SEASON3B::TextDraw((HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 8,
                         0xFFFFFFFF, 0x3091FF96, ListItemWH, 0, 3, "Pass"); //
    }

    //===Info Item
    SEASON3B::TextDraw((HFONT)g_hFontBold, KhungInfoX + ListItemWH + 10,
                       KhungInfoY, 0xFFFFFFFF, 0x3091FF96, (ListItemWH + 49), 0,
                       3, ChoTroiGetItemName(CTItemIndex)); // Name Item
    SEASON3B::TextDraw((HFONT)g_hFont, KhungInfoX + ListItemWH + 10,
                       KhungInfoY + 12, 0xFF833096, 0x0, (ListItemWH + 49), 0,
                       3, gTextClient.txtClient_ChoTroi[11],
                       gCusChoTroi.m_DataChoTroi[n].Name); // Name Seller
    SEASON3B::TextDraw(
        (HFONT)g_hFontBold, KhungInfoX + ListItemWH + 10, KhungInfoY + 12 * 2,
        0xFFDF2B96, 0x0, (ListItemWH + 49), 0, 3,
        gTextClient.txtClient_ChoTroi[12],
        gInterface->NumberFormat(gCusChoTroi.m_DataChoTroi[n].Price),
        NameGiaCoin[gCusChoTroi.m_DataChoTroi[n].PriceType]); // Gia Ban
    SEASON3B::TextDraw(
        (HFONT)g_hFont, KhungInfoX + ListItemWH + 10, KhungInfoY + 12 * 3,
        0xFAFAFA96, 0x0, (ListItemWH + 49), 0, 3,
        gTextClient.txtClient_ChoTroi[13],
        SelectBarListChoTroi[gCusChoTroi.m_DataChoTroi[n].TypeItem == 255
                                 ? 11
                                 : gCusChoTroi.m_DataChoTroi[n]
                                       .TypeItem]); // Slot Type

    if (SelectBarChoTroiNumber == 12) {
      //===Draw Button Thu Hoi
      if (gInterface->DrawButton(KhungInfoX + ListItemWH + 10,
                                 KhungInfoY + (12 * 3) + 15, 100, 12,
                                 gTextClient.txtClient_ChoTroi[14], 115) &&
          !SelectBarChoTroi && !gInterface->Data[eWindowNhapPass].OnShow) {
        // pDrawMessage("Thu Hoi Item", 1);
        CustomChoTroi::PMSG_REQ_MARKET_BUY pMsg;
        pMsg.h.set(0xD3, 0x21, sizeof(pMsg));
        pMsg.Result = 2;
        pMsg.ID = gCusChoTroi.m_DataChoTroi[n].ID;
        DataSend((LPBYTE)&pMsg, pMsg.h.size);
      }
    } else {
      float ButtonWBuy = 55;
      if (ChoTroiCanPreviewEquip(CTItem->Type) &&
          gCusChoTroi.m_DataChoTroi[n].TypeItem >= 1 &&
          gCusChoTroi.m_DataChoTroi[n].TypeItem <= 8) {
        //===Draw Button Thu Do
        if (gInterface->DrawButton(KhungInfoX + ListItemWH + 10 + 58,
                                   KhungInfoY + (12 * 3) + 15, 100, 12,
                                   gTextClient.txtClient_ChoTroi[15], 55) &&
            !SelectBarChoTroi && !gInterface->Data[eWindowNhapPass].OnShow) {
          Script_Item *pItemAttr = GMItemMng->find(CTItem->Type);
          if (pItemAttr != NULL) {
            m_PhotoViewChotroi.SetAutoupdatePlayer(FALSE);
            m_PhotoViewChotroi.CopyPlayer();
            m_PhotoViewChotroi.UpdateItemChar(pItemAttr->m_byItemSlot, CTItem);
          }
        }
      } else {
        ButtonWBuy = 115;
      }
      //===Draw Button Buy
      if (gInterface->DrawButton(
              KhungInfoX + ListItemWH + 10, KhungInfoY + (12 * 3) + 15, 100, 12,
              gTextClient.txtClient_ChoTroi[16], ButtonWBuy) &&
          !SelectBarChoTroi && !gInterface->Data[eWindowNhapPass].OnShow &&
          !gInterface->Data[eWindowMessageBox].OnShow) {
        BuyID = gCusChoTroi.m_DataChoTroi[n].ID;
        if (gCusChoTroi.m_DataChoTroi[n].Pass >= 0) {
          SetChoTroiBuyPassText("");
          gInterface->Data[eWindowNhapPass].OnShow = 1;
          ChoTroiNhapPassNeedFocus = true;
          ChoTroiNhapPassManualFocus = true;
          ChoTroiNhapPassReplaceAll = true;
        } else {
          OpenChoTroiBuyConfirm(gCusChoTroi.m_DataChoTroi[n], -1);
          BuyID = -1;
        }
      }
    }
    //===Break;
    DeleteChoTroiTempItem(CTItem);
    CountList++;
    if (CountList >= MaxPerPageChoTroi) {
      break;
    }
    if (CountList % 1 == 0 && CountList != 0) {
      countx = 0;
      county++;
    } else {
      countx++;
    }
  }
  if (DataListItem > 0 && CountList == 0) {
    SEASON3B::TextDraw((HFONT)g_hFontMini, ListItemX + 8, ListItemY + 8,
                       0xFF3030FF, 0x00000096, 190, 0, 1,
                       "[ChoTroi] Loaded:%d Render:0 Create:%d Type:%d Filter:%d",
                       DataListItem, SkipCreateItem, SkipUnsafeType,
                       SkipFilter);
  }

  //==Select Form
  EnableAlphaTest(true);
  glColor3f(1.0, 1.0, 1.0);
  gCusChoTroi.m_TabBtn.ChangeRadioButtonInfo(true, StartX + 230, StartY + 35,
                                             56, 22);
  gCusChoTroi.m_TabBtn.Render();
  int iPrevOpenTab = gCusChoTroi.m_iNumCurOpenTab;
  int iNumCurOpenTab = gCusChoTroi.m_TabBtn.UpdateMouseEvent();
  if (iNumCurOpenTab != SEASON3B::RADIOGROUPEVENT_NONE) {
    gCusChoTroi.m_iNumCurOpenTab = iNumCurOpenTab;
    if (iNumCurOpenTab == 0 && iPrevOpenTab != 0) {
      RequestChoTroiList(SelectBarChoTroiNumber);
    } else if (iNumCurOpenTab == 1 && iPrevOpenTab != 1) {
      ClearChoTroiExpiredPendingSellCache();
    }
  }
  DrawInfoPhai(StartX + 230, StartY + 65);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  EnableAlphaTest(0);

  //===Window Passs
  float WindowWP = 0.0f;
  float WindowHP = 0.0f;
  float StartXP = 0.0f;
  float StartYP = 0.0f;
  float NhapPassBoxX = 0.0f;
  float NhapPassBoxY = 0.0f;
  float NhapPassBoxW = 0.0f;
  float NhapPassBoxH = 0.0f;
  float NTCoinX2 = 0.0f;
  float NTCoinY2 = 0.0f;
  GetChoTroiNhapPassLayout(WindowWP, WindowHP, StartXP, StartYP,
                           NhapPassBoxX, NhapPassBoxY, NhapPassBoxW,
                           NhapPassBoxH, NTCoinX2, NTCoinY2);

  if (gInterface->Data[eWindowNhapPass].OnShow) {

    gInterface->gDrawWindowCustom(&StartXP, &StartYP, WindowWP, WindowHP,
                                  eWindowNhapPass, "Pass");
    if (!gInterface->Data[eWindowNhapPass].OnShow) {
      ChoTroiNhapPassManualFocus = false;
      ChoTroiNhapPassReplaceAll = false;
      SetChoTroiBuyPassText("");
      SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);
      return;
    }

    GetChoTroiNhapPassLayout(WindowWP, WindowHP, StartXP, StartYP,
                             NhapPassBoxX, NhapPassBoxY, NhapPassBoxW,
                             NhapPassBoxH, NTCoinX2, NTCoinY2);

    SEASON3B::TextDraw((HFONT)g_hFont, NhapPassBoxX, StartYP + 38,
                       0xFFFFFFC8, 0x0, (int)NhapPassBoxW, 0, 3, "Pass");
    RenderBitmap(SEASON3B::CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
                 NhapPassBoxX, NhapPassBoxY, NhapPassBoxW, NhapPassBoxH, 0.0,
                 0.0, 0.82, 0.71, 1, 1, 0.0);

    SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);
    RenderChoTroiBuyPassText(NTCoinX2, NTCoinY2, CHOTROI_BUY_PASS_INPUT_WIDTH);

    if (gInterface->DrawButton(StartXP + (WindowWP / 2) - 70, StartYP + 80, 100,
                               12, gTextClient.txtClient_ChoTroi[16], 55)) {
      int buyPass = (szNhapPassChoTroi[0] == 0) ? -1 : atoi(szNhapPassChoTroi);
      const CustomChoTroi::MARKET_DATA *marketData =
          FindChoTroiMarketDataByID(BuyID);

      memset(&szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi)); // HSD Them
      SetChoTroiBuyPassText("");
      gInterface->Data[eWindowNhapPass].OnShow = 0;
      ChoTroiNhapPassNeedFocus = false;
      ChoTroiNhapPassManualFocus = false;
      ChoTroiNhapPassReplaceAll = false;
      SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);

      if (marketData != NULL) {
        OpenChoTroiBuyConfirm(*marketData, buyPass);
      }

      BuyID = -1;
    }

    if (gInterface->DrawButton(StartXP + (WindowWP / 2) + 10, StartYP + 80, 100,
                               12, "Đóng", 55)) {
      // hienpass = false;
      gInterface->Data[eWindowNhapPass].OnShow = 0;
      ChoTroiNhapPassNeedFocus = false;
      ChoTroiNhapPassManualFocus = false;
      ChoTroiNhapPassReplaceAll = false;
      memset(&szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi)); // HSD Them
      SetChoTroiBuyPassText("");
      SetChoTroiInputState(NhapPassChoTroi, UISTATE_HIDE);
      BuyID = -1;
    }
  }
  //==31422 Select Bar
  DWORD SelectBarHoverBG = 0x0;
  float SelectBarY = StartY;
  int SelectBarHover = 0;
  RenderBitmap(SEASON3B::CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX,
               ListItemX - 2, SelectBarY + 37, 110, 20, 0.0, 0.0, 0.82, 0.71, 1,
               1, 0.0);
  if (SEASON3B::CheckMouseIn(
          ListItemX - 2, SelectBarY + 37, 110,
          20) /* && !gInterface->Data[eWindowChotroiRaoBan].Hover*/) {

    SelectBarHover = 1;
    SelectBarHoverBG = 0x66646450;
    if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) // Click
    {
      if (GetKeyState(VK_LBUTTON) & 0x8000) {
        SelectBarHover = 2;
        gInterface->Data[eTIME].EventTick = GetTickCount();
        PlayBuffer(25, 0, 0);
        SelectBarChoTroi ^= 1;
      }
    }
  }
  RenderBitmap(SEASON3B::CNewUICastleWindow::IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN,
               ListItemX + 2, SelectBarY + 39.5, 13.5, 13.5, 0.0,
               0.21 * SelectBarHover, 0.91, 0.21, 1, 1, 0.0);
  SEASON3B::TextDraw((HFONT)g_hFontBold, ListItemX + 15, SelectBarY + 41,
                     0xFFFFFFFF, SelectBarHoverBG, 85, 0, 3,
                     gTextClient.txtClient_ChoTroi[19],
                     SelectBarListChoTroi[SelectBarChoTroiNumber]); //
  if (SelectBarChoTroi) {
    int CountSelectBar = 1;
    gInterface->DrawBarForm(ListItemX + 15, SelectBarY + 41 + (15), 85, 15 * 12,
                            0.0, 0.0, 0.0, 0.8);
    for (int i = 0; i < 13; i++) {
      if (SelectBarChoTroiNumber == i)
        continue;
      DWORD SelectHover = 0x0;
      if (SEASON3B::CheckMouseIn(ListItemX + 15,
                                 SelectBarY + 41 + (15 * CountSelectBar), 130,
                                 15)) {
        SelectHover = 0x66646450;
        if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) // Click
        {
          if (GetKeyState(VK_LBUTTON) & 0x8000) {
            SelectBarChoTroiNumber = i;
            gInterface->Data[eTIME].EventTick = GetTickCount();
            PlayBuffer(25, 0, 0);
            SelectBarChoTroi ^= 1;
            RequestChoTroiList(i);
          }
        }
      }
      SEASON3B::TextDraw((HFONT)g_hFontBold, ListItemX + 15,
                         SelectBarY + 41 + (15 * CountSelectBar), 0xFFFFFFFF,
                         SelectHover, 85, 0, 3, "%s",
                         SelectBarListChoTroi[i]); //
      CountSelectBar++;
    }
  }

  //===Show Info
  if (ShowInfoItem != -1 /*&& !gInterface->Data[eWindowChotroiRaoBan].Hover*/) {
    // int BGetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36),
    // gInterface->m_DataChoTroi[ShowInfoItem].Item); JCCoord B; B.X =
    // (int)pCursorX + 75; B.Y = (int)pCursorY;
    // BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, BGetInfoLoadItem, 0, 0,
    // 0);
    ITEM *CTItem =
        g_pNewItemMng->CreateItem(gCusChoTroi.m_DataChoTroi[ShowInfoItem].Item);
    if (CTItem != NULL && IsChoTroiItemTypeSafe(CTItem->Type)) {
      RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
    }
    DeleteChoTroiTempItem(CTItem);
  }
  ShowWindowChoOK = true;
}
bool CacheForm = false;
void CustomChoTroi::BDrawWindowChoTroi() {
  if (!CacheForm) {
    std::list<unicode::t_string> ltext;
    ltext.push_back(gTextClient.txtClient_ChoTroi[45]); // MUA
    ltext.push_back(gTextClient.txtClient_ChoTroi[46]); // Ban

    gCusChoTroi.m_TabBtn.CreateRadioGroup(
        2, SEASON3B::CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON, TRUE);
    gCusChoTroi.m_TabBtn.ChangeRadioText(ltext);
    gCusChoTroi.m_TabBtn.ChangeRadioButtonInfo(true, 0, 0, 76, 22);
    gCusChoTroi.m_TabBtn.ChangeFrame(gCusChoTroi.m_iNumCurOpenTab);

    m_PhotoViewChotroi.Init(0);
    m_PhotoViewChotroi.SetSize(130, 130);
    m_PhotoViewChotroi.CopyPlayer();
    m_PhotoViewChotroi.SetAutoupdatePlayer(TRUE);
    m_PhotoViewChotroi.SetAnimation(AT_STAND1);
    m_PhotoViewChotroi.SetAngle(120);
    m_PhotoViewChotroi.SetZoom(0.6f);
    m_PhotoViewChotroi.SetZoomLimit(0.5f, 0.8f);
    m_PhotoViewChotroi.SetPosition(0, 0);
    m_PhotoViewChotroi.SetOption(UIPHOTOVIEWER_CANCONTROL);
    CacheForm = true;
  }

  DrawWindowChoTroi();
}

void CustomChoTroi::GetOpenChoTroiWinDow() {
  if (GetTickCount() - gInterface->Data[eWindowChoTroi].EventTick < 500) {
    return;
  }
  gInterface->Data[eWindowChoTroi].EventTick = GetTickCount();

  if (gInterface->Data[eWindowChoTroi].OnShow) {
    CloseChoTroiWindow(true);
    return;
  } else {
    ClearChoTroiSellCache(false);

    XULY_CGPACKET pMsg;
    pMsg.header.set(0xD3, 0x10, sizeof(pMsg));
    pMsg.ThaoTac = 2; //
    DataSend((LPBYTE)&pMsg, pMsg.header.size);
  }
}

void CustomChoTroi::CloseWindow(bool sendRollback) {
  CloseChoTroiWindow(sendRollback);
}

bool CustomChoTroi::IsWindowActive() const {
  return (gInterface->Data[eWindowChoTroi].OnShow ||
          gInterface->Data[eWindowNhapPass].OnShow || ShowWindowChoOK);
}

bool CustomChoTroi::IsBuyPassWindowActive() const {
  return gInterface->Data[eWindowNhapPass].OnShow;
}

bool CustomChoTroi::HandleEscapeKey() {
  if (ChoTroiBlockEscapeUntilRelease) {
    if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0) {
      ChoTroiBlockEscapeUntilRelease = false;
      return false;
    }

    SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
        VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
    return true;
  }

  if (!IsWindowActive()) {
    return false;
  }

  CloseChoTroiWindow(true);
  ChoTroiBlockEscapeUntilRelease = true;
  SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
      VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
  PlayBuffer(25, 0, 0);
  return true;
}

bool CustomChoTroi::SendItemRaoBan(ITEM *ItemSell, int Slot, bool KeyClick) {
  const bool isChoTroiSellTab =
      (m_iNumCurOpenTab == 1 || m_TabBtn.GetCurButtonIndex() == 1);

  if (KeyClick && gInterface->Data[eWindowChoTroi].OnShow &&
      isChoTroiSellTab) // Send Item Rao Ban
  {
    const DWORD currentTick = GetTickCount();

    if (ItemSell == NULL || !IsChoTroiItemTypeSafe(ItemSell->Type) ||
        Slot < 0) {
      return 0;
    }

    ClearChoTroiExpiredPendingSellCache();

    if (gCusChoTroi.ItemCacheSelect != NULL && !gCusChoTroi.ItemCacheShow) {
      // Wait for GS 0xD3/0x01 confirmation. Rolling back here can return the
      // just-clicked item to inventory and leave a hidden sell-slot cache.
      return 0;
    }

    if (gCusChoTroi.ItemCacheSelect != NULL && gCusChoTroi.ItemCacheShow) {
      // gInterface->DrawMessage(1, "Item Khong Hop Le" );
      return 0;
    }

    ITEM *pSellCacheItem = g_pNewItemMng->DuplicateItem(ItemSell);
    if (pSellCacheItem == NULL) {
      return 0;
    }

    gCusChoTroi.ItemCacheSelect = pSellCacheItem;
    gCusChoTroi.ItemCacheTime = currentTick;
    gCusChoTroi.ItemCacheShow = false;
    ChoTroiOwnsSellCacheItem = true;
    ChoTroiSellCacheSourceSlot = Slot;
    CacheItemRaoBan = 1;

    PMSG_ITEM_MOVE_RECV pMsg = {0};

    pMsg.h.set(0xD3, 0x13, sizeof(pMsg));
    pMsg.Target = -1;
    pMsg.sFlag = 0;
    pMsg.tFlag = 0;
    pMsg.Target = 0;
    pMsg.Source = Slot;
    ::PlayBuffer(SOUND_GET_ITEM01);
    DataSend((BYTE *)&pMsg, pMsg.h.size);
    return 1;
  }

  return 0;
}
void CustomChoTroi::SetShowItemCache(BYTE *Recv) {
  XULY_CGPACKET *lpMsg = (XULY_CGPACKET *)Recv;
  if (lpMsg->ThaoTac == 1) {
    if (gCusChoTroi.ItemCacheSelect != NULL) {
      gCusChoTroi.ItemCacheShow = 1;
      CacheItemRaoBan = 1;
      if (ChoTroiSellCacheSourceSlot >= 0 && g_pMyInventory != NULL) {
        g_pMyInventory->DeleteItem(ChoTroiSellCacheSourceSlot);
      }
      ChoTroiSellCacheSourceSlot = -1;
      SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
    } else {
      ClearChoTroiSellCache(false);
    }
  } else {
    ClearChoTroiSellCache(false);

    if (gInterface->Data[eWindowChoTroi].OnShow) {
      RequestChoTroiList(SelectBarChoTroiNumber);
    }
  }
  return;
}
void CustomChoTroi::GCSetListChoTroi(BYTE *Recv, int Size) {
  gCusChoTroi.OnCointType = 0;
  gCusChoTroi.m_DataChoTroi.clear();

  int packetSize = ChoTroiGetPacketSize(Recv, Size);
  int packetCount = 0;
  int packetType = 0;
  int dataOffset = 16;
  int rowSize = 52;
  bool compactLayout = false;

  if (!ChoTroiTryPacketLayout(Recv, packetSize, 8, 52, &packetCount,
                              &packetType)) {
    compactLayout =
        ChoTroiTryPacketLayout(Recv, packetSize, 5, 51, &packetCount,
                               &packetType);
    if (compactLayout) {
      dataOffset = 13;
      rowSize = 51;
    }
  }

  if (packetCount < 0 || packetCount > MARKET_ITEM_MAX) {
    packetCount = 0;
  }

  gCusChoTroi.OnCointType = packetType;
  if (!IsChoTroiCoinTypeEnabled(TypeCoinBan)) {
    TypeCoinBan = GetFirstEnabledChoTroiCoinType();
  }

  for (int n = 0; n < packetCount; n++) {
    BYTE *row = Recv + dataOffset + (rowSize * n);
    MARKET_DATA info;
    memset(&info, 0, sizeof(info));

    info.ID = ChoTroiReadInt(row + 0);
    memcpy(info.Name, row + 4, MARKET_NAME_LEN);
    info.Name[MARKET_NAME_LEN - 1] = 0;
    memcpy(info.Item, row + 15, MARKET_ITEM_BUFFER);
    ChoTroiNormalizeClientItem(info.Item);

    if (compactLayout) {
      info.PriceType = ChoTroiReadInt(row + 31);
      info.Price = ChoTroiReadInt(row + 35);
      info.TypeItem = ChoTroiReadInt(row + 39);
      info.TimeItemRaoBan = ChoTroiReadInt(row + 43);
      info.Pass = ChoTroiReadInt(row + 47);
    } else {
      info.PriceType = ChoTroiReadInt(row + 32);
      info.Price = ChoTroiReadInt(row + 36);
      info.TypeItem = ChoTroiReadInt(row + 40);
      info.TimeItemRaoBan = ChoTroiReadInt(row + 44);
      info.Pass = ChoTroiReadInt(row + 48);
    }

    gCusChoTroi.m_DataChoTroi.push_back(info);
  }

  if (packetCount > 0 && gCusChoTroi.m_DataChoTroi.empty()) {
    gInterface->DrawMessage(1, "[ChoTroiMain] Recv Count=%d Parsed=0 Size=%d",
                            packetCount, packetSize);
  }

  UpdateMaxPosChoTroi = true;
  ShowInfoItem = -1;

  if (WindowChoTroiScrollBar != NULL) {
    WindowChoTroiScrollBar->SetCurPos(0);
  }

  if (!gInterface->Data[eWindowChoTroi].OnShow) {
    gInterface->Data[eWindowChoTroi].OnShow = 1;
  }
}
#endif
