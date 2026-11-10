#include "pi_Memory.h"

#include <chrono>
#include <ctime>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#else
#include <cstdio>
#include <cstring>
#endif

using namespace std;

namespace
{

struct MemorySample
{
	double workingSetMb;
	double virtualMb;
	double physicalAvailableMb;
};

constexpr double Megabyte = 1024.0 * 1024.0;

#ifdef _WIN32

bool SampleMemory(MemorySample& sample)
{
	PERFORMANCE_INFORMATION pi;
	PROCESS_MEMORY_COUNTERS pmc;
	if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) || !GetPerformanceInfo(&pi, sizeof(PERFORMANCE_INFORMATION)))
		return false;
	sample.workingSetMb = pmc.WorkingSetSize / Megabyte;
	sample.virtualMb = pmc.PagefileUsage / Megabyte;
	sample.physicalAvailableMb = pi.PhysicalAvailable * pi.PageSize / Megabyte;
	return true;
}

#else

bool ReadKilobytes(const char* path, const char* key, double& megabytes)
{
	FILE* file = fopen(path, "r");
	if (file == nullptr)
		return false;
	char line[256];
	const size_t keyLength = strlen(key);
	bool found = false;
	while (!found && fgets(line, sizeof line, file) != nullptr)
	{
		unsigned long long kilobytes = 0;
		if (strncmp(line, key, keyLength) == 0 && sscanf(line + keyLength, " %llu", &kilobytes) == 1)
		{
			megabytes = kilobytes * 1024.0 / Megabyte;
			found = true;
		}
	}
	fclose(file);
	return found;
}

bool SampleMemory(MemorySample& sample)
{
	return ReadKilobytes("/proc/self/status", "VmRSS:", sample.workingSetMb)
		&& ReadKilobytes("/proc/self/status", "VmSize:", sample.virtualMb)
		&& ReadKilobytes("/proc/meminfo", "MemAvailable:", sample.physicalAvailableMb);
}

#endif

string CurrentTime()
{
	const time_t now = time(nullptr);
	tm local{};
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif
	char text[15] = {0};
	strftime(text, sizeof text, "%m-%d %H:%M:%S", &local);
	return text;
}

}

pi_Memory::pi_Memory(const char* fileName) : m_minuteInterval(0), m_bStop(false)
{
	m_szfileName = fileName;
}

pi_Memory::~pi_Memory(void)
{
	stopMonitor();
	wait();
}

bool pi_Memory::startMonitor(int minuteInterval)
{
	if (m_thread.joinable())
		return false;

	this->m_minuteInterval = minuteInterval;

	m_outputFile.open(m_szfileName.c_str(), ios::out );
	m_outputFile << "time,memory size,virtual memory size,physical available size" << endl;

	m_thread = std::thread(&pi_Memory::Run, this);
	return true;
}

bool pi_Memory::stopMonitor()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bStop = true;
	}
	m_stopChanged.notify_all();

	return true;
}

bool pi_Memory::wait()
{
	if (m_thread.joinable())
		m_thread.join();

	if(m_outputFile.is_open())
		m_outputFile.close();

	return true;
}

void pi_Memory::Run()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while(!m_bStop)
	{
		lock.unlock();
		MemorySample sample;
		if (SampleMemory(sample))
		{
			m_outputFile << CurrentTime() << "," << sample.workingSetMb << "," << sample.virtualMb << ","
					<< sample.physicalAvailableMb << endl;
		}
		lock.lock();

		m_stopChanged.wait_for(lock, std::chrono::minutes(m_minuteInterval), [this] { return m_bStop; });
	}
}
