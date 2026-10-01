

#include "thread.h"
#include <chrono>



Thread::Thread()
    :   m_stop(false)
{
}


Thread::~Thread()
{
    Terminate();
}


bool Thread::Begin()
{
    if(m_thread.joinable())
    {
        return false;
    }
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_stop = false;
    }
    m_thread = std::thread([this]
    {
        try
        {
            Run();
        }
        catch(...)
        {
        }
    });
    return true;
}


bool Thread::Terminate()
{
    if(!m_thread.joinable())
    {
        return false;
    }
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_stop = true;
    }
    m_wake.notify_all();
    if(m_thread.get_id() == std::this_thread::get_id())
    {
        m_thread.detach();
    }
    else
    {
        m_thread.join();
    }
    return true;
}


bool Thread::Stopping()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_stop;
}


bool Thread::Pause(unsigned milliseconds)
{
    std::unique_lock<std::mutex> guard(m_mutex);
    return !m_wake.wait_for(guard, std::chrono::milliseconds(milliseconds), [this] { return m_stop; });
}
