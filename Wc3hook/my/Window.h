#ifndef WINDOW_H
#define WINDOW_H
#include "Globals.h"
#define WM_TRAY_MESSAGE WM_USER+666
extern HWND hDummyWnd;
extern HWND hConsoleWnd;
extern bool hidden;
extern HHOOK hhk;
LRESULT CALLBACK wireKeyboardProc(int code,WPARAM wParam,LPARAM lParam);
extern NOTIFYICONDATA m_TrayData;
LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg,WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Wc3KeyboardHook(int code,WPARAM wParam,LPARAM lParam);
DWORD WINAPI MessageThread(LPVOID lp);
LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
void OnTrayLButtonDown(POINT pt);
void OnTrayRButtonDown(POINT pt);
void Minimize2Tray(HINSTANCE inst,HWND wnd);
bool GetTrayRect(LPRECT rect);
HWND GetMainHwnd(DWORD prcId,DWORD &outThreadId);
extern clock_t LastToggle;
extern 	WNDCLASSEX WndClsEx;
#endif