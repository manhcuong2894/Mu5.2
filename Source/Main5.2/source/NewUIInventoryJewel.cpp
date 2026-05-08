#include "stdafx.h"
#include "NewUIInventoryJewel.h"
#include "TextClient.h"
#include "UIControls.h"
#include "WSclient.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"

namespace {
const float JEWEL_WINDOW_WIDTH = 190.0f;
const float JEWEL_WINDOW_HEIGHT = 429.0f;

const float HEADER_POS_X = 12.0f;
const float HEADER_POS_Y = 18.0f;
const float HEADER_WIDTH = 166.0f;

const float LIST_POS_X = 12.0f;
const float LIST_POS_Y = 80.0f;
const float ROW_WIDTH = 166.0f;
const float ROW_HEIGHT = 44.0f;

const float FOOTER_PAGE_POS_X = 66.0f;
const float FOOTER_PAGE_POS_Y = 390.0f;
const float FOOTER_PAGE_WIDTH = 58.0f;
const float FOOTER_PAGE_HEIGHT = 22.0f;

const float ICON_POS_X = 6.0f;
const float ICON_POS_Y = 11.0f;
const float ICON_SIZE = 18.0f;

const float AMOUNT_BOX_POS_X = 30.0f;
const float AMOUNT_BOX_WIDTH = 18.0f;

const float BUTTON_ROW_POS_Y = 25.0f;
const int QUICK_ACTION_COUNT = 3;
const float QUICK_BUTTON_WIDTH = 17.0f;
const float QUICK_BUTTON_HEIGHT = 16.0f;
const float QUICK_BUTTON_POS_X[QUICK_ACTION_COUNT] = {56.0f, 81.0f, 106.0f};
const float RAW_BUTTON_POS_X = 131.0f;
const float RAW_BUTTON_WIDTH = 22.0f;
const float ACTION_BUTTON_TEXTURE_WIDTH = 46.0f;
const float ACTION_BUTTON_TEXTURE_HEIGHT = 36.0f;
const float POPUP_POS_X = 20.0f;
const float POPUP_POS_Y = 150.0f;
const float POPUP_WIDTH = 150.0f;
const float POPUP_HEIGHT = 84.0f;
const float POPUP_INPUT_POS_X = 35.0f;
const float POPUP_INPUT_POS_Y = 34.0f;
const float POPUP_INPUT_WIDTH = 80.0f;
const float POPUP_INPUT_HEIGHT = 14.0f;
const float POPUP_BUTTON_POS_Y = 57.0f;
const float POPUP_OK_WIDTH = 34.0f;
const float POPUP_CLOSE_WIDTH = 42.0f;

void DrawDarkBlock(float x, float y, float width, float height, float alpha) {
  EnableAlphaTest(true);
  glColor4f(0.0f, 0.0f, 0.0f, alpha);
  RenderColor(x, y, width, height);
  EndRenderColor();
}
} // namespace

SEASON3B::CNewUIInventoryJewel::CNewUIInventoryJewel() { Initialize(); }

SEASON3B::CNewUIInventoryJewel::~CNewUIInventoryJewel() { Release(); }

bool SEASON3B::CNewUIInventoryJewel::Create(CNewUIManager *pNewUIMng, int x,
                                            int y) {
  if (pNewUIMng == NULL) {
    return false;
  }

  m_pNewUIMng = pNewUIMng;

  SetPos(x, y);

  LoadBitmap("Interface\\InGameShop\\IGS_Storage_Page.tga",
             CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE, GL_LINEAR);
  LoadBitmap("Interface\\InGameShop\\IGS_Storage_Page_Left.tga",
             CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_LEFT, GL_LINEAR);
  LoadBitmap("Interface\\InGameShop\\IGS_Storage_Page_Right.tga",
             CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_RIGHT, GL_LINEAR);
  LoadBitmap("Interface\\HUD\\newui_Btn_BGQuan.tga", IMAGE_OPTION_BUTTON,
             GL_LINEAR);

  m_pWithdrawEditBox = new CUITextInputBox;
  m_pWithdrawEditBox->Init(gwinhandle->GethWnd(), (int)POPUP_INPUT_WIDTH,
                           (int)POPUP_INPUT_HEIGHT, 10);
  m_pWithdrawEditBox->SetOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);
  m_pWithdrawEditBox->SetTextColor(255, 255, 230, 210);
  m_pWithdrawEditBox->SetBackColor(0, 0, 0, 0);
  m_pWithdrawEditBox->SetFont(g_hFont);
  m_pWithdrawEditBox->SetState(UISTATE_HIDE);

  InitButtons();
  m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY_JEWEL, this);
  UpdatePageButtons();
  return true;
}

