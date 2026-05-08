#include "stdafx.h"
#include "jpexs.h"
#include "UIBaseDef.h"
#include "UIControls.h"
#include "CBInterface.h"
#include "ZzzInterface.h"
#include "NewUISystem.h"
#include "NewUICommon.h"
#include "MapManager.h"
#include "CB_QuaPhucLoi.h"
#include "Protocol.h"
#include "TextClient.h"
#include "NewUIQuestProgress.h"
#include "CustomVongQuay.h"

using namespace SEASON3B;

CB_Interface* CB_Interface::Instance()
{
	static CB_Interface s_Instance;
	return &s_Instance;
}

CB_Interface::CB_Interface()
{
	this->BLockButtonMoveWindow = false;
	ZeroMemory(&Data, sizeof(Data));
	this->CloseAllWindowCustom();
	BButtonClickTime = 0;
	BLockButtonHover = false;
	BLockMessageBox = false;
	AutoHP = false;
	AutoCtrlPK = false;

	//==MessBox
	this->MsgBoxCaption.clear();
	this->MsgBoxText.clear();
	this->MsgBoxType = 0;
	this->MsgLine = 0;

	this->vCaptcha = this->generateCaptcha(4);

#if(CUSTOM_CHANGEITEM)
	this->CItem_ActiveMix = false;
	this->CItem_EffectsMix = false;
	this->SetStateDoiItem = false;
	this->CostWC = 0;
	this->CostWP = 0;
	this->CostGP = 0;
	this->ChangeItemOpenMap = -1;
	this->ChangeItemOpenX = -1;
	this->ChangeItemOpenY = -1;
	memset(this->requiredInfoItem, 0, sizeof(this->requiredInfoItem));
	memset(this->CItem_ItemChinh, 0xFF, sizeof(this->CItem_ItemChinh));
	memset(this->CItem_ItemPhu, 0xFF, sizeof(this->CItem_ItemPhu));

	for (int n = 0; n < 12; n++)
	{
		memset(this->CItem_ItemKetQua[n], 0xFF, sizeof(this->CItem_ItemKetQua[n]));
	}
#endif

}

CB_Interface::~CB_Interface()
{
}


