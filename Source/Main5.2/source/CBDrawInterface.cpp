
#include "stdafx.h"
#include "APICB.h"
#include "CBDrawInterface.h"
#include "CBInterface.h"
#include "CB_QuaPhucLoi.h"
#include "CustomChoTroi.h"
#include "CustomEventTime.h"
#include "CustomMocNap.h"
#include "CustomVongQuay.h"
#include "H_BotMix.h"
#include "H_RankingDmgBoss.h"
#include "H_ViewCharInfo.h"
#include "NewUICommon.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "TextClient.h"
#include "UIBaseDef.h"
#include "UIControls.h"
#include "ZzzInterface.h"
#include "jpexs.h"
#include "wsclientinline.h"

using namespace SEASON3B;

namespace {
const eTopMostOverlayWindow kTopMostOverlayWindows[] = {
    eTopMostOverlayQuaPhucLoi, eTopMostOverlayEventTime,
    eTopMostOverlayVongQuay,   eTopMostOverlayMocNap,
    eTopMostOverlayMiniMap,    eTopMostOverlayRankingTop,
    eTopMostOverlayChangeItem,
};
}

bool IsTopMostOverlayWindowVisible(eTopMostOverlayWindow windowType) {
  switch (windowType) {
  case eTopMostOverlayQuaPhucLoi:
    return (gInterface->Data[eWindowQuaPhucLoi].OnShow != 0);
  case eTopMostOverlayEventTime:
    return (gInterface->Data[eWindowEventTime].OnShow != 0);
  case eTopMostOverlayVongQuay:
    return (gInterface->Data[eWindowVongQuay].OnShow != 0);
  case eTopMostOverlayMocNap:
    return (gInterface->Data[eWindowMocNap].OnShow != 0 ||
            gInterface->Data[eWindowMocNapList].OnShow != 0);
  case eTopMostOverlayMiniMap:
    return (g_pNewUISystem &&
            g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP));
  case eTopMostOverlayRankingTop:
    return (g_pNewUISystem &&
            g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_RANKING_TOP));
  case eTopMostOverlayChangeItem:
    return (gInterface->Data[eWindowChangeItem].OnShow != 0 ||
            gInterface->SetStateDoiItem != 0);
  }

  return false;
}

bool CanOpenTopMostOverlayWindow(eTopMostOverlayWindow windowType) {
  for (size_t i = 0;
       i < (sizeof(kTopMostOverlayWindows) / sizeof(kTopMostOverlayWindows[0]));
       ++i) {
    if (kTopMostOverlayWindows[i] != windowType &&
        IsTopMostOverlayWindowVisible(kTopMostOverlayWindows[i])) {
      return false;
    }
  }

  return true;
}

void RenderTopMostOverlayWindows() {
  if (gCB_QuaPhucLoi &&
      IsTopMostOverlayWindowVisible(eTopMostOverlayQuaPhucLoi)) {
    gCB_QuaPhucLoi->DrawWindow();
  }

  if (g_CustomEventTime &&
      IsTopMostOverlayWindowVisible(eTopMostOverlayEventTime)) {
    g_CustomEventTime->DrawEventTimePanelWindow();
  }

  if (g_pNewUIMiniMap &&
      IsTopMostOverlayWindowVisible(eTopMostOverlayMiniMap)) {
    g_pNewUIMiniMap->RenderTopMost();
  }

  if (gNewUIRankingTop &&
      IsTopMostOverlayWindowVisible(eTopMostOverlayRankingTop)) {
    gNewUIRankingTop->RenderTopMost();
  }

#if (CUSTOM_CHANGEITEM)
  if (IsTopMostOverlayWindowVisible(eTopMostOverlayChangeItem)) {
    gInterface->DrawChangeItem();
  }
#endif
}

CBDrawInterface *CBDrawInterface::Instance() {
  static CBDrawInterface s_Instance;
  return &s_Instance;
}

CBDrawInterface::CBDrawInterface() {}

