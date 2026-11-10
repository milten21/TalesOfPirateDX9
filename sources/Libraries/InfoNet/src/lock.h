
/* * * * * * * * * * * * * * * * * * * *

    线程同步锁
    2006/3/27
    Jampe

 * * * * * * * * * * * * * * * * * * * */


#if !defined __LOCK_H__
#define __LOCK_H__


#include <mutex>


class Lock
{
public:
    Lock();
    virtual ~Lock();
    virtual void lock();
    virtual void unlock();

protected:
    std::recursive_mutex    m_mutex;
};



#endif