int CB_Interface::DrawMessage(int Mode, LPCSTR Text, ...)
{
	// ----
	va_list args;
	va_start(args, Text);
	TCHAR Buff[256];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);
	// ----
	CreateNotice(Buff, Mode);
	return 1;
}
void CB_Interface::CloseAllWindowCustom()
{
	for (int i = eBeginWindowCustom; i < eEndWindowCustom; i++) this->Data[i].Close(); //Close All window
}
void CB_Interface::SetBlockCur(bool Delay)
{
	if (Delay)
	{
		BLockMouseMoveClick = GetTickCount() + 300;
	}
	else
	{
		BLockMouseMoveClick = 0;
	}

}
bool CB_Interface::BtnProcess()
{
	if (BLockMouseMoveClick > GetTickCount())
	{
		return true;

	}
	else
	{
		BLockMouseMoveClick = 0;
	}
	return false;
}
void CB_Interface::MoveWindows(int eNumWindow)
{
	if (GetKeyState(VK_LBUTTON) & 0x8000 && GetTickCount() - this->Data[eNumWindow].EventTick > 300)
	{
		if (SEASON3B::CheckMouseIn(this->Data[eNumWindow].X, this->Data[eNumWindow].Y, this->Data[eNumWindow].Width - 70, 30))
		{
			if (!this->Data[eNumWindow].OnClick) {
				this->Data[eNumWindow].OnClick = true;
				this->Data[eNumWindow].curX = MouseX;
				this->Data[eNumWindow].curY = MouseY;
			}
		}
		if (this->Data[eNumWindow].OnClick)
		{
			this->BLockButtonMoveWindow = true;
			this->Data[eNumWindow].X += MouseX - this->Data[eNumWindow].curX;
			this->Data[eNumWindow].Y += MouseY - this->Data[eNumWindow].curY;
			this->Data[eNumWindow].curX = MouseX;
			this->Data[eNumWindow].curY = MouseY;
			//===
			if (this->Data[eNumWindow].X < 0)
				this->Data[eNumWindow].X = 0;
			if (this->Data[eNumWindow].Y < 0)
				this->Data[eNumWindow].Y = 0;

			if ((this->Data[eNumWindow].X + this->Data[eNumWindow].Width) > gwinhandle->GetScreenX())
				this->Data[eNumWindow].X = gwinhandle->GetScreenX() - (this->Data[eNumWindow].Width);
			if ((this->Data[eNumWindow].Y + this->Data[eNumWindow].Height + 46) > gwinhandle->GetScreenY())
				this->Data[eNumWindow].Y = gwinhandle->GetScreenY() - (this->Data[eNumWindow].Height + 46);
		}
	}
	else {
		if (this->Data[eNumWindow].OnClick)
		{
			this->Data[eNumWindow].OnClick = false;
			this->BLockButtonMoveWindow = false;
		}
	}
}
void CB_Interface::DrawBarForm(float PosX, float PosY, float Width, float Height, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	EnableAlphaTest(true);
	glColor4f((GLfloat)red, (GLfloat)green, (GLfloat)blue, (float)alpha);
	RenderColor(PosX, PosY, Width, Height, 0.0f, 0);
	EndRenderColor();
	glColor3f(1, 1, 1);
	EnableAlphaTest(false);;
}
int CB_Interface::TextDraw(HFONT font, int PosX, int PosY, DWORD color, DWORD bkcolor, int Width, int Height, BYTE Align, LPCTSTR Text, ...)
{
	va_list args;
	va_start(args, Text);
	TCHAR Buff[256];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);

	int LineCount = 0;
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	char* Line = strtok(Buff, "\n##");
	DWORD BKTextColor = g_pRenderText->GetTextColor();
	DWORD BKBGColor = g_pRenderText->GetBgColor();
	g_pRenderText->SetFont(font);
	g_pRenderText->SetTextColor((color & 0xff000000) >> 24, (color & 0x00ff0000) >> 16, (color & 0x0000ff00) >> 8, color & 0x000000ff);
	g_pRenderText->SetBgColor((bkcolor & 0xff000000) >> 24, (bkcolor & 0x00ff0000) >> 16, (bkcolor & 0x0000ff00) >> 8, bkcolor & 0x000000ff);
	while (Line != NULL)
	{
		g_pRenderText->RenderText(PosX, PosY, Line, Width, Height, Align, 0);
		PosY += 10;
		Line = strtok(NULL, "\n##");

		LineCount++;
	}
	EnableAlphaTest(false);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(BKTextColor);
	g_pRenderText->SetBgColor(BKBGColor);
	return LineCount;
}
bool CB_Interface::gDrawWindowCustom(float* StartX, float* StartY, float CuaSoW, float CuaSoH, int eNumWindow, LPCSTR Text, ...)
{
	va_list args;
	va_start(args, Text);
	TCHAR Buff[90];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);

	if (!this->Data[eNumWindow].OnShow)
	{
		return 0;
	}
	if (!this->Data[eNumWindow].FirstLoad)
	{
		this->Data[eNumWindow].Width = CuaSoW;
		this->Data[eNumWindow].Height = CuaSoH;
		this->Data[eNumWindow].X = *StartX;
		this->Data[eNumWindow].Y = *StartY;
		this->Data[eNumWindow].FirstLoad = true;
	}
	else
	{
		(*StartX) = this->Data[eNumWindow].X;
		(*StartY) = this->Data[eNumWindow].Y;
	}


	if (this->Data[eNumWindow].AllowMove)
	{
		MoveWindows(eNumWindow);
	}


	float closeOffsetX = -27.0f;

	if (eNumWindow == eTopDame)
	{
		closeOffsetX = -5.0f;
	}

	int CloseX = (int)(this->Data[eNumWindow].X + CuaSoW + closeOffsetX);
	int CloseY = (int)(this->Data[eNumWindow].Y + 5.0f);
	bool isHoverWindow = (SEASON3B::CheckMouseIn(this->Data[eNumWindow].X, this->Data[eNumWindow].Y, CuaSoW, CuaSoH) == 1);
	bool isHoverClose = (SEASON3B::CheckMouseIn(CloseX, CloseY, 36, 36) == 1);

	if (isHoverWindow || isHoverClose)
	{
		this->SetBlockCur(TRUE);
		this->Data[eNumWindow].Hover = true;
	}
	else
	{
		this->Data[eNumWindow].Hover = false;
	}
	float ScaleW = (225 / CuaSoW) / g_fScreenRate_x;
	float ScaleH = (225 / CuaSoH) / g_fScreenRate_y;

	EnableAlphaTest();
	window_backmsg(this->Data[eNumWindow].X, this->Data[eNumWindow].Y, CuaSoW, CuaSoH);
	//======Close
	SEASON3B::RenderImageF(IMAGE_BUTTON_CLOSE_NAVI, CloseX, CloseY, 16.0, 16.0, 0.0, 0.0, 15.0, 15.0);
	if (isHoverClose)
	{
		if (GetKeyState(VK_LBUTTON) & 0x8000 && GetTickCount() - this->Data[eNumWindow].EventTick > 300 /*&& !BLockButtonHover && !BLockMessageBox*/ && !BLockButtonMoveWindow)
		{
			if (eNumWindow == eWindowVongQuay && gVongQuay.ShouldBlockClose())
			{
				this->Data[eNumWindow].EventTick = GetTickCount();
			}
			else
			{
				PlayBuffer(25, 0, 0);
				glColor3f(1.0f, 0.0f, 0.0f);
				BLockMouseMoveClick = GetTickCount() + 500;
				this->Data[eNumWindow].Close();
			}
		}
		SEASON3B::RenderImageF(IMAGE_BUTTON_CLOSE_NAVI, CloseX, CloseY, 16.0, 16.0, RGBA(255, 204, 20, 130), 0.0, 0.0, 15.0, 15.0);
		RenderTipText(CloseX + 40, CloseY + 15, (eNumWindow == eWindowVongQuay && gVongQuay.ShouldBlockClose()) ? gTextClient.txtClient_VQMM[1] : gTextClient.txtClient_JewelBank[7]);
	}
	//================================================
	float titleX = this->Data[eNumWindow].X;
	float titleY = this->Data[eNumWindow].Y + 10.0f;
	float titleWidth = CuaSoW;
	DWORD titleBackColor = 0x0;
	BYTE titleAlign = 3;

	if (eNumWindow == eTopDame)
	{
		SIZE titleSize = { 0 };
		float titleTextWidth = 0.0f;
		float titleTextHeight = 0.0f;
		float titlePaddingX = 8.0f;
		float titlePaddingY = 2.0f;
		float titleBarY = 0.0f;
		float titleBarHeight = 0.0f;
		HFONT fontBack = g_pRenderText->GetFont();

		g_pRenderText->SetFont(g_hFontBold);

		if (g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Buff, lstrlen(Buff), &titleSize) == FALSE)
		{
			titleSize.cx = (int)CuaSoW;
			titleSize.cy = 11;
		}

		g_pRenderText->SetFont(fontBack);

		titleTextWidth = ((float)titleSize.cx / g_fScreenRate_x);
		titleTextHeight = ((float)titleSize.cy / g_fScreenRate_y);

		if (titleTextWidth < 1.0f)
		{
			titleTextWidth = CuaSoW;
		}

		if (titleTextHeight < 1.0f)
		{
			titleTextHeight = 11.0f;
		}

		titleWidth = titleTextWidth + (titlePaddingX * 2.0f);
		titleX = this->Data[eNumWindow].X + ((CuaSoW - titleWidth) * 0.5f);
		titleY = this->Data[eNumWindow].Y - titleTextHeight + 2.0f;
		titleBarY = titleY - titlePaddingY;
		titleBarHeight = titleTextHeight + (titlePaddingY * 2.0f);

		this->DrawBarForm(titleX, titleBarY, titleWidth, titleBarHeight, 0.0f, 0.0f, 0.0f, 0.65f);

		titleBackColor = 0x0;
		titleAlign = 3;
	}

	this->TextDraw(g_hFontBold, (int)titleX, (int)titleY, 0xFFFFFFFF, titleBackColor, (int)titleWidth, 0, titleAlign, "%s", Buff); //Buoc Vao

	glColor3f(1, 1, 1);
	EnableAlphaTest(false);

	return 1;
}


