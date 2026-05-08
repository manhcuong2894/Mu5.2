
#include "stdafx.h"
#include "CustomVongQuay.h"
#include "CBInterface.h"
#include "Interface.h"
#include "NewUIQuestProgress.h"
#include "NewUISystem.h"
#include "UIControls.h"
#include "Util.h"
#include "ZzzBMD.h"
#include "ZzzInventory.h"
#include <TextClient.h>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

using namespace SEASON3B;

extern char GetSoVong[5];
extern int AlphaBlendType;

namespace {
void ResetSpinInputState();
void EnsureSpinCountInput(float posX, float posY, float width, float height);
void NormalizeSpinCountInput(bool syncInputBox);
void ReleaseSpinCountInputFocus();
} // namespace

CVongQuay gVongQuay;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVongQuay::CVongQuay() { this->Init(); }

CVongQuay::~CVongQuay() {}

void CVongQuay::Init() {
  this->ListItemVongQuay.clear();
  this->DanhSachVongQuay.clear();
  this->StartRollSau = 0;
  this->IndexItemSau = -1;
  this->IndexYC = -1;
  this->CountItem = 0;
  this->WCYC = 0;
  this->WPYC = 0;
  this->GPYC = 0;
  this->DiemTichLuyVQ = 0;
  this->NhanThuongTichLuyVQ = 0;
  this->ResetTichLuyTime = 0;
  this->TichLuyPage = 0;
  this->ListTichLuyVongQuay.clear();
  GetSoVong[0] = '1';
  GetSoVong[1] = 0;
}

void CVongQuay::OpenVongQuay() {
  if ((GetTickCount() - gInterface->Data[eWindowVongQuay].EventTick) <= 300) {
    return;
  }

  if (gInterface->Data[eWindowVongQuay].OnShow && this->ShouldBlockClose()) {
    return;
  }

  gInterface->Data[eWindowVongQuay].EventTick = GetTickCount();

  if (gInterface->Data[eWindowVongQuay].OnShow) {
    gInterface->Data[eWindowVongQuay].OnShow = 0;
    ResetSpinInputState();

    return;
  }

  XULY_CGPACKET pMsg;
  pMsg.header.set(0xD3, 0x8C, sizeof(pMsg));
  pMsg.ThaoTac = 1;
  DataSend((LPBYTE)&pMsg, pMsg.header.size);

  pMsg.header.set(0xD3, 0x8B, sizeof(pMsg));
  pMsg.ThaoTac = 1; //
  DataSend((LPBYTE)&pMsg, pMsg.header.size);
}

CNewUIScrollBar *ListVongQuay = nullptr;

