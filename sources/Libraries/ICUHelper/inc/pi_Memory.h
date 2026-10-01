#pragma once

#include <condition_variable>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

class pi_Memory
{
public:
	pi_Memory(const char* fileName = "memorymonitor.log");
	~pi_Memory(void);

public:
	bool startMonitor(int minuteInterval);
	bool stopMonitor();
	bool wait();

private:
	void Run();

private:
	int m_minuteInterval;
	bool m_bStop;

	std::mutex m_mutex;
	std::condition_variable m_stopChanged;
	std::fstream m_outputFile;
	std::thread m_thread;
	std::string m_szfileName;
};