bool CB_Interface::CheckWindow(int WindowID)
{
	return g_pNewUISystem->IsVisible(WindowID);
}

bool CB_Interface::DrawButton(float PosX, float PosY, float SizeButton, int FontSize, const char* ButtonText, float mSizeButtonW, bool IsEnter)
{
	EnableAlphaTest();
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//============Button
	float SizeButtonW = SizeButton;
	float SizeButtonH = (SizeButton * 20) / 100;

	if (mSizeButtonW != 0)
	{
		SizeButtonW = mSizeButtonW;
	}
	float StartX = PosX;
	float StartY = PosY;
	bool mreturn = false;
	float ScaleX = 0.0, ScaleY = 0.227;
	//=====================Button 1
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
	{
		BLockMouseMoveClick = GetTickCount() + 500;
		ScaleX = ScaleY;
		if (GetTickCount() - BButtonClickTime > 500) //Click
		{
			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{

				ScaleX = ScaleY * 2;
				BButtonClickTime = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
			}
		}
	}
	if ((GetKeyState(VK_RETURN) & 0x8000) && IsEnter)
	{
		//g_pBCustomMenuInfo->CloseWindow(ChatWindow);
		PlayBuffer(25, 0, 0);
		mreturn = true;

	}
	//31563
	//RenderBitmap(31563, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	RenderBitmap(BITMAP_btn_empty, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, ScaleX, 0.830, ScaleY, 1, 1, 0);
	TextDraw(g_hFontBold, StartX, (StartY + (SizeButtonH / 2) - (FontSize / 2.7)), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, ButtonText); //Buoc Vao
	//=====================================
	//DeleteObject(ButtonText);
	glColor3f(1, 1, 1);
	EnableAlphaTest(false);
	return mreturn;
}

void CBRenderText(char* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort)
{
	if (!g_pRenderText) return;
	g_pRenderText->SetFont(g_hFont);

	DWORD backuptextcolor = g_pRenderText->GetTextColor();
	DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

	g_pRenderText->SetTextColor(color);
	g_pRenderText->SetBgColor(backcolor);
	g_pRenderText->RenderText(x, y, text, sx, sy, sort);

	g_pRenderText->SetTextColor(backuptextcolor);
	g_pRenderText->SetBgColor(backuptextbackcolor);
}

int CB_Interface::DrawFormat(DWORD Color, int PosX, int PosY, int Width, int Align, LPCSTR Text, ...)
{
	va_list args;
	va_start(args, Text);
	TCHAR Buff[256];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);

	//CBRenderText(Buff, PosX, PosY, Width, 0, Color, 0, Align);
	//return 1;

	int LineCount = 0;

	char* Line = strtok(Buff, "\n");

	while (Line != NULL)
	{
		CBRenderText(Line, PosX, PosY, Width, 0, Color, 0, Align);

		PosY += 10;
		Line = strtok(NULL, "\n");
	}

	return PosY;
}


char* CB_Interface::NumberFormat(int Number)
{


	if (Number < 0) { return "0"; }

	char OutPut[15];

	if (Number < 1000) {
		sprintf(OutPut, "%d", Number);
		return strdup(OutPut);
	}
	else if (Number < 1000000) {
		int part1 = Number % 1000;
		int part2 = (Number - part1) / 1000;
		sprintf(OutPut, "%d,%03d", part2, part1);
		return strdup(OutPut);
	}
	else if (Number < 1000000000) {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;

		sprintf(OutPut, "%d,%03d,%03d", part3, part2, part1);
		return strdup(OutPut);
	}
	else {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;
		int part4 = (Number - part3) / 1000;

		sprintf(OutPut, "%d,%03d,%03d,%03d", part4, part3, part2, part1);
		return strdup(OutPut);
	}
	return "0";
}


bool CB_Interface::DrawButtonGUI(int IDGUID, float PosX, float PosY, float SizeW, float SizeH, int TypeButton)
{
	EnableAlphaTest();
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	bool mreturn = false;
	//=====================Button 1

	if (TypeButton == 3)
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;

			if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - 
				
				
				gInterface->Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
			{
				gInterface->Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 2);
			}
			else {
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 1);
			}

		}
		else {
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
		}
	}
	else if (TypeButton == 2)
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;

			if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface->Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
			{
				gInterface->Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 1);
			}
			else {
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
			}

		}
		else {
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
		}
	}

	else
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;
			if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow) //Click
			{
				if ((GetKeyState(VK_LBUTTON) & 0x8000))
				{
					gInterface->Data[eTIME].EventTick = GetTickCount();
					PlayBuffer(25, 0, 0);
					mreturn = true;
				}
			}
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0, 0, RGBA(255, 228, 107, 255));
		}
		else
		{
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH);
		}
	}
	glColor3f(1, 1, 1);
	EnableAlphaTest(false);
	return mreturn;
}

