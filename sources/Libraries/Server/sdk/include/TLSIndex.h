#pragma once
#include "excp.h"

#include <atomic>
#include <cstddef>
#include <vector>

_DBC_BEGIN

//==============================================================================
struct TLSIndex
{
	TLSIndex():index(NextIndex())
	{
	}
	LPVOID	GetPointer()
	{
		std::vector<LPVOID> &l_slots =Slots();
		return index <l_slots.size()?l_slots[index]:nullptr;
	}
	BOOL	SetPointer(LPVOID p)
	{
		std::vector<LPVOID> &l_slots =Slots();
		if(index >=l_slots.size())
		{
			l_slots.resize(index +1,nullptr);
		}
		l_slots[index]	=p;
		return TRUE;
	}
private:
	static std::size_t NextIndex()
	{
		static std::atomic<std::size_t> s_next{0};
		return s_next++;
	}
	static std::vector<LPVOID> &Slots()
	{
		thread_local std::vector<LPVOID> s_slots;
		return s_slots;
	}

	std::size_t	index;
};

_DBC_END

