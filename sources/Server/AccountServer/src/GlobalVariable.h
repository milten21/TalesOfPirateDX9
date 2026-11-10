#pragma once

#ifndef _GLOBALVARIABLE_H_
#define _GLOBALVARIABLE_H_



#include "TLSIndex.h"
#include "DataBaseCtrl.h"
#include "MainQueue.h"

#include "i18n.h"							//Add by lark.li 20080307

// Add by lark.li 20080730 begin
#include "pi_Alloc.h"
#include "pi_Memory.h"
// End

//#define _RELOGIN_MODE_					//新的倒计时重复登陆模式

extern std::string g_strCfgFile;

extern dbc::TLSIndex	g_TlsIndex;
extern CDataBaseCtrl	g_MainDBHandle;
extern MainQueue		g_MainQueue;


#ifdef _WIN32
extern HANDLE hConsole;

#define C_PRINT(s, ...) \
	SetConsoleTextAttribute(hConsole, 14); \
	printf(s, __VA_ARGS__); \
	SetConsoleTextAttribute(hConsole, 10);

#define C_TITLE(s) \
	char szPID[32]; \
	_snprintf_s(szPID,sizeof(szPID),_TRUNCATE, "%d", GetCurrentProcessId()); \
	std::string strConsoleT; \
	strConsoleT += "[PID:"; \
	strConsoleT += szPID; \
	strConsoleT += "]"; \
	strConsoleT += s; \
	SetConsoleTitle(strConsoleT.c_str());
#else
#define C_PRINT(s, ...) printf(s, ##__VA_ARGS__);
#define C_TITLE(s)
#endif


#endif