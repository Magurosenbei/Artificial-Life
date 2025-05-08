// stdafx.cpp : source file that includes just the standard includes
// Standard Frame.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

/* Debug Codes */
/*
static char szBuffer[512];
_snprintf(szBuffer, 511, "(Score: %i)", g_Engine->m_iScore);
*/

CRITICAL_SECTION _critSection;

void __cdecl OutputPrintf(const char *format, ...)
{
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

    va_start(args, format);
    n = _vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
    va_end(args);

    p += (n < 0) ? sizeof buf - 3 : n;

    while ( p > buf  &&  isspace(p[-1]) )
            *--p = '\0';

    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';

    OutputDebugString(buf);
}
