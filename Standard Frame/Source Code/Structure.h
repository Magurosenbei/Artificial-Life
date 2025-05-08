#pragma once

// increases readability (case insensitive and Unicode safe)
#define MATCH(szOne, szTwo) (_tcsicmp(szOne, szTwo) == 0)
#define NMATCH(szOne, szTwo, nLen) (_tcsnicmp(szOne, szTwo, nLen) == 0)

// perform a safe sizeof regardless if we use Unicode or not
#define SIZE(szBuffer) (sizeof(szBuffer) / sizeof(TCHAR))
#define REAL_SIZE(szBuffer) ((sizeof(szBuffer) / sizeof(TCHAR)) - sizeof(TCHAR))

// macro to simplify the testing of the version of Windows the program is run on
#define WIN_MIN_VERSION(nMajor, nMinor) (((BYTE)LOBYTE(LOWORD(GetVersion())) >= nMajor) && \
	((BYTE)HIBYTE(LOWORD(GetVersion())) >= nMinor))

// macro to simplify the testing of whether or not the host os is nt-based or 9x
#define IS_WINNT (GetVersion() < (DWORD)0x80000000)

/*/
/ / HWND_BOTTOM, HWND_TOP, etc. are already defined in Winuser.h, but
/ / I don't want to cuase possible compatibility issues in the future.
/ / So, I'm defining my own constants for use with MoveWnd().
/*/
#define MV_BOTTOM	0x0001
#define MV_CENTER	0x0002
#define MV_LEFT		0x0004
#define MV_MIDDLE	0x0008
#define MV_RIGHT	0x0010
#define MV_TOP		0x0020

// vertical sync options
typedef enum tagVSYNC
{
	VSync_Default		= 0x0001,
	VSync_On			= 0x0002,
	VSync_Off			= 0x0003

} VSYNC, *PVSYNC;

// windowing functions
bool MoveWnd (HWND hwnd, unsigned short nOrientation, HWND hwndPreferred);

// inline message box function to simplify reading of resource strings
extern int MsgBox(HWND hWnd, unsigned int uText, unsigned int uType);

// registry functions
extern bool GetCmdLineValue (const LPTSTR szArg, LPTSTR szDest, size_t nLen);
extern bool GetSysValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
extern bool GetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
extern bool SetSysValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
extern bool SetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);

// display functions
extern void SetVSync (VSYNC Status);

#ifdef _DEBUG
	// helper macro(s) for OGL error reporting
	#define ENTER_GL			while((glGetError() != GL_NO_ERROR) && (glGetError() != GL_INVALID_OPERATION));
	#define LEAVE_GL(szProc)	OnGLError(szProc);

	// helper function(s) for OGL error reporting
	extern bool OnGLError (LPCSTR szProcedure);
#endif

typedef struct
{
	VSYNC Status;		// flag to indicate if we enable or disable or leave alone vsync
	HWND hWnd;			// handle to the calling window
	HDC hDC;			// handle to the dc of the client area of the calling window
	BYTE nBPP;			// bits per pixel the application is trying to use (ignored if windowed)
	BYTE nRefresh;		// vertical refresh rate of the display in hertz (ignored if windowed)
	bool bFullscreen;	// flag to indicate to the thread if we are in fullscreen mode
	bool bZoomed;		// flag to indicate to the thread if we are to maximize the main window

}  RENDERARGS, *PRENDERARGS;

// function prototypes
extern unsigned int __stdcall InitScene (void * pParam);

// user defined window messages the render thread uses to communicate
#define UWM_PAUSE	(WM_APP + 1)
#define UWM_RESIZE	(WM_APP + 2)
#define UWM_SHOW	(WM_APP + 3)
#define UWM_STOP	(WM_APP + 4)