void SEASON3B::CNewUIInventoryJewel::Initialize() {
  m_pNewUIMng = NULL;
  m_Pos.x = 0;
  m_Pos.y = 0;

  m_dwCurIndex = -1;
  m_dwSelIndex = -1;
  m_nSelPage = 0;
  m_nMaxPage = 0;
  m_pWithdrawEditBox = NULL;
  m_bIsEnableWithdrawPopup = false;

  ZeroMemory(&m_nRectItem, sizeof(m_nRectItem));
  m_bItems.clear();
}

void SEASON3B::CNewUIInventoryJewel::Release() {
  if (m_pNewUIMng) {
    m_pNewUIMng->RemoveUIObj(this);
    m_pNewUIMng = NULL;
  }

  CloseWithdrawPopup();
  SAFE_DELETE(m_pWithdrawEditBox);
  m_bItems.clear();
}

void SEASON3B::CNewUIInventoryJewel::ConfigureActionButton(CNewUIButton &button,
                                                           const char *text,
                                                           float width) {
  button.ChangeButtonImgState(true, IMAGE_OPTION_BUTTON, true, false, false,
                              0.0f, 0.0f, ACTION_BUTTON_TEXTURE_WIDTH,
                              ACTION_BUTTON_TEXTURE_HEIGHT);
  button.ChangeButtonInfo(0.0f, 0.0f, width, QUICK_BUTTON_HEIGHT);
  button.ChangeText(text);
  button.SetFont(g_hFontBold);
  button.ChangeTextColor(0xFFF6E6B7);
  button.ChangeTextBackColor(0x00000000);
  button.MoveTextPos(0, -1);
  button.ChangeAlpha(0.95f);
}

void SEASON3B::CNewUIInventoryJewel::SetPos(int x, int y) {
  m_Pos.x = x;
  m_Pos.y = y;

  m_nRectItem.top = 0;
  m_nRectItem.left = 0;

  LayoutButtons();
  LayoutWithdrawPopup();
}

void SEASON3B::CNewUIInventoryJewel::LayoutButtons() {
  m_ButtonBack.ChangeButtonInfo(m_Pos.x + 37.0f, m_Pos.y + 390.0f, 20.0f,
                                22.0f);
  m_ButtonNext.ChangeButtonInfo(m_Pos.x + 133.0f, m_Pos.y + 390.0f, 20.0f,
                                22.0f);

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    const float rowBaseX = m_Pos.x + LIST_POS_X;
    const float rowBaseY =
        m_Pos.y + LIST_POS_Y + (ROW_HEIGHT * row) + BUTTON_ROW_POS_Y;

    m_ButtonWithdrawSingle[row].ChangeButtonInfo(rowBaseX + AMOUNT_BOX_POS_X,
                                                 rowBaseY, AMOUNT_BOX_WIDTH,
                                                 QUICK_BUTTON_HEIGHT);

    for (int button = 0; button < QUICK_BUTTON_COUNT; button++) {
      m_ButtonWithdrawQuick[row][button].ChangeButtonInfo(
          rowBaseX + QUICK_BUTTON_POS_X[button], rowBaseY, QUICK_BUTTON_WIDTH,
          QUICK_BUTTON_HEIGHT);
    }

    m_ButtonWithdrawRaw[row].ChangeButtonInfo(rowBaseX + RAW_BUTTON_POS_X,
                                              rowBaseY, RAW_BUTTON_WIDTH,
                                              QUICK_BUTTON_HEIGHT);
  }
}

