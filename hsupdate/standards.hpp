#pragma once

#pragma comment( linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
		version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/* undefine UNICODE in order to disable it */
#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

/* disable secure-warnings */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _STL_SECURE_NO_WARNINGS
#define _STL_SECURE_NO_WARNINGS
#endif

/* important includes */
#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <Windows.h>

#include <CommCtrl.h>
#include <WindowsX.h>

#include <iostream>
#include <string>

/* preprocessor macros */
#ifndef Padding
#define Padding(x) struct { unsigned char __padding##x[(x)]; };
#endif