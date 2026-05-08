
#include "stdAfx.h"
#include "CB_QuaPhucLoi.h"
#include "CBDrawInterface.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "NewUIBase.h"
#include "NewUIItemTooltip.h"
#include "NewUISystem.h"
#include "Util.h"
#include "ZzzInterface.h"


CB_QuaPhucLoi *gCB_QuaPhucLoi;
time_t server_time_at_receive = 0;
DWORD local_tick_at_receive = 0;

void SetServerTime(time_t server_time) {
  server_time_at_receive = server_time;
  local_tick_at_receive = GetTickCount();
}

time_t GetCurrentServerTime() {
  DWORD current_tick = GetTickCount();
  DWORD elapsed = current_tick - local_tick_at_receive;
  return server_time_at_receive + elapsed / 1000;
}
struct CountdownResult {
  int days;
  int hours;
  int minutes;
  int seconds;
};

void AdjustTimeGMT7(std::tm &timeinfo) {
  // time_t rawtime = mktime(&timeinfo);
  // rawtime += 7 * 3600; // GTM 7 time việtn nam
  // timeinfo = *localtime(&rawtime);
}

CountdownResult GetCountdownToEndOfDay(std::tm currentTime) {
  AdjustTimeGMT7(currentTime);

  std::tm endOfDay = currentTime;
  endOfDay.tm_hour = 23;
  endOfDay.tm_min = 59;
  endOfDay.tm_sec = 59;

  time_t now = mktime(&currentTime);
  time_t end = mktime(&endOfDay);

  int diff = static_cast<int>(difftime(end, now));

  CountdownResult result;
  result.days = diff / (24 * 3600);
  diff %= (24 * 3600);
  result.hours = diff / 3600;
  diff %= 3600;
  result.minutes = diff / 60;
  result.seconds = diff % 60;

  return result;
}

CountdownResult GetCountdownToStartOfWeek(std::tm currentTime) {
  AdjustTimeGMT7(currentTime);

  int wday = currentTime.tm_wday;
  if (wday == 0)
    wday = 7;

  std::tm nextMonday = currentTime;
  nextMonday.tm_mday += (8 - wday);
  nextMonday.tm_hour = 0;
  nextMonday.tm_min = 0;
  nextMonday.tm_sec = 0;

  time_t now = mktime(&currentTime);
  time_t monday = mktime(&nextMonday);

  int diff = static_cast<int>(difftime(monday, now));

  CountdownResult result;
  result.days = diff / (24 * 3600);
  diff %= (24 * 3600);
  result.hours = diff / 3600;
  diff %= 3600;
  result.minutes = diff / 60;
  result.seconds = diff % 60;

  return result;
}

CountdownResult GetCountdownToStartOfMonth(std::tm currentTime) {
  AdjustTimeGMT7(currentTime);

  std::tm firstDayNextMonth = currentTime;
  firstDayNextMonth.tm_mday = 1;
  firstDayNextMonth.tm_mon += 1;
  firstDayNextMonth.tm_hour = 0;
  firstDayNextMonth.tm_min = 0;
  firstDayNextMonth.tm_sec = 0;

  time_t now = mktime(&currentTime);
  time_t firstNextMonth = mktime(&firstDayNextMonth);

  int diff = static_cast<int>(difftime(firstNextMonth, now));

  CountdownResult result;
  result.days = diff / (24 * 3600);
  diff %= (24 * 3600);
  result.hours = diff / 3600;
  diff %= 3600;
  result.minutes = diff / 60;
  result.seconds = diff % 60;

  return result;
}

