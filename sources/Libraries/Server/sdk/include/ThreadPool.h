//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#pragma once

#include "DBCCommon.h"
#include "PreAlloc.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

class Task;

#define	NOTWAITSTATE	(0x3FFFFFFF)
//========ThreadPool==============================================================================
class ThreadPool
{
	friend class Task;
public:
	static ThreadPool* CreatePool(long idle=0,long max=0x10,long taskmaxnum=0x100
		,int nPriority=THREAD_PRIORITY_NORMAL);
	void DestroyPool();
	void AddTask(Task *task);
	long WaitForTask(Task *task);
	long GetTaskCount();

	long getCurr();
	long getCurrFree();

	InterLockedLong			m_taskexitflag;
private:
	ThreadPool(long idle,long max,long taskmaxnum);
	virtual ~ThreadPool();
	void StartThread();
	void ThreadExcute();
	Task *NextTask();

	InterLockedLong			m_exitflag;
	const long				m_max,m_idle,m_taskmax;

	std::mutex				m_mtx;
	std::condition_variable	m_cvTaskAdded;
	std::condition_variable	m_cvTaskTaken;
	std::condition_variable	m_cvThreadExit;
	long					m_curr{},m_currfree{};
	std::deque<Task*>		m_tasks;
};
//========Task==================================================================================
class Task{
	friend class ThreadPool;
public:
	Task() = default;
	bool				GetExitFlag();
protected:
	virtual				~Task() = default;
	virtual	Task	*	Lastly(){Free();return 0;}
private:
	virtual	void		Free(){delete this;}
	virtual	long		Process()=0;
	Task			*	TaskExec(ThreadPool *pool);
	long				WaitMe();

	std::atomic<ThreadPool*>	__pool{nullptr};
	std::atomic<bool>			__queued{false};

	std::mutex					__mtxtaskwait;
	std::condition_variable		__cvtaskdone;
	bool						__canwait{false};
	unsigned long				__runcount{0};
	long						__retval{0};
};
//========PreAllocTask======================================================================
class PreAllocTask:public PreAllocStru,public Task{
public:
	PreAllocTask(uLong size):PreAllocStru(size){};
	virtual void Free(){PreAllocStru::Free();}
};
#pragma pack(pop)
_DBC_END
