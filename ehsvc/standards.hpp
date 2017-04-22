#pragma once

#pragma comment( linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
		version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/* Undefine UNICODE to disable it */
#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

/* Disable secure-warnings */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _STL_SECURE_NO_WARNINGS
#define _STL_SECURE_NO_WARNINGS
#endif

#include <Windows.h>

#include <CommCtrl.h>
#include <windowsx.h>

#include <iostream>
#include <string>

/* preprocessor macros */
#ifndef Padding
#define Padding(x) struct { unsigned char __padding##x[(x)]; };
#endif