int SelectTypeVQ = 1;
int Chay = -1;
float CountVong = 0.5;
bool KickHoatQuay = false;
DWORD LastRollFrameTick = 0;
CUITextInputBox *InputSoVong = NULL;
char GetSoVong[5];
int PageQuay = 0;
namespace {
const DWORD kRollClickDelayMs = 300;
const DWORD kRollFastDelayMs = 55;
const DWORD kRollSlowDelayMs = 145;
const DWORD kRollSlowStepMs = 8;
const DWORD kQueuedBatchSpinDelayMs = 500;
const int kRollMinSlowdownSteps = 8;
const int kRollMaxSlowdownSteps = 12;

DWORD LastRollRequestTick = 0;
DWORD LastQueuedRollTick = 0;
DWORD CurrentRollDelayMs = kRollFastDelayMs;
int PendingSlowdownSteps = 0;
int RequestedSpinCount = 1;
int PendingQueuedSpinCount = 0;
DWORD RandomSpinSeed = 0;

void RestoreVongQuayAlphaBlendState(int alphaBlendType) {
  switch (alphaBlendType) {
  case 0:
    DisableAlphaBlend();
    break;
  case 1:
    EnableLightMap();
    break;
  case 2:
    EnableAlphaTest(true);
    break;
  case 3:
    EnableAlphaBlend();
    break;
  case 4:
    EnableAlphaBlendMinus();
    break;
  case 5:
    EnableAlphaBlend2();
    break;
  case 6:
    EnableAlphaBlend3();
    break;
  case 7:
    EnableAlphaBlend4();
    break;
  default:
    DisableAlphaBlend();
    break;
  }
}

void RenderVongQuayLocalItem3D(float sx, float sy, float width, float height,
                               int type, int level, int option1,
                               int extOption, bool pickUp, float scale) {
  int backupAlphaBlendType = AlphaBlendType;
  SEASON3B::RenderLocalItem3D(sx, sy, width, height, type, level, option1,
                              extOption, pickUp, scale);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  RestoreVongQuayAlphaBlendState(backupAlphaBlendType);
}

void ResetSpinAnimation() {
  KickHoatQuay = false;
  LastRollFrameTick = 0;
  LastQueuedRollTick = 0;
  CurrentRollDelayMs = kRollFastDelayMs;
  PendingSlowdownSteps = 0;
  RequestedSpinCount = 1;
  PendingQueuedSpinCount = 0;
}

void ResetSpinInputState() {
  bool hadInputFocus = (InputSoVong != NULL) && InputSoVong->HaveFocus();

  GetSoVong[0] = '1';
  GetSoVong[1] = 0;

  if (InputSoVong != NULL) {
    InputSoVong->SetText(GetSoVong);
    InputSoVong->SetState(UISTATE_HIDE);
    delete InputSoVong;
    InputSoVong = NULL;
  }

  if (hadInputFocus) {
    SetFocus(gwinhandle->GethWnd());
  }
}

void EnsureSpinCountInput(float posX, float posY, float width, float height) {
  if (InputSoVong == NULL) {
    gInterface->RenderInputBox(posX, posY, width, height, GetSoVong,
                               InputSoVong, UIOPTION_NUMBERONLY,
                               sizeof(GetSoVong) - 1, false);
  }

  if (InputSoVong != NULL) {
    InputSoVong->SetState(UISTATE_NORMAL);
    InputSoVong->SetTextColor(255, 255, 157, 0);
    InputSoVong->SetBackColor(0, 0, 0, 0);
    InputSoVong->SetPosition(posX, posY);
  }
}

void NormalizeSpinCountInput(bool syncInputBox) {
  if (GetSoVong[0] != 0 && atoi(GetSoVong) < 1) {
    GetSoVong[0] = '1';
    GetSoVong[1] = 0;

    if (syncInputBox && InputSoVong != NULL) {
      InputSoVong->SetText(GetSoVong);
    }
  }
}

void ReleaseSpinCountInputFocus() {
  if (InputSoVong != NULL && InputSoVong->HaveFocus()) {
    SetFocus(gwinhandle->GethWnd());
  }
}

int GetRandomSpinIndex(int itemCount, int currentIndex) {
  if (itemCount <= 0) {
    return -1;
  }

  if (itemCount == 1) {
    return 0;
  }

  if (RandomSpinSeed == 0) {
    RandomSpinSeed = GetTickCount();
  }

  RandomSpinSeed = (RandomSpinSeed * 1103515245) + 12345 + GetTickCount();
  int nextIndex = (int)((RandomSpinSeed >> 16) % itemCount);

  if (nextIndex == currentIndex) {
    nextIndex =
        (nextIndex + 1 + (int)(RandomSpinSeed % (itemCount - 1))) % itemCount;
  }

  return nextIndex;
}

void StartSpinAnimation() {
  gVongQuay.StartRollSau = 1;
  gVongQuay.IndexItemSau = -1;

  if (gVongQuay.ListItemVongQuay.empty()) {
    Chay = -1;
  } else if (Chay < 0 || Chay >= (int)gVongQuay.ListItemVongQuay.size()) {
    Chay = GetRandomSpinIndex((int)gVongQuay.ListItemVongQuay.size(), Chay);
  }

  CurrentRollDelayMs = kRollFastDelayMs;
  PendingSlowdownSteps = 0;
  LastRollFrameTick = GetTickCount();
  LastRollRequestTick = LastRollFrameTick;
}

void SendSpinRequest(DWORD spinMode) {
  CVongQuay::XULY_CGPACKET_SOLAN pMsg;
  pMsg.header.set(0xD3, 0x8A, sizeof(pMsg));
  pMsg.ThaoTac = SelectTypeVQ;
  pMsg.SoLan = spinMode;
  DataSend((LPBYTE)&pMsg, pMsg.header.size);
  StartSpinAnimation();
  LastQueuedRollTick = LastRollFrameTick;
}

void QueueSpinResultStop(int itemCount, int resultIndex) {
  if (itemCount <= 0 || resultIndex < 0 || resultIndex >= itemCount) {
    PendingSlowdownSteps = 0;
    return;
  }

  int extraSteps = itemCount;

  if (extraSteps < kRollMinSlowdownSteps) {
    extraSteps = kRollMinSlowdownSteps;
  }

  if (extraSteps > kRollMaxSlowdownSteps) {
    extraSteps = kRollMaxSlowdownSteps;
  }

  PendingSlowdownSteps = extraSteps;

  if (CurrentRollDelayMs < (kRollFastDelayMs + kRollSlowStepMs)) {
    CurrentRollDelayMs = kRollFastDelayMs + kRollSlowStepMs;
  }
}

float GetTextPixelWidth(const std::string &text) {
  if (text.empty()) {
    return 0.0f;
  }

  SIZE textSize = {0, 0};
  g_pRenderText->SetFont(g_hFont);
  g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(),
                                          text.c_str(), lstrlen(text.c_str()),
                                          &textSize);

  return (float)textSize.cx / g_fScreenRate_x;
}

void PushWrappedWord(std::vector<std::string> &lines, std::string &line,
                     const std::string &word, float maxWidth) {
  if (word.empty()) {
    return;
  }

  std::string candidate = line.empty() ? word : (line + " " + word);

  if (line.empty() || GetTextPixelWidth(candidate) <= maxWidth) {
    line = candidate;
    return;
  }

  lines.push_back(line);
  line = word;
}

std::vector<std::string> WrapTooltipText(const char *text, float maxWidth) {
  std::vector<std::string> lines;
  std::string line;
  std::string word;

  if (text == NULL || text[0] == 0) {
    return lines;
  }

  for (const char *cursor = text;; cursor++) {
    char ch = *cursor;

    if (ch == ' ' || ch == '\n' || ch == 0) {
      PushWrappedWord(lines, line, word, maxWidth);
      word.clear();

      if (ch == '\n' && !line.empty()) {
        lines.push_back(line);
        line.clear();
      }

      if (ch == 0) {
        break;
      }

      continue;
    }

    word += ch;
  }

  if (!line.empty()) {
    lines.push_back(line);
  }

  return lines;
}

bool IsConfiguredClientText(const char *text) {
  return text != NULL && text[0] != 0 && strcmp(text, "Null") != 0;
}

const char *GetTichLuyResetText() {
  int textIndex = 15;

  if (gVongQuay.ResetTichLuyTime == 1) {
    textIndex = 16;
  } else if (gVongQuay.ResetTichLuyTime == 2) {
    textIndex = 17;
  }

  if (IsConfiguredClientText(gTextClient.txtClient_VQMM[textIndex])) {
    return gTextClient.txtClient_VQMM[textIndex];
  }

  if (gVongQuay.ResetTichLuyTime == 1) {
    return "0h thứ 2 hàng tuần";
  }

  if (gVongQuay.ResetTichLuyTime == 2) {
    return "0h ngày đầu tiên của tháng";
  }

  return "0h hàng ngày";
}

const char *GetTichLuyHelpText() {
  static char helpText[512];
  const char *configText = gTextClient.txtClient_VQMM[14];

  if (IsConfiguredClientText(configText)) {
    std::string text = configText;
    const char *resetText = GetTichLuyResetText();
    size_t tokenPos = 0;

    while ((tokenPos = text.find("%s", tokenPos)) != std::string::npos) {
      text.replace(tokenPos, 2, resetText);
      tokenPos += strlen(resetText);
    }

    strncpy_s(helpText, sizeof(helpText), text.c_str(), _TRUNCATE);
    return helpText;
  }

  std::string fallback =
      "Mỗi lần quay được 1 điểm tích lũy.Đạt mốc yêu cầu để nhận thưởng. "
      "Điểm và mốc đã nhận sẽ làm mới vào lúc ";
  fallback += GetTichLuyResetText();
  strncpy_s(helpText, sizeof(helpText), fallback.c_str(), _TRUNCATE);

  return helpText;
}