void PrintCountdown(const CountdownResult &result) {
  std::cout << result.days << " ngày, " << result.hours << " giờ, "
            << result.minutes << " phút, " << result.seconds << " giây\n";
}
CB_QuaPhucLoi::CB_QuaPhucLoi() {
  this->m_ButtoNText.push_back("Gift Code");
  this->m_ButtoNText.push_back("Quà Nạp Đầu");
  this->m_ButtoNText.push_back("Quà Nạp Ngày");
  this->m_ButtoNText.push_back("Quà Nạp Tháng");
  this->m_ButtoNText.push_back("Quà Nạp Tích Lũy");
  this->m_ButtoNText.push_back("Quà Tiêu Phí Ngày");
  this->m_ButtoNText.push_back("Quà Tiêu Phí Tháng");
  this->m_ButtoNText.push_back("Quà Tiêu Phí Tích Lũy");

  this->StatePage = -1;
}

CB_QuaPhucLoi::~CB_QuaPhucLoi() {}
bool CB_QuaPhucLoi::OpenWindow() {
  static bool bImageLoaded = false;
  if (!bImageLoaded) {
    gInterface->Data[IMG_GiftCodeFrame].ModelID = 81504; // ID trong de load anh
    LoadBitmap("Interface\\GiftCodeBG.tga", 81504, GL_LINEAR, 0x2900, 1,
               0); // 0x2900 = GL_CLAMP
    bImageLoaded = true;
  }
  if ((GetTickCount() - gInterface->Data[eWindowQuaPhucLoi].EventTick) > 300) {
    gInterface->Data[eWindowQuaPhucLoi].EventTick = GetTickCount();

    if (gInterface->Data[eWindowQuaPhucLoi].OnShow) {
      gInterface->Data[eWindowQuaPhucLoi].OnShow = 0;

      return true;
    }

    if (!CanOpenTopMostOverlayWindow(eTopMostOverlayQuaPhucLoi)) {
      return false;
    }

    gInterface->Data[eWindowQuaPhucLoi].OnShow ^= 1; // On/Off
    return true;
  }

  return false;
}

