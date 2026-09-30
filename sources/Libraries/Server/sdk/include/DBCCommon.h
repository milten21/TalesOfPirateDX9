//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// The last modification of this program is in 2003.7
//=================================================================
//include file;
#pragma once

#include "SocketTypes.h"
#include "Portability.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>


//----------------------------------------------------------------
#define _DBC_BEGIN	namespace dbc {
#define _DBC_END		};
#define _DBC_USING	using namespace dbc;

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

//=================================================================
#define	nil 0
//typedef type define
typedef char Char;							//带符号8个二进制位(=1个字节)整型/字符数据
typedef wchar_t wChar;
typedef short Short;						//带符号2字节整型数据
typedef long Long;							//带符号4字节整型数据
typedef int Int;							//带符号系统依赖整数数据(32位机实现为uLong)
typedef const char cChar;
typedef const wchar_t cwChar;
typedef const short cShort;
typedef const long cLong;
typedef const int cInt;
typedef unsigned char uChar;				//无符号8个二进制位(=1个字节)整型/字符数据
typedef unsigned short uShort;				//无符号2字节整型数据
typedef unsigned long uLong;				//无符号4字节整型数据
typedef unsigned int uInt;					//无符号系统依赖整数数据(32位机实现为uLong)
typedef const unsigned char cuChar;
typedef const unsigned short cuShort;
typedef const unsigned long cuLong;
typedef const unsigned int cuInt;
typedef wchar_t	WChar;						//2字节字符数据
typedef std::int64_t	LLong;						//带符号8字节整型数据
#define SIGN32	uLong(0x80000000)
#define SIGN16	uShort(0x8000)
#define SIGN8	uChar(0x80)
//=========================InterLockedLong========================================
#pragma pack(push)
#pragma pack(4)
class InterLockedLong
{
public:
	InterLockedLong(LONG lInitVal=0):m_plVal(lInitVal){}
	InterLockedLong(const InterLockedLong &val):m_plVal(LONG(val)){}
	operator LONG()const				{return m_plVal.load();}
	LONG operator=(LONG val)			{Assign(val);return val;}
	LONG operator=(const InterLockedLong &val){LONG l_val =val;Assign(l_val);return l_val;}
	LONG operator++()					{return Increment();}
	LONG operator++(int)				{LONG lret =Increment();return lret -1;}
	LONG operator--()					{return Decrement();}
	LONG operator--(int)				{LONG lret =Decrement();return lret +1;}
	LONG operator+=(LONG val)			{LONG lret =Add(val);return lret +val;}
	LONG operator-=(LONG val)			{LONG lret =Add(-val);return lret -val;}
	LONG operator+=(InterLockedLong	&val){LONG l_val =val;LONG lret =Add(l_val);return lret +l_val;}
	LONG operator-=(InterLockedLong	&val){LONG l_val =val;LONG lret =Add(-l_val);return lret -l_val;}
	LONG SetZero()						{return Assign(0);}//返回原始值
	LONG Increment()					{return ++m_plVal;}				//The return value is the resulting incremented value
	LONG Decrement()					{return --m_plVal;}				//The return value is the resulting decremented value
	LONG Add(LONG Value)				{return m_plVal.fetch_add(Value);}	//The return value is the initial value
	LONG Assign(LONG newval)			{return m_plVal.exchange(newval);}	//The return value is the initial value
	LONG CompareAssign(LONG Comperand,LONG newval)	//相等时候才赋值,The return value is the initial value
	{
		LONG l_val =Comperand;
		m_plVal.compare_exchange_strong(l_val,newval);
		return l_val;
	}
private:
	std::atomic<LONG>	m_plVal;
};
#pragma pack(pop)
struct RefArmor
{
	RefArmor(InterLockedLong &count):m_count(count){++m_count;}
	~RefArmor(){--m_count;}
private:
	InterLockedLong	&m_count;
};
//==============================BitMaskStatus===================================
//
class BitMaskStatus
{
public:
	BitMaskStatus(uLong initmask =0):m_BitMask(initmask){}
	int operator=(uLong mask)			{return (m_BitMask  = mask);}
	void ClearBit(uLong mask)		{m_BitMask &=~mask;}
	void SetBit(uLong mask)			{m_BitMask |= mask;}
	bool IsTrue()					{return (m_BitMask		 )?true:false;}
	bool IsFalse()					{return (m_BitMask		 )?false:true;}
	bool IsTrue(uLong mask)			{return (m_BitMask & mask)?true:false;}
	bool IsFalse(uLong mask)		{return (m_BitMask & mask)?false:true;}
private:
	uLong m_BitMask;
};