void SEASON3B::CNewUIInventoryJewel::LayoutWithdrawPopup() {
  const float popupX = m_Pos.x + POPUP_POS_X;
  const float popupY = m_Pos.y + POPUP_POS_Y;

  m_ButtonWithdrawPopupOk.ChangeButtonInfo(popupX + 20.0f,
                                           popupY + POPUP_BUTTON_POS_Y,
                                           POPUP_OK_WIDTH, QUICK_BUTTON_HEIGHT);
  m_ButtonWithdrawPopupClose.ChangeButtonInfo(
      popupX + 88.0f, popupY + POPUP_BUTTON_POS_Y, POPUP_CLOSE_WIDTH,
      QUICK_BUTTON_HEIGHT);

  if (m_pWithdrawEditBox != NULL) {
    m_pWithdrawEditBox->SetPosition((int)(popupX + POPUP_INPUT_POS_X),
                                    (int)(popupY + POPUP_INPUT_POS_Y));
  }
}

void SEASON3B::CNewUIInventoryJewel::InitButtons() {
  m_ButtonBack.ChangeButtonImgState(
      true, CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_LEFT, true);
  m_ButtonBack.ChangeToolTipText(gTextClient.txtClient_JewelBank[4], true);

  m_ButtonNext.ChangeButtonImgState(
      true, CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_RIGHT, true);
  m_ButtonNext.ChangeToolTipText(gTextClient.txtClient_JewelBank[5], true);
  ConfigureActionButton(m_ButtonWithdrawPopupOk, gTextClient.txtClient_JewelBank[6], POPUP_OK_WIDTH);
  ConfigureActionButton(m_ButtonWithdrawPopupClose, gTextClient.txtClient_JewelBank[7], POPUP_CLOSE_WIDTH);
  m_ButtonWithdrawPopupOk.ChangeToolTipText(gTextClient.txtClient_JewelBank[8], true);
  m_ButtonWithdrawPopupClose.ChangeToolTipText(gTextClient.txtClient_JewelBank[9], true);

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    ConfigureActionButton(m_ButtonWithdrawSingle[row], "1", AMOUNT_BOX_WIDTH);
    ConfigureActionButton(m_ButtonWithdrawQuick[row][0], "10",
                          QUICK_BUTTON_WIDTH);
    ConfigureActionButton(m_ButtonWithdrawQuick[row][1], "20",
                          QUICK_BUTTON_WIDTH);
    ConfigureActionButton(m_ButtonWithdrawQuick[row][2], "30",
                          QUICK_BUTTON_WIDTH);
    ConfigureActionButton(m_ButtonWithdrawRaw[row], gTextClient.txtClient_JewelBank[6], RAW_BUTTON_WIDTH);

    m_ButtonWithdrawSingle[row].ChangeToolTipText(gTextClient.txtClient_JewelBank[10], true);
    m_ButtonWithdrawQuick[row][0].ChangeToolTipText(gTextClient.txtClient_JewelBank[11], true);
    m_ButtonWithdrawQuick[row][1].ChangeToolTipText(gTextClient.txtClient_JewelBank[12], true);
    m_ButtonWithdrawQuick[row][2].ChangeToolTipText(gTextClient.txtClient_JewelBank[13], true);
    m_ButtonWithdrawRaw[row].ChangeToolTipText(gTextClient.txtClient_JewelBank[14], true);
  }

  LayoutButtons();
  LayoutWithdrawPopup();
}

bool SEASON3B::CNewUIInventoryJewel::Render() {
  EnableAlphaTest();
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  RenderFrame();
  RenderInter();
  RenderTexts();
  RenderButtons();
  RenderHoly();
  RenderWithdrawPopup();

  DisableAlphaBlend();
  return true;
}

bool SEASON3B::CNewUIInventoryJewel::Update() { return true; }

int SEASON3B::CNewUIInventoryJewel::GetVectorIndexByRow(int row) const {
  return (m_nSelPage * PAGE_ROW_COUNT) + row;
}

SEASON3B::WareHoly *SEASON3B::CNewUIInventoryJewel::GetItemByRow(int row) {
  const int index = GetVectorIndexByRow(row);

  if (index < 0 || index >= (int)m_bItems.size()) {
    return NULL;
  }

  return &m_bItems[index];
}

const SEASON3B::WareHoly *
SEASON3B::CNewUIInventoryJewel::GetItemByRow(int row) const {
  const int index = GetVectorIndexByRow(row);

  if (index < 0 || index >= (int)m_bItems.size()) {
    return NULL;
  }

  return &m_bItems[index];
}