void CB_QuaPhucLoi::DrawPageDef(float X, float Y, float W, float H) {
  gInterface->DrawBarForm(X, Y, W, H, 0.0, 0.0, 0.0, 0.7);
  float TextX = X + 5;
  float TextY = Y;
  SEASON3B::TextDraw((HFONT)g_hFontBig, TextX, TextY, 0x19FF9FFF, 0x0, 0, 0, 1,
                     "QUÀ PHÚC LỢI"); //
  TextY += 20;
  SEASON3B::TextDraw((HFONT)g_hFont, TextX + 10, TextY, 0xE0D19DFF, 0x0, 0, 0,
                     1, "- Chức năng phúc lợi tài khoản !"); //
  TextY += 12;
  SEASON3B::TextDraw(
      (HFONT)g_hFont, TextX + 10, TextY, 0xE0D19DFF, 0x0, 0, 0, 1,
      "- Click chọn ở Menu bên trái để xem các Phúc Lợi đang có"); //
  TextY += 12;
  SEASON3B::TextDraw((HFONT)g_hFont, TextX + 10, TextY, 0xE0D19DFF, 0x0, 0, 0,
                     1,
                     "- Vui lòng tự sắp xếp thùng đồ trước khi nhận Item để "
                     "tránh mất item"); //
}
CUITextInputBox *InputGiftCode = NULL;
char GetTextGiftCode[21] = {
    0,
};
void CB_QuaPhucLoi::DrawPageGift(float X, float Y, float W, float H) {

  gInterface->DrawBarForm(X, Y, W, H, 0.0, 0.0, 0.0, 0.7);
  float TextX = X + 5;
  float TextY = Y;
  SEASON3B::TextDraw((HFONT)g_hFontBig, TextX, TextY, 0x19FF9FFF, 0x0, 0, 0, 1,
                     "Gift Code"); //
  TextY += 20;
  SEASON3B::TextDraw(
      (HFONT)g_hFont, TextX, TextY, 0xE0D19DFF, 0x0, 0, 0, 1,
      "- Nhận qua các hoạt động, sự kiện cộng đồng tài Fanpage"); //
  TextY += 10;
  SEASON3B::TextDraw(
      (HFONT)g_hFont, TextX, TextY, 0xE0D19DFF, 0x0, 0, 0, 1,
      "- Bạn nên chủ động sắp xếp túi đồ trước khi thực hiện!"); //
  TextY += 20;
  // TextDraw((HFONT)g_hFont, TextX, TextY, 0xE0D19DFF, 0x0, 0, 0, 1, "- Bạn nên
  // chủ động sắp xếp túi đồ trước khi thực hiện!"); // TextY += 10;
  SEASON3B::RenderImage(gInterface->Data[IMG_GiftCodeFrame].ModelID, TextX,
                        TextY - 10, W - 10, H - 40, 0.0, 0.0, 0.55, 0.8);

  float TCoinX = TextX + 106;
  float TCoinY = Y + (H - 31);
  gInterface->DrawBarForm(TCoinX - 3, TCoinY - 2.5, 100, 13, 1.0, 1.0, 1.0,
                          1.0);

  if (gInterface->RenderInputBox(TCoinX, TCoinY, 100, 14, GetTextGiftCode,
                                 InputGiftCode, UIOPTION_NOLOCALIZEDCHARACTERS,
                                 sizeof(GetTextGiftCode) - 1, false)) {
    InputGiftCode->SetTextColor(255, 0, 0, 0); // Den
    InputGiftCode->SetBackColor(0, 0, 0, 0);   // Trong suot
    InputGiftCode->GetText(GetTextGiftCode, sizeof(GetTextGiftCode));
  }

  if (gInterface->DrawButton(X + (W / 2) - 42, Y + (H - 14), 90, 12,
                             "Nhận Quà")) {
    if (strlen(GetTextGiftCode) < 1) {
      return;
    }
    CB_QuaPhucLoi::CGPACKET_SENDRECV pMsg;
    pMsg.header.set(0xD3, 0x4F, sizeof(pMsg));
    pMsg.ThaoTac = eAccepGift; //
    memcpy(pMsg.GiftCode, GetTextGiftCode, sizeof(pMsg.GiftCode));
    DataSend((LPBYTE)&pMsg, pMsg.header.size);
    // gInterface->DrawMessage(1, GetTextGiftCode);
  }
}

bool GetCustomItemGlowColor(int type, float &r, float &g, float &b) {
  switch (type) {
  case GET_ITEM(12, 30):
    r = 0.7f;
    g = 0.2f;
    b = 1.0f;
    return true;
  case GET_ITEM(12, 31):
    r = 1.0f;
    g = 0.6f;
    b = 0.8f;
    return true;
  case GET_ITEM(12, 141):
    r = 1.0f;
    g = 1.0f;
    b = 0.0f;
    return true;
  case GET_ITEM(12, 136):
    r = 1.0f;
    g = 1.0f;
    b = 0.0f;
    return true;
  case GET_ITEM(12, 137):
    r = 1.0f;
    g = 0.6f;
    b = 0.8f;
    return true;
  case GET_ITEM(12, 138):
    r = 0.0f;
    g = 0.6f;
    b = 1.0f;
    return true;
  case GET_ITEM(12, 140):
    r = 0.0f;
    g = 0.6f;
    b = 1.0f;
    return true;

  default:
    return false;
  }
}

