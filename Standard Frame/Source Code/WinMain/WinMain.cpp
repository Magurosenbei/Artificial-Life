// Standard Frame.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WinMain.h"

// Globals
bool KeyBoard[256];
bool Running = false;
static bool bGoFullscreen = false;
static unsigned int nRenderThreadID = 0;
static HANDLE hRenderThread = NULL;

// local prototypes
static LRESULT CALLBACK WndProc			(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static bool          GoFullscreen		(HWND hWnd, unsigned int nWidth, unsigned int nHeight, BYTE nBits, BYTE nRefresh);
static void          GoWindowed			(void);
static bool          ProcStartOptions	(PRENDERARGS pArgs, PRECT pWndRect, HANDLE *pMutex);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND		hWnd = NULL;		// handle to the main window
	HDC			hDC = NULL;			// handle to the window's device context
	RECT		rcWndPos = {0};		// position to use when creating the main app window
	HANDLE		hMutex = NULL;		// handle to the single instance mutex
	MSG			msg = {0};			// message struct for the queue
	RENDERARGS	Args;				// arugements to be passed to the render thread

	Args.bFullscreen = bGoFullscreen = false;
	if(!ProcStartOptions(&Args, &rcWndPos, &hMutex))
		return static_cast<int>(msg.wParam); 
	WNDCLASS wc = {0};			// window's class struct
	DWORD dwWindowStyle = 0;	// style bits to use when creating the main app window
	HBRUSH hBrush = NULL;		// will contain the background color of the main window

	// set the background color to black (this may be changed to whatever is needed)
	hBrush = CreateSolidBrush(RGB(0, 0, 0));
	
	// we must specify the attribs for the window's class
	wc.style			= CS_HREDRAW|CS_VREDRAW|CS_OWNDC;	// style bits (CS_OWNDC very important for OGL)
	wc.lpfnWndProc		= (WNDPROC)WndProc;					// window procedure to use
	wc.cbClsExtra		= 0;								// no extra data
	wc.cbWndExtra		= 0;								// no extra data
	wc.hInstance		= hInstance;						// associated instance
	wc.hIcon			= NULL;								// use default icon temporarily
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);		// use default cursor (Windows owns it, so don't unload)
	wc.hbrBackground	= hBrush;							// background brush (don't destroy, let windows handle it)
	wc.lpszClassName	= APP_CLASSNAME;					// class name

	// this will create or not create a menu for the main window (depends on app settings)
	wc.lpszMenuName = (APP_ALLOW_MENU) ? MAKEINTRESOURCE(IDR_MAINFRAME) : NULL;

	// now, we can register this class
	RegisterClass(&wc);

	// here we determine the correct style bits to use for the man window (depends on settings)
	if(bGoFullscreen)
		dwWindowStyle = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_POPUP|WS_CLIPCHILDREN;
	else
		dwWindowStyle = (!APP_ALLOW_RESIZE) ? WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPED | WS_SYSMENU |WS_CLIPCHILDREN : WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPED | WS_SYSMENU | WS_CLIPCHILDREN;
	
	hWnd =  CreateWindowEx(	WS_EX_APPWINDOW,	
							APP_CLASSNAME,						// class to use
							APP_NAME,							// title for window
							dwWindowStyle,						// style bits
							rcWndPos.left, rcWndPos.top,		// position (x, y)
							rcWndPos.right, rcWndPos.bottom,	// width/height
							NULL,								// no parent
							NULL,								// no menu
							hInstance,							// associated instance
							NULL);	

	if(hWnd == NULL)
	{
		MessageBox(NULL, "APP Class Name might be used", "Window Failed", MB_OK|MB_ICONERROR);
		if(hBrush != NULL) 
			DeleteObject(hBrush);
		return static_cast<int>(msg.wParam);
	}
	PIXELFORMATDESCRIPTOR pfd={0};	// pixel descriptor struct
	int nFormat = 0;				// index of the closest matching pixel format provided by the system
	HICON hIcon = NULL;				// handle to the main window's large icon
	HICON hIconSmall = NULL;		// handle to the main window's small icon

	if(bGoFullscreen)				// attempt to enter fullscreen mode
		if(!GoFullscreen(hWnd, rcWndPos.right, rcWndPos.bottom, Args.nBPP, Args.nRefresh))
		{
			MessageBox(NULL, "Unable to Use FullScreen", "FullScreen Failed", MB_OK|MB_ICONERROR);
			return false;
		}
	if(!Args.bZoomed && ((rcWndPos.left == 0) && (rcWndPos.top == 0))) 
		MoveWnd(hWnd, MV_CENTER|MV_MIDDLE, NULL);

	hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

	hIconSmall = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	// Use our custom icons for the window instead of the Windows'
	// default icons on the titlebar and when using Alt+Tab key
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

	// Now set-up the display settings and the render context
	// to use with the GL.  We then enter the traditional message loop
	// for the window.  Afterwards, we need to perform some clean-up.
		
	
	hDC = GetDC(hWnd);	// get the window's device context

	// set the pixel format for the DC
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = Args.nBPP;		// BPP is ignored for windowed mode
	pfd.cDepthBits = 16;			// 16-bit z-buffer
	pfd.iLayerType = PFD_MAIN_PLANE;

	// attempt to get a pixel format we'll use for the DC via the RC
	nFormat = ChoosePixelFormat(hDC, &pfd);

	if(nFormat == 0)
		MsgBox(hWnd, IDS_ERR_PIXFORMAT, MB_OK|MB_ICONERROR);
	else
	{
		DWORD dwCode = 0;
		
		SetPixelFormat(hDC, nFormat, &pfd);	// set the dc to the format we want
		Args.hWnd = hWnd;					// set params to send that haven't been set yet
		Args.hDC = hDC;

		//long StartPt, EndPt;
		#if defined(DEBUG) | defined(_DEBUG)
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		#endif

		srand(GetTickCount());
		g_Engine->BootUp();
		// initialize the scene in a separate thread (do not use CreateThread()
		// to avoid leaks caused by the CRT when trying to use standard CRT libs)
		InitializeCriticalSection(&_critSection);
		hRenderThread = (HANDLE)_beginthreadex(NULL, 0, InitScene, &Args, 0, &nRenderThreadID);
		Running = true;
		while(GetMessage(&msg, NULL, 0, 0) != 0 && Running)
		{	
			// send the message off to the appropriate window procedure, as a security
			// precaution only do so if it belongs to the the main window in question
			if(msg.hwnd != hWnd) continue;
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(500);
		if(hRenderThread) CloseHandle(hRenderThread);
	}
	ReleaseDC(hWnd, hDC);

	if(bGoFullscreen) 
		GoWindowed();
	if(hMutex)		CloseHandle(hMutex);
	if(hIcon)		DestroyIcon(hIcon);
	if(hIconSmall)	DestroyIcon(hIconSmall);
	if(hBrush)		DeleteObject(hBrush);

	g_Engine->BootDown();
	delete g_Engine;
	return static_cast<int>(msg.wParam);
}

