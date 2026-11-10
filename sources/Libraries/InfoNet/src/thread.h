/* * * * * * * * * * * * * * * * * * * *

    线程封装
    Jampe
    2006/3/27

 * * * * * * * * * * * * * * * * * * * */


#if !defined __THREAD_H__
#define __THREAD_H__

#include <condition_variable>
#include <mutex>
#include <thread>

class Thread
{
public:
    Thread();
    virtual ~Thread();

    virtual bool Begin();  //  开始线程
    virtual bool Terminate();
    virtual unsigned Run() = 0;

protected:
    bool Stopping();
    bool Pause(unsigned milliseconds);

private:
    std::thread             m_thread;
    std::mutex              m_mutex;
    std::condition_variable m_wake;
    bool                    m_stop;
};


#endif