namespace {
void DrawActiveTabGlow(float x, float y, float width, float height) {
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

void CB_QuaPhucLoi::DrawPageListItem(float X, float Y, float W, float H) {
  gInterface->DrawBarForm(X, Y, W, H, 0.0, 0.0, 0.0, 0.7);
  float TextX = X + 2;
  float TextY = Y;
  SEASON3B::TextDraw((HFONT)g_hFontBig, TextX, TextY, 0x19FF9FFF, 0x0, 0, 0, 1,
                     strdup(m_ButtoNText[this->StatePage].c_str()));

  time_t rawtime = GetCurrentServerTime();
  std::tm currentTime = *localtime(&rawtime);

  if (this->StatePage == eQuaNapThang || this->StatePage == eQuaTieuPhiThang) {
    CountdownResult startOfWeek = GetCountdownToStartOfMonth(currentTime);
    SEASON3B::TextDraw((HFONT)g_hFont, TextX + (W - 150), TextY, 0xC95BFCFF,
                       0x0, 0, 0, 1, "Còn %02d ngày %02d giờ %02d phút",
                       startOfWeek.days, startOfWeek.hours,
                       startOfWeek.minutes);
  } else if (this->StatePage == eQuaNapNgay ||
             this->StatePage == eQuaTieuPhiNgay) {
    CountdownResult startOfWeek = GetCountdownToEndOfDay(currentTime);
    SEASON3B::TextDraw((HFONT)g_hFont, TextX + (W - 150), TextY, 0xC95BFCFF,
                       0x0, 0, 0, 1, "Còn %02d giờ %02d phút",
                       startOfWeek.hours, startOfWeek.minutes);
  }

  if (this->m_DataListPhucLoi.empty())
    return;

  TextY += 30;
  if (this->StatePage == eQuaNapThang || this->StatePage == eQuaNapDau ||
      this->StatePage == eQuaNapNgay || this->StatePage == eQuaNapTichLuy) {
    SEASON3B::TextDraw((HFONT)g_hFont, TextX, TextY, 0xE0D19DFF, 0x0, 0, 0, 1,
                       "- Đã Nạp: ");
    SEASON3B::TextDraw((HFONT)g_hFontBold, TextX + 130, TextY, 0x34E08DFF, 0x0,
                       0, 0, 1, "%s ATM",
                       gInterface->NumberFormat(this->m_DataListCoin));
  } else if (this->StatePage == eQuaTieuPhiNgay ||
             this->StatePage == eQuaTieuPhiThang ||
             this->StatePage == eQuaTieuPhiTichLuy) {
    SEASON3B::TextDraw((HFONT)g_hFont, TextX, TextY, 0xE0D19DFF, 0x0, 0, 0, 1,
                       "- Đã Tiêu: ");
    SEASON3B::TextDraw((HFONT)g_hFontBold, TextX + 130, TextY, 0x34E08DFF, 0x0,
                       0, 0, 1, "%s WCoinC",
                       gInterface->NumberFormat(this->m_DataListCoin));
  }

  float PosXBoxItem = TextX;
  float PosYBoxItem = TextY + 15;
  float WBox = 20;
  float KhoangCach = 35;
  float WProcess = 90;
  int TongSoDaNap = this->m_DataListCoin;

  ITEM *pItemCache = NULL;

  for (int iy = 0; iy < (int)this->m_DataListPhucLoi.size(); iy++) {
    if (iy > 5)
      break;

    for (int ix = 0; ix < 6; ix++) {
      if (this->m_DataListPhucLoi[iy].mListItem[ix].Count == 0)
        continue;

      ITEM *pItem = g_pNewItemMng->CreateItem(
          this->m_DataListPhucLoi[iy].mListItem[ix].Item);
      if (pItem == NULL)
        continue;

      float x = PosXBoxItem + (KhoangCach * ix);
      float y = PosYBoxItem + (KhoangCach * iy);
      float w = WBox;
      float h = WBox;

      float perimeter = (w + h) * 2.0f;
      int length = (int)(perimeter / 4.5f);
      if (length < 10)
        length = 90;
      if (length > 60)
        length = 190;

      bool needGlow = true;
      float gr = 1.0f, gg = 1.0f, gb = 1.0f;

      int level = (pItem->Level >> 3) & 0x0F;

      if (GetCustomItemGlowColor(pItem->Type, gr, gg, gb)) {
      } else {
        bool isExcellent = false;
        if (pItem->Option1 != 0)
          isExcellent = true;

        if (isExcellent) {
          gr = 0.0f;
          gg = 1.0f;
          gb = 0.0f;
        } else if (pItem->Type == GET_ITEM(14, 11) && level >= 9 &&
                   level <= 12) {
          gr = 1.0f;
          gg = 1.0f;
          gb = 0.0f;
        } else {
          gr = 1.0f;
          gg = 1.0f;
          gb = 1.0f;
        }
      }
      gInterface->DrawInfoBox(x, y, w, h, 0x00000096, 0);
      if (needGlow) {
        gInterface->DrawGlowAroundBox2(x - 0.8f, y - 1.5f, w + 9.0f, h + 9.0f,
                                       length, gr, gg, gb);
      }
      // Fix Widescreen Isometric Perspective Drift (Pixel-perfect offsets for 5
      // boxes)
      float AntiDriftX[6] = {0.0f, 3.0f, 5.0f, 5.0f, 5.0f, 5.0f};
      float renderX = x + AntiDriftX[ix];

      float itemScale = this->m_DataListPhucLoi[iy].mListItem[ix].SizeBMD;
      if (itemScale <= 0.0f) {
        itemScale = 1.0f;
      }

      SEASON3B::RenderLocalItem3D(renderX, y, w, h, pItem->Type, pItem->Level,
                                  pItem->Option1, pItem->ExtOption, false,
                                  itemScale);
      glColor4f(1.f, 1.f, 1.f, 1.f);
      SEASON3B::TextDraw((HFONT)g_hFont, x + 5, y, 0xE0FF14A5, 0x0, WBox, 0, 4,
                         "x%d",
                         this->m_DataListPhucLoi[iy].mListItem[ix].Count);

      if (SEASON3B::CheckMouseIn(x, y, w, h)) {
        pItemCache = g_pNewItemMng->CreateItem(
            this->m_DataListPhucLoi[iy].mListItem[ix].Item);
      }
    }

    int CoinNhan = this->m_DataListPhucLoi[iy].DieuKienNhan;

    if (TongSoDaNap < CoinNhan) {
      glColor4f(1.f, 1.f, 1.f, 1.f);
      SEASON3B::TextDraw((HFONT)g_hFont, TextX + (W - 105),
                         TextY + 12 + (KhoangCach * iy), eGold, 0x0, 100, 0, 3,
                         "Điều kiện nhận");

      if (this->StatePage == eQuaNapThang || this->StatePage == eQuaNapDau ||
          this->StatePage == eQuaNapNgay || this->StatePage == eQuaNapTichLuy) {
        SEASON3B::TextDraw((HFONT)g_hFontBold, TextX + (W - 105),
                           TextY + 20 + (KhoangCach * iy), 0x6ED7E0FF, 0x0, 100,
                           0, 3, "%s ATM", gInterface->NumberFormat(CoinNhan));
      } else if (this->StatePage == eQuaTieuPhiNgay ||
                 this->StatePage == eQuaTieuPhiThang ||
                 this->StatePage == eQuaTieuPhiTichLuy) {
        SEASON3B::TextDraw((HFONT)g_hFontBold, TextX + (W - 105),
                           TextY + 20 + (KhoangCach * iy), 0x6ED7E0FF, 0x0, 100,
                           0, 3, "%s WCoinC",
                           gInterface->NumberFormat(CoinNhan));
      }

      gInterface->DrawBarForm(TextX + (W - 100), TextY + 30 + (KhoangCach * iy),
                              WProcess, 6, 0.74, 0.74, 0.74, 1.0);

      float PhanTramTienDo = (TongSoDaNap) * 100 / (CoinNhan);
      float TyLeTGA = (WProcess * PhanTramTienDo) / 100;
      if (TyLeTGA > WProcess) {
        TyLeTGA = WProcess;
      }

      gInterface->DrawBarForm(TextX + (W - 100), TextY + 30 + (KhoangCach * iy),
                              TyLeTGA, 6, 1, 0.7942, 0.05, 1.0);
    } else if (m_DataListMocDaNhan > iy) {
      glColor4f(1.f, 1.f, 1.f, 1.f);
      SEASON3B::TextDraw((HFONT)g_hFont, TextX + (W - 105),
                         TextY + 12 + (KhoangCach * iy), 0x6ED7E0FF, 0x0, 100,
                         0, 3, "Đã Nhận Rồi");
    } else {
      if (gInterface->DrawButton(TextX + (W - 100),
                                 TextY + 15 + (KhoangCach * iy), 120, 12,
                                 "Nhận Quà", 90) &&
          (GetTickCount() - gInterface->Data[eWindowQuaPhucLoi].EventTick) >
              500) {
        gInterface->Data[eWindowQuaPhucLoi].EventTick = GetTickCount();
        CB_QuaPhucLoi::CGPACKET_SENDRECV_A pMsg;
        pMsg.header.set(0xD3, 0x4E, sizeof(pMsg));
        pMsg.Index = iy;
        pMsg.Type = this->StatePage;
        DataSend((LPBYTE)&pMsg, pMsg.header.size);
      }
    }

    if (pItemCache != NULL) {
      g_pNewItemTooltip->RenderItemTooltip(MouseX + 75, MouseY, pItemCache, 0,
                                           0, false, false);
    }
  }
}

void CB_QuaPhucLoi::DrawWindow() {
  // if (gInterface->CheckWindow(Interface::MoveList) ||
  // gInterface->CheckWindow(Interface::ObjWindow::CashShop) /*||
  // gInterface->CheckWindow(Interface::ObjWindow::SkillTree)*/ ||
  // gInterface->CheckWindow(Interface::ObjWindow::FullMap)
  //	|| (gInterface->CheckWindow(Interface::Inventory)
  //		&& gInterface->CheckWindow(Interface::ExpandInventory)
  //		&& gInterface->CheckWindow(Interface::Store))
  //	|| (gInterface->CheckWindow(Interface::Inventory)
  //		&& gInterface->CheckWindow(Interface::Warehouse)
  //		&& gInterface->CheckWindow(Interface::ExpandWarehouse)))
  //{
  //	gInterface->Data[eWindowQuaPhucLoi].OnShow = false;
  //	return;
  // }

  if (!gInterface->Data[eWindowQuaPhucLoi].OnShow) {
    this->StatePage = -1;

    if (InputGiftCode) {
      delete InputGiftCode;
      InputGiftCode = NULL;
      ZeroMemory(GetTextGiftCode, sizeof(GetTextGiftCode));
    }
    return;
  }
  float WindowW = 440;
  float WindowH = 280;
  float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
  float StartY = (MAX_WIN_HEIGHT / 2) - (WindowH / 2);

  gInterface->Data[eWindowQuaPhucLoi].X = StartX;
  gInterface->Data[eWindowQuaPhucLoi].Y = StartY;

  gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH,
                                eWindowQuaPhucLoi, "QUÀ PHÚC LỢI");

  //==Button
  float ButtonWX = StartX + 10;
  float ButtonWY = StartY + 30;
  float ButtonWW = 100;
  float ButtonWH = WindowH - 50;
  const float TabDrawSize = 150.0f;
  const float TabWidth = ButtonWW + 5.0f;
  const float TabHeight = (TabDrawSize * 20.0f) / 100.0f;
  gInterface->DrawInfoBox(ButtonWX, ButtonWY, ButtonWW, ButtonWH, 0x00000096,
                          0);

  std::vector<int> allowedButtons;
  if (gmProtect->GiftCode)
    allowedButtons.push_back(CB_QuaPhucLoi::eGiftCode);
  if (gmProtect->NapDau)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaNapDau);
  if (gmProtect->NapNgay)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaNapNgay);
  if (gmProtect->NapThang)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaNapThang);
  if (gmProtect->NapTichLuy)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaNapTichLuy);
  if (gmProtect->TieuPhiNgay)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaTieuPhiNgay);
  if (gmProtect->TieuPhiThang)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaTieuPhiThang);
  if (gmProtect->TieuPhiTichLuy)
    allowedButtons.push_back(CB_QuaPhucLoi::eQuaTieuPhiTichLuy);

  for (int i = 0; i < allowedButtons.size(); ++i) {
    int n = allowedButtons[i];
    float tabY = ButtonWY + (i * 30);

    if (gInterface->DrawButton(ButtonWX, tabY, TabDrawSize, 12,
                               strdup(m_ButtoNText[n].c_str()), TabWidth) &&
        (GetTickCount() - gInterface->Data[eWindowQuaPhucLoi].EventTick) >
            500) {
      gInterface->Data[eWindowQuaPhucLoi].EventTick = GetTickCount();
      CB_QuaPhucLoi::CGPACKET_SENDRECV_B pMsg;
      pMsg.header.set(0xD3, 0x4D, sizeof(pMsg));
      pMsg.ThaoTac = n;
      DataSend((LPBYTE)&pMsg, pMsg.header.size);
      m_DataListPhucLoi.clear();
    }

    if (this->StatePage == n) {
      DrawActiveTabGlow(ButtonWX - 1.0f, tabY - 1.0f, TabWidth + 2.0f,
                        TabHeight);
    }
  }
  // Draw Page
  float PageWX = ButtonWX + ButtonWW + 10;
  float PageWY = StartY + 30;
  float PageWW = WindowW - 130;
  float PageWH = WindowH - 40;
  switch (this->StatePage) {
  case CB_QuaPhucLoi::eGiftCode:
    DrawPageGift(PageWX, PageWY, PageWW, PageWH);
    break;
  case CB_QuaPhucLoi::eQuaNapDau:
  case CB_QuaPhucLoi::eQuaNapNgay:
  case CB_QuaPhucLoi::eQuaNapThang:
  case CB_QuaPhucLoi::eQuaNapTichLuy:
  case CB_QuaPhucLoi::eQuaTieuPhiNgay:
  case CB_QuaPhucLoi::eQuaTieuPhiThang:
  case CB_QuaPhucLoi::eQuaTieuPhiTichLuy:
    DrawPageListItem(PageWX, PageWY, PageWW, PageWH);
    break;
  default:
    DrawPageDef(PageWX, PageWY, PageWW, PageWH);
    break;
  }
}

void CB_QuaPhucLoi::RecvClientButton(BYTE *aRecv) {
  if (!aRecv)
    return;
  CGPACKET_SENDRECV_B *lpMsg = (CGPACKET_SENDRECV_B *)aRecv;

  this->StatePage = lpMsg->ThaoTac;
  SetServerTime(lpMsg->server_time);
  // gInterface->DrawMessage(1, "RecvClientButton %d", this->StatePage);
}

void CB_QuaPhucLoi::RecvDataInfo(BYTE *lpMsg) {
  if (!lpMsg)
    return;

  m_DataListPhucLoi.clear();

  PMSG_CBLISTTHUONG_SEND *mRecv = (PMSG_CBLISTTHUONG_SEND *)lpMsg;
  this->m_DataListCoin = mRecv->CoinCache;
  this->m_DataListMocDaNhan = mRecv->MocDaNhan;
  for (int i = 0; i < mRecv->count; i++) {
    DATALIST_QUAPL lpInfo =
        *(DATALIST_QUAPL *)(((BYTE *)lpMsg) + sizeof(PMSG_CBLISTTHUONG_SEND) +
                            (sizeof(DATALIST_QUAPL) * i));

    m_DataListPhucLoi.push_back(lpInfo);
  }
}
