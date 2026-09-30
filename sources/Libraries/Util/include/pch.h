
// pch.h
//  created by claude fan at 2004-8-31
//  for precompiled-header

#if !defined(LOGUTIL_PRECOMPILED_HEADER_FILE)
#define LOGUTIL_PRECOMPILED_HEADER_FILE

// 平台无关宏定义
#define LINE_COMMENT /##/
#define LC LINE_COMMENT
#define $ LINE_COMMENT

// C标准库
#include <stdio.h>
#include <time.h>

// C++标准库
#include <list>
#include <map>
#include <list>
#include <string>

// 平台相关定义
#include "Portability.h"

#if defined(WIN32)
	// Win32平台所需定义和包含
    #pragma warning(disable : 4251)
    #pragma warning(disable : 4786)

	#include <io.h>
	#include <direct.h>
#endif


#endif
