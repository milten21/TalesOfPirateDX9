#include <string>
#include "dstring.h"
#include "rstring.h"
#include "DBCCommon.h"
#include "PreAlloc.h"

_DBC_USING

//=======PreAllocHeapPtr=======================================================================
uLong PreAllocStru::Size()	//后裔类Override返回当前结构管理的缓存尺寸
{
	return __preAllocHeapPtr?__preAllocHeapPtr->m_unitsize:(__preAllocHeap?__preAllocHeap->m_unitsize:0);
}
void PreAllocStru::Free()
{
	if(__preAllocHeapPtr)
	{
		*__preAllocHeapPtr<<this;
	}else if(__preAllocHeap)
	{
		*__preAllocHeap<<this;
	};
};
