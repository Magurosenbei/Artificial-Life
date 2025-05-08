// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
// Warnings
#pragma warning (disable:4512)	// no Assignment Operator
#pragma warning (disable:4127)	// constant expression 1 == 1
#pragma warning (disable:4800)	// Performance Warning -- Force BOOL to bool
#pragma warning (disable:4100)	// unreferenced formal parameter
#pragma warning (disable:4189)	// unreferenced formal parameter
#pragma warning (disable:4239)	// conversion to reference
#pragma warning (disable:4099)	// Luabind warnings
#pragma warning (disable:4996)	// copy(...) unsafe parameter --- don't really know what it meant (Luabind)

// Windows Header Files:
#include <windows.h>			// fundamental Windows header file
#include <windowsx.h>			// useful Windows programming extensions
// C RunTime Header Files
#include <tchar.h>				// generic text character mapping
#include <string>				// includes string manipulation routines
#include <stdlib.h>				// includes standard libraries
#include <stdio.h>				// includes standard input/output routines
#include <process.h>			// threading routines for the CRT

// OpenGl Header
#include <OpenGL\Glee.h>
#include <gl\gl.h>				// the OpenGL Library
#include <gl\glu.h>				// the OpenGL Utility Library
#include <OpenGL\glext.h>

// Structure Header
#include "Structure.h"

// TODO: reference additional headers your program requires here
#include <tbb/concurrent_vector.h>		// use this instead of STL
#include <tbb/tick_count.h>				// use this to count for time, it reads in seconds instead of milli
#include <tbb/task_scheduler_init.h>	// For Tasking Schedule, sort of WaitForMultiple(...)
#include <tbb/task.h>					// If anything that needs to be done on the run, inherit this and overload exercute()
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_do.h>

// Lua
extern "C" 
{
#include "Lua/lua.h"
#include "Lua/lualib.h"
#include "Lua/lauxlib.h"
}
#include <luabind/config.hpp>
#include <luabind/class.hpp>
#include <luabind/operator.hpp>
#include <luabind/function.hpp>
#include <luabind/open.hpp>

// Self Made
#include "Engine/engine.h"
#include "TTimer.h"

// Standard Gobals
static const Vector Ref_Vec_Z(0,0,1);
static const Vector Ref_Vec_X(1,0,0);
static Vector Ref_Vec_Y(0,1,0);

// Definition
#define Key(value) (GetAsyncKeyState(value)&0x8000)? true : false
#define PI 3.14159265f
#define MESH_RES 4.0f			//	pixels per vertex
#define MESH_HGT_SCALE 1.0f		//	Height Scaling

extern bool KeyBoard[256];
extern bool Running;

// namespaces

using namespace std;

// Assert
#ifdef _MSC_VER
 #ifdef NDEBUG

	#define ASSERT(exp, msg)	{}
	#define VERIFY(exp, msg)	(exp);

 #else // NDEBUG

	#define ASSERT(exp, msg)	if( !(exp) ) std::_Debug_message(L#msg, _CRT_WIDE(__FILE__), __LINE__);
	#define VERIFY(exp, msg)	if( !(exp) ) std::_Debug_message(L#msg, _CRT_WIDE(__FILE__), __LINE__);

 #endif // end NDEBUG
#else // _MSC_VER
 #ifdef NDEBUG

	#define ASSERT(exp, msg)	{}
	#define VERIFY(exp, msg)	(exp);

 #else // NDEBUG

	#include <cassert>

	#define ASSERT(exp, msg)		assert( (exp) );
	#define VERIFY(exp, msg)		assert( (exp) );

 #endif // end NDEBUG
#endif // end _MSC_VER


// Disable some warnnings
#ifdef _MSC_VER
	#pragma warning(disable:4127)
#endif

#if defined(DEBUG) | defined(_DEBUG)
	#define CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

#define _2D_MODE	// Switch modes
#ifdef NDEBUG
#define WriteCommandLine
#else
#define WriteCommandLine OutputPrintf
#endif

void __cdecl OutputPrintf(const char *format, ...);

extern CRITICAL_SECTION _critSection;