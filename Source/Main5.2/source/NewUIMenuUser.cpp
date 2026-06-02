
#include "stdafx.h"
#include "NewUIMenuUser.h"
#include "CBDrawInterface.h"
#include "CBInterface.h"
#include "CGMProtect.h"
#include "CustomChoTroi.h"
#include "CustomEventTime.h"
#include "CustomMocNap.h"
#include "CustomVongQuay.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "TextClient.h"
namespace {
enum MenuAction {
  MENU_ACTION_EVENT_TIME = 0,
  MENU_ACTION_VONG_QUAY,
  MENU_ACTION_MOC_NAP,
  MENU_ACTION_VIP_SHOP,
  MENU_ACTION_RANKING,
  MENU_ACTION_COMMAND,
  MENU_ACTION_OPTIONS,
  MENU_ACTION_CHO_TROI,
};

struct MenuEntry {
  int ActionId;
  const char *Label;
};

const int kMenuMaxEntries = 8;
const float kMenuWindowWidth = 214.0f;
const float kMenuWindowHeight = 248.0f;
const float kMenuContentX = 8.0f;
const float kMenuContentY = 32.0f;
const float kMenuContentWidth = 190.0f;
const float kMenuContentHeight = 196.0f;
const float kMenuHeaderX = 14.0f;
const float kMenuHeaderY = 38.0f;
const float kMenuHintY = 54.0f;
const float kMenuEntryX = 14.0f;
const float kMenuEntryY = 76.0f;
const float kMenuEntryWidth = 182.0f;
const float kMenuEntryHeight = 22.0f;
const float kMenuEntryStep = 28.0f;
const float kMenuFooterY = 212.0f;
const float kMenuCloseOffsetX = 187.0f;
const float kMenuCloseOffsetY = 5.0f;
const float kMenuCloseSize = 36.0f;

const char *ResolveMenuUserText(int index, const char *fallback) {
  const char *text = gTextClient.txtClient_MenuUser[index];

  if (text[0] == 0 || strcmp(text, "Null") == 0) {
    return fallback;
  }

  return text;
}

float GetMenuWindowHeight(int entryCount) {
  if (entryCount <= 5) {
    return kMenuWindowHeight;
  }

  return kMenuWindowHeight + ((entryCount - 5) * kMenuEntryStep);
}

float GetMenuContentHeight(int entryCount) {
  if (entryCount <= 5) {
    return kMenuContentHeight;
  }

  return kMenuContentHeight + ((entryCount - 5) * kMenuEntryStep);
}

void GetCenteredMenuPosition(float windowWidth, float windowHeight, float &x,
                             float &y) {
  x = (MAX_WIN_WIDTH / 2.0f) - (windowWidth / 2.0f);
  y = (MAX_WIN_HEIGHT / 2.0f) - (windowHeight / 2.0f);

  if (x < 0.0f) {
    x = 0.0f;
  }

  if (y < 0.0f) {
    y = 0.0f;
  }
}

void ApplyCenteredMenuPosition(float windowWidth, float windowHeight, float &x,
                               float &y) {
  GetCenteredMenuPosition(windowWidth, windowHeight, x, y);

  gInterface->Data[eWindowMenuUser].X = x;
  gInterface->Data[eWindowMenuUser].Y = y;
  gInterface->Data[eWindowMenuUser].Width = windowWidth;
  gInterface->Data[eWindowMenuUser].Height = windowHeight;
  gInterface->Data[eWindowMenuUser].AllowMove = false;
  gInterface->Data[eWindowMenuUser].FirstLoad = true;
}

int BuildMenuEntries(MenuEntry *entries, int maxEntries) {
  int count = 0;

  if (gmProtect->MenuButtonEventTime && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_EVENT_TIME;
    entries[count].Label = gTextClient.txtClient_MenuUser[3];
    ++count;
  }

  if (gmProtect->MenuButtonVQMM && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_VONG_QUAY;
    entries[count].Label = gTextClient.txtClient_MenuUser[9];
    ++count;
  }

  if (gmProtect->MenuButtonMocNap && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_MOC_NAP;
    entries[count].Label = gTextClient.txtClient_MenuUser[10];
    ++count;
  }

  if (gmProtect->MenuButtonVipShop && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_VIP_SHOP;
    entries[count].Label = gTextClient.txtClient_MenuUser[4];
    ++count;
  }

  if (gmProtect->MenuButtonRankTop && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_RANKING;
    entries[count].Label = gTextClient.txtClient_MenuUser[5];
    ++count;
  }

  if (gmProtect->MenuButtonCommand && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_COMMAND;
    entries[count].Label = gTextClient.txtClient_MenuUser[6];
    ++count;
  }

  if (gmProtect->MenuButtonOptions && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_OPTIONS;
    entries[count].Label = gTextClient.txtClient_MenuUser[7];
    ++count;
  }

#if CUSTOM_CHOTROI
  if (gmProtect->MenuButtonChoTroi && count < maxEntries) {
    entries[count].ActionId = MENU_ACTION_CHO_TROI;
    entries[count].Label = ResolveMenuUserText(11, "Chợ Trời");
    ++count;
  }
#endif

  return count;
}
} // namespace