float WrapPos(float s, float P)
{
	s = fmodf(s, P);
	if (s < 0) s += P;
	return s;
}
void DrawSegmentOnPerimeter(
	float s, float segLen, float alpha,
	float x, float y, float w, float h, float P,
	float thickness,
	float r, float g, float b
)
{
	float a = WrapPos(s - segLen * 0.5f, P);
	float bS = WrapPos(s + segLen * 0.5f, P);

	float fromArr[2] = { a, 0.0f };
	float toArr[2] = { bS, bS };
	int count = (a <= bS) ? 1 : 2;
	if (count == 2) toArr[0] = P;

	for (int k = 0; k < count; ++k)
	{
		float cur = fromArr[k];
		float to = toArr[k];

		while (cur < to)
		{
			float gx = x, gy = y;
			float remain = to - cur;
			float edgeEnd;
			int edge;

			if (cur < w) { edge = 0; edgeEnd = w; }
			else if (cur < w + h) { edge = 1; edgeEnd = w + h; }
			else if (cur < 2.0f * w + h) { edge = 2; edgeEnd = 2.0f * w + h; }
			else { edge = 3; edgeEnd = P; }

			float stepLen = min(remain, edgeEnd - cur);
			float pos = cur;

			if (edge == 0)
				gInterface->DrawBarForm(x + pos, y - thickness * 0.5f, stepLen, thickness, r, g, b, alpha);
			else if (edge == 1)
				gInterface->DrawBarForm(x + w - thickness * 0.5f, y + (pos - w), thickness, stepLen, r, g, b, alpha);
			else if (edge == 2)
				gInterface->DrawBarForm(x + w - (pos - (w + h)) - stepLen, y + h - thickness * 0.5f, stepLen, thickness, r, g, b, alpha);
			else
				gInterface->DrawBarForm(x - thickness * 0.5f, y + h - (pos - (2.0f * w + h)) - stepLen, thickness, stepLen, r, g, b, alpha);

			cur += stepLen;
		}
	}
}

void CB_Interface::DrawGlowAroundBox2(
	float x, float y, float w, float h,
	int length, float r, float g, float b)
{
	if (w <= 0 || h <= 0) return;
	if (length <= 0) length = 1;

	float P = 2.0f * (w + h);
	float t = (float)(GetTickCount() % 5000) / 5000.0f;

	float head1 = P * t;
	float head2 = fmodf(head1 + P * 0.5f, P);

	float thickness = 1.0f;
	float ds = 1.0f;
	float seg = 6.0f;

	for (int k = 0; k < 2; ++k)
	{
		float head = (k == 0) ? head1 : head2;

		for (int i = 0; i < length; ++i)
		{
			float fade = (float)i / (float)length;
			float alpha = 0.95f * (1.0f - fade);
			float s = WrapPos(head - i * ds, P);

			DrawSegmentOnPerimeter(
				s, seg, alpha,
				x, y, w, h, P,
				thickness,
				r, g, b
			);
		}
	}
}


void CB_Interface::DrawInfoBox(float PosX, float PosY, float Width, float Height, DWORD bkcolor, int Arg6, int Arg7)
{
	float Red = (float)((bkcolor & 0xff000000) >> 24) / 255.0f;
	float Green = (float)((bkcolor & 0x00ff0000) >> 16) / 255.0f;
	float Blue = (float)((bkcolor & 0x0000ff00) >> 8) / 255.0f;
	float Alpha = (float)(bkcolor & 0x000000ff) / 255.0f;
	EnableAlphaTest();
	if (bkcolor)
	{
		glColor4f(Red, Green, Blue, Alpha);
		RenderColor(PosX, PosY, Width + 6.5, Height + 5, 0, Arg6); //central
		EndRenderColor();
	}
	float RightX = PosX - 2.0f;
	float LeftX = (PosX + Width) - 2.0f;
	float TopY = PosY - 2.0f;
	float DownY = (PosY + Height) - 2.0f;
	bool horizontal = true;
	bool vertical = true;
	//--
	float IniciarX = PosX - 2.0f;
	float w = 14.0f;
	//--
	float IniciarY = PosY - 2.0f;
	float H = 14.0f;


	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


	while (vertical || horizontal)
	{
		if (vertical)
		{
			IniciarX += 14.0f;

			if ((IniciarX + w) > LeftX)
			{
				vertical = false;
				w = LeftX - IniciarX;
			}
			RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL, IniciarX, TopY, w, 14.0f); //Ngang

			RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL, IniciarX, DownY, w, 14.0f); //Ngang 2
		}
		//-- abajo
		if (horizontal)
		{
			IniciarY += 14.0f;

			if ((IniciarY + H) > DownY)
			{
				horizontal = false;
				H = DownY - IniciarY;
			}

			RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL, LeftX, IniciarY, 14.0f, H); //Doc 2

			RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL, RightX, IniciarY, 14.0f, H); // Doc 1


		}
	}

	RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT, RightX, TopY, 14.0f, 14.0f); //Goc tren ben trai

	RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT, LeftX, TopY, 14.0f, 14.0f); //Goc tren ben phai

	RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT, RightX, DownY, 14.0f, 14.0f);  //Goc duoi trai

	RenderImage(CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT, LeftX, DownY, 14.0f, 14.0f);  //Goc Duoi phai
	glColor3f(1, 1, 1);
	EnableAlphaTest(false);
}
bool CB_Interface::RenderCheckBox(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...)
{
	bool mreturn = false;
	va_list args;
	va_start(args, Text);
	TCHAR Buff[256];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);

	if (Checkis)
	{
		RenderImage(CNewUIOptionWindow::IMAGE_OPTION_BTN_CHECK, PosX, PosY, 15.0, 15.0, 0.0, 0.0);
	}
	else
	{
		RenderImage(CNewUIOptionWindow::IMAGE_OPTION_BTN_CHECK, PosX, PosY, 15.0, 15.0, 0.0, 15.0);
	}
	TextDraw(g_hFont, PosX + 16, PosY + 3, TextColor, 0x0, 0, 0, 1, Text);

	if (SEASON3B::CheckMouseIn(PosX, PosY, 15, 15) == 1)
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface->Data[eTIME].EventTick > 500)
		{
			gInterface->Data[eTIME].EventTick = GetTickCount();
			PlayBuffer(25, 0, 0);
			BButtonClickTime = GetTickCount();
			mreturn = true;
		}
	}
	return mreturn;
}




