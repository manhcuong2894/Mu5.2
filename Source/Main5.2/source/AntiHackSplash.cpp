#include "stdafx.h"
#include "AntiHackSplash.h"

namespace
{
	const char* SPLASH_CLASS_NAME = "MuAntiHackSplashWindow";
	const char* SPLASH_IMAGE_PATH = ".\\Data\\Local\\AntihackSplash.bmp";

	void DrawCenteredText(HDC hdc, const char* text, RECT rect, HFONT font, COLORREF color)
	{
		HGDIOBJ oldFont = SelectObject(hdc, font);
		SetTextColor(hdc, color);
		SetBkMode(hdc, TRANSPARENT);
		DrawTextA(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SelectObject(hdc, oldFont);
	}
}

CAntiHackSplash gAntiHackSplash;

CAntiHackSplash::CAntiHackSplash()
{
	this->m_hWnd = 0;
	this->m_hBitmap = 0;
	this->m_Width = 450;
	this->m_Height = 270;
	this->m_ShowTick = 0;
}

CAntiHackSplash::~CAntiHackSplash()
{
	this->Hide();
}

void CAntiHackSplash::Show(HINSTANCE hInstance)
{
	if (this->m_hWnd != 0)
	{
		return;
	}

	this->LoadBitmapFile(SPLASH_IMAGE_PATH);

	WNDCLASSA wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = CAntiHackSplash::WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = SPLASH_CLASS_NAME;

	RegisterClassA(&wc);

	int x = (GetSystemMetrics(SM_CXSCREEN) - this->m_Width) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - this->m_Height) / 2;

	this->m_hWnd = CreateWindowExA(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		SPLASH_CLASS_NAME,
		"",
		WS_POPUP,
		x,
		y,
		this->m_Width,
		this->m_Height,
		0,
		0,
		hInstance,
		this);

	if (this->m_hWnd == 0)
	{
		this->ReleaseBitmap();
		return;
	}

	ShowWindow(this->m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->m_hWnd);
	this->m_ShowTick = GetTickCount();
	this->PumpMessages();
}

void CAntiHackSplash::Hide()
{
	if (this->m_hWnd != 0)
	{
		DestroyWindow(this->m_hWnd);
		this->m_hWnd = 0;
	}

	this->ReleaseBitmap();
	this->m_ShowTick = 0;
}

void CAntiHackSplash::HideAfterMinimum(DWORD minimumMs)
{
	if (this->m_hWnd == 0)
	{
		return;
	}

	DWORD elapsed = GetTickCount() - this->m_ShowTick;

	while (elapsed < minimumMs)
	{
		this->PumpMessages();
		Sleep(10);
		elapsed = GetTickCount() - this->m_ShowTick;
	}

	this->Hide();
}

void CAntiHackSplash::PumpMessages()
{
	if (this->m_hWnd == 0)
	{
		return;
	}

	MSG msg;

	while (PeekMessage(&msg, this->m_hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK CAntiHackSplash::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CAntiHackSplash* splash = (CAntiHackSplash*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (message == WM_NCCREATE)
	{
		CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
		splash = (CAntiHackSplash*)createStruct->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)splash);
	}

	if (splash == 0)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		splash->Draw(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void CAntiHackSplash::Draw(HDC hdc)
{
	RECT rect = { 0, 0, this->m_Width, this->m_Height };

	if (this->m_hBitmap != 0)
	{
		HDC memDc = CreateCompatibleDC(hdc);
		HGDIOBJ oldBitmap = SelectObject(memDc, this->m_hBitmap);
		BitBlt(hdc, 0, 0, this->m_Width, this->m_Height, memDc, 0, 0, SRCCOPY);
		SelectObject(memDc, oldBitmap);
		DeleteDC(memDc);
		return;
	}

	HBRUSH background = CreateSolidBrush(RGB(2, 18, 34));
	FillRect(hdc, &rect, background);
	DeleteObject(background);

	HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(0, 220, 255));
	HGDIOBJ oldPen = SelectObject(hdc, borderPen);
	HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, 42, 78, this->m_Width - 42, 154);
	SelectObject(hdc, oldBrush);
	SelectObject(hdc, oldPen);
	DeleteObject(borderPen);

	HFONT titleFont = CreateFontA(54, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	HFONT smallFont = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	RECT titleRect = { 0, 82, this->m_Width, 150 };
	DrawCenteredText(hdc, "ANTIHACK", titleRect, titleFont, RGB(255, 255, 255));

	RECT statusRect = { 0, 158, this->m_Width, 184 };
	DrawCenteredText(hdc, "CONNECTING TO SERVER PRIVATE...", statusRect, smallFont, RGB(0, 230, 255));

	RECT topRect = { 0, 24, this->m_Width - 18, 44 };
	SetTextColor(hdc, RGB(0, 230, 255));
	SetBkMode(hdc, TRANSPARENT);
	HGDIOBJ oldFont = SelectObject(hdc, smallFont);
	DrawTextA(hdc, "PROFESSIONAL SERVER", -1, &topRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, oldFont);

	DeleteObject(titleFont);
	DeleteObject(smallFont);
}

bool CAntiHackSplash::LoadBitmapFile(const char* path)
{
	this->ReleaseBitmap();

	this->m_hBitmap = (HBITMAP)LoadImageA(0, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (this->m_hBitmap == 0)
	{
		this->m_Width = 450;
		this->m_Height = 270;
		return false;
	}

	BITMAP bitmap;
	GetObject(this->m_hBitmap, sizeof(bitmap), &bitmap);
	this->m_Width = bitmap.bmWidth;
	this->m_Height = bitmap.bmHeight;
	return true;
}

void CAntiHackSplash::ReleaseBitmap()
{
	if (this->m_hBitmap != 0)
	{
		DeleteObject(this->m_hBitmap);
		this->m_hBitmap = 0;
	}
}
