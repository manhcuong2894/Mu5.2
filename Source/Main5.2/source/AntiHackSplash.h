#pragma once

class CAntiHackSplash
{
public:
	CAntiHackSplash();
	~CAntiHackSplash();

	void Show(HINSTANCE hInstance);
	void Hide();
	void HideAfterMinimum(DWORD minimumMs);
	void PumpMessages();

private:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Draw(HDC hdc);
	bool LoadBitmapFile(const char* path);
	void ReleaseBitmap();

	HWND m_hWnd;
	HBITMAP m_hBitmap;
	int m_Width;
	int m_Height;
	DWORD m_ShowTick;
};

extern CAntiHackSplash gAntiHackSplash;
