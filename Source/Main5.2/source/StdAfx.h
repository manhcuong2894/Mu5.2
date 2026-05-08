// stdafx.h : include file for standard system include files,
#pragma once

// warining
#pragma warning(disable : 4067)
#pragma warning(disable : 4099)
#pragma warning(disable : 4786)
#pragma warning(disable : 4800)
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#pragma warning(disable : 4237)
#pragma warning(disable : 4305)
#pragma warning(disable : 4503)
#pragma warning(disable : 4267)
#pragma warning(disable : 4091)
#pragma warning(disable : 4819)
#pragma warning(disable : 4505)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4702)
#pragma warning(disable : 4838)
#pragma warning(disable : 5208)

#pragma warning(disable : 28159)
#pragma warning(disable : 26812)
#pragma warning(disable : 28251)
#pragma warning(disable : 26451)
#pragma warning(disable : 26819)

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#ifndef _QWORD_DEFINED_
#define _QWORD_DEFINED_
typedef unsigned __int64 QWORD;
#endif

#ifndef _USE_32BIT_TIME_T
#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif //_WIN64
#endif //_USE_32BIT_TIME_T

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#pragma warning(push, 3)

#include <windows.h>

// windows
#include <WinSock2.h>
#include <mmsystem.h>
#include <shellapi.h>

// c runtime
#include <assert.h>
#include <conio.h>
#include <malloc.h>
#include <math.h>
#include <mbstring.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include <chrono> // Para std::chrono::milliseconds
#include <random>
#include <thread> // Para std::this_thread::sleep_for

#include <cmath>
#include <fstream>
#include <unordered_map>
#pragma warning(pop)

#include <ft2build.h>
#include FT_FREETYPE_H
// opengl - GLEW MUST BE INCLUDED BEFORE GL.H
#include <gl/glew.h>
#include <gl/gl.h>

#ifndef MAIN_UPDATE
#define MAIN_UPDATE 603
#endif //_USE_32BIT_TIME_T
// patch
// winmain
#include "Defined_Global.h"
#include "Winmain.h"

// client
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"
#include "_TextureIndex.h"

#include "_GlobalFunctions.h"
#include "w_WindowMessageHandler.h"

#include "WINHANDLE.h"
#include "imgui.h"

#include "./Math/ZzzMathLib.h"
#include "CBInterface.h"
#include "CGMCharacter.h"
#include "CGMProtect.h"
#include "InfoHelperFunctions.h"
#include "NewUICommon.h"
#include "UIDefaultBase.h"
#include "Util.h"
#include "ZzzOpenglUtil.h"