void RenderWrappedTipText(int sx, int sy, const char *text,
                          float maxTextWidth) {
  std::vector<std::string> lines = WrapTooltipText(text, maxTextWidth);

  if (lines.empty()) {
    return;
  }

  float contentWidth = 0.0f;

  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       it++) {
    float lineWidth = GetTextPixelWidth(*it);

    if (contentWidth < lineWidth) {
      contentWidth = lineWidth;
    }
  }

  if (contentWidth > maxTextWidth) {
    contentWidth = maxTextWidth;
  }

  const float padding = 4.0f;
  const float lineHeight = 10.0f;
  float boxW = contentWidth + (padding * 2.0f);
  float boxH = ((float)lines.size() * lineHeight) + (padding * 2.0f);
  float boxX = (float)sx;
  float boxY = (float)sy;

  if ((boxX + boxW) > (gwinhandle->GetScreenX() - 5)) {
    boxX = (float)gwinhandle->GetScreenX() - boxW - 5.0f;
  }

  if (boxX < 5.0f) {
    boxX = 5.0f;
  }

  if ((boxY + boxH) > (gwinhandle->GetScreenY() - 5)) {
    boxY = (float)gwinhandle->GetScreenY() - boxH - 5.0f;
  }

  if (boxY < 5.0f) {
    boxY = 5.0f;
  }

  int backupAlphaBlendType = AlphaBlendType;
  EnableAlphaTest();
  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
  RenderColor(boxX - 1.0f, boxY - 1.0f, boxW + 2.0f, 1.0f);
  RenderColor(boxX - 1.0f, boxY - 1.0f, 1.0f, boxH + 2.0f);
  RenderColor(boxX + boxW, boxY - 1.0f, 1.0f, boxH + 2.0f);
  RenderColor(boxX - 1.0f, boxY + boxH, boxW + 2.0f, 1.0f);
  glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
  RenderColor(boxX, boxY, boxW, boxH);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_TEXTURE_2D);

  for (size_t n = 0; n < lines.size(); n++) {
    TextDraw(g_hFont, (int)(boxX + padding),
             (int)(boxY + padding + (n * lineHeight)), 0xFFFFFFFF, 0x0,
             (int)maxTextWidth, 0, 1, "%s", lines[n].c_str());
  }

  RestoreVongQuayAlphaBlendState(backupAlphaBlendType);
}

bool DrawTichLuyClaimButton(float x, float y, float width, float height,
                            const char *text, bool enabled, bool claimed) {
  bool hover = enabled && SEASON3B::CheckMouseIn(x, y, width, height);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  SEASON3B::RenderImageF(BITMAP_btn_empty_small, x, y, width, height, 0.0f,
                         0.0f, 64.0f, 29.0f);

  DWORD textColor = claimed ? 0xA0A0A0FF : (enabled ? 0x00FF00FF : 0xFF4040FF);
  TextDraw(g_hFontBold, x, y + ((height - 9.0f) / 2.0f), textColor, 0x0,
           (int)width, 0, 3, text);

  return hover && SEASON3B::IsRelease(VK_LBUTTON);
}

void DrawActiveVongQuayTabGlow(float x, float y, float width, float height) {
  float perimeter = (width + height) * 2.0f;
  int glowLength = (int)(perimeter / 7.0f);

  if (glowLength < 24) {
    glowLength = 24;
  } else if (glowLength > 52) {
    glowLength = 52;
  }

  gInterface->DrawGlowAroundBox2(x, y, width, height, glowLength, 0.15f, 1.0f,
                                 0.55f);
}
} // namespace

bool CVongQuay::IsSpinAnimating() const {
  return (this->StartRollSau >= 1) || (PendingSlowdownSteps > 0);
}

bool CVongQuay::ShouldBlockClose() const { return this->IsSpinAnimating(); }

void quay10lan() { SendSpinRequest(2); }
bool UpdateMaxPosSVQ = false;

