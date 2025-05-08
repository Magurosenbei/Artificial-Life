#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER																		// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501																// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT																// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501															// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS																// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410														// Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE																	// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600															// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN															// allow the exclusion of uncommon features

#define APP_ALLOW_FULLSCREEN	true												// should the app allow fullscreen mode
#define APP_ALLOW_RESIZE		true												// can the main window to be resized? (windowed only)
#define APP_ALLOW_MENU			false												// do we have a default menu
#define APP_ALLOW_VSYNC			true												// do allow the enabling/disabling of vertical sync?
#define APP_CLASSNAME			_T("STDFRAME_9826C328_598D_4C2E_85D4_0FF8E0310366")	// classname of the main application window
#define APP_DEF_BPP				32													// default bits-per-pixel
#define APP_DEF_FULLSCREEN		false												// should the app default to fullscreen or windowed
#define APP_DEF_HEIGHT			600													// default height of the resolution
#define APP_DEF_WIDTH			800													// default width of the resolution
#define APP_MAX_REFRESH			120													// default max refresh rate to use for fullscreen mode (in hertz)
#define APP_MIN_REFRESH			60													// default min refresh rate to use for fullscreen mode (in hertz)
#define APP_MIN_WINVER_MAJOR	4													// min os version the app can run on
#define APP_MIN_WINVER_MINOR	0													// min os version the app can run on
#define APP_MIN_HEIGHT			0													// mininum height of the main window (zero means no min)
#define APP_MIN_WIDTH			0													// mininum width of the main window (zero means no min)
#define APP_NAME				_T("Artificial Life Demo")							// name of the application (independent of window title)
#define APP_SINGLE_INSTANCE		true												// do we allow single or multiple instances of the app
#define APP_WINVER_NTONLY		false												// should app be ran on NT only?
#define COMPANY_NAME			_T("Trempis Entertainment")							// name of the company responsible for this software
#define MAX_LOADSTRING			256													// max buffer to load for simple string data

#define APP_UPDATERATE			60

#include "../Resource/resource.h"