void SEASON3B::CNewUIInventoryJewel::UpdatePageButtons() {
  if (m_nSelPage < 0) {
    m_nSelPage = 0;
  }

  if (m_nSelPage > m_nMaxPage) {
    m_nSelPage = m_nMaxPage;
  }

  if (m_nSelPage <= 0) {
    m_ButtonBack.Lock();
  } else {
    m_ButtonBack.UnLock();
  }

  if (m_nSelPage >= m_nMaxPage) {
    m_ButtonNext.Lock();
  } else {
    m_ButtonNext.UnLock();
  }
}

bool SEASON3B::CNewUIInventoryJewel::UpdateMouseEvent() {
  m_dwCurIndex = -1;

  if (m_bIsEnableWithdrawPopup) {
    return UpdateWithdrawPopupMouseEvent();
  }

  if (m_ButtonNext.UpdateMouseEvent()) {
    if (m_nSelPage < m_nMaxPage) {
      m_nSelPage++;
      UpdatePageButtons();
    }
    return false;
  }

  if (m_ButtonBack.UpdateMouseEvent()) {
    if (m_nSelPage > 0) {
      m_nSelPage--;
      UpdatePageButtons();
    }
    return false;
  }

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    WareHoly *item = GetItemByRow(row);

    if (item == NULL) {
      continue;
    }

    const int vectorIndex = GetVectorIndexByRow(row);
    const float rowX = m_Pos.x + LIST_POS_X;
    const float rowY = m_Pos.y + LIST_POS_Y + (ROW_HEIGHT * row);

    if (CheckMouseIn(rowX, rowY, ROW_WIDTH, ROW_HEIGHT - 2.0f)) {
      m_dwCurIndex = vectorIndex;
    }

    if (m_ButtonWithdrawSingle[row].UpdateMouseEvent()) {
      m_dwSelIndex = vectorIndex;
      SendRequestRemoveJewelOfInventory(item->GetKeyIndex(), 0, 1);
      return false;
    }

    if (m_ButtonWithdrawQuick[row][0].UpdateMouseEvent()) {
      m_dwSelIndex = vectorIndex;
      SendRequestRemoveJewelOfInventory(item->GetKeyIndex(), 1, 1);
      return false;
    }

    if (m_ButtonWithdrawQuick[row][1].UpdateMouseEvent()) {
      m_dwSelIndex = vectorIndex;
      SendRequestRemoveJewelOfInventory(item->GetKeyIndex(), 2, 1);
      return false;
    }

    if (m_ButtonWithdrawQuick[row][2].UpdateMouseEvent()) {
      m_dwSelIndex = vectorIndex;
      SendRequestRemoveJewelOfInventory(item->GetKeyIndex(), 3, 1);
      return false;
    }

    if (m_ButtonWithdrawRaw[row].UpdateMouseEvent()) {
      m_dwSelIndex = vectorIndex;
      OpenWithdrawPopup();
      return false;
    }

    if (CheckMouseIn(rowX, rowY, ROW_WIDTH, ROW_HEIGHT - 2.0f) &&
        SEASON3B::IsRelease(VK_LBUTTON)) {
      m_dwSelIndex = vectorIndex;
      return false;
    }
  }

  return true;
}

bool SEASON3B::CNewUIInventoryJewel::UpdateKeyEvent() {
  if (m_bIsEnableWithdrawPopup) {
    if (SEASON3B::IsPress(VK_ESCAPE)) {
      CloseWithdrawPopup();
      return false;
    }

    if (SEASON3B::IsPress(VK_RETURN)) {
      return SubmitWithdrawPopup();
    }

    return false;
  }

  return true;
}

float SEASON3B::CNewUIInventoryJewel::GetLayerDepth() { return 4.25f; }

bool SEASON3B::CNewUIInventoryJewel::CheckExpansionInventory() {
  if (IsVisible()) {
    return CheckMouseIn(m_Pos.x, m_Pos.y, JEWEL_WINDOW_WIDTH,
                        JEWEL_WINDOW_HEIGHT) != false;
  }

  return false;
}

