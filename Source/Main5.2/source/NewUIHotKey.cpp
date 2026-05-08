

#include "stdafx.h"

#include "CSChaosCastle.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "GMBattleCastle.h"
#include "MapManager.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "NewUIHotKey.h"
#include "NewUISystem.h"
#include "CBDrawInterface.h"
#include "UIMng.h"
#include "ZzzEffect.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "w_CursedTemple.h"
#include "wsclientinline.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "GameShop/InGameShopSystem.h"
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "CAIController.h"
#include "CGMProtect.h"
#include "CustomChoTroi.h"
#include "CustomEventTime.h"
#include "CustomVongQuay.h"
#include "H_ViewCharInfo.h"
#include "H_BotMix.h"

using namespace SEASON3B;

namespace {
bool s_blockEscapeAfterCloseEventTime = false;

bool IsReconnectWindowBlockingF7() {
  return (g_pNewUISystem != NULL &&
          g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RECONNECT));
}

bool IsCustomMenuChildWindowVisible() {
  if (gInterface->Data[eWindowEventTime].OnShow ||
      gInterface->Data[eWindowVongQuay].OnShow ||
      gInterface->Data[eWindowMocNap].OnShow ||
      gInterface->Data[eWindowMocNapList].OnShow) {
    return true;
  }

#if CUSTOM_CHOTROI
  if (gCusChoTroi.IsWindowActive()) {
    return true;
  }
#endif

  if (g_pNewUISystem == NULL) {
    return false;
  }

  return (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION) ||
          g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SHOW_VIP) ||
          g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP) ||
          g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND_LIST));
}

bool CloseCustomMenuChildWindowsForF7() {
  if (gInterface->Data[eWindowEventTime].OnShow) {
    gInterface->Data[eWindowEventTime].Close();
  }

  if (gInterface->Data[eWindowVongQuay].OnShow) {
    if (gVongQuay.ShouldBlockClose()) {
      return false;
    }

    gInterface->Data[eWindowVongQuay].OnShow = false;
  }

  if (gInterface->Data[eWindowMocNap].OnShow ||
      gInterface->Data[eWindowMocNapList].OnShow) {
    gInterface->Data[eWindowMocNapList].Close();
    gInterface->Data[eWindowMocNap].Close();
  }

#if CUSTOM_CHOTROI
  if (gCusChoTroi.IsWindowActive()) {
    gCusChoTroi.CloseWindow(true);
  }
#endif

  if (g_pNewUISystem == NULL) {
    return true;
  }

  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION)) {
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
  }

  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SHOW_VIP)) {
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_SHOW_VIP);
  }

  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_RANKING_TOP);
  }

  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND_LIST)) {
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
  }

  return true;
}

bool OpenCustomMenuWithF7() {
  if (!CloseCustomMenuChildWindowsForF7()) {
    return false;
  }

  if (g_pNewUISystem == NULL) {
    return false;
  }

  g_pNewUISystem->Show(SEASON3B::INTERFACE_CUSTOM_MENU);
  return true;
}
}

SEASON3B::CNewUIHotKey::CNewUIHotKey()
    : m_pNewUIMng(NULL), m_bStateGameOver(false) {
  s_blockEscapeAfterCloseEventTime = false;
}

SEASON3B::CNewUIHotKey::~CNewUIHotKey() { Release(); }

bool SEASON3B::CNewUIHotKey::Create(CNewUIManager *pNewUIMng) {
  if (NULL == pNewUIMng)
    return false;

  m_pNewUIMng = pNewUIMng;
  m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HOTKEY, this);
  Show(true);
  return true;
}

void SEASON3B::CNewUIHotKey::Release() {
  if (m_pNewUIMng) {
    m_pNewUIMng->RemoveUIObj(this);
    m_pNewUIMng = NULL;
  }
}

