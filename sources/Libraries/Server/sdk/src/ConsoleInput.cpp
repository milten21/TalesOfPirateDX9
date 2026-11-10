#include "ConsoleInput.h"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <unistd.h>
#endif

namespace dbc
{
struct ConsoleInput::Lines
{
	std::mutex mutex;
	std::condition_variable pushed;
	std::deque<std::string> lines;
};
}

namespace
{
constexpr auto PollInterval = std::chrono::milliseconds(200);

void Trim(std::string& line)
{
	constexpr auto whitespace = " \t\r\n";
	const auto last = line.find_last_not_of(whitespace);
	if (last == std::string::npos)
	{
		line.clear();
		return;
	}
	line.erase(last + 1);
	line.erase(0, line.find_first_not_of(whitespace));
}

long ReadStdin(char* buffer, std::size_t size)
{
#ifdef _WIN32
	DWORD count = 0;
	if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, static_cast<DWORD>(size), &count, nullptr))
	{
		return -1;
	}
	return static_cast<long>(count);
#else
	for (;;)
	{
		const ssize_t count = read(STDIN_FILENO, buffer, size);
		if (count >= 0 || errno != EINTR)
		{
			return static_cast<long>(count);
		}
	}
#endif
}

void Push(dbc::ConsoleInput::Lines& lines, std::string line)
{
	{
		auto const lock = std::lock_guard{lines.mutex};
		lines.lines.push_back(std::move(line));
	}
	lines.pushed.notify_one();
}

void ReadUntilEndOfInput(const std::shared_ptr<dbc::ConsoleInput::Lines>& lines)
{
	std::string pending;
	char buffer[512];
	for (long count = ReadStdin(buffer, sizeof(buffer)); count > 0; count = ReadStdin(buffer, sizeof(buffer)))
	{
		pending.append(buffer, static_cast<std::size_t>(count));
		for (auto end = pending.find('\n'); end != std::string::npos; end = pending.find('\n'))
		{
			Push(*lines, pending.substr(0, end));
			pending.erase(0, end + 1);
		}
	}
	if (!pending.empty())
	{
		Push(*lines, std::move(pending));
	}
}
}

namespace dbc
{
ConsoleInput::ConsoleInput(std::function<bool()> exitRequested)
	: m_exitRequested(std::move(exitRequested)), m_lines(std::make_shared<Lines>())
{
	std::thread(ReadUntilEndOfInput, m_lines).detach();
}

bool ConsoleInput::ReadLine(std::string& line)
{
	auto lock = std::unique_lock{m_lines->mutex};
	while (!m_exitRequested())
	{
		if (!m_lines->lines.empty())
		{
			line = std::move(m_lines->lines.front());
			m_lines->lines.pop_front();
			Trim(line);
			return true;
		}
		m_lines->pushed.wait_for(lock, PollInterval);
	}
	return false;
}
}