CBDrawInterface::~CBDrawInterface() {}
DWORD SleepTimeHP = 0;
void CBDrawInterface::RenderFrame() {
#if (ANTIHACK_GGNEW)
    gAPICB.Work();
#endif

    // Auto HP Icon rendering
// 1. Cho nút này ẩn sau phím M (MoveMap), phím A (Master Skill) và phím X
// (InGameShop) để tránh đè lên UI
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP) ||
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL) ||
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP)) {
        return;
    }

    float fAutoPKX = 4.0f;
    // 3. Di chuyển nút xuống dưới dòng chữ "Cấp tài khoản"
    // Tăng giá trị Y để đẩy Element xuống dưới (Mặc định ở MU là 0 từ trên cùng)
    float fAutoPKY = 140.0f;

    bool bMouseHover =
        (SEASON3B::CheckMouseIn(fAutoPKX, fAutoPKY, 20.0f, 20.0f) == 1);

    EnableAlphaTest(true);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    BITMAP_t* pIcon = &Bitmaps[IMAGE_AUTOPK_ICON];
    float uWidth = pIcon->output_width / 6.0f;
    float uHeight = pIcon->output_height / 3.0f;

    float su = (uWidth * 1.0f) / pIcon->Width; // Cột thứ 2
    float sv = 0.0f;                           // Row 1 (Xám)

    if (gInterface->AutoPK) {
        sv = (uHeight * 1.0f) / pIcon->Height; // Row 2 (Xanh)
    }
    else if (bMouseHover) {
        sv = (uHeight * 2.0f) / pIcon->Height; // Row 3 (Cam)
    }

    RenderBitmap(IMAGE_AUTOPK_ICON, fAutoPKX, fAutoPKY, 20.0f, 20.0f, su, sv,
        uWidth / pIcon->Width, uHeight / pIcon->Height, true, true, 0.0);

    if (bMouseHover) {
        // Kích hoạt cờ khóa click chuột xuyên qua UI của CNewUIMainFrameWindow ->
        // ngưng di chuyển NV
        gInterface->SetBlockCur(true);

        RenderTipText((int)(fAutoPKX + 50), (int)fAutoPKY + 5, "AUTO PK+HP");

        if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) {
            if ((GetKeyState(VK_LBUTTON) & 0x8000)) {
                gInterface->Data[eTIME].EventTick = GetTickCount();
                PlayBuffer(25, 0, 0);
                gInterface->AutoPK = !gInterface->AutoPK;
            }
        }
    }

    g_pRenderText->SetFont(g_hFontBold);
    // 2. Xóa cái viền nền bị ám màu trong lần bật V/C/Tab trước đó
    g_pRenderText->SetBgColor(0);

    if (gInterface->AutoPK) {
        g_pRenderText->SetTextColor(0, 255, 0, 255);
        g_pRenderText->RenderText((int)(fAutoPKX + 25), (int)(fAutoPKY + 5), "ON");

        // Auto bơm máu ở mức 100%
        if (CharacterAttribute->Life < CharacterAttribute->LifeMax) {
            if (GetTickCount() - SleepTimeHP > 300) {
                SleepTimeHP = GetTickCount();

                // Quét túi đồ cá nhân + túi mở rộng lấy bình máu
                // Ưu tiên: Máu Lớn (3) -> Máu Vừa (2) -> Máu Nhỏ (1) -> Apple (0)
                int hpIndex = -1;
                if ((hpIndex = g_pMyInventory->FindItemReverseIndex(ITEM_POTION + 3)) !=
                    -1) {
                    SendRequestUse(hpIndex, 0);
                }
                else if ((hpIndex = g_pMyInventory->FindItemReverseIndex(ITEM_POTION +
                    2)) != -1) {
                    SendRequestUse(hpIndex, 0);
                }
                else if ((hpIndex = g_pMyInventory->FindItemReverseIndex(ITEM_POTION +
                    1)) != -1) {
                    SendRequestUse(hpIndex, 0);
                }
                else if ((hpIndex = g_pMyInventory->FindItemReverseIndex(ITEM_POTION +
                    0)) != -1) {
                    SendRequestUse(hpIndex, 0);
                }
            }
        }
    }
    else {
        g_pRenderText->SetTextColor(255, 0, 0, 255);
        g_pRenderText->RenderText((int)(fAutoPKX + 25), (int)(fAutoPKY + 5), "OFF");
    }

    // Auto HP Icon rendering
    // float fAutoHPX = 4.0f;
    // float fAutoHPY = fAutoPKY + 25.0f; // Ngay dưới AutoPK

    // bool bMouseHoverPK =
    //     (SEASON3B::CheckMouseIn(fAutoPKX, fAutoPKY, 20.0f, 20.0f) == 1);

    // EnableAlphaTest(true);
    // if (bMouseHoverPK) {
    //   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    // } else {
    //   glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    // }
    // SEASON3B::RenderImage(IMAGE_AUTOPK_ICON, fAutoHPX, fAutoHPY, 20.0f, 20.0f);

    // if (bMouseHoverPK) {
    //   gInterface->SetBlockCur(true);

    //  RenderTipText((int)(fAutoHPX + 50), (int)fAutoHPY + 5, "AUTO HP");

    //  if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) {
    //    if ((GetKeyState(VK_LBUTTON) & 0x8000)) {
    //      gInterface->Data[eTIME].EventTick = GetTickCount();
    //      PlayBuffer(25, 0, 0);
    //      gInterface->AutoHP = !gInterface->AutoHP;
    //    }
    //  }
    //}

    // g_pRenderText->SetFont(g_hFontBold);
    // g_pRenderText->SetBgColor(0);

    // if (gInterface->AutoHP) {
    //   g_pRenderText->SetTextColor(0, 255, 0, 255);
    //   g_pRenderText->RenderText((int)(fAutoHPX + 25), (int)(fAutoHPY + 5),
    //   "ON");
    // } else {
    //   g_pRenderText->SetTextColor(255, 0, 0, 255);
    //   g_pRenderText->RenderText((int)(fAutoHPX + 25), (int)(fAutoHPY + 5),
    //   "OFF");
    // }
#if (H_VIEWCHARINFO)
  if (gH_ViewCharInfo)
    gH_ViewCharInfo->DrawWindow();
#endif

  if (SEASON3B::IsPress(VK_F3)) {
    if (g_pNewUISystem &&
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CUSTOM_MENU)) {
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_F3, SEASON3B::CNewKeyInput::KEY_NONE);
    } else if (gCB_BotMix && gCB_BotMix->IsInputLockActive()) {
      SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(
          VK_F3, SEASON3B::CNewKeyInput::KEY_NONE);
    } else if (gCB_QuaPhucLoi) {
      gCB_QuaPhucLoi->OpenWindow();
    }
  }

#if (H_RANKINGDMGBOSS)
  gDmgBoss.DmgDraw();
#endif

  gVongQuay.DrawWindowVQ();
  gMocNap.DrawWindow();
  gMocNap.DrawXemMocNap();
#if CUSTOM_CHOTROI
  gCusChoTroi.BDrawWindowChoTroi();
#endif

  gInterface->DrawMessageBox();

}