bool CB_Interface::DrawSelectBar(int X, int Y, int W, int H, int* SelectNum, std::vector<std::string> Data)
{
	if (Data.empty()) return 0;
	//==31422 Select Bar
	static int ShowBarSelect = 0;
	int SelectBarHover = 0;
	DWORD SelectBarHoverBG = 0x0;
	float SelectBarY = Y;
	bool KQClick = false;
	RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, X - 2, Y + 37, W, H, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
	if (SEASON3B::CheckMouseIn(X - 2, Y + 37, W, H))
	{

		SelectBarHover = 1;
		SelectBarHoverBG = 0x66646450;
		if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) //Click
		{
			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				SelectBarHover = 2;
				gInterface->Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				ShowBarSelect ^= 1;
			}
		}
		this->SetBlockCur(TRUE);
	}
	RenderBitmap(CNewUICastleWindow::IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN, X + 2, Y + 39.5, 13.5, 13.5, 0.0, 0.21 * SelectBarHover, 0.91, 0.21, 1, 1, 0.0);
	TextDraw(g_hFont, X + 15, Y + 41, 0xFFFFFFFF, SelectBarHoverBG, W - 25, 0, 3, Data[*SelectNum].c_str());// 
	if (ShowBarSelect)
	{
		int CountSelectBar = 1;
		gInterface->DrawBarForm(X + 15, Y + 41 + (15), W - 25, (H - 5) * (Data.size()), 0.0, 0.0, 0.0, 0.8);
		for (int i = 0; i < Data.size(); i++)
		{
			DWORD SelectHover = 0x0;
			if (SEASON3B::CheckMouseIn(X + 15, Y + 41 + (15 * CountSelectBar), W + 20, 15))
			{
				SelectHover = 0x66646450;
				if (GetTickCount() - gInterface->Data[eTIME].EventTick > 500) //Click
				{
					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						*SelectNum = i;
						gInterface->Data[eTIME].EventTick = GetTickCount();
						PlayBuffer(25, 0, 0);
						ShowBarSelect ^= 1;
						KQClick = true;

					}
				}
				this->SetBlockCur(TRUE);
			}
			TextDraw(g_hFont, X + 15, Y + 41 + (15 * CountSelectBar), 0xFFFFFFFF, SelectHover, W - 25, 0, 3, Data[i].c_str());// 
			CountSelectBar++;
		}
	}
	return KQClick;
}




bool CB_Interface::RenderInputBox(float PosX, float PosY, float W, float H, char* TextSet, CUITextInputBox*& Input, UIOPTIONS UiOption, int MaxChar, bool isPass)
{
	// Input Box
	if (!Input)
	{
		Input = new CUITextInputBox;
		Input->Init(gwinhandle->GethWnd(), W, H, MaxChar, isPass);
		Input->SetPosition(PosX, PosY);
		Input->SetTextColor(255, 255, 230, 230);
		//Input->SetBackColor(0, 0, 0, 25);
		Input->SetBackColor(128, 128, 128, 255);
		Input->SetFont(g_hFont);
		Input->SetState(UISTATE_NORMAL);
		Input->SetOption(UiOption);
		Input->SetText(TextSet);
	}
	else
	{
		Input->Render();
		if (SEASON3B::CheckMouseIn(PosX - 3, PosY - 2.5, W + 6, H + 5) == 1)
		{

			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				Input->GiveFocus(1);
			}
		}
		return  1;
	}
	return 0;
}


void CB_Interface::DrawMessageBox()
{
	if (!gInterface->Data[eWindowMessageBox].OnShow)
	{
		return;
	}

	// ESC key to close the message box
	if ((GetKeyState(VK_ESCAPE) & 0x8000) && GetTickCount() - gInterface->Data[eTIME].EventTick > 500)
	{
		gInterface->Data[eTIME].EventTick = GetTickCount();
		gInterface->Data[eWindowMessageBox].OnShow = 0;
		this->MsgBoxCallback = 0;
		PlayBuffer(25, 0, 0);
		return;
	}

	float CuaSoW = 220;
	float CuaSoH = 80 + (this->MsgLine * 10);

	float StartX = (MAX_WIN_WIDTH / 2) - (CuaSoW / 2);
	float StartY = 15.0;

	gInterface->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowMessageBox, this->MsgBoxCaption.c_str());
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	gInterface->SetBlockCur(TRUE);
	this->MsgLine = TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFFFFFFFF, 0x0, CuaSoW, 0, 3, this->MsgBoxText.c_str());
	if (gInterface->Data[eWindowMessageBox].Height < (80 + (this->MsgLine * 10))) gInterface->Data[eWindowMessageBox].Height = (float)(80 + (this->MsgLine * 10));

	if (this->MsgBoxType == 0)
	{
		float BtnHeight = 20.0;
		float BtnWidth = 120.0;
		if (gInterface->DrawButton(StartX + CuaSoW / 2 - (BtnWidth / 2), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, gTextClient.txtClient_Khac[13]))
		{
			gInterface->Data[eWindowMessageBox].OnShow = 0;
		}
	}
	else if (this->MsgBoxType == 1)
	{
		float BtnHeight = 20.0;
		float BtnWidth = 55.0;
		if (gInterface->DrawButton(StartX + CuaSoW / 2 - (BtnWidth + 7.5), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, gTextClient.txtClient_Khac[12]))
		{
			this->MsgBoxCallback(this);
			this->MsgBoxCallback = 0;
			gInterface->Data[eWindowMessageBox].OnShow = 0;
		}

		if (gInterface->DrawButton(StartX + CuaSoW / 2 + (7.5), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, gTextClient.txtClient_Khac[13]))
		{
			gInterface->Data[eWindowMessageBox].OnShow = 0;
		}
	}
	DisableAlphaBlend();
	EnableAlphaTest(0);

}