void CVongQuay::DrawWindowVQ() {
  bool hasBlockingWindow =
      gInterface->CheckWindow(CB_Interface::ObjWindow::MoveList) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::CashShop) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::SkillTree) ||
      gInterface->CheckWindow(CB_Interface::ObjWindow::FullMap) ||
      (gInterface->CheckWindow(CB_Interface::Inventory) &&
       gInterface->CheckWindow(CB_Interface::ExpandInventory) &&
       gInterface->CheckWindow(CB_Interface::Store)) ||
      (gInterface->CheckWindow(CB_Interface::Inventory) &&
       gInterface->CheckWindow(CB_Interface::Warehouse) &&
       gInterface->CheckWindow(CB_Interface::ExpandWarehouse)) ||
      gInterface->CheckWindow(CB_Interface::ChaosBox);

  if (hasBlockingWindow && !this->ShouldBlockClose()) {
    gInterface->Data[eWindowVongQuay].OnShow = 0;
    ResetSpinInputState();

    return;
  }
  if (!gInterface->Data[eWindowVongQuay].OnShow) {
    if (ListVongQuay)
      ListVongQuay = nullptr;
    if (SelectTypeVQ != 1)
      SelectTypeVQ = 1;
    gVongQuay.StartRollSau = -1;
    gVongQuay.IndexItemSau = -1;
    Chay = -1;
    ResetSpinAnimation();
    this->ListItemVongQuay.clear();
    ResetSpinInputState();

    return;
  }
  int MaxListVQInPage = 3;
  int MaxListItemVQInPage = 9;
  float WindowW = 380;
  float WindowH = 310;
  gInterface->Data[eWindowVongQuay].Width = WindowW;
  float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
  float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
  if (gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH,
                                    eWindowVongQuay,
                                    gTextClient.txtClient_VQMM[0])) {

    int backupAlphaBlendType = AlphaBlendType;
    EnableAlphaBlend();
    glColor3f(1.0, 1.0, 1.0);
    //===Info Yeu Cau Moc Nap
    float InfoMocNapX = (StartX + 10) + 3;
    float InfoMocNapY = (StartY + 15);
    float TyleInfoYeuCau = 7.5f;
    float WInfo = (WindowW - 20) / 10;
    float WProcess = (WInfo * (TyleInfoYeuCau - 2.7));
    float WButton = 88;
    float HInfo = WindowH - 160;
    const int kGridRows = 4;
    const int kGridColumns = 4;
    const int kGridSlotCount = kGridRows * kGridColumns;
    const float kItemSlotSize = 30.0f;
    const float kItemSlotGap = 8.0f;
    const float gridStartX = StartX + 18;
    const float tabStartY = StartY + 25;
    const float tabButtonGap = 8.0f;
    const float gridStartY = StartY + 62;
    const float gridWidth =
        (kItemSlotSize * kGridColumns) + (kItemSlotGap * (kGridColumns - 1));
    const float gridHeight =
        (kItemSlotSize * kGridRows) + (kItemSlotGap * (kGridRows - 1));
    const float previewBoxX = gridStartX + gridWidth + 22;
    const float previewBoxY =
        gridStartY + ((gridHeight - kItemSlotSize) / 2.0f);
    const float spinInputTextX = gridStartX;
    const float spinInputTextY = (gridStartY + gridHeight) + 18;
    const float spinInputX = spinInputTextX + 88;
    const float spinInputY = spinInputTextY;
    const float rollButtonX = gridStartX + 45;
    const float rollButtonY = spinInputTextY + 55;
    const float tichLuyPanelX = (StartX + WindowW) - 150;
    const float tichLuyPanelY = InfoMocNapY + 46;
    const float tichLuyPanelW = 130.0f;
    const float tichLuyPanelH = 210.0f;
    int BBShowInfoTichLuy = -1;

    gInterface->DrawInfoBox(tichLuyPanelX, tichLuyPanelY, tichLuyPanelW,
                            tichLuyPanelH, 0x00000096, 0, 0);

    int DataListVQ = gVongQuay.DanhSachVongQuay.size();
    int sotrang = (DataListVQ + MaxListVQInPage - 1) / MaxListVQInPage;

    if (sotrang <= 0) {
      sotrang = 1;
    }

    if (PageQuay >= sotrang) {
      PageQuay = sotrang - 1;
    }

    if (PageQuay < 0) {
      PageQuay = 0;
    }

    float KhoangCachXMocNap = WButton + tabButtonGap;
    float tabGroupWidth =
        (WButton * MaxListVQInPage) + (tabButtonGap * (MaxListVQInPage - 1));

    if (DataListVQ > MaxListVQInPage) {
      float arrowY = tabStartY + 2;
      float arrowLeftX = StartX + ((WindowW - tabGroupWidth) / 2.0f) - 24;
      float arrowRightX = StartX + ((WindowW + tabGroupWidth) / 2.0f) + 7;
      bool canPrevPage = (PageQuay > 0);
      bool canNextPage = (PageQuay < (sotrang - 1));

      SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_L, arrowLeftX,
                                arrowY, canPrevPage);
      SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_R,
                                arrowRightX, arrowY, canNextPage);

      if (canPrevPage && SEASON3B::CheckMouseFetch(arrowLeftX, arrowY, true) &&
          SEASON3B::IsRelease(VK_LBUTTON)) {
        PageQuay--;
      } else if (canNextPage &&
                 SEASON3B::CheckMouseFetch(arrowRightX, arrowY, true) &&
                 SEASON3B::IsRelease(VK_LBUTTON)) {
        PageQuay++;
      }
    }

    int VisibleVongQuayCount = DataListVQ - (PageQuay * MaxListVQInPage);
    if (VisibleVongQuayCount > MaxListVQInPage) {
      VisibleVongQuayCount = MaxListVQInPage;
    }
    if (VisibleVongQuayCount < 0) {
      VisibleVongQuayCount = 0;
    }

    float visibleTabGroupWidth =
        (VisibleVongQuayCount > 0)
            ? ((WButton * VisibleVongQuayCount) +
               (tabButtonGap * (VisibleVongQuayCount - 1)))
            : 0.0f;
    float tabStartX = StartX + ((WindowW - visibleTabGroupWidth) / 2.0f);

    // int MixItemListPage = ListVongQuay->GetCurPos();
    int MaxList = 0;

    for (int n = (PageQuay * MaxListVQInPage); n < DataListVQ; n++) {
      //==Xem
      float buttonX = tabStartX + (MaxList * KhoangCachXMocNap);
      int tabIndex = gVongQuay.DanhSachVongQuay[n].IndexVongQuay;
      bool tabClicked =
          gInterface->DrawButton(buttonX, tabStartY, 110, 11,
                                 gVongQuay.DanhSachVongQuay[n].Name, WButton);

      if (SelectTypeVQ == tabIndex) {
        DrawActiveVongQuayTabGlow(buttonX - 0.8f, tabStartY - 1.5f,
                                  WButton + 1.6f, 25.0f);
      }

      if (tabClicked &&
          (GetTickCount() - gInterface->Data[eWindowVongQuay].EventTick) >
              300 &&
          gVongQuay.StartRollSau < 1) //"Xem"
      {

        SelectTypeVQ = tabIndex;
        Chay = -1;
        ResetSpinAnimation();
        gInterface->Data[eWindowVongQuay].OnShow = true;
        gInterface->Data[eWindowVongQuay].EventTick = GetTickCount();
        XULY_CGPACKET pMsg;
        pMsg.header.set(0xD3, 0x8B, sizeof(pMsg));
        pMsg.ThaoTac = SelectTypeVQ; //
        DataSend((LPBYTE)&pMsg, pMsg.header.size);
        gVongQuay.IndexItemSau = -1;
      }

      MaxList++;

      if (MaxList >= MaxListVQInPage)
        break;
    }

    bool canRequestRoll =
        !gVongQuay.IsSpinAnimating() &&
        ((GetTickCount() - LastRollRequestTick) > kRollClickDelayMs);
    const char *rollButtonLabel = gVongQuay.IsSpinAnimating()
                                      ? gTextClient.txtClient_VQMM[1]
                                      : gTextClient.txtClient_VQMM[2];

    if (gInterface->DrawButton(rollButtonX, rollButtonY, 100, 11,
                               rollButtonLabel, 60) &&
        canRequestRoll) {
      int spinCount = atoi(GetSoVong);

      if (spinCount < 1) {
        spinCount = 1;
        GetSoVong[0] = '1';
        GetSoVong[1] = 0;

        if (InputSoVong != NULL) {
          InputSoVong->SetText(GetSoVong);
        }
      }

      RequestedSpinCount = spinCount;
      PendingQueuedSpinCount = (spinCount > 1) ? (spinCount - 1) : 0;
      KickHoatQuay = (PendingQueuedSpinCount > 0);
      SendSpinRequest((spinCount > 1) ? 2 : 1);
    }

    float NTCoinX = spinInputX + 40;
    float NTCoinY = spinInputY;
    float NTW = 25;
    gInterface->DrawBarForm(NTCoinX, NTCoinY - 5, 25, 15, 0.0, 0.0, 0.0, 0.5);
    bool lockSpinCountInput = gVongQuay.IsSpinAnimating();
    EnsureSpinCountInput(NTCoinX, NTCoinY - 2, NTW, 14);

    if (InputSoVong != NULL) {
      if (lockSpinCountInput) {
        ReleaseSpinCountInputFocus();
        InputSoVong->Render();
      } else if (gInterface->RenderInputBox(
                     NTCoinX, NTCoinY - 2, NTW, 14, GetSoVong, InputSoVong,
                     UIOPTION_NUMBERONLY, sizeof(GetSoVong) - 1, false)) {
        InputSoVong->GetText(GetSoVong, sizeof(GetSoVong));
      }

      NormalizeSpinCountInput(true);
    } else {
      GetSoVong[0] = '1';
      GetSoVong[1] = 0;
    }
    int spinCountInput = atoi(GetSoVong);
    if (spinCountInput < 1) {
      spinCountInput = 1;
    }

    if (!gVongQuay.IsSpinAnimating() && PendingQueuedSpinCount > 0 &&
        (GetTickCount() - LastQueuedRollTick) > kQueuedBatchSpinDelayMs) {
      PendingQueuedSpinCount--;
      KickHoatQuay = (PendingQueuedSpinCount > 0);
      quay10lan();
    } else if (PendingQueuedSpinCount <= 0) {
      KickHoatQuay = false;
    }

    float requirementTextX = gridStartX;
    float PosYCoinNhan = (gridStartY + gridHeight) + 30;
    int requirementLine = 1;

    TextDraw(g_hFont, requirementTextX, PosYCoinNhan, 0xFFDE26FF, 0x0, 90, 0, 1,
             gTextClient.txtClient_VQMM[3]);

    if (gVongQuay.WCYC > 0) {
      TextDraw(g_hFont, requirementTextX, PosYCoinNhan + (10 * requirementLine),
               0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_VQMM[6],
               gInterface->NumberFormat(gVongQuay.WCYC));
      requirementLine++;
    }

    if (gVongQuay.WPYC > 0) {
      TextDraw(g_hFont, requirementTextX, PosYCoinNhan + (10 * requirementLine),
               0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_VQMM[7],
               gInterface->NumberFormat(gVongQuay.WPYC));
      requirementLine++;
    }

    if (gVongQuay.GPYC > 0) {
      TextDraw(g_hFont, requirementTextX, PosYCoinNhan + (10 * requirementLine),
               0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_VQMM[8],
               gInterface->NumberFormat(gVongQuay.GPYC));
      requirementLine++;
    }

    TextDraw(g_hFont, spinInputTextX, spinInputTextY, 0xFF8214FF, 0x0, 130, 0,
             1, gTextClient.txtClient_VQMM[5]);
    TextDraw(g_hFont, previewBoxX - 10, previewBoxY - 14, 0xFFDE26FF, 0x0, 60,
             0, 3, gTextClient.txtClient_VQMM[9]);

    const int kTichLuyRowsPerPage = 5;
    int tichLuyCount = (int)gVongQuay.ListTichLuyVongQuay.size();
    int tichLuyPages =
        (tichLuyCount + kTichLuyRowsPerPage - 1) / kTichLuyRowsPerPage;

    if (tichLuyPages <= 0) {
      tichLuyPages = 1;
    }

    if (gVongQuay.TichLuyPage >= tichLuyPages) {
      gVongQuay.TichLuyPage = tichLuyPages - 1;
    }

    if (gVongQuay.TichLuyPage < 0) {
      gVongQuay.TichLuyPage = 0;
    }

    float tichLuyTextX = tichLuyPanelX + 8;
    float tichLuyRowY = tichLuyPanelY + 30;
    bool showTichLuyHelpTip = false;
    int tichLuyHelpTipX = 0;
    int tichLuyHelpTipY = 0;

    TextDraw(g_hFontBold, tichLuyPanelX, tichLuyPanelY + 8, 0xFFDE26FF, 0x0,
             (int)tichLuyPanelW, 0, 3, gTextClient.txtClient_VQMM[10],
             gVongQuay.DiemTichLuyVQ);

    float tichLuyHelpSize = 16.0f;
    float tichLuyHelpX = tichLuyPanelX + tichLuyPanelW - 22.0f;
    float tichLuyHelpY = tichLuyPanelY + 6.0f;
    RenderBitmap(BITMAP_INTERFACE_EX + 20, tichLuyHelpX, tichLuyHelpY,
                 tichLuyHelpSize, tichLuyHelpSize, 0.0f, 0.0f, 1.0f, 1.0f);

    if (SEASON3B::CheckMouseIn(tichLuyHelpX, tichLuyHelpY, tichLuyHelpSize,
                               tichLuyHelpSize)) {
      showTichLuyHelpTip = true;
      tichLuyHelpTipX = (int)(tichLuyHelpX - 104.0f);
      tichLuyHelpTipY = (int)(tichLuyHelpY + tichLuyHelpSize + 3.0f);
    }

    for (int n = 0; n < kTichLuyRowsPerPage; n++) {
      int rewardIndex = (gVongQuay.TichLuyPage * kTichLuyRowsPerPage) + n;

      if (rewardIndex >= tichLuyCount) {
        break;
      }

      INFO_VONGQUAY_LOCAL_TICHLUY &reward =
          gVongQuay.ListTichLuyVongQuay[rewardIndex];
      DWORD claimBit = (rewardIndex < 32) ? (1u << rewardIndex) : 0;
      bool claimed =
          (claimBit != 0) && ((gVongQuay.NhanThuongTichLuyVQ & claimBit) != 0);
      bool canClaim =
          !claimed && (gVongQuay.DiemTichLuyVQ >= (DWORD)reward.RequiredSpin);
      DWORD rowTextColor = canClaim ? 0xFFFFFFFF : 0xA0A0A0FF;
      float itemBoxX = tichLuyPanelX + 66;
      float itemBoxY = tichLuyRowY - 3;
      float claimButtonW = 42.0f;
      float claimButtonH = 16.0f;
      float claimButtonX = tichLuyPanelX + tichLuyPanelW - claimButtonW - 6.0f;
      float claimButtonY = tichLuyRowY - 2.0f;
      const char *claimButtonText = claimed ? gTextClient.txtClient_VQMM[12]
                                            : gTextClient.txtClient_VQMM[13];

      TextDraw(g_hFont, tichLuyTextX - 5, tichLuyRowY + 2, rowTextColor, 0x0,
               62, 0, 1, gTextClient.txtClient_VQMM[11], reward.RequiredSpin);
      gInterface->DrawInfoBox(itemBoxX - 12, itemBoxY - 3, 18, 18,
                              claimed ? 0x30303096 : 0x00000096, 0, 0);

      if (reward.Item != NULL) {
        RenderVongQuayLocalItem3D(
            itemBoxX + reward.PosX, itemBoxY + reward.PosY, 18, 18,
            reward.Index, reward.Item->Level, reward.Item->Option1,
            reward.Item->ExtOption, 0, reward.SizeBMD);
      }

      if (SEASON3B::CheckMouseIn(itemBoxX, itemBoxY, 18, 18)) {
        BBShowInfoTichLuy = rewardIndex;
      }

      if (DrawTichLuyClaimButton(claimButtonX, claimButtonY, claimButtonW,
                                 claimButtonH, claimButtonText, canClaim,
                                 claimed) &&
          (GetTickCount() - gInterface->Data[eWindowVongQuay].EventTick) >
              300) {
        XULY_CGPACKET_NHANTICHLUY pMsg;
        pMsg.header.set(0xD3, 0x8D, sizeof(pMsg));
        pMsg.Index = (DWORD)rewardIndex;
        DataSend((LPBYTE)&pMsg, pMsg.header.size);
        gInterface->Data[eWindowVongQuay].EventTick = GetTickCount();
      }

      tichLuyRowY += 30.0f;
    }

    if (tichLuyPages > 1) {
      float pageArrowY = tichLuyPanelY + tichLuyPanelH - 21;
      float leftArrowX = tichLuyPanelX + 37;
      float rightArrowX = tichLuyPanelX + 82;
      bool canPrevTichLuy = (gVongQuay.TichLuyPage > 0);
      bool canNextTichLuy = (gVongQuay.TichLuyPage < (tichLuyPages - 1));

      SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_L, leftArrowX,
                                pageArrowY, canPrevTichLuy);
      SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_R,
                                rightArrowX, pageArrowY, canNextTichLuy);
      TextDraw(g_hFontBold, tichLuyPanelX, pageArrowY + 4, 0xE6FCF7FF, 0x0,
               (int)tichLuyPanelW, 0, 3, "%d / %d", gVongQuay.TichLuyPage + 1,
               tichLuyPages);

      if (canPrevTichLuy &&
          SEASON3B::CheckMouseFetch(leftArrowX, pageArrowY, true) &&
          SEASON3B::IsRelease(VK_LBUTTON)) {
        gVongQuay.TichLuyPage--;
      } else if (canNextTichLuy &&
                 SEASON3B::CheckMouseFetch(rightArrowX, pageArrowY, true) &&
                 SEASON3B::IsRelease(VK_LBUTTON)) {
        gVongQuay.TichLuyPage++;
      }
    }

    if (showTichLuyHelpTip) {
      RenderWrappedTipText(tichLuyHelpTipX, tichLuyHelpTipY,
                           GetTichLuyHelpText(), 118.0f);
    }

    int DataListItem = gVongQuay.ListItemVongQuay.size();
    int VisibleGridItemCount =
        (DataListItem > kGridSlotCount) ? kGridSlotCount : DataListItem;
    float WBox = kItemSlotSize;
    int BBShowInfoItem = -1;
    DWORD boxColor = 0x00000096;
    if ((gVongQuay.StartRollSau >= 1 || PendingSlowdownSteps > 0) &&
        DataListItem > 0) {
      DWORD currentTick = GetTickCount();
      DWORD stepDelay =
          (PendingSlowdownSteps > 0) ? CurrentRollDelayMs : kRollFastDelayMs;

      if (LastRollFrameTick == 0 ||
          (currentTick - LastRollFrameTick) >= stepDelay) {
        LastRollFrameTick = currentTick;

        Chay = GetRandomSpinIndex(DataListItem, Chay);

        if (PendingSlowdownSteps > 0) {
          PendingSlowdownSteps--;

          if (CurrentRollDelayMs < kRollSlowDelayMs) {
            CurrentRollDelayMs += kRollSlowStepMs;

            if (CurrentRollDelayMs > kRollSlowDelayMs) {
              CurrentRollDelayMs = kRollSlowDelayMs;
            }
          }

          if (PendingSlowdownSteps <= 0) {
            Chay = gVongQuay.IndexItemSau;
            gVongQuay.StartRollSau = 0;
            KickHoatQuay = (PendingQueuedSpinCount > 0);
            CountVong = 1.0f;
            LastRollFrameTick = 0;
            CurrentRollDelayMs = kRollFastDelayMs;
          }
        }
      }
    } else {
      CountVong = 1.0f;
      LastRollFrameTick = 0;
      CurrentRollDelayMs = kRollFastDelayMs;

      if (DataListItem <= 0) {
        Chay = -1;
      } else if (Chay >= DataListItem) {
        Chay = DataListItem - 1;
      }
    }

    int resultIndex =
        (gVongQuay.IndexItemSau >= 0 && gVongQuay.IndexItemSau < DataListItem)
            ? gVongQuay.IndexItemSau
            : -1;
    bool showFinalResult = (resultIndex != -1) && !gVongQuay.IsSpinAnimating();
    int displaySpinCount =
        (PendingQueuedSpinCount > 0 || gVongQuay.IsSpinAnimating())
            ? RequestedSpinCount
            : spinCountInput;

    if (showFinalResult) {
      if (displaySpinCount == 1 && resultIndex < VisibleGridItemCount) {
        Chay = resultIndex;
      } else if (displaySpinCount > 1) {
        Chay = -1;
      }
    }

    for (int n = 0; n < kGridSlotCount; n++) {
      int row = n / kGridColumns;
      int column = n % kGridColumns;
      float PosXBoxItem = gridStartX + (column * (WBox + kItemSlotGap));
      float PosYBoxItem = gridStartY + (row * (WBox + kItemSlotGap));
      bool hasItem = (n < VisibleGridItemCount);

      if (hasItem && n == Chay) {
        boxColor = 0xD4966396;
      } else {
        boxColor = 0x00000096;
      }

      gInterface->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, boxColor, 0,
                              0);

      if (!hasItem) {
        continue;
      }

      RenderVongQuayLocalItem3D(
          PosXBoxItem + gVongQuay.ListItemVongQuay[n].PosX,
          PosYBoxItem + gVongQuay.ListItemVongQuay[n].PosY, WBox, WBox,
          gVongQuay.ListItemVongQuay[n].Index,
          gVongQuay.ListItemVongQuay[n].Item->Level,
          gVongQuay.ListItemVongQuay[n].Item->Option1,
          gVongQuay.ListItemVongQuay[n].Item->ExtOption, 0,
          gVongQuay.ListItemVongQuay[n].SizeBMD);

      // Render Star rating
      if (gVongQuay.ListItemVongQuay[n].Star > 0) {
        int starCount = gVongQuay.ListItemVongQuay[n].Star;
        if (starCount > 6)
          starCount = 6;
        char starBuf[8] = {0};
        for (int s = 0; s < starCount; s++)
          starBuf[s] = '*';
        TextDraw(g_hFont, PosXBoxItem, PosYBoxItem - 1, 0xFFFF00FF, 0x0,
                 (int)WBox, 0, 3, starBuf);
      }

      // Render Quantity label
      if (gVongQuay.ListItemVongQuay[n].Quantity > 1) {
        TextDraw(g_hFont, PosXBoxItem, PosYBoxItem + WBox - 10, 0x00FF00FF, 0x0,
                 (int)WBox - 2, 0, 2, "x%d",
                 gVongQuay.ListItemVongQuay[n].Quantity);
      }

      if (SEASON3B::CheckMouseIn(PosXBoxItem, PosYBoxItem, WBox, WBox)) {
        BBShowInfoItem = n;
      }
    }
    gInterface->DrawInfoBox(previewBoxX, previewBoxY, WBox, WBox,
                            showFinalResult ? 0xD4966396 : 0x00000096, 0, 0);

    if (showFinalResult) {
      RenderVongQuayLocalItem3D(
          previewBoxX + gVongQuay.ListItemVongQuay[resultIndex].PosX,
          previewBoxY + gVongQuay.ListItemVongQuay[resultIndex].PosY, WBox,
          WBox, gVongQuay.ListItemVongQuay[resultIndex].Index,
          gVongQuay.ListItemVongQuay[resultIndex].Item->Level,
          gVongQuay.ListItemVongQuay[resultIndex].Item->Option1,
          gVongQuay.ListItemVongQuay[resultIndex].Item->ExtOption, 0,
          gVongQuay.ListItemVongQuay[resultIndex].SizeBMD);

      if (SEASON3B::CheckMouseIn(previewBoxX, previewBoxY, WBox, WBox)) {
        BBShowInfoItem = resultIndex;
      }
    }

    //===Coin

    if (BBShowInfoTichLuy != -1 &&
        BBShowInfoTichLuy < (int)this->ListTichLuyVongQuay.size()) {
      RenderItemInfo(MouseX + 75, MouseY,
                     this->ListTichLuyVongQuay[BBShowInfoTichLuy].Item, 0, 0,
                     false, false);
    } else if (BBShowInfoItem != -1) {
      RenderItemInfo(MouseX + 75, MouseY,
                     this->ListItemVongQuay[BBShowInfoItem].Item, 0, 0, false,
                     false);
    }
    /*float CenterX = StartX + (WindowW / 3) + 20;
    float CenterY = (StartY + WindowH) - 5;
    const BYTE state[3] = {0, 1, 2};
    ::EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderBitmap(32344, CenterX - (60 / 2), CenterY - 22, 60.f, 22.f, 0, 0,
                 80.f / 128.f, 30.f / 34.f, 1, 1, 0.0);
    SEASON3B::TextDraw(g_hFont, CenterX - (60 / 2), CenterY - 22 + 5,
                       0xffffffff, 0x0, 60, 0, 3, "%d / %d", PageQuay + 1,
                       sotrang);*/

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RestoreVongQuayAlphaBlendState(backupAlphaBlendType);
  }
}

