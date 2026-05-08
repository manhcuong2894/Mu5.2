#pragma once

#define MAX_LOADSTRING 100

#define WM_TIMER_1000 100
#define WM_TIMER_2000 101
#define WM_TIMER_10000 102

#define WM_JOIN_SERVER_MSG_PROC (WM_USER+1)
#define WM_DATA_SERVER_MSG_PROC (WM_USER+2)
#define WM_XSHIELD_SERVER_MSG_PROC (WM_USER+3)

#if(ANTIHACK_GGNEW)
#define WM_TIMER_VIEWHACKLOG 103
#endif
#if(ANTIHACK_GGNEW)
LRESULT CALLBACK ViewLogHack(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance,int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK UserOnline(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
HWND hWndComboBox;