void CB_Interface::OpenMessageBox(char* caption, char* Format, ...)
{
	if (gInterface->Data[eWindowMessageBox].OnShow)
	{
		return;
	}

	char text[1024] = { 0 };
	va_list va;
	va_start(va, Format);
	vsprintf_s(text, Format, va);
	va_end(va);
	this->MsgLine = 0;
	this->MsgBoxCaption.clear();
	this->MsgBoxText.clear();
	this->MsgBoxCaption = caption;
	this->MsgBoxText = text;

	this->MsgLine = 1;
	gInterface->Data[eWindowMessageBox].OnShow = true;
	this->MsgBoxType = 0;
}

void CB_Interface::OpenMessageBoxOkCancel(PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, char* caption, char* Format, ...)
{
	if (gInterface->Data[eWindowMessageBox].OnShow)
	{
		return;
	}

	char text[1024] = { 0 };
	va_list va;
	va_start(va, Format);
	vsprintf_s(text, Format, va);
	va_end(va);

	this->MsgLine = 0;
	this->MsgBoxCaption.clear();
	this->MsgBoxText.clear();
	this->MsgBoxCaption = caption;
	this->MsgBoxText = text;
	this->MsgLine = 1;
	gInterface->Data[eWindowMessageBox].OnShow = true;
	this->MsgBoxType = 1;
	this->MsgBoxCallback = pCallbackFunc;
}


std::string CB_Interface::generateCaptcha(int n)
{
	time_t t;
	srand((unsigned)time(&t));
	char* required_chars = "0123456789";
	std::string captcha = "";
	while (n--)
		captcha.push_back(required_chars[rand() % sizeof(required_chars)]);
	return captcha;
}

bool CB_Interface::check_Captcha(std::string& captcha, std::string& user_input)
{
	return captcha.compare(user_input) == 0;
}
void CB_Interface::RenderCaptchaNumber(float PosX, float PosY, CUITextInputBox* a6, LPCSTR Text, ...)
{
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);


	gInterface->DrawBarForm(PosX, PosY + 3.5, 120, 17, 0.0, 0.0, 0.0, 1.0);
	gInterface->DrawBarForm(PosX + 2, PosY + 4, 50, 15, 1.0, 0.2167, 0.0, 1.0);
	TextDraw((HFONT)g_hFontBold, PosX + 2, PosY + 4 + 2, 0xFFFFFFB8, 0x0, 50, 0, 3, Buff); //Ma Xac Nhan
	a6->SetPosition(PosX + 60, PosY + 6.5);
	a6->Render();

	if (SEASON3B::CheckMouseIn(PosX - 5, PosY - 5, 120, 30) == 1)
	{

		if (GetKeyState(VK_LBUTTON) & 0x8000)
		{
			a6->GiveFocus(1);
			PlayBuffer(25, 0, 0);
		}
	}
	//gCentral.DrawStringInput(PosX + 60, PosY + 5, 50, 13, 3, a6, 0x57575750, 0x0, 0xFFFFFFFF);

}


void* BCacheInfoItemDD;
DWORD CacheTimeShowInfoItem = 0;
bool CB_Interface::ChangeSendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface->Data[eWindowChangeItem].OnShow)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xD3, 0x6B, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)&pMsg, pMsg.h.size);
		return 1;
	}

	return 0;
}
void sfasfDrawInfoItemSet()
{

	if (BCacheInfoItemDD && CacheTimeShowInfoItem > GetTickCount())
	{
		g_pNewItemTooltip->RenderItemTooltip(MouseX + 75, MouseY, (ITEM*)BCacheInfoItemDD, false, 0, false, false);
	}
	else if (BCacheInfoItemDD)
	{
		BCacheInfoItemDD = nullptr;
	}

}
void BackDoiItem(int Type)
{
	if ((GetTickCount() - gInterface->Data[eWindowChangeItem].EventTick) > 300)
	{

		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x6C, sizeof(pMsg));
		pMsg.ThaoTac = Type; //
		DataSend((LPBYTE)&pMsg, pMsg.header.size);
		gInterface->Data[eWindowChangeItem].EventTick = GetTickCount();
	}
}

int ChangeItem = -1;
int ChangeItemPage = 0;
namespace {
	int GetDisplayLineCount(const char* text) {
		if (text == 0 || text[0] == 0) {
			return 1;
		}

		int lineCount = 1;

		for (const char* current = text; *current != 0; current++) {
			if (*current == '\n') {
				lineCount++;
			}
		}

		return lineCount;
	}

	void AppendChangeItemCostPart(std::string& text, const char* label, int value) {
		if (value <= 0) {
			return;
		}

		if (!text.empty()) {
			text += "     ";
		}

		char buffer[32];
		if (value < 1000) {
			sprintf_s(buffer, "%d", value);
		}
		else if (value < 1000000) {
			int part1 = value % 1000;
			int part2 = (value - part1) / 1000;
			sprintf_s(buffer, "%d,%03d", part2, part1);
		}
		else if (value < 1000000000) {
			int part1 = value % 1000;
			int temp = (value - part1) / 1000;
			int part2 = temp % 1000;
			temp = (temp - part2) / 1000;
			int part3 = temp % 1000;
			sprintf_s(buffer, "%d,%03d,%03d", part3, part2, part1);
		}
		else {
			sprintf_s(buffer, "%d", value);
		}

		text += label;
		text += ": ";
		text += buffer;
	}
}