void CVongQuay::GetListVQ(BYTE *Recv) {
  if (!Recv)
    return;

  gVongQuay.DanhSachVongQuay.clear();
  PMSG_VONGQUAY_SEND *mRecv = (PMSG_VONGQUAY_SEND *)Recv;

  for (int i = 0; i < mRecv->count; i++) {
    ListVongQuaySend lpInfo =
        *(ListVongQuaySend *)(((BYTE *)Recv) + sizeof(PMSG_VONGQUAY_SEND) +
                              (sizeof(ListVongQuaySend) * i));
    gVongQuay.DanhSachVongQuay.push_back(lpInfo);
  }
}

void CVongQuay::RecvListItemVQ(BYTE *Recv) {
  if (!Recv)
    return;

  gVongQuay.ListItemVongQuay.clear();

  PMSG_YCVONGQUAY_SEND *mRecv = (PMSG_YCVONGQUAY_SEND *)Recv;
  gVongQuay.IndexYC = -1;
  gVongQuay.CountItem = 0;
  gVongQuay.WCYC = mRecv->WCYC;
  gVongQuay.WPYC = mRecv->WPYC;
  gVongQuay.GPYC = mRecv->GPYC;

  for (int i = 0; i < mRecv->count; i++) {
    LISTITEMVONGQUAY_SENDINFO lpInfo =
        *(LISTITEMVONGQUAY_SENDINFO *)(((BYTE *)Recv) +
                                       sizeof(PMSG_YCVONGQUAY_SEND) +
                                       (sizeof(LISTITEMVONGQUAY_SENDINFO) * i));
    //==SetINfoItem
    INFO_VONGQUAY_LOCAL_ITEM infoItemLocal = {0};
    infoItemLocal.SizeBMD = lpInfo.SizeBMD;
    infoItemLocal.PosX = lpInfo.PosX;
    infoItemLocal.PosY = lpInfo.PosY;
    infoItemLocal.Index = lpInfo.Index;
    infoItemLocal.Star = lpInfo.Star;
    infoItemLocal.Quantity = lpInfo.Quantity;
    infoItemLocal.Item = g_pNewItemMng->CreateItem(lpInfo.Item);
    infoItemLocal.Item->Durability = lpInfo.Dur;
    if (lpInfo.PeriodTime) {
      infoItemLocal.Item->bPeriodItem = 1;
      infoItemLocal.Item->lExpireTime = lpInfo.PeriodTime;
    }
    gVongQuay.ListItemVongQuay.push_back(infoItemLocal);
  }
  gInterface->Data[eWindowVongQuay].OnShow = 1;
}