void SEASON3B::CNewUIInventoryJewel::OpenningProcess() {
  m_nSelPage = 0;
  m_dwSelIndex = -1;
  m_dwCurIndex = -1;
  CloseWithdrawPopup();
  UpdatePageButtons();
}

void SEASON3B::CNewUIInventoryJewel::ClosingProcess() {
  m_nSelPage = 0;
  m_dwSelIndex = -1;
  m_dwCurIndex = -1;
  CloseWithdrawPopup();
  UpdatePageButtons();
}

void SEASON3B::CNewUIInventoryJewel::OpenWithdrawPopup() {
  if (m_pWithdrawEditBox == NULL) {
    return;
  }

  m_bIsEnableWithdrawPopup = true;
  m_pWithdrawEditBox->SetState(UISTATE_NORMAL);
  m_pWithdrawEditBox->SetText("1");
  m_pWithdrawEditBox->GiveFocus(TRUE);
}

void SEASON3B::CNewUIInventoryJewel::CloseWithdrawPopup(bool bClearText) {
  m_bIsEnableWithdrawPopup = false;

  if (m_pWithdrawEditBox != NULL) {
    if (bClearText) {
      m_pWithdrawEditBox->SetText(NULL);
    }

    m_pWithdrawEditBox->SetState(UISTATE_HIDE);
  }

  SetFocus(gwinhandle->GethWnd());
}

bool SEASON3B::CNewUIInventoryJewel::UpdateWithdrawPopupMouseEvent() {
  if (m_pWithdrawEditBox != NULL) {
    if (SEASON3B::IsRelease(VK_LBUTTON)) {
      if (CheckMouseIn((float)m_pWithdrawEditBox->GetPosition_x(),
                       (float)m_pWithdrawEditBox->GetPosition_y(),
                       (float)m_pWithdrawEditBox->GetWidth(),
                       (float)m_pWithdrawEditBox->GetHeight())) {
        m_pWithdrawEditBox->SetState(UISTATE_NORMAL);
        m_pWithdrawEditBox->GiveFocus();
      } else {
        SetFocus(gwinhandle->GethWnd());
      }
    }

    m_pWithdrawEditBox->DoAction();
  }

  if (m_ButtonWithdrawPopupOk.UpdateMouseEvent()) {
    return SubmitWithdrawPopup();
  }

  if (m_ButtonWithdrawPopupClose.UpdateMouseEvent()) {
    CloseWithdrawPopup();
    return false;
  }

  if (CheckMouseIn(m_Pos.x, m_Pos.y, JEWEL_WINDOW_WIDTH, JEWEL_WINDOW_HEIGHT)) {
    return false;
  }

  return true;
}

bool SEASON3B::CNewUIInventoryJewel::SubmitWithdrawPopup() {
  if (m_pWithdrawEditBox == NULL) {
    CloseWithdrawPopup();
    return false;
  }

  char szCount[16] = {0};
  m_pWithdrawEditBox->GetText(szCount, sizeof(szCount));

  const DWORD dwCount = (DWORD)strtoul(szCount, NULL, 10);

  if (dwCount == 0) {
    m_pWithdrawEditBox->GiveFocus(TRUE);
    return false;
  }

  check_budget(dwCount);
  CloseWithdrawPopup();
  return false;
}

void SEASON3B::CNewUIInventoryJewel::RenderFrame() {
  if (gmProtect->checkold_school()) {
    RenderInventoryInterface(m_Pos.x, m_Pos.y, 0);
  } else {
    RenderImage(IMAGE_INVENTORY_BACK, m_Pos.x, m_Pos.y, JEWEL_WINDOW_WIDTH,
                JEWEL_WINDOW_HEIGHT);
    RenderImage(IMAGE_INVENTORY_BACK_TOP2, m_Pos.x, m_Pos.y, JEWEL_WINDOW_WIDTH,
                64.0f);
    RenderImage(IMAGE_INVENTORY_BACK_LEFT, m_Pos.x, m_Pos.y + 64.0f, 21.0f,
                320.0f);
    RenderImage(IMAGE_INVENTORY_BACK_RIGHT, m_Pos.x + 169.0f, m_Pos.y + 64.0f,
                21.0f, 320.0f);
    RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, m_Pos.x, m_Pos.y + 384.0f,
                JEWEL_WINDOW_WIDTH, 45.0f);
  }

  RenderImageF(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE,
               m_Pos.x + FOOTER_PAGE_POS_X, m_Pos.y + FOOTER_PAGE_POS_Y,
               FOOTER_PAGE_WIDTH, FOOTER_PAGE_HEIGHT, 0.0f, 0.0f, 80.0f, 30.0f);
}

