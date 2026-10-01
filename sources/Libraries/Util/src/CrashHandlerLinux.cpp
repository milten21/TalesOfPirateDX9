#include "Stacktrace.h"
#include "log.h"

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>

#include <execinfo.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace
{

constexpr int FatalSignals[] = { SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT };
constexpr int MaxFrames = 128;
constexpr size_t AlternateStackSize = 64 * 1024;

char g_crashLogPath[4096];
std::atomic<pid_t> g_handlingThread{0};
std::mutex g_translatorsMutex;
int g_translators = 0;
sigjmp_buf g_unwindFault;
thread_local std::unique_ptr<char[]> t_alternateStack;

const char* SignalName(int sig)
{
	switch (sig)
	{
	case SIGSEGV: return "SIGSEGV";
	case SIGBUS: return "SIGBUS";
	case SIGFPE: return "SIGFPE";
	case SIGILL: return "SIGILL";
	case SIGABRT: return "SIGABRT";
	default: return "signal";
	}
}

void WriteAll(int fd, const char* text, size_t length)
{
	while (length > 0)
	{
		const ssize_t written = write(fd, text, length);
		if (written <= 0)
		{
			if (written < 0 && errno == EINTR)
				continue;
			return;
		}
		text += written;
		length -= static_cast<size_t>(written);
	}
}

void WriteText(int fd, const char* text)
{
	WriteAll(fd, text, strlen(text));
}

void WriteUnsigned(int fd, std::uintptr_t value, unsigned base)
{
	char digits[32];
	size_t length = 0;
	do
	{
		digits[sizeof digits - 1 - length] = "0123456789abcdef"[value % base];
		value /= base;
		++length;
	} while (value != 0 && length < sizeof digits);
	WriteAll(fd, digits + sizeof digits - length, length);
}

void WriteHeader(int fd, int sig, const siginfo_t* info)
{
	WriteText(fd, "\n*** Fatal ");
	WriteText(fd, SignalName(sig));
	WriteText(fd, " (");
	WriteUnsigned(fd, static_cast<std::uintptr_t>(sig), 10);
	WriteText(fd, ") in process ");
	WriteUnsigned(fd, static_cast<std::uintptr_t>(getpid()), 10);
	WriteText(fd, " at unix time ");
	WriteUnsigned(fd, static_cast<std::uintptr_t>(time(nullptr)), 10);
	if (info->si_code > 0)
	{
		WriteText(fd, ", fault address 0x");
		WriteUnsigned(fd, reinterpret_cast<std::uintptr_t>(info->si_addr), 16);
	}
	WriteText(fd, "\nStack trace:\n");
}

void WriteToBoth(int logFd, const char* text)
{
	WriteText(STDERR_FILENO, text);
	if (logFd >= 0)
		WriteText(logFd, text);
}

void OnFatalSignal(int sig, siginfo_t* info, void*)
{
	const pid_t self = static_cast<pid_t>(syscall(SYS_gettid));
	pid_t handlingThread = 0;
	if (!g_handlingThread.compare_exchange_strong(handlingThread, self))
	{
		if (handlingThread == self)
			siglongjmp(g_unwindFault, 1);
		for (;;)
			pause();
	}

	const int logFd = g_crashLogPath[0] != '\0' ? open(g_crashLogPath, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644) : -1;
	WriteHeader(STDERR_FILENO, sig, info);
	if (logFd >= 0)
		WriteHeader(logFd, sig, info);

	if (sigsetjmp(g_unwindFault, 1) == 0)
	{
		void* frames[MaxFrames];
		const int frameCount = backtrace(frames, MaxFrames);
		backtrace_symbols_fd(frames, frameCount, STDERR_FILENO);
		if (logFd >= 0)
			backtrace_symbols_fd(frames, frameCount, logFd);
	}
	else
	{
		WriteToBoth(logFd, "(unavailable: unwinding the corrupt stack faulted)\n");
	}
	WriteToBoth(logFd, "*** End of stack trace, dumping core\n");
	if (logFd >= 0)
		close(logFd);

	signal(sig, SIG_DFL);
	raise(sig);
}

void EnableCoreDumps()
{
	rlimit limit{};
	if (getrlimit(RLIMIT_CORE, &limit) == 0 && limit.rlim_cur != limit.rlim_max)
	{
		limit.rlim_cur = limit.rlim_max;
		setrlimit(RLIMIT_CORE, &limit);
	}
}

}

SEHTranslator::SEHTranslator()
{
	std::string dir;
	LG_GetDir(dir);
	if (dir.empty())
		dir = ".";
	const std::string path = dir + "/crash.log";
	if (path.size() < sizeof g_crashLogPath)
		memcpy(g_crashLogPath, path.c_str(), path.size() + 1);

	void* warmup[1];
	backtrace(warmup, 1);

	EnableCoreDumps();

	if (!t_alternateStack)
		t_alternateStack = std::make_unique<char[]>(AlternateStackSize);
	stack_t stack{};
	stack.ss_sp = t_alternateStack.get();
	stack.ss_size = AlternateStackSize;
	sigaltstack(&stack, nullptr);

	const std::lock_guard<std::mutex> lock(g_translatorsMutex);
	if (g_translators++ != 0)
		return;

	struct sigaction action{};
	action.sa_sigaction = OnFatalSignal;
	action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER;
	sigemptyset(&action.sa_mask);
	for (const int sig : FatalSignals)
		sigaction(sig, &action, nullptr);
}

SEHTranslator::~SEHTranslator()
{
	stack_t stack{};
	stack.ss_flags = SS_DISABLE;
	sigaltstack(&stack, nullptr);

	const std::lock_guard<std::mutex> lock(g_translatorsMutex);
	if (--g_translators != 0)
		return;

	for (const int sig : FatalSignals)
		signal(sig, SIG_DFL);
}