static bool ProcStartOptions(PRENDERARGS pArgs, PRECT pWndRect, HANDLE *pMutex)
{
	bool bReturn = true;
	TCHAR szBuff[MAX_LOADSTRING] = {0};

	// first and foremost, make sure the host os meets our requirements
	// depending on config, check for version and nt only or both nt and 9x (no check for only 9x)
	if(!WIN_MIN_VERSION(APP_MIN_WINVER_MAJOR, APP_MIN_WINVER_MINOR) || (APP_WINVER_NTONLY && !IS_WINNT))
	{
		MsgBox(NULL, IDS_ERR_WINVER, MB_OK|MB_ICONERROR);
		bReturn = false;
	}
	else
	{
		if(APP_SINGLE_INSTANCE && (pMutex != NULL))
		{
			*pMutex = CreateMutex(NULL, true, APP_CLASSNAME);

			// limit this app to a single instance only (make sure classname is unique)
			// NOTE: using a mutex is MUCH safer than using FindWindow() alone, if the
			// exe gets executed back-to-back - as in a script - mutexes still work
			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				HWND hPrevious = NULL;

				// set focus to the other main window (if possible, may not be created yet) then exit
				// if the class name is unique we don't worry about searching against the title
				if((hPrevious = FindWindow(APP_CLASSNAME, NULL)) != NULL)
					ShowWindow(hPrevious, SW_SHOWNORMAL);

				bReturn = false;
			}
		}

		// don't bother processing the rest if the app is just going to be shutdown
		if(bReturn && (pArgs != NULL) && (pWndRect != NULL))
		{
			DWORD dwTemp = 0; // used to pull dword values from the registry

			/*/
			/ / Here, we need to determine if this app allows fullscreen mode. If so, do we default
			/ / to it or not? If not, then process nothing and stay windowed. If it is allowed, then
			/ / we need to specify the fullscreen cmd arg and set it's value to yes or no (default yes).
			/*/
			if(APP_ALLOW_FULLSCREEN)
			{
				// read the cmd ling arg to see what we should do about fullscreen
				if(GetCmdLineValue(_T("fullscreen"), szBuff, REAL_SIZE(szBuff)))
				{
					// any value other than "no" (including no value) is considered a yes
					if(!MATCH(szBuff, _T("no"))) bGoFullscreen = true;
				}
				else
				{
					// command line arg not found, but we need to check if we default to fullscreen
					if(APP_DEF_FULLSCREEN) bGoFullscreen = true;
				}

				// set the param that will eventually be sent to the render thread
				pArgs->bFullscreen = bGoFullscreen;
			}

			// get the bits per pixel data (if any) from the registry, can only be 8, 16, 24, or 32
			dwTemp = 0;
			if(!GetUserValue(NULL, _T("BPP"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
				pArgs->nBPP = APP_DEF_BPP;
			else
			{
				// to be valid, it must be a mulitple of 8 in the range of 8-32, otherwise use the default
				if(dwTemp < 8 || dwTemp > 32) dwTemp = APP_DEF_BPP;
				if((dwTemp % 8) != 0) dwTemp = APP_DEF_BPP;

				// finally set the BPP value
				pArgs->nBPP = (BYTE)dwTemp;
			}

			// get the vertical refresh rate data (if any) from the registry, must be between min and max
			dwTemp = 0;
			if(!GetUserValue(NULL, _T("Refresh"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
				pArgs->nRefresh = APP_MIN_REFRESH;
			else
			{
				// to be valid, it must be between the min and max, otherwise use the default
				if(dwTemp > APP_MAX_REFRESH)
					pArgs->nRefresh = APP_MAX_REFRESH;

				else if(dwTemp < APP_MIN_REFRESH)
					pArgs->nRefresh = APP_MIN_REFRESH;

				else
					pArgs->nRefresh = (BYTE)dwTemp;
			}

			/*/
			/ / Here, we need to determine if we need to set the vertical on or off. It's important to
			/ / note, on nVidia cards, this can be overriden so the app is not able to adjust this.
			/ / If it is not allowed, the whatever setting/default set by the video card will take effect.
			/*/
			if(APP_ALLOW_VSYNC)
			{
				// assume vsync is false if no setting is found
				dwTemp = 0;
				if(!GetUserValue(NULL, _T("VSync"), REG_DWORD, &dwTemp, sizeof(dwTemp)))

					// if no option is set, then default
					pArgs->Status = VSync_Default;
				else
					// ensure we are working with proper boolean data
					pArgs->Status = (dwTemp == 0) ? VSync_Off : VSync_On;
			}
			else
				// not allowed, so default to leaving vsync alone
				pArgs->Status = VSync_Default;

			/*/
			/ / At this point we need to see if the registry contains data regarding the main window pos.
			/ / Of course, we only care about this if not in fullscreen mode, but check now to reduce flicker.
			/*/

			// independently test left/top (if not in registry then we center the window later on)
			// if we are in fullscreen mode, always leave these set to zero
			if(!bGoFullscreen)
			{
				GetUserValue(NULL, _T("Left"), REG_DWORD, &pWndRect->left, sizeof(LONG));
				GetUserValue(NULL, _T("Top"), REG_DWORD, &pWndRect->top, sizeof(LONG));
			}

			// just in case the data was corrupted, perform a bit of checking (no negative values are allowed)
			pWndRect->left = (unsigned long)pWndRect->left;
			pWndRect->top = (unsigned long)pWndRect->top;

			/*/
			/ / IMPORTANT: if we do not allow the user to resize then never, ever use the values from the registry as this
			/ / will create an exploit, so in this case we always use the defaults and allow the programmer to adjust them
			/ / as needed, also if the app will only run in fullscreen mode it should always turn APP_ALLOW_RESIZE off
			/*/
			if(APP_ALLOW_RESIZE)
			{
				// if this data doesn't exist or is bogus, we just revert back to the default width/height
				GetUserValue(NULL, _T("Width"), REG_DWORD, &pWndRect->right, sizeof(LONG));
				GetUserValue(NULL, _T("Height"), REG_DWORD, &pWndRect->bottom, sizeof(LONG));

				pWndRect->right = APP_DEF_WIDTH;	//(pWndRect->right <= 0) ? APP_DEF_WIDTH : pWndRect->right;
				pWndRect->bottom = APP_DEF_HEIGHT;	//(pWndRect->bottom <= 0) ? APP_DEF_HEIGHT : pWndRect->bottom;

				if(!bGoFullscreen)
				{
					// if we are in windowed mode, get the maximized state so we can restore it if needed
					dwTemp = 0;
					if(!GetUserValue(NULL, _T("Zoom"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
						pArgs->bZoomed = false;
					else
						// ensure we are working with proper boolean data
						pArgs->bZoomed = (dwTemp == 0) ? false : true;
				}
			}
			else
			{
				pWndRect->right = APP_DEF_WIDTH;
				pWndRect->bottom = APP_DEF_HEIGHT;
			}
		}
	}
	return bReturn;
}

static bool GoFullscreen(HWND hWnd, unsigned int nWidth, unsigned int nHeight, BYTE nBits, BYTE nRefresh)
{
	DEVMODE dm = {0};	// device mode structure

	dm.dmSize       = sizeof(DEVMODE);
	dm.dmPelsWidth	= nWidth;									// screen width
	dm.dmPelsHeight	= nHeight;									// screen height
	dm.dmBitsPerPel	= nBits;									// display bits per pixel (BPP)
	dm.dmFields     = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;	// flags

	// as a safey precaution, double check to make sure the refresh rate is in range
	if((nRefresh >= APP_MIN_REFRESH) && (nRefresh <= APP_MAX_REFRESH))
		dm.dmDisplayFrequency = nRefresh;
	// attempt to move to fullscreen
	if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		return false;
	else
	{
		if(GetMenu(hWnd) == NULL) 
			ShowCursor(false);		// hide the cursor (if in fullscreen with no menu)
		return true;
	}
}

static void GoWindowed(void)
{
	ChangeDisplaySettings(NULL, 0);	// switch back to the desktop
	ShowCursor(true);				// show the cursor again
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lReturn = false;
	switch(uMsg)
	{
	case WM_CREATE:
			break;
	case UWM_SHOW:
		
			// the render thread will send this message when its inits are done
			// wParam will contain true if we are to maximize the window
			if((bool)wParam == true)
			{
				// if the user left the window maxed on close, then restore the state
				// note: must be done this way b/c when the window style is set to
				// overlapped window, Windows ignores the maxed style bit
				ShowWindow(hWnd, SW_MAXIMIZE);
			}
			else
				ShowWindow(hWnd, SW_SHOW);

			SetForegroundWindow(hWnd);	// give a slightly higher priority and set keyboard focus
			SetFocus(hWnd);
			return true;	// let the caller know this message was processed

	case WM_CLOSE:
			if(!bGoFullscreen && !IsIconic(hWnd))
			{
				RECT rcWndPos = {0}; // contains the pos and size of the window

				GetWindowRect(hWnd, &rcWndPos);

				// save width/height data if allowed to resize
				if(APP_ALLOW_RESIZE)
				{
					bool bZoomed = IsZoomed(hWnd);

					// save position data (even if maximized as it will tell us which monitor to maximize on)
					SetUserValue(NULL, _T("Left"), REG_DWORD, &rcWndPos.left, sizeof(LONG));
					SetUserValue(NULL, _T("Top"), REG_DWORD, &rcWndPos.top, sizeof(LONG));

					// only save size data if the app is not maximized
					if(!bZoomed)
					{
						long lWidth = 0, lHeight = 0;

						lWidth = rcWndPos.right - rcWndPos.left;
						lHeight = rcWndPos.bottom - rcWndPos.top;

						SetUserValue(NULL, _T("Width"), REG_DWORD, &lWidth, sizeof(lWidth));
						SetUserValue(NULL, _T("Height"), REG_DWORD, &lHeight, sizeof(lHeight));
					}
					// if we are in windowed mode, save the maxed state to restore later
					SetUserValue(NULL, _T("Zoom"), REG_DWORD, &bZoomed, sizeof(bZoomed));
				}
				else
				{
					// save position data
					SetUserValue(NULL, _T("Left"), REG_DWORD, &rcWndPos.left, sizeof(LONG));
					SetUserValue(NULL, _T("Top"), REG_DWORD, &rcWndPos.top, sizeof(LONG));
				}
			}

			// we need to shutdown the render thread, so we signal it to stop
			// to play nice, let it finish before proceeding (up to 5 seconds)
			if((hRenderThread) != NULL && (nRenderThreadID > 0))
			{
				PostThreadMessage(nRenderThreadID, UWM_STOP, 0, 0);
				WaitForSingleObject(hRenderThread, 5000);
			}

			// destroy this window and thus the app as requsted
			DestroyWindow(hWnd);
			break;

	case WM_DESTROY:
			PostQuitMessage(true);
			break;

	case WM_ENTERMENULOOP:
			// NOTE: always leave this whether or not the app has a menu! (remember the sys menu)
			// suspend the render thread if the user is navigating menus (if not minimized)
			if(!IsIconic(hWnd)) PostThreadMessage(nRenderThreadID, UWM_PAUSE, true, 0);
			break;
	case WM_EXITMENULOOP:
			// NOTE: always leave this whether or not the app has a menu! (remember the sys menu)
			// start the render thread back up if the user leaves the menu (if not minimized)
			if(!IsIconic(hWnd)) PostThreadMessage(nRenderThreadID, UWM_PAUSE, false, 0);
			break;

	case WM_GETMINMAXINFO:
			// must specify a minimun for both aspects before we care
			if(APP_ALLOW_RESIZE && (APP_MIN_HEIGHT > 0) && (APP_MIN_WIDTH > 0))
			{
				MINMAXINFO *pInfo = (MINMAXINFO *)lParam;

				// we use the message to enforce a minimum width and height of the window (if desired)
				pInfo->ptMinTrackSize.x = APP_MIN_HEIGHT;
				pInfo->ptMinTrackSize.y = APP_MIN_WIDTH;
			}
			break; 
	
	case WM_SIZE:
			switch(wParam)
			{
				case SIZE_MINIMIZED:
					PostThreadMessage(nRenderThreadID, UWM_PAUSE, true, 0);	// play nice and don't hog the computer if minimized
					break;

				case SIZE_RESTORED:
					// we need to resize the viewport for OGL, but it must be done in the
					// context of the render thread, so set the signal to be picked up
					PostThreadMessage(nRenderThreadID, UWM_RESIZE, 0, 0);
					// back in action, so let the threads continue
					PostThreadMessage(nRenderThreadID, UWM_PAUSE, false, 0);
					break;
			}
			break;

	case WM_KEYDOWN:
			#ifndef NDEBUG
				if(Key(VK_ESCAPE))
					PostThreadMessage(nRenderThreadID, UWM_STOP, 0, 0), Running = false;
			#endif
			#ifdef _2D_MODE
				if(Key(VK_ESCAPE))
					PostThreadMessage(nRenderThreadID, UWM_STOP, 0, 0), Running = false;
			#endif
			KeyBoard[wParam] = true;
			break;
	case WM_KEYUP:
			KeyBoard[wParam] = false;
			break;

	case WM_LBUTTONDOWN:
			//LMouseButton = true;
			break;

	case WM_RBUTTONDOWN:
			//RMouseButton = true;
			break;

	case WM_LBUTTONUP:
			//LMouseButton = false;
			break;

	case WM_RBUTTONUP:
			//RMouseButton = false;
			break;
	default:
			lReturn = DefWindowProc(hWnd, uMsg, wParam, lParam);

	}
	return lReturn;
}