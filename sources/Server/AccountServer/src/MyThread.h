//  [10/14/2005]	从AccountServer2文件里进行模块拆分,分离出此模块 - by Arcol
#pragma once

#ifndef _MYTHREAD_H_
#define _MYTHREAD_H_

#include "Comm.h"

#include <atomic>
#include <thread>

// 线程基类
class MyThread
{
public:
	MyThread();
	virtual ~MyThread();

	bool Launch();
	void NotifyToExit();
	int WaitForExit(int ms); // block calling thread

protected:
	virtual int Run();
	bool GetExitFlag() const {return m_bExitFlag;}
	void ExitThread() {m_bExitThrd = true;}

private:
	std::atomic<bool> m_bExitFlag;
	std::atomic<bool> m_bExitThrd;
	int m_nExitCode;
	std::thread m_Thread;
};

#endif