void SEASON3B::CNewUIInventoryJewel::RenderTexts() {
  char strText[128];

  g_pRenderText->SetFont(g_hFontBold);
  g_pRenderText->SetBgColor(0, 0, 0, 0);

  g_pRenderText->SetTextColor(255, 214, 150, 0xFFu);
  g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 10, gTextClient.txtClient_JewelBank[0],
                            (int)JEWEL_WINDOW_WIDTH, 0, 3, 0);

  g_pRenderText->SetTextColor(236, 162, 74, 0xFFu);
  g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + 48,
                            gTextClient.txtClient_JewelBank[1], 166, 0, 3, 0);

  // g_pRenderText->SetTextColor(220, 220, 220, 0xFFu);
  // g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + 40, "Hoac bam nut Gui o
  // tung dong ben duoi", 166, 0, 3, 0);

  // g_pRenderText->SetTextColor(152, 232, 146, 0xFFu);
  // g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + 53, "Rut nhanh bang goi
  // 10 / 20 / 30 hoac 1 vien", 166, 0, 3, 0);

  sprintf_s(strText, "%d / %d", m_nSelPage + 1, m_nMaxPage + 1);
  g_pRenderText->SetTextColor(255, 255, 255, 0xFFu);
  g_pRenderText->RenderText(m_Pos.x + FOOTER_PAGE_POS_X,
                            m_Pos.y + FOOTER_PAGE_POS_Y + 3.0f, strText,
                            (int)FOOTER_PAGE_WIDTH, 0, 3, 0);

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    const WareHoly *item = GetItemByRow(row);

    if (item == NULL) {
      continue;
    }

    const float rowX = m_Pos.x + LIST_POS_X;
    const float rowY = m_Pos.y + LIST_POS_Y + (ROW_HEIGHT * row);
    const Script_Item *itemInfo = GMItemMng->find(item->GetIndex());
    const char *itemName = (itemInfo != NULL && itemInfo->Name[0] != '\0')
                               ? itemInfo->Name
                               : "Jewel Unknown";

    g_pRenderText->SetTextColor(245, 225, 185, 0xFFu);
    g_pRenderText->RenderText((int)(rowX + AMOUNT_BOX_POS_X),
                              (int)(rowY + 2.0f), itemName, 122, 0, 0);

    if (item->GetValue() > 0) {
      g_pRenderText->SetTextColor(255, 214, 118, 0xFFu);
    } else {
      g_pRenderText->SetTextColor(155, 155, 155, 0xFFu);
    }

    sprintf_s(strText, gTextClient.txtClient_JewelBank[2], item->GetValue());
    g_pRenderText->RenderText((int)(rowX + AMOUNT_BOX_POS_X),
                              (int)(rowY + 13.0f), strText, 122, 0, 0);
  }

  if (m_bItems.empty()) {
    g_pRenderText->SetTextColor(210, 210, 210, 0xFFu);
    g_pRenderText->RenderText(m_Pos.x + 16, m_Pos.y + 194,
                                gTextClient.txtClient_JewelBank[3], 158, 0, 3, 0);
  }
}

void SEASON3B::CNewUIInventoryJewel::RenderInter() {
  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    const WareHoly *item = GetItemByRow(row);

    if (item == NULL) {
      continue;
    }

    const int vectorIndex = GetVectorIndexByRow(row);
    const float rowX = m_Pos.x + LIST_POS_X;
    const float rowY = m_Pos.y + LIST_POS_Y + (ROW_HEIGHT * row);
    DWORD borderColor = RGBA(95, 95, 95, 255);

    DrawDarkBlock(rowX, rowY, ROW_WIDTH, ROW_HEIGHT - 2.0f, 0.78f);

    if (m_dwSelIndex == vectorIndex) {
      borderColor = RGBA(181, 144, 76, 255);
    } else if (m_dwCurIndex == vectorIndex) {
      borderColor = RGBA(125, 125, 125, 255);
    }

    FrameTarget(rowX, rowY, ROW_WIDTH, ROW_HEIGHT - 2.0f, borderColor);
  }
}

