#include "stdafx.h"
#include "Window.h"
HWND hDummyWnd;
HWND hConsoleWnd;
bool hidden;
NOTIFYICONDATA m_TrayData;
HHOOK hhk=NULL;
HWND GetMainHwnd(DWORD prcId,DWORD &outThreadId)
{
	HWND h = ::GetTopWindow(0);
	while ( h )
	{
		DWORD pid;
		DWORD dwThreadId = ::GetWindowThreadProcessId( h,&pid);
		if ( (pid == prcId )&& (GetParent(h)==NULL)&&(h!=hConsoleWnd))
		{
			outThreadId=dwThreadId;
			return h;
		}
		h = ::GetNextWindow( h , GW_HWNDNEXT);
	}
	return NULL;
}

LRESULT CALLBACK Wc3KeyboardHook(int code,WPARAM wParam,LPARAM lParam)
{  
	if(code==HC_ACTION)
	{
		switch(wParam)
		{
			case VK_F5:
			{
				if(!LastToggle)
				{
					EnableBasicMH(!MHEnabled);
					LastToggle=clock();
				}
				else
				{
					clock_t now = clock();
					if((now-LastToggle)>1000)
					{
						EnableBasicMH(!MHEnabled);
						LastToggle=clock();
					}
				}
			}break;		
		}
	}
	CallNextHookEx(hhk,code,wParam,lParam);
	return 0;
}
LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg,WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_TRAY_MESSAGE:
			OnTrayNotify(wParam,lParam);
			break;
		case WM_CLOSE:
			DestroyWindow(hDummyWnd);
			UnregisterClass(WndClsEx.lpszClassName,WndClsEx.hInstance);
		// If the user wants to close the application
		/*case WM_KEYUP:
		case WM_KEYDOWN:
			printf("LOWORD PARAM %d",LOWORD(wParam));
			switch (LOWORD(wParam)) 
			{
				case ID_ACCELERATOR40004:
				{
					

				}break;

			}*/

		default:
			// Process the left-over messages
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	// If something was not done, let it go
	return 0;

}
DWORD WINAPI MessageThread(LPVOID lp)
{
	ZeroMemory(&LastToggle,sizeof(clock_t));
	LPCTSTR ClsName = "kMapwindow";/////////
	LPSTR WndName = "kMap";///////
	HWND       hWnd;
	HINSTANCE hInstance = (HINSTANCE)lp;
	// Create the application window
	WndClsEx.cbSize        = sizeof(WNDCLASSEX);
	WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
	WndClsEx.lpfnWndProc   = WndProcedure;
	WndClsEx.cbClsExtra    = 0;
	WndClsEx.cbWndExtra    = 0;
	WndClsEx.hIcon         = NULL;
	WndClsEx.hCursor       = NULL;
	WndClsEx.hbrBackground = NULL;/////
	WndClsEx.lpszMenuName  = NULL;
	WndClsEx.lpszClassName = ClsName;
	WndClsEx.hInstance     = hInstance;
	WndClsEx.hIconSm       = NULL;
	// Register the application
	RegisterClassEx(&WndClsEx);
	// Create the window object
	hWnd = CreateWindow(ClsName,
		WndName,
		0,
		0,
		0,
		0,
		0,
		HWND_MESSAGE,
		NULL,
		hInstance,
		NULL);
	if( !hWnd ) 
	{
		printf("Couldnt create my window ;( getlasterror %d\n",GetLastError());
		return 0;
	}
	hDummyWnd=hWnd;
	Minimize2Tray(hInstance,hWnd);
	DWORD MainThId;
	HWND hWc3Wnd = GetMainHwnd(GetCurrentProcessId(),MainThId);
	if(!(hhk=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)Wc3KeyboardHook,NULL,MainThId)))
	{
		printf("Couldnt hook keyboard, getlasterror :%d, toggling wont work\n",GetLastError());
	}
	/*HACCEL hAccel =LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if(hAccel==NULL)
		MessageBox(NULL,"SCREAM","ASDF",MB_OK);
	HWND wnd= FindWindow(NULL,"Warcraft III");
	DWORD wc3Id = GetWindowThreadProcessId(wnd,NULL);
	cout<<wc3Id;
	DWORD consoleThread = GetWindowThreadProcessId(hConsoleWnd,NULL);
	cout<<consoleThread;
	AttachThreadInput(GetCurrentThreadId(),consoleThread,TRUE);
	AttachThreadInput(GetCurrentThreadId(),wc3Id,TRUE);*/
	while(run)
	{
		MSG msg={0};
		BOOL MsgReturn = GetMessage(&msg,NULL,0,0);
		if ( MsgReturn )
		{
			/*if(!TranslateAccelerator(hDummyWnd,hAccel,&msg))
			{*/
				//char buf[10];sprintf(buf,"%d",GetLastError());
		    	//MessageBox(NULL,buf,"ASDF",MB_OK);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			/*}
			else
			{
			}*/
		}
	}
	return TRUE;
}
LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	POINT pt; 
	switch (lParam)
	{
		case WM_LBUTTONUP:
		{
			GetCursorPos(&pt);
			ClientToScreen(hConsoleWnd,&pt);
			OnTrayLButtonDown(pt);
		}break;
		case WM_RBUTTONUP:
		{
			GetCursorPos(&pt);
			ClientToScreen(hConsoleWnd,&pt);
			OnTrayRButtonDown(pt);
		}break;
	} 
	return 0; 
}
bool GetTrayRect(LPRECT rect)
{
	HWND hShellTrayWnd=FindWindowEx(NULL,NULL,TEXT("Shell_TrayWnd"),NULL);
	if(hShellTrayWnd)
	{
		HWND hTrayNotifyWnd=FindWindowEx(hShellTrayWnd,NULL,TEXT("TrayNotifyWnd"),NULL);
		if(hTrayNotifyWnd)
		{
			GetWindowRect(hTrayNotifyWnd,rect);
			return true;
		}
	}

	/*APPBARDATA appBarData;
	appBarData.cbSize=sizeof(appBarData);
	if(SHAppBarMessage(ABM_GETTASKBARPOS,&appBarData))
	{
		switch(appBarData.uEdge)
		{
			case ABE_LEFT:
			case ABE_RIGHT:
			rect->top=appBarData.rc.bottom-100;
			rect->bottom=appBarData.rc.bottom-16;
			rect->left=appBarData.rc.left;
			rect->right=appBarData.rc.right;
			break;
			case ABE_TOP:
			case ABE_BOTTOM:
			rect->top=appBarData.rc.top;
			rect->bottom=appBarData.rc.bottom;
			rect->left=appBarData.rc.right-100;
			rect->right=appBarData.rc.right-16;
			break;
		}
		return true;
	}*/
	return false;
}
void OnTrayLButtonDown(POINT pt)
{
	RECT rcFrom,rcTo;
	if(hidden)
	{
		if(GetTrayRect(&rcFrom))
		{
			GetCursorPos(&pt);
			GetWindowRect(hConsoleWnd,&rcTo);
			DrawAnimatedRects(hConsoleWnd,IDANI_CAPTION,&rcFrom,&rcTo);
		}
		ShowWindow(hConsoleWnd,SW_SHOW);
		SetForegroundWindow(hConsoleWnd);
		SetFocus(hConsoleWnd);
		hidden = false;
	}
	else
	{
		if(GetTrayRect(&rcTo))
		{
			GetWindowRect(hConsoleWnd,&rcFrom);
			DrawAnimatedRects(hConsoleWnd,IDANI_CAPTION,&rcFrom,&rcTo);
		}
		ShowWindow(hConsoleWnd,SW_HIDE);
		hidden = true;
	}
}
void OnTrayRButtonDown(POINT pt)
{
	GetCursorPos(&pt);
	HMENU popUp=LoadMenu(ths,MAKEINTRESOURCE(IDR_MENU1));
	popUp=GetSubMenu(popUp,0);
	if(popUp==NULL)
		printf("CANT LOAD MENU ASDASF HAX :(\n");
	SetForegroundWindow(hDummyWnd);
	int sel = TrackPopupMenuEx(popUp,TPM_CENTERALIGN|TPM_RETURNCMD,pt.x,pt.y,hDummyWnd,NULL);
	PostMessage(hDummyWnd, WM_NULL, 0, 0);
	switch(sel)
	{
		case ID_CONTEXTMENU_OFF:
			EnableBasicMH(false);
			break;
		case ID_CONTEXTMENU_ON:
			EnableBasicMH(true);
			break;
		case ID_CONTEXTMENU_UNLOAD:
			Unhook();
			Sleep(10);
			SetEvent(unload);
			break;
	}
}
void Minimize2Tray(HINSTANCE inst,HWND wnd)
{
	HICON hMyIcon = LoadIcon(inst, MAKEINTRESOURCE(IDI_TRAYICON));
	memset(&m_TrayData, 0, sizeof(m_TrayData));
	m_TrayData.cbSize = sizeof(NOTIFYICONDATA);
	m_TrayData.hWnd  = wnd;
	m_TrayData.uID = 666;
	m_TrayData.uCallbackMessage  = WM_TRAY_MESSAGE;
	m_TrayData.hIcon = hMyIcon;
	sprintf(m_TrayData.szTip,"kMap v \\1.24c/ by kolkoo is currently [%s] ",MHEnabled?"ON":"OFF");
	//strcpy(m_TrayData.szInfo,"kMap has been minimized.");
	m_TrayData.uFlags = NIF_ICON|NIF_TIP|NIF_MESSAGE;
	m_TrayData.dwInfoFlags = NIIF_USER|NIIF_NOSOUND;
	BOOL bSuccess = FALSE;
	bSuccess = Shell_NotifyIcon(NIM_DELETE,&m_TrayData);
	bSuccess = Shell_NotifyIcon(NIM_ADD,&m_TrayData);
	hidden=false;
	if(!(bSuccess))
	{
		printf("ohcrap\n %d ",GetLastError());
	}
}