void CVongQuay::RecvTichLuyVQ(BYTE *Recv) {
  if (!Recv)
    return;

  gVongQuay.ListTichLuyVongQuay.clear();

  PMSG_VONGQUAY_TICHLUY_SEND *mRecv = (PMSG_VONGQUAY_TICHLUY_SEND *)Recv;
  gVongQuay.DiemTichLuyVQ = mRecv->DiemTichLuy;
  gVongQuay.NhanThuongTichLuyVQ = mRecv->NhanThuongMask;
  gVongQuay.ResetTichLuyTime = (int)mRecv->ResetTichLuyTime;
  if (gVongQuay.ResetTichLuyTime < 0 || gVongQuay.ResetTichLuyTime > 2) {
    gVongQuay.ResetTichLuyTime = 0;
  }

  for (int i = 0; i < mRecv->count; i++) {
    LISTVONGQUAY_TICHLUY_SENDINFO lpInfo =
        *(LISTVONGQUAY_TICHLUY_SENDINFO
              *)(((BYTE *)Recv) + sizeof(PMSG_VONGQUAY_TICHLUY_SEND) +
                 (sizeof(LISTVONGQUAY_TICHLUY_SENDINFO) * i));

    INFO_VONGQUAY_LOCAL_TICHLUY infoItemLocal = {0};
    infoItemLocal.RequiredSpin = lpInfo.RequiredSpin;
    infoItemLocal.SizeBMD = lpInfo.SizeBMD;
    infoItemLocal.PosX = lpInfo.PosX;
    infoItemLocal.PosY = lpInfo.PosY;
    infoItemLocal.Index = lpInfo.Index;
    infoItemLocal.Item = g_pNewItemMng->CreateItem(lpInfo.Item);

    if (infoItemLocal.Item != NULL) {
      infoItemLocal.Item->Durability = lpInfo.Dur;

      if (lpInfo.PeriodTime) {
        infoItemLocal.Item->bPeriodItem = 1;
        infoItemLocal.Item->lExpireTime = lpInfo.PeriodTime;
      }
    }

    gVongQuay.ListTichLuyVongQuay.push_back(infoItemLocal);
  }
}

