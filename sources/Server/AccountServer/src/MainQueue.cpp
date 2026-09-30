#include "MainQueue.h"

#include <utility>

void MainQueue::Post(MainMessage message)
{
	{
		auto const lock = std::lock_guard{m_mutex};
		m_messages.push_back(std::move(message));
	}
	m_posted.notify_one();
}

std::optional<MainMessage> MainQueue::Wait(std::chrono::milliseconds timeout)
{
	auto lock = std::unique_lock{m_mutex};
	if (!m_posted.wait_for(lock, timeout, [this] { return !m_messages.empty(); }))
	{
		return std::nullopt;
	}
	auto message = std::move(m_messages.front());
	m_messages.pop_front();
	return message;
}

void MainQueue::RequestClose()
{
	m_closeRequested = true;
}

bool MainQueue::IsCloseRequested() const
{
	return m_closeRequested;
}
