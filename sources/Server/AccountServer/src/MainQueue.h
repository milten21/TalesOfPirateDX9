#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <string>

struct sUserLog
{
	bool bLogin;
	int nUserID;
	std::string strUserName;
	std::string strPassport;
	std::string strLoginIP;
};

enum class MainMessageType
{
	UserLog,
	UserLogMap,
};

struct MainMessage
{
	MainMessageType type;
	sUserLog log;
};

class MainQueue
{
public:
	void Post(MainMessage message);
	std::optional<MainMessage> Wait(std::chrono::milliseconds timeout);

	void RequestClose();
	bool IsCloseRequested() const;

private:
	std::mutex m_mutex;
	std::condition_variable m_posted;
	std::deque<MainMessage> m_messages;
	std::atomic<bool> m_closeRequested{false};
};