void CVongQuay::GetInfoVQ(BYTE *Recv) {
  if (!Recv)
    return;
  XULY_CGPACKET_VONGQUAY *mRecv = (XULY_CGPACKET_VONGQUAY *)Recv;
  // gInterface->DrawMessage(1, "%d ~ %d", mRecv->StartRoll, mRecv->IndexWin);
  gVongQuay.StartRollSau = mRecv->StartRoll;
  gVongQuay.IndexItemSau =
      (mRecv->IndexWin == ((DWORD)-1)) ? -1 : (int)mRecv->IndexWin;

  if (gVongQuay.IndexItemSau >= 0) {
    QueueSpinResultStop((int)gVongQuay.ListItemVongQuay.size(),
                        gVongQuay.IndexItemSau);

    if (PendingSlowdownSteps <= 0) {
      gVongQuay.StartRollSau = 0;
      Chay = gVongQuay.IndexItemSau;
      KickHoatQuay = (PendingQueuedSpinCount > 0);
      LastRollFrameTick = 0;
      CurrentRollDelayMs = kRollFastDelayMs;
    }
  } else if (gVongQuay.StartRollSau >= 1) {
    PendingSlowdownSteps = 0;
    CurrentRollDelayMs = kRollFastDelayMs;
  } else {
    ResetSpinAnimation();
    CountVong = 1.0f;
    Chay = -1;
  }
}