void SEASON3B::CNewUIInventoryJewel::RenderButtons() {
  UpdatePageButtons();

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    const WareHoly *item = GetItemByRow(row);

    if (item == NULL) {
      continue;
    }

    const bool hasStock = (item->GetValue() > 0);
    const bool hasBundle = (item->GetBundledIndex() != -1);

    if (hasStock) {
      m_ButtonWithdrawSingle[row].UnLock();
      m_ButtonWithdrawSingle[row].ChangeTextColor(0xFFF7E2AC);
    } else {
      m_ButtonWithdrawSingle[row].Lock();
      m_ButtonWithdrawSingle[row].ChangeTextColor(0xFF8F8F8F);
    }

    m_ButtonWithdrawSingle[row].Render();

    for (int button = 0; button < QUICK_BUTTON_COUNT; button++) {
      if (hasStock && hasBundle) {
        m_ButtonWithdrawQuick[row][button].UnLock();
        m_ButtonWithdrawQuick[row][button].ChangeTextColor(0xFFF7E2AC);
      } else {
        m_ButtonWithdrawQuick[row][button].Lock();
        m_ButtonWithdrawQuick[row][button].ChangeTextColor(0xFF8F8F8F);
      }

      m_ButtonWithdrawQuick[row][button].Render();
    }

    if (hasStock) {
      m_ButtonWithdrawRaw[row].UnLock();
      m_ButtonWithdrawRaw[row].ChangeTextColor(0xFFF7E2AC);
    } else {
      m_ButtonWithdrawRaw[row].Lock();
      m_ButtonWithdrawRaw[row].ChangeTextColor(0xFF8F8F8F);
    }

    m_ButtonWithdrawRaw[row].Render();
  }

  m_ButtonBack.Render();
  m_ButtonNext.Render();
}

void SEASON3B::CNewUIInventoryJewel::RenderHoly() {
  SEASON3B::begin3D();

  for (int row = 0; row < PAGE_ROW_COUNT; row++) {
    const WareHoly *item = GetItemByRow(row);

    if (item == NULL) {
      continue;
    }

    const float rowX = m_Pos.x + LIST_POS_X;
    const float rowY = m_Pos.y + LIST_POS_Y + (ROW_HEIGHT * row);

    Render2Item3D(rowX + ICON_POS_X, rowY + ICON_POS_Y, ICON_SIZE, ICON_SIZE,
                  item->GetIndex(), item->GetLevel(), 0, 0, false);
  }

  SEASON3B::endrender3D();
}

void SEASON3B::CNewUIInventoryJewel::RenderWithdrawPopup() {
  if (m_bIsEnableWithdrawPopup == false) {
    return;
  }

  const float popupX = m_Pos.x + POPUP_POS_X;
  const float popupY = m_Pos.y + POPUP_POS_Y;
  const WareHoly *item =
      (m_dwSelIndex >= 0 && m_dwSelIndex < (int)m_bItems.size())
          ? &m_bItems[m_dwSelIndex]
          : NULL;
  char strText[64];

  DrawDarkBlock(m_Pos.x + HEADER_POS_X, m_Pos.y + HEADER_POS_Y, HEADER_WIDTH,
                360.0f, 0.38f);
  DrawDarkBlock(popupX, popupY, POPUP_WIDTH, POPUP_HEIGHT, 0.92f);
  FrameTarget(popupX, popupY, POPUP_WIDTH, POPUP_HEIGHT,
              RGBA(181, 144, 76, 255));

  g_pRenderText->SetFont(g_hFontBold);
  g_pRenderText->SetBgColor(0, 0, 0, 0);
  g_pRenderText->SetTextColor(255, 224, 168, 0xFFu);
  g_pRenderText->RenderText((int)popupX, (int)(popupY + 8.0f),
                            gTextClient.txtClient_JewelBank[14], (int)POPUP_WIDTH, 0, 3, 0);

  if (item != NULL) {
    sprintf_s(strText, gTextClient.txtClient_JewelBank[2], item->GetValue());
    g_pRenderText->SetTextColor(220, 220, 220, 0xFFu);
    g_pRenderText->RenderText((int)popupX, (int)(popupY + 20.0f), strText,
                              (int)POPUP_WIDTH, 0, 3, 0);
  }

  if (m_pWithdrawEditBox != NULL) {
    m_pWithdrawEditBox->Render();
  }

  m_ButtonWithdrawPopupOk.Render();
  m_ButtonWithdrawPopupClose.Render();
}

