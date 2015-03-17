#ifndef NSBASE_PLATFORM_H_
#define NSBASE_PLATFORM_H_

//Toolchain Detection
#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
  //MSVC toolchain already imply the target OS
  #define NCORE_WINDOWS
  //Platform Detection
  #if defined(_M_IX86)
    #define NCORE_X86
  #elif defined(_M_X64)
    #define NCORE_X64
  #endif
  //Configuration Detection
  #if defined(NDEBUG)
    #define NCORE_RELEASE
  #elif defined(_DEBUG)
    #define NCORE_DEBUG
  #endif
#endif

#if defined NCORE_WINDOWS
  #define NOMINMAX
  #include <winsock2.h>
  //avoiding conflicit with winsock.h
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <dwmapi.h>
  #include <windowsx.h>
  #include <ws2tcpip.h>
  #include <mswsock.h>
  #include <shlobj.h>
  #include <aclapi.h>
  #include <accctrl.h>
  #include <sddl.h>
#else
  #error Unspported OS
#endif

#if !defined(NCORE_X86) && !defined(NCORE_X64)
  #error Unspported Architecture
#endif

#if !defined(NCORE_DEBUG) && !defined(NCORE_RELEASE)
  #error Unsupported Configuration
#endif


#include <assert.h>
#include <ctype.h>
#include <memory.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace ncore
{
#ifdef NCORE_WINDOWS

static const size_t kMaxPath16 = 264;
static const size_t kMaxPath8 = 1048;
static const size_t kMaxPath = kMaxPath8;

typedef HKEY RegKey;

#endif


}

#endif