SEASON3B::CNewUIMenuUser::CNewUIMenuUser() {
  m_pNewUIMng = NULL;
  m_Pos.x = 0;
  m_Pos.y = 0;
}

SEASON3B::CNewUIMenuUser::~CNewUIMenuUser() { Release(); }

bool SEASON3B::CNewUIMenuUser::Create(CNewUIManager *pNewUIMng, float x,
                                      float y) {
  bool Success = false;

  if (pNewUIMng) {
    m_pNewUIMng = pNewUIMng;

    m_pNewUIMng->AddUIObj(INTERFACE_CUSTOM_MENU, this);

    this->LoadImages();

    this->SetPos(x, y);

    this->Show(false);

    Success = true;
  }
  return Success;
}

void SEASON3B::CNewUIMenuUser::Release() {
  if (m_pNewUIMng) {
    m_pNewUIMng->RemoveUIObj(this);

    this->UnloadImages();
  }
}

void SEASON3B::CNewUIMenuUser::SetPos(float x, float y) {
  m_Pos.x = x;
  m_Pos.y = y;
}

void SEASON3B::CNewUIMenuUser::LoadImages() {}

void SEASON3B::CNewUIMenuUser::UnloadImages() {}

bool SEASON3B::CNewUIMenuUser::UpdateKeyEvent() {
  if (IsVisible() == true) {
    if (SEASON3B::IsPress(VK_ESCAPE) || SEASON3B::IsPress(VK_F7)) {
      g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }

    return false;
  }
  return true;
}

bool SEASON3B::CNewUIMenuUser::UpdateMouseEvent() {
  if (IsVisible() == false) {
    return true;
  }

  float windowX = (gInterface->Data[eWindowMenuUser].FirstLoad
                       ? gInterface->Data[eWindowMenuUser].X
                       : (float)m_Pos.x);
  float windowY = (gInterface->Data[eWindowMenuUser].FirstLoad
                       ? gInterface->Data[eWindowMenuUser].Y
                       : (float)m_Pos.y);
  MenuEntry entries[kMenuMaxEntries];
  int entryCount = BuildMenuEntries(entries, kMenuMaxEntries);
  float menuWindowHeight = GetMenuWindowHeight(entryCount);

  GetCenteredMenuPosition(kMenuWindowWidth, menuWindowHeight, windowX, windowY);

  if (SEASON3B::CheckMouseIn(windowX + kMenuCloseOffsetX,
                             windowY + kMenuCloseOffsetY, kMenuCloseSize,
                             kMenuCloseSize)) {
    return false;
  }

  for (int i = 0; i < entryCount; ++i) {
    float entryY = windowY + kMenuEntryY + (i * kMenuEntryStep);

    if (SEASON3B::CheckMouseIn(windowX + kMenuEntryX, entryY, kMenuEntryWidth,
                               kMenuEntryHeight)) {
      if (SEASON3B::IsRelease(VK_LBUTTON)) {
        ExecuteMenuAction(entries[i].ActionId);
      }

      return false;
    }
  }

  return !SEASON3B::CheckMouseIn(windowX, windowY, kMenuWindowWidth,
                                 menuWindowHeight);
}

bool SEASON3B::CNewUIMenuUser::Render() {
  EnableAlphaTest(true);

  glColor4f(1.f, 1.f, 1.f, 1.f);

  RenderFrame();

  if (IsVisible() == false ||
      gInterface->Data[eWindowMenuUser].OnShow == false) {
    DisableAlphaBlend();
    return true;
  }

  RenderButtons();

  RenderTexte();

  DisableAlphaBlend();

  return true;
}

bool SEASON3B::CNewUIMenuUser::Update() {
  if (IsVisible() && gInterface->Data[eWindowMenuUser].OnShow == false) {
    g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
  }

  return true;
}

float SEASON3B::CNewUIMenuUser::GetLayerDepth() { return 10.0f; }

float SEASON3B::CNewUIMenuUser::GetKeyEventOrder() { return 9.5f; }

void SEASON3B::CNewUIMenuUser::OpenningProcess() {
  MenuEntry entries[kMenuMaxEntries];
  int entryCount = BuildMenuEntries(entries, kMenuMaxEntries);
  float windowX = 0.0f;
  float windowY = 0.0f;
  float menuWindowHeight = GetMenuWindowHeight(entryCount);

  ApplyCenteredMenuPosition(kMenuWindowWidth, menuWindowHeight, windowX,
                            windowY);
  SetPos(windowX, windowY);
  gInterface->Data[eWindowMenuUser].Open();
  gInterface->Data[eWindowMenuUser].AllowMove = false;
}

void SEASON3B::CNewUIMenuUser::ClosingProcess() {
  gInterface->Data[eWindowMenuUser].Close();
  gInterface->Data[eWindowMenuUser].FirstLoad = false;
}

void SEASON3B::CNewUIMenuUser::RenderFrame() {
  MenuEntry entries[kMenuMaxEntries];
  int entryCount = BuildMenuEntries(entries, kMenuMaxEntries);
  float menuWindowHeight = GetMenuWindowHeight(entryCount);
  float menuContentHeight = GetMenuContentHeight(entryCount);
  float x = 0.0f;
  float y = 0.0f;

  ApplyCenteredMenuPosition(kMenuWindowWidth, menuWindowHeight, x, y);

  if (gInterface->gDrawWindowCustom(
          &x, &y, kMenuWindowWidth, menuWindowHeight, eWindowMenuUser,
          gTextClient.txtClient_MenuUser[0]) == false ||
      gInterface->Data[eWindowMenuUser].OnShow == false) {
    g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
    return;
  }

  SetPos(x, y);
  gInterface->DrawInfoBox(x + kMenuContentX, y + kMenuContentY,
                          kMenuContentWidth, menuContentHeight, 0x00000096, 0,
                          0);
}

void SEASON3B::CNewUIMenuUser::RenderTexte() {
  float windowX = (float)m_Pos.x;
  float windowY = (float)m_Pos.y;
  MenuEntry entries[kMenuMaxEntries];
  int entryCount = BuildMenuEntries(entries, kMenuMaxEntries);

  g_pRenderText->SetBgColor(0);
  g_pRenderText->SetFont(g_hFontBold);
  g_pRenderText->SetTextColor(CLRDW_WHITE);

  SEASON3B::TextDraw((HFONT)g_hFontBold, (int)(windowX + kMenuHeaderX),
                     (int)(windowY + kMenuHeaderY), 0x19FF9FFF, 0x0, 0, 0, 1,
                     gTextClient.txtClient_MenuUser[1]);
  SEASON3B::TextDraw((HFONT)g_hFont, (int)(windowX + kMenuHeaderX),
                     (int)(windowY + kMenuHintY), 0xFFFFFFFF, 0x0, 0, 0, 1,
                     gTextClient.txtClient_MenuUser[2]);

  if (entryCount == 0) {
    SEASON3B::TextDraw((HFONT)g_hFont, (int)(windowX + kMenuHeaderX),
                       (int)(windowY + kMenuEntryY), 0xE0D19DFF, 0x0, 0, 0, 1,
                       gTextClient.txtClient_MenuUser[8]);
  }

  for (int i = 0; i < entryCount; ++i) {
    float entryY = windowY + kMenuEntryY + (i * kMenuEntryStep);
    DWORD textColor = (SEASON3B::CheckMouseIn(windowX + kMenuEntryX, entryY,
                                              kMenuEntryWidth, kMenuEntryHeight)
                           ? 0xFFF2C28C
                           : 0xFFFFFFFF);

    SEASON3B::TextDraw((HFONT)g_hFontBold, (int)(windowX + kMenuEntryX + 10.0f),
                       (int)(entryY + 5.0f), textColor, 0x0, 0, 0, 1, "%s",
                       entries[i].Label);
  }

  // SEASON3B::TextDraw((HFONT)g_hFont, (int)(windowX + kMenuHeaderX),
  // (int)(windowY + kMenuFooterY), 0xE0D19DFF, 0x0, 0, 0, 1, "F7 / ESC : Dong
  // Menu");
}

