//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================

#include <iostream>
#include <ios>
#include <LogStream.h>

#include "ThreadPool.h"

#include <chrono>
#include <system_error>
#include <thread>
#include <typeinfo>

_DBC_USING
ThreadPool::ThreadPool(long idle,long max,long taskmaxnum)
	:m_max((max<1)?1:max)
	,m_idle((idle<1)?1:((idle>m_max)?m_max:idle))
	,m_taskmax((taskmaxnum<1)?1:taskmaxnum)
	,m_exitflag(0),m_taskexitflag(0)
{
	auto const l_lock = std::lock_guard{m_mtx};
	long l_errcount = 0;
	for (long i = 0; i < m_idle; i++)
	{
		try
		{
			StartThread();
		}
		catch (const std::system_error&)
		{
			if (++l_errcount >= 20)
				THROW_EXCP(excpThrd, "Thread pool failed to start its threads");
			Sleep(50);
			i--;
		}
	}
}
ThreadPool::~ThreadPool()
{
	while (!m_tasks.empty())
	{
		Task* l_task = m_tasks.front();
		m_tasks.pop_front();
		l_task->__queued = false;
		try
		{
			while (l_task = l_task->Lastly()) {}
		}
		catch (...) {}
	}
}
ThreadPool* ThreadPool::CreatePool(long idle,long max,long taskmaxnum,int nPriority)
{
	return new ThreadPool(idle,max,taskmaxnum);
}
void ThreadPool::DestroyPool()
{
	m_taskexitflag	=1;
	m_exitflag		=1;
	auto l_lock = std::unique_lock{m_mtx};
	m_cvTaskAdded.notify_all();
	m_cvTaskTaken.notify_all();
	if (!m_cvThreadExit.wait_for(l_lock, std::chrono::seconds(10), [this] { return m_curr == 0; }))
	{
		LogLine l_line(g_dbclog);
		l_line << newln << "ThreadPool::DestroyPool: " << m_curr << " thread(s) still running after 10s, pool left allocated";
		return;
	}
	l_lock.unlock();
	delete this;
}
//==============================================================================
Task	*	Task::TaskExec(ThreadPool *pool)
{
	long l_retval	=-10000;

	__pool =pool;

	try {
#ifdef _DEBUG
		std::cout << typeid(*this).name() << std::endl;
#endif
		l_retval = Process();
	}
	catch (...) {}

	{
		auto const l_lock = std::lock_guard{__mtxtaskwait};
		__canwait	=false;
		__retval	=l_retval;
		++__runcount;
		__cvtaskdone.notify_all();
	}

	__pool =nullptr;
	Task	* l_nextTask	=0;
	try{l_nextTask	=Lastly();}catch(...){}
	return l_nextTask;
}
bool Task::GetExitFlag()
{
	ThreadPool *l_pool =__pool;
	return l_pool?l_pool->m_taskexitflag!=0:false;
}
long Task::WaitMe()
{
	auto l_lock = std::unique_lock{__mtxtaskwait};
	if(!__canwait)
	{
		return NOTWAITSTATE;
	}
	unsigned long const l_runcount =__runcount;
	__cvtaskdone.wait(l_lock,[this,l_runcount]{return __runcount !=l_runcount;});
	return __retval;
}
//==============================================================================
void ThreadPool::StartThread()
{
	std::thread(&ThreadPool::ThreadExcute, this).detach();
	m_curr++;
	m_currfree++;
}
void ThreadPool::ThreadExcute()
{
	{
		LogLine l_line(g_dbclog);
		l_line<<newln<<"Thread:["<<std::this_thread::get_id()<<"] Create!";
	}
	for (Task* l_task = NextTask(); l_task; l_task = NextTask())
	{
		try
		{
			while (l_task = l_task->TaskExec(this)) {}
		}
		catch (...) {}

		auto const l_lock = std::lock_guard{m_mtx};
		m_currfree++;
	}
	LogLine l_line(g_dbclog);
	l_line<<newln<<"Thread:["<<std::this_thread::get_id()<<"] Exit!";
}
//==============================================================================
Task	* ThreadPool::NextTask()
{
	auto l_lock = std::unique_lock{m_mtx};
	while (true)
	{
		if (!m_tasks.empty())
		{
			Task* l_task = m_tasks.front();
			m_tasks.pop_front();
			l_task->__queued = false;
			m_cvTaskTaken.notify_one();
			if (m_taskexitflag)
			{
				l_lock.unlock();
				try
				{
					while (l_task = l_task->Lastly()) {}
				}
				catch (...) {}
				l_lock.lock();
				continue;
			}
			m_currfree--;
			if ((m_currfree < m_idle) && (m_curr < m_max))
			{
				try
				{
					StartThread();
				}
				catch (const std::system_error& e)
				{
					std::cout << e.what() << std::endl;
				}
			}
			return l_task;
		}
		if (m_exitflag)
		{
			break;
		}
		if (m_currfree > m_idle)
		{
			if (!m_cvTaskAdded.wait_for(l_lock, std::chrono::milliseconds(50), [this] { return !m_tasks.empty() || m_exitflag; })
				&& m_currfree > m_idle)
			{
				break;
			}
		}
		else
		{
			m_cvTaskAdded.wait(l_lock, [this] { return !m_tasks.empty() || m_exitflag; });
		}
	}
	m_currfree--;
	m_curr--;
	m_cvThreadExit.notify_all();
	return nullptr;
}
//==============================================================================
long ThreadPool::WaitForTask(Task *task)
{
	return task->WaitMe();
}
void ThreadPool::AddTask(Task *task)
{
	if (!task) return;

	bool l_queued = false;
	if (!task->__queued.compare_exchange_strong(l_queued, true))
	{
		return;
	}
	{
		auto const l_lock = std::lock_guard{task->__mtxtaskwait};
		task->__canwait = true;
	}

	auto l_lock = std::unique_lock{m_mtx};
	m_cvTaskTaken.wait(l_lock, [this] { return long(m_tasks.size()) < m_taskmax || m_exitflag; });
	m_tasks.push_back(task);
	m_cvTaskAdded.notify_one();
}
long ThreadPool::GetTaskCount()
{
	auto const l_lock = std::lock_guard{m_mtx};
	return long(m_tasks.size());
}
long ThreadPool::getCurr()
{
	auto const l_lock = std::lock_guard{m_mtx};
	return m_curr;
}
long ThreadPool::getCurrFree()
{
	auto const l_lock = std::lock_guard{m_mtx};
	return m_currfree;
}