bool SEASON3B::CNewUIHotKey::UpdateMouseEvent() {
  extern int SelectedCharacter;

  CameraFactorPtr->ZoomInNearFar();

  CameraFactorPtr->RotateInNearFar();

  if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch)) {
    return true;
  }

  if (SelectedCharacter >= 0) {
    if (SEASON3B::IsRepeat(VK_MENU) && SEASON3B::IsRelease(VK_RBUTTON) &&
        gMapManager->InChaosCastle() == false &&
        gMapManager->IsCursedTemple() == false) {
      CHARACTER *pCha = gmCharacters->GetCharacter(SelectedCharacter);

      if (pCha->Object.Kind != KIND_PLAYER) {
        return false;
      }

      if ((pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_DEER) ||
          (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN) ||
          (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SSANTA)) {
        return false;
      }

      if (::IsStrifeMap(gMapManager->currentMap) &&
          Hero->m_byGensInfluence != pCha->m_byGensInfluence)
        return false;

      float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
      float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
      float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));

      if (fDistance < 300.f) {
        int x, y;
        x = MouseX + 10;
        y = MouseY - 50;
        if (y < 0) {
          y = 0;
        }
        g_pQuickCommand->OpenQuickCommand(pCha->ID, SelectedCharacter, x, y);
      } else {
        g_pChatListBox->AddText("", GlobalText[1388],
                                SEASON3B::TYPE_ERROR_MESSAGE);
        g_pQuickCommand->CloseQuickCommand();
      }

      return false;
    }
  }
  return true;
}

bool SEASON3B::CNewUIHotKey::UpdateKeyEvent() {
  if (s_blockEscapeAfterCloseEventTime &&
      ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0)) {
    s_blockEscapeAfterCloseEventTime = false;
  }

  if (SEASON3B::IsPress(VK_ESCAPE) == true) {
    if (s_blockEscapeAfterCloseEventTime) {
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      return false;
    }

#if CUSTOM_CHOTROI
    if (gCusChoTroi.HandleEscapeKey()) {
      s_blockEscapeAfterCloseEventTime = true;
      return false;
    }
#endif

    if (gInterface->Data[eWindowQuaPhucLoi].OnShow) {
      gInterface->Data[eWindowQuaPhucLoi].OnShow = false;
      s_blockEscapeAfterCloseEventTime = true;
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }

    if (gInterface->Data[eWindowEventTime].OnShow) {
      gInterface->Data[eWindowEventTime].Close();
      s_blockEscapeAfterCloseEventTime = true;
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }

    if (gInterface->Data[eWindowVongQuay].OnShow) {
      if (gVongQuay.ShouldBlockClose()) {
        SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
            VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
        return false;
      }
      gInterface->Data[eWindowVongQuay].OnShow = false;
      s_blockEscapeAfterCloseEventTime = true;
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }

    if (gInterface->Data[eWindowMocNap].OnShow ||
        gInterface->Data[eWindowMocNapList].OnShow) {
      gInterface->Data[eWindowMocNapList].Close();
      gInterface->Data[eWindowMocNap].Close();
      s_blockEscapeAfterCloseEventTime = true;
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }

#if(CUSTOM_CHANGEITEM)
    if (gInterface->Data[eWindowChangeItem].OnShow) {
      gInterface->Data[eWindowChangeItem].OnShow = false;
      s_blockEscapeAfterCloseEventTime = true;
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }
#endif
#if (H_VIEWCHARINFO)
    if (gH_ViewCharInfo != nullptr && gH_ViewCharInfo->ShouldBlockEscapeKey()) {
      return false;
    }
#endif
    if (gmAIController->IsRunning() == true) {
      SendRequestStartHelper(true);
    }
    g_pNewUIMiniMap->StopMove();

    if (g_MessageBox->IsEmpty()) {
      SEASON3B::CreateMessageBox(
          MSGBOX_LAYOUT_CLASS(SEASON3B::CSystemMenuMsgBoxLayout));
      PlayBuffer(SOUND_CLICK01);
      return false;
    }
  }

  if (m_bStateGameOver == true)
    return false;

  if (SEASON3B::IsPress(VK_F7) && IsReconnectWindowBlockingF7()) {
    return false;
  }

  if (SEASON3B::IsPress(VK_F7) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CUSTOM_MENU) &&
      IsCustomMenuChildWindowVisible()) {
    if (OpenCustomMenuWithF7()) {
      PlayBuffer(SOUND_CLICK01);
    }
    return false;
  }