//=================================================================
template <typename T>
inline T* ToPointer(std::uintptr_t address) {
	return reinterpret_cast<T*>(address);
}

inline std::uintptr_t ToAddress(void* pointer) {
	return reinterpret_cast<std::uintptr_t>(pointer);
}
//=================================================================
//TODO(Ogge): Remove MemCpy and MemSet
inline char* MemCpy(char *dest,cChar *src,uLong size)
{
	return (char*)memcpy(dest,src,size);
}
inline char* MemSet(char *dest,int val,uLong size)
{
	return (char*)memset(dest,val,size);
}
//=================================================================
class Sema
{
public:
	Sema() = default;
	Sema(LONG lInitialCount,LONG lMaximumCount,cChar *lpName =0){Create(lInitialCount,lMaximumCount,lpName);}
	Sema(const Sema &) = delete;
	Sema &operator=(const Sema &) = delete;
	~Sema(){Destroy();}
	void Destroy()
	{
		{
			auto const l_lock = std::lock_guard{m_mtx};
			m_created =false;
		}
		m_cond.notify_all();
	}
	operator bool(){auto const l_lock = std::lock_guard{m_mtx};return m_created;}
	BOOL Create(LONG lInitialCount,LONG lMaximumCount,cChar *lpName =0)
	{
		auto const l_lock = std::lock_guard{m_mtx};
		if(m_created ||lMaximumCount <=0 ||lInitialCount <0 ||lInitialCount >lMaximumCount)
		{
			return FALSE;
		}
		m_count		=lInitialCount;
		m_max		=lMaximumCount;
		m_created	=true;
		return TRUE;
	}

	DWORD lock()		{return timelock(INFINITE);}
	DWORD trylock()		{return timelock(0);}
	DWORD timelock(DWORD dwMilliseconds)
	{
		auto l_lock = std::unique_lock{m_mtx};
		auto const l_ready = [this]{return !m_created ||m_count >0;};
		if(dwMilliseconds ==INFINITE)
		{
			m_cond.wait(l_lock,l_ready);
		}else if(!m_cond.wait_for(l_lock,std::chrono::milliseconds(dwMilliseconds),l_ready))
		{
			return WAIT_TIMEOUT;
		}
		if(!m_created)
		{
			return WAIT_FAILED;
		}
		--m_count;
		return WAIT_OBJECT_0;
	}
	BOOL  unlock(Long relcount =1)
	{
		{
			auto const l_lock = std::lock_guard{m_mtx};
			if(!m_created ||relcount <=0 ||relcount >m_max -m_count)
			{
				return FALSE;
			}
			m_count	+=relcount;
		}
		if(relcount ==1)
		{
			m_cond.notify_one();
		}else
		{
			m_cond.notify_all();
		}
		return TRUE;
	}
private:
	std::mutex				m_mtx;
	std::condition_variable	m_cond;
	LONG					m_count{};
	LONG					m_max{};
	bool					m_created{false};
};

class ScopedSema
{
public:
	ScopedSema(Sema& sema) : m_sema(sema)
	{
		m_sema.lock();
	}

	~ScopedSema()
	{
		m_sema.unlock();
	}

private:
	Sema& m_sema;
};

#pragma pack(pop)
_DBC_END
