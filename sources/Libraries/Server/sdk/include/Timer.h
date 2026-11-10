#pragma once

#include "ThreadPool.h"

#include <chrono>
#include <mutex>

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)
//==============��ʱ��======================================
class Timekeeper
{
public:
	Timekeeper(bool mode =false):m_mode(mode)
	{
	}

	void  Begin()
	{
		if(m_mode)
		{
			m_startTime	=std::chrono::steady_clock::now();
		}
		else
		{
			m_ulStartTime	=GetTickCount();
		}
	}

	uLong End()
	{
		if(m_mode)
		{
			m_ulTimeCount	=uLong(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -m_startTime).count());
		}
		else
		{
			m_ulTimeCount	=GetTickCount()-m_ulStartTime;
		}
		return m_ulTimeCount;
	}

	uLong GetTimeCount()
	{
		return m_ulTimeCount;
	}

private:
	bool	const	m_mode;
	std::chrono::steady_clock::time_point	m_startTime;
	uLong			m_ulStartTime{};

	uLong           m_ulTimeCount{};
};
//==============��ʱ��======================================
class Timer
{
	friend class TimerMgr;
protected:
	Timer(uLong interval);
	virtual ~Timer(){}
private:
	bool operator()();
	virtual void Process(){}
	virtual void Free(){delete this;}

	uLong		m_starttick;
	uLong		m_lasttick;

	uLong		m_interval;
};
class TimerMgr :public Task
{
	friend class Timer;
public:
	TimerMgr();
	virtual ~TimerMgr();
	bool AddTimer(Timer *timer);
private:
	long Process();
	void Free();
	std::recursive_mutex					m_mtxtime;
	Timer* volatile	m_timer[100]{};
	uLong		volatile	m_timercount;
	uLong					m_starttick;
};


#pragma pack(pop)
_DBC_END