#if CUSTOM_CHOTROI
  if (gCusChoTroi.IsBuyPassWindowActive()) {
    return false;
  }
#endif

#if(CUSTOM_CHANGEITEM)
  if (gInterface->Data[eWindowChangeItem].OnShow) {
    return false;
  }
#endif

  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CUSTOM_MENU)) {
    return false;
  }

  if (gCB_BotMix != nullptr && gCB_BotMix->IsInputLockActive()) {
    return false;
  }

  if (gInterface->Data[eWindowQuaPhucLoi].OnShow) {
    //if (SEASON3B::IsPress(VK_F8) || SEASON3B::IsPress(VK_TAB) || SEASON3B::IsPress('H')) {
      return false;
    //}
  }

  if (gInterface->Data[eWindowVongQuay].OnShow) {
    return false;
  }

  if (gInterface->Data[eWindowMocNap].OnShow ||
      gInterface->Data[eWindowMocNapList].OnShow) {
    return false;
  }

#if MAIN_UPDATE > 303
  if (SEASON3B::IsPress(VK_TAB) == false &&
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP) == true) {
    return false;
  }
#endif

  if (SEASON3B::IsPress(VK_F10)) {
    CameraFactorPtr->Toggle();
    PlayBuffer(SOUND_CLICK01, 0, 0);
  }
  if (SEASON3B::IsPress(VK_F11)) {
    CameraFactorPtr->Backup();
    PlayBuffer(SOUND_CLICK01, 0, 0);
  }

  if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch)) {
    if (SEASON3B::IsPress('M') == true) {
      g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
      PlayBuffer(SOUND_CLICK01);
    }
    return false;
  }

  if (AutoGetItem() == true)
    return false;

  if (CanUpdateKeyEventRelatedMyInventory() != true) {
    if (CanUpdateKeyEvent() == false)
      return true;

#if (H_VIEWCHARINFO)
    if (gH_ViewCharInfo != nullptr && gH_ViewCharInfo->IsOpen()) {
      if (SEASON3B::IsPress(VK_F7) || SEASON3B::IsPress(VK_F8)) {
        return false;
      }
    }
#endif
    if (SEASON3B::IsPress(VK_F7)) {
      if (IsReconnectWindowBlockingF7()) {
        return false;
      }

      if (OpenCustomMenuWithF7()) {
        PlayBuffer(SOUND_CLICK01);
      }
      return false;
    }

    if (SEASON3B::IsPress('F')) {
      if (gMapManager->InChaosCastle() == true) {
        return true;
      }

      int iLevel = CharacterAttribute->Level;

      if (iLevel > 6) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
      } else {
        if (g_pChatListBox->CheckChatRedundancy(GlobalText[1067]) == FALSE) {
          g_pChatListBox->AddText("", GlobalText[1067],
                                  SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
      }

      PlayBuffer(SOUND_CLICK01);
      return false;
    }

    if (SEASON3B::IsPress('I') || SEASON3B::IsPress('V')) {
      if (g_pNPCShop->IsSellingItem() == false) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
    } else {
      if (SEASON3B::IsPress('C')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('J') &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY_JEWEL)) {
          g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY_JEWEL);
          g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
        } else {
          g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
          g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY_JEWEL);
        }
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('O') &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_OPTION);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('T')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYQUEST);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('P')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PARTY);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('G')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GUILDINFO);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }

