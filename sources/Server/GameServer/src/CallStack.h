#pragma once

#ifndef __CALLSTACK_H__
#define __CALLSTACK_H__

#include "StackWalker.h"
#include "log.h"

// Simple implementation of an additional output to the console:
class MyStackWalker : public StackWalker
{
public:
	MyStackWalker() : StackWalker() {}
	MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
	virtual void OnOutput(LPCSTR szText) 
	{
		//	计算文件名字。
		std::string sName;
		GetLGDir( sName );
		sName += "/exception" + std::to_string( GetCurrentThreadId() ) + ".txt";

		//
		FILE* fp;

		fp = fopen( sName.c_str(), "a+" );

		if( !fp )
		{
			fp = fopen( sName.c_str(), "w+" );
		}

		if( fp )
		{
			fprintf( fp,szText );
			fclose( fp );
		}

		//
		StackWalker::OnOutput(szText);
	}
};

// Exception handling and stack-walking example:
inline LONG WINAPI ExpFilter(EXCEPTION_POINTERS* pExp, DWORD dwExpCode)
{
	MyStackWalker sw;
	sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif //__CALLSTACK_H__