void SEASON3B::CNewUIMenuUser::RenderButtons() {
  float windowX = (float)m_Pos.x;
  float windowY = (float)m_Pos.y;
  MenuEntry entries[kMenuMaxEntries];
  int entryCount = BuildMenuEntries(entries, kMenuMaxEntries);

  for (int i = 0; i < entryCount; ++i) {
    float entryY = windowY + kMenuEntryY + (i * kMenuEntryStep);
    RenderButton(windowX + kMenuEntryX, entryY, kMenuEntryWidth,
                 kMenuEntryHeight);
  }
}

void SEASON3B::CNewUIMenuUser::RenderButton(float x, float y, float width,
                                            float height) {
  bool isHover = (SEASON3B::CheckMouseIn(x, y, width, height) == 1);

  gInterface->DrawBarForm(x, y, width, height, 0.04f, 0.04f, 0.06f,
                          (isHover ? 0.95f : 0.72f));
  gInterface->DrawBarForm(x, y, width, 1.0f, 0.38f, 0.29f, 0.14f,
                          (isHover ? 0.95f : 0.75f));
  gInterface->DrawBarForm(x, y + height - 1.0f, width, 1.0f, 0.15f, 0.15f,
                          0.18f, 0.95f);
  gInterface->DrawBarForm(x, y, 1.0f, height, 0.24f, 0.24f, 0.28f, 0.95f);
  gInterface->DrawBarForm(x + width - 1.0f, y, 1.0f, height, 0.24f, 0.24f,
                          0.28f, 0.95f);

  if (isHover) {
    gInterface->DrawBarForm(x + 2.0f, y + 2.0f, width - 4.0f, height - 4.0f,
                            0.20f, 0.23f, 0.27f, 0.70f);
  }
}

bool SEASON3B::CNewUIMenuUser::ExecuteMenuAction(int actionId) {
  switch (actionId) {
  case MENU_ACTION_EVENT_TIME:
    if (g_CustomEventTime->OnOffWindow()) {
      g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
      PlayBuffer(SOUND_CLICK01);
    }
    return true;

  case MENU_ACTION_VONG_QUAY:
    if (gInterface->Data[eWindowVongQuay].OnShow ||
        CanOpenTopMostOverlayWindow(eTopMostOverlayVongQuay)) {
      gVongQuay.OpenVongQuay();
      g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
      PlayBuffer(SOUND_CLICK01);
    }
    return true;

  case MENU_ACTION_MOC_NAP:
    if (gInterface->Data[eWindowMocNap].OnShow ||
        CanOpenTopMostOverlayWindow(eTopMostOverlayMocNap)) {
      gMocNap.OpenWindowMocNap();
      g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
      PlayBuffer(SOUND_CLICK01);
    }
    return true;

  case MENU_ACTION_VIP_SHOP:
    if (gmProtect->WindowsVipShop) {
      g_pNewUISystem->Show(INTERFACE_SHOW_VIP);
    }
    PlayBuffer(SOUND_CLICK01);
    return true;

  case MENU_ACTION_RANKING:
    if (gmProtect->WindowsRankTop &&
        (IsTopMostOverlayWindowVisible(eTopMostOverlayRankingTop) ||
         CanOpenTopMostOverlayWindow(eTopMostOverlayRankingTop))) {
      g_pNewUISystem->Show(INTERFACE_RANKING_TOP);
      PlayBuffer(SOUND_CLICK01);
    }
    return true;

  case MENU_ACTION_COMMAND:
    if (gmProtect->WindowsCommand) {
      g_pNewUISystem->Show(INTERFACE_COMMAND_LIST);
    }
    PlayBuffer(SOUND_CLICK01);
    return true;

  case MENU_ACTION_OPTIONS:
    g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
    g_pNewUISystem->Show(INTERFACE_OPTION);
    PlayBuffer(SOUND_CLICK01);
    return true;

#if CUSTOM_CHOTROI
  case MENU_ACTION_CHO_TROI:
    gCusChoTroi.GetOpenChoTroiWinDow();
    g_pNewUISystem->Hide(INTERFACE_CUSTOM_MENU);
    PlayBuffer(SOUND_CLICK01);
    return true;
#endif
  }

  return false;
}