#if MAIN_UPDATE > 303
      if (GMProtect->IsMasterSkill() && SEASON3B::IsPress('A') &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        if (Hero->IsMasterLevel() == true) {
          g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
        }
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
      if (SEASON3B::IsPress('U') &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
#endif
      if (gMapManager->InChaosCastle() == false && SEASON3B::IsPress('D')) {
        if (::IsStrifeMap(gMapManager->currentMap)) {
          if (g_pChatListBox->CheckChatRedundancy(GlobalText[2989]) == FALSE)
            g_pChatListBox->AddText("", GlobalText[2989],
                                    SEASON3B::TYPE_SYSTEM_MESSAGE);
        } else {
          g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
          PlayBuffer(SOUND_CLICK01);
        }
        return false;
      }
      if (SEASON3B::IsPress('M') == true) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
        PlayBuffer(SOUND_CLICK01);

        return false;
      }
      if (SEASON3B::IsPress(VK_TAB) == true &&
          gMapManager->InBattleCastle() == true &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_SIEGEWARFARE);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }

      if (SEASON3B::IsPress(VK_TAB) == true &&
          !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP)) {
        if (IsTopMostOverlayWindowVisible(eTopMostOverlayMiniMap) ||
            CanOpenTopMostOverlayWindow(eTopMostOverlayMiniMap)) {
          if (g_pNewUIMiniMap->IsSuccess())
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MINI_MAP);
          else
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
          PlayBuffer(SOUND_CLICK01);
        }
        return false;
      }

      if (GMProtect->IsInGameShop() && SEASON3B::IsPress('X') == true) {
        if (g_pInGameShop->IsInGameShopOpen() == false)
          return false;

        if (g_InGameShopSystem->IsScriptDownload() == true) {
          if (g_InGameShopSystem->ScriptDownload() == false)
            return false;
        }
        if (g_InGameShopSystem->IsBannerDownload() == true) {
          if (g_InGameShopSystem->BannerDownload() == true) {
            g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(),
                                      g_InGameShopSystem->GetBannerURL());
          }
        }
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) ==
            false) {
          if (g_InGameShopSystem->GetIsRequestShopOpenning() == false) {
            SendRequestIGS_CashShopOpen(0);
            g_InGameShopSystem->SetIsRequestShopOpenning(true);
            g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
          }
        } else {
          SendRequestIGS_CashShopOpen(1);
          g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
        }
        return false;
      }
#if MAIN_UPDATE > 303
      if (SEASON3B::IsPress('B')) {
        if (!g_pNewUIGensRanking->SetGensInfo())
          return false;

        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GENSRANKING);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
#endif
      if (GMProtect->IsInvExt() && SEASON3B::IsPress('K')) {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)) {
          if (g_pNPCShop->IsSellingItem() == false) {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY_EXTENSION);
            PlayBuffer(SOUND_CLICK01);
          }
        }
        return false;
      }

      if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) == false) {
        if (gmProtect->WindowsRankTop == TRUE && SEASON3B::IsPress(VK_F8)) {
          if (IsTopMostOverlayWindowVisible(eTopMostOverlayRankingTop) ||
              CanOpenTopMostOverlayWindow(eTopMostOverlayRankingTop)) {
            g_pNewUISystem->Toggle(INTERFACE_RANKING_TOP);
            PlayBuffer(SOUND_CLICK01);
          }
          return false;
        }
        if (gmProtect->WindowsEventTime == TRUE && SEASON3B::IsPress('H')) {
          if (g_CustomEventTime->OnOffWindow()) {
            PlayBuffer(SOUND_CLICK01);
          }
          return false;
        }
      }

#ifdef EFFECT_KERNEL_VERTEX
      if (SEASON3B::IsPress('Y')) {
        if (FindText(Hero->ID, "webzen") ||
            g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) ||
            ((Hero->CtlCode == CTLCODE_20OPERATOR) ||
             (Hero->CtlCode == CTLCODE_08OPERATOR))) {
          if (g_MessageBox->IsEmpty()) {
            SEASON3B::CreateMessageBox(
                MSGBOX_LAYOUT_CLASS(CNewUIMakeRenderMessageBoxLayout));
            PlayBuffer(SOUND_CLICK01);
          }
          return false;
        }
      }
#endif // EFFECT_KERNEL_VERTEX
    }
    if (GMProtect->shutdown_oficial_helper()) {
#if MAIN_UPDATE > 303
      if (SEASON3B::IsPress(VK_HOME) && GetFocus() == gwinhandle->GethWnd()) {
        if (gmAIController->IsRunning() == true)
          SendRequestStartHelper(true);
        else
          SendRequestStartHelper(false);
      } else if (SEASON3B::IsPress('Z')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MACRO_OFICIAL);
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MACRO_OFICIAL_SUB)) {
          g_pNewUISystem->Hide(SEASON3B::INTERFACE_MACRO_OFICIAL_SUB);
        }
      }
