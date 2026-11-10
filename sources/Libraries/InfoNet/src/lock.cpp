
#include "lock.h"


Lock::Lock()
{
}


Lock::~Lock()
{
}


void Lock::lock()
{
    m_mutex.lock();
}


void Lock::unlock()
{
    m_mutex.unlock();
}