void SEASON3B::CNewUIInventoryJewel::FrameTarget(float iPos_x, float iPos_y,
                                                 float width, float height,
                                                 DWORD color) {
  EnableAlphaTest(true);

  const BYTE red = GetRed(color);
  const BYTE green = GetGreen(color);
  const BYTE blue = GetBlue(color);

  glColor4ub(red, green, blue, 30);
  RenderColor(iPos_x, iPos_y, width, height, 0.0, 0);

  glColor4ub(red, green, blue, 255);
  RenderColor(iPos_x, iPos_y, width, 1.0f, 0.0f, 0);
  RenderColor(iPos_x, iPos_y + height, width, 1.0f, 0.0f, 0);
  RenderColor(iPos_x, iPos_y + 1.0f, 1.0f, height - 1.0f, 0.0f, 0);
  RenderColor(iPos_x + width - 1.0f, iPos_y + 1.0f, 1.0f, height - 1.0f, 0.0f,
              0);

  EndRenderColor();
}

void SEASON3B::CNewUIInventoryJewel::render_option_group() {}

void SEASON3B::CNewUIInventoryJewel::RemoveData() {
  CloseWithdrawPopup();
  m_bItems.clear();
  m_dwCurIndex = -1;
  m_dwSelIndex = -1;
  m_nSelPage = 0;
  m_nMaxPage = 0;
  UpdatePageButtons();
}

int SEASON3B::CNewUIInventoryJewel::GetCurrentPage() const {
  return m_nSelPage;
}

void SEASON3B::CNewUIInventoryJewel::SetCurrentPage(int nPage) {
  m_nSelPage = nPage;
  UpdatePageButtons();
}

int SEASON3B::CNewUIInventoryJewel::GetMaxPageFromCount(int itemCount) const {
  if (itemCount <= 0) {
    return 0;
  }

  return (itemCount - 1) / PAGE_ROW_COUNT;
}

void SEASON3B::CNewUIInventoryJewel::InsertData(BYTE Index, short ItemIndex,
                                                short ItemLevel, __int64 count,
                                                short BundledIndex) {
  if (ItemIndex >= ITEM_WING && ItemIndex < MAX_ITEM) {
    m_bItems.push_back(
        WareHoly(Index, ItemIndex, ItemLevel, count, BundledIndex));
  }

  m_nMaxPage = GetMaxPageFromCount((int)m_bItems.size());
  UpdatePageButtons();
}

bool SEASON3B::CNewUIInventoryJewel::check_budget(DWORD _iCount) {
  if (m_dwSelIndex >= 0 && m_dwSelIndex < (int)m_bItems.size()) {
    SendRequestRemoveJewelOfInventory(m_bItems[m_dwSelIndex].GetKeyIndex(), 0,
                                      _iCount);
  }

  return false;
}

int SEASON3B::CNewUIInventoryJewel::FindBankIndexByItemType(int nItemType) {
  for (size_t i = 0; i < m_bItems.size(); i++) {
    if (m_bItems[i].GetIndex() == nItemType ||
        m_bItems[i].GetBundledIndex() == nItemType) {
      return m_bItems[i].GetKeyIndex();
    }
  }

  return -1;
}

void SEASON3B::CNewUIInventoryJewel::ProcessInvenItem() {
  if (IsVisible() == false) {
    return;
  }

  if (m_bIsEnableWithdrawPopup) {
    return;
  }

  int invenIndex = -1;
  ITEM *pClickedItem =
      g_pMyInventory->FindItemAtPt(MouseX, MouseY, &invenIndex);

  if (pClickedItem == NULL) {
    return;
  }

  const int bankIndex = FindBankIndexByItemType(pClickedItem->Type);

  if (bankIndex == -1) {
    return;
  }

  SendRequestAddJewelOfInventory((BYTE)bankIndex, 0, 0);
}