#else
      if (SEASON3B::IsPress(VK_F9) && GetFocus() == gwinhandle->GethWnd()) {
        if (gmAIController->IsRunning() == true)
          SendRequestStartHelper(true);
        else
          SendRequestStartHelper(false);
      } else if (SEASON3B::IsPress('Z')) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MACRO_OFICIAL);

        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MACRO_OFICIAL_SUB)) {
          g_pNewUISystem->Hide(SEASON3B::INTERFACE_MACRO_OFICIAL_SUB);
        }
      }
#endif
    }
    return true;
  }

  if (SEASON3B::IsPress('I') || SEASON3B::IsPress('V')) {
    if (g_pNPCShop->IsSellingItem() == false) {
      g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
      PlayBuffer(SOUND_CLICK01);
      return false;
    }
    return true;
  }

  if (GMProtect->IsInvExt() && SEASON3B::IsPress('K')) {
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)) {
      if (g_pNPCShop->IsSellingItem() == false) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY_EXTENSION);
        PlayBuffer(SOUND_CLICK01);
      }
    }
    return false;
  }

  if (GMProtect->IsBaulExt() && SEASON3B::IsPress('H')) {
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)) {
      if (g_pStorageExpansion->IsExpansionStorage()) {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_STORAGE_EXTENSION);
        PlayBuffer(SOUND_CLICK01);
        return false;
      }
    }
  }

  return true;
}

bool SEASON3B::CNewUIHotKey::Update() { return true; }

bool SEASON3B::CNewUIHotKey::Render() { return true; }

bool SEASON3B::CNewUIHotKey::CanUpdateKeyEventRelatedMyInventory() {
  if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)) {
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)) {
      if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)) {
        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)) {
          if (!g_pNewUISystem->IsVisible(
                  SEASON3B::INTERFACE_MYSHOP_INVENTORY)) {
            if (!g_pNewUISystem->IsVisible(
                    SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)) {
              if (!g_pNewUISystem->IsVisible(
                      SEASON3B::INTERFACE_LUCKYITEMWND)) {
                return false;
              }
            }
          }
        }
      }
    }
  }
  return true;
}

bool SEASON3B::CNewUIHotKey::CanUpdateKeyEvent() {
  if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CATAPULT) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCQUEST) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SENATUS) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATEKEEPER) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUARDSMAN) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATESWITCH) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DUELWATCH) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN) ||
      g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC) ||
      g_pNewUISystem->IsVisible(
          SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)) {
    return false;
  }

  return true;
}

float SEASON3B::CNewUIHotKey::GetLayerDepth() { return 1.0f; }

float SEASON3B::CNewUIHotKey::GetKeyEventOrder() { return 1.0f; }

void SEASON3B::CNewUIHotKey::SetStateGameOver(bool bGameOver) {
  m_bStateGameOver = bGameOver;
}

bool SEASON3B::CNewUIHotKey::IsStateGameOver() { return m_bStateGameOver; }

bool SEASON3B::CNewUIHotKey::AutoGetItem() {
  if (CNewUIInventoryCtrl::GetPickedItem() == NULL) {
    if (SEASON3B::IsPress(VK_SPACE)) {
      if (g_pChatInputBox->HaveFocus() == false) {
        if (SEASON3B::CheckMouseIn(0, 0, GetScreenWidth(),
                                   (int)(GetWindowsY - 51))) {
          for (int i = 0; i < MAX_ITEMS; ++i) {
            OBJECT *pObj = &Items[i].Object;

            if (pObj->Live && pObj->Visible) {
              vec3_t vDir;
              VectorSubtract(pObj->Position, Hero->Object.Position, vDir);
              if (VectorLength(vDir) < 300) {
                Hero->MovementType = MOVEMENT_GET;
                ItemKey = i;
                g_bAutoGetItem = true;
                Action(Hero, pObj, true);
                Hero->MovementType = MOVEMENT_MOVE;
                g_bAutoGetItem = false;

                return true;
              }
            }
          }
        }
      }
    }
  }

  return false;
}