void CB_Interface::DrawChangeItem()
{
	if (gInterface->Data[eWindowChangeItem].OnShow)
	{
		if (this->ChangeItemOpenMap == -1)
		{
			this->ChangeItemOpenMap = gMapManager->currentMap;
			this->ChangeItemOpenX = Hero->PositionX;
			this->ChangeItemOpenY = Hero->PositionY;
		}
		else if (this->ChangeItemOpenMap != gMapManager->currentMap ||
			this->ChangeItemOpenX != Hero->PositionX ||
			this->ChangeItemOpenY != Hero->PositionY)
		{
			gInterface->Data[eWindowChangeItem].OnShow = 0;
		}
	}

	if (!gInterface->Data[eWindowChangeItem].OnShow)
	{
		gInterface->ChangeItemOpenMap = -1;
		gInterface->ChangeItemOpenX = -1;
		gInterface->ChangeItemOpenY = -1;

		if (gInterface->SetStateDoiItem == 1)
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x6A, sizeof(pMsg));
			pMsg.ThaoTac = 111; //
			DataSend((LPBYTE)&pMsg, pMsg.header.size);
			gInterface->SetStateDoiItem = false;
			gInterface->CItem_EffectsMix = false;
			gInterface->CItem_ActiveMix = false;
			gInterface->CostWC = 0;
			gInterface->CostWP = 0;
			gInterface->CostGP = 0;
			memset(gInterface->requiredInfoItem, 0, sizeof(gInterface->requiredInfoItem));
			ChangeItem = -1;
			ChangeItemPage = 0;
		}
		return;
	}
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
	}

	//float StartX = 10;
	//float StartY = 20;

	float StartX = 0;
	float StartY = 0;
	float WindowW = 350;
	int RequiredInfoLineCount = GetDisplayLineCount(this->requiredInfoItem);
	float ExtraInfoHeight = (float)((RequiredInfoLineCount > 1) ? ((RequiredInfoLineCount - 1) * 10) : 0);
	float WindowH = 350 + ExtraInfoHeight;
	int CheckItemOK = 0;

	this->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowChangeItem, gTextClient.txtClient_ChangeItem[0]);



	float BoxItemWH = 35;
	auto renderChangeItemSlot = [](float PosX, float PosY, float Width, float Height, ITEM* lpItem) {
		if (lpItem == 0 || lpItem->Type == 0x1FFF)
		{
			return;
		}

		glColor4f(1.f, 1.f, 1.f, 1.f);
		SEASON3B::RenderLocalItem3D(PosX, PosY, Width, Height, lpItem->Type, lpItem->Level, lpItem->Option1, lpItem->ExtOption, false, 0.004f);
		glColor4f(1.f, 1.f, 1.f, 1.f);
	};
	//===Item Chinh
	float PosXGroup1 = StartX + 20;
	float PosYGroup1 = StartY + 40;
	this->DrawInfoBox(PosXGroup1 - 10, PosYGroup1 - 5, WindowW - 30, 170, 0x00000096, 0, 0);

	TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYGroup1 + 5, 0xEBA000FF, 0x0, 0, 0, 1, gTextClient.txtClient_ChangeItem[1]);//
	//===Box Item Chinh
	this->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup1 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); // Chinh

	ITEM* CTItemChinh = g_pNewItemMng->CreateItem(gInterface->CItem_ItemChinh);
	if (CTItemChinh->Type != 0x1FFF)
	{
		renderChangeItemSlot((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2) + 7), (PosYGroup1 + 20) + 7, BoxItemWH, BoxItemWH, CTItemChinh);//BMD MOdel
		if (SEASON3B::CheckMouseIn(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup1 + 20, 50, 50) == 1)
		{
			BCacheInfoItemDD = CTItemChinh;
			CacheTimeShowInfoItem = GetTickCount() + 50;
			if (GetKeyState(VK_RBUTTON) & 0x8000)
			{
				BackDoiItem(0);
			}
		}
	}
	//pDrawColorButton(71520, PosXGroup1, PosYGroup1 + 75, (WindowW - 40), 1, 0, 0, pMakeColor(158, 158, 158, 130));//-- Divisor

	//===Item Ket Qua
	float PosYGroup2 = PosYGroup1 + 85;
	float ResultRowY = PosYGroup2 + 20;
	float ResultSlotPosX[3] = {
		PosXGroup1 + 50,
		PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2),
		PosXGroup1 + ((WindowW - 40) / 2) + 70
	};

	ITEM* VisibleResultItems[12] = { 0 };
	int VisibleResultBackSlots[12] = { 0 };
	int VisibleResultCount = 0;

	for (int n = 0; n < 12; n++)
	{
		ITEM* pResultItem = g_pNewItemMng->CreateItem(gInterface->CItem_ItemKetQua[n]);

		if (pResultItem->Type != 0x1FFF)
		{
			VisibleResultItems[VisibleResultCount] = pResultItem;
			VisibleResultBackSlots[VisibleResultCount] = (n + 1);
			VisibleResultCount++;
		}
	}

	const int ChangeItemPageSize = 3;
	int ChangeItemPageCount = ((VisibleResultCount + ChangeItemPageSize - 1) / ChangeItemPageSize);

	if (ChangeItemPageCount <= 0)
	{
		ChangeItemPageCount = 1;
	}

	if (ChangeItemPage >= ChangeItemPageCount)
	{
		ChangeItemPage = (ChangeItemPageCount - 1);
	}

	if (ChangeItemPage < 0)
	{
		ChangeItemPage = 0;
	}

	int ResultStartIndex = (ChangeItemPage * ChangeItemPageSize);
	bool SelectedResultVisible = false;

	TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYGroup2, 0xEBA000FF, 0x0, 0, 0, 1, gTextClient.txtClient_ChangeItem[2]);//

	if (ChangeItemPageCount > 1)
	{
		TextDraw((HFONT)g_hFontBold, PosXGroup1 + 215, PosYGroup2, 0xEBA000FF, 0x0, 90, 0, 3, "%d/%d", (ChangeItemPage + 1), ChangeItemPageCount);

		float ArrowLeftX = PosXGroup1 + 10;
		float ArrowButtonY = ResultRowY + 10;
		float ArrowRightX = PosXGroup1 + WindowW - 57;
		bool CanPrevPage = (ChangeItemPage > 0);
		bool CanNextPage = ((ResultStartIndex + ChangeItemPageSize) < VisibleResultCount);

		SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_L, ArrowLeftX, ArrowButtonY, CanPrevPage);
		SEASON3B::RenderlookFetch(CNewUIQuestProgress::IMAGE_QP_BTN_R, ArrowRightX, ArrowButtonY, CanNextPage);

		if (CanPrevPage && SEASON3B::CheckMouseFetch(ArrowLeftX, ArrowButtonY, true) && SEASON3B::IsRelease(VK_LBUTTON))
		{
			ChangeItemPage--;
			ChangeItem = -1;
		}
		else if (CanNextPage && SEASON3B::CheckMouseFetch(ArrowRightX, ArrowButtonY, true) && SEASON3B::IsRelease(VK_LBUTTON))
		{
			ChangeItemPage++;
			ChangeItem = -1;
		}
	}
	for (int n = 0; n < ChangeItemPageSize; n++)
	{
		int ResultIndex = (ResultStartIndex + n);
		int ResultBackSlot = 0;
		ITEM* pResultItem = 0;

		if (ResultIndex < VisibleResultCount)
		{
			ResultBackSlot = VisibleResultBackSlots[ResultIndex];
			pResultItem = VisibleResultItems[ResultIndex];
		}

		this->DrawInfoBox(ResultSlotPosX[n], ResultRowY, BoxItemWH, BoxItemWH, ((ResultBackSlot != 0 && ChangeItem == ResultBackSlot) ? 0x26E0A596 : 0x00000096), 0, 0);

		if (pResultItem != 0)
		{
			renderChangeItemSlot(ResultSlotPosX[n] + 7, ResultRowY + 7, BoxItemWH, BoxItemWH, pResultItem);

			if (SEASON3B::CheckMouseIn(ResultSlotPosX[n], ResultRowY, 50, 50) == 1)
			{
				BCacheInfoItemDD = pResultItem;
				CacheTimeShowInfoItem = GetTickCount() + 50;

				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					ChangeItem = ResultBackSlot;
				}
			}

			if (ChangeItem == ResultBackSlot)
			{
				SelectedResultVisible = true;
			}
		}
	}

	if (ChangeItem > 0 && SelectedResultVisible == false)
	{
		ChangeItem = -1;
	}

	float ResultInfoBoxY = ResultRowY + 60;
	this->DrawInfoBox(PosXGroup1 - 10, ResultInfoBoxY, WindowW - 30, 70 + ExtraInfoHeight, 0x00000096, 0, 0);
	float PosYTextInfo = ResultInfoBoxY + 5;
	TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYTextInfo, 0xEBA000FF, 0x0, 0, 0, 1, gTextClient.txtClient_ChangeItem[3]);//

	float RequiredInfoPosY = PosYTextInfo + 20;
	int RequiredInfoDrawCount = TextDraw((HFONT)g_hFontBold, PosXGroup1, RequiredInfoPosY, 0xEBA000FF, 0x0, WindowW - 50, 0, 1, requiredInfoItem);//

	if (RequiredInfoDrawCount <= 0) {
		RequiredInfoDrawCount = 1;
	}

	std::string costInfoText;
	AppendChangeItemCostPart(costInfoText, "WC", gInterface->CostWC);
	AppendChangeItemCostPart(costInfoText, "WP", gInterface->CostWP);
	AppendChangeItemCostPart(costInfoText, "GP", gInterface->CostGP);

	if (!costInfoText.empty())
	{
		float CostPosY = RequiredInfoPosY + (RequiredInfoDrawCount * 10) + 10;
		TextDraw((HFONT)g_hFontBold, PosXGroup1, CostPosY, 0x62FF00FF, 0x0, WindowW - 50, 0, 1, "%s", costInfoText.c_str());//
	}

	float ButtonW = 115;
	float ButtonH = 25;
	float ButtonX = PosXGroup1 + ((WindowW - 40) / 2) - (ButtonW / 2);
	float ButtonY = (StartY + WindowH) - 20 - ButtonH;
	//if (gElemental.gDrawButton(ButtonX, ButtonY, ButtonW, 12, "Dung Luy?n")) //Dung Luyen

	if (this->DrawButton(ButtonX, ButtonY, ButtonW, 12, gTextClient.txtClient_ChangeItem[9]))
	{
		if (ChangeItem > 0)
		{
			PlayBuffer(25, 0, 0);
			BackDoiItem(ChangeItem);
			ChangeItem = -1;
			gInterface->Data[eWindowChangeItem].OnShow ^= 1;
		}
	}

	if (SEASON3B::CheckMouseIn(StartX, StartY, WindowW, WindowH) == 1)
	{
		this->SetBlockCur(TRUE);
		MouseLButton = false;
		MouseLButtonPush = false;
		MouseLButtonPop = false;
		MouseRButton = false;
		MouseRButtonPush = false;
		MouseRButtonPop = false;
		MouseMButton = false;
		MouseMButtonPush = false;
		MouseMButtonPop = false;
	}

	sfasfDrawInfoItemSet();
}

