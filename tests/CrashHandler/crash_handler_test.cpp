#include "Stacktrace.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

[[gnu::noinline]] void TalesCrashTestFault()
{
	volatile int* address = nullptr;
	*address = 1;
}

[[gnu::noinline]] void TalesCrashTestJumpToGarbage()
{
	volatile std::uintptr_t target = 0x244;
	reinterpret_cast<void (*)()>(target)();
}

namespace
{

int failures = 0;

void Check(bool condition, const std::string& scenario, const std::string& what)
{
	if (!condition)
	{
		std::cerr << "FAIL [" << scenario << "]: " << what << "\n";
		++failures;
	}
}

bool Contains(const std::string& text, const char* part)
{
	return text.find(part) != std::string::npos;
}

std::string ReadAll(int fd)
{
	std::string text;
	char buffer[4096];
	ssize_t count;
	while ((count = read(fd, buffer, sizeof buffer)) > 0)
		text.append(buffer, static_cast<size_t>(count));
	return text;
}

std::string ReadFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	std::ostringstream text;
	text << file.rdbuf();
	return text.str();
}

struct CrashResult
{
	int status = 0;
	std::string stderrText;
	std::string crashLog;
};

CrashResult Crash(void (*fault)())
{
	CrashResult result;
	char workDirTemplate[] = "/tmp/tales-crash-XXXXXX";
	const char* workDir = mkdtemp(workDirTemplate);
	int pipeFds[2];
	if (workDir == nullptr || pipe(pipeFds) != 0)
	{
		std::perror("crash test setup");
		std::exit(2);
	}

	const pid_t child = fork();
	if (child == 0)
	{
		close(pipeFds[0]);
		dup2(pipeFds[1], STDERR_FILENO);
		close(pipeFds[1]);
		if (chdir(workDir) != 0)
			_exit(90);
		rlimit core{};
		getrlimit(RLIMIT_CORE, &core);
		core.rlim_cur = 0;
		setrlimit(RLIMIT_CORE, &core);
		SEHTranslator translator;
		getrlimit(RLIMIT_CORE, &core);
		if (core.rlim_cur != core.rlim_max)
			_exit(92);
		core.rlim_cur = core.rlim_max = 1;
		setrlimit(RLIMIT_CORE, &core);
		fault();
		_exit(91);
	}

	close(pipeFds[1]);
	result.stderrText = ReadAll(pipeFds[0]);
	close(pipeFds[0]);
	waitpid(child, &result.status, 0);
	result.crashLog = ReadFile(std::string(workDir) + "/log/crash.log");
	return result;
}

void CheckFatalSegv(const CrashResult& result, const std::string& scenario)
{
	Check(!WIFEXITED(result.status) || WEXITSTATUS(result.status) != 92, scenario, "the handler raises the core size limit to the hard limit");
	Check(WIFSIGNALED(result.status) && WTERMSIG(result.status) == SIGSEGV, scenario, "the process terminates with SIGSEGV");
	Check(Contains(result.stderrText, "Fatal SIGSEGV"), scenario, "stderr names the fatal signal");
	Check(Contains(result.crashLog, "Fatal SIGSEGV"), scenario, "log/crash.log names the fatal signal");
	Check(Contains(result.stderrText, "dumping core"), scenario, "stderr reports the core dump");
	if (failures != 0)
		std::cerr << "--- " << scenario << " stderr ---\n" << result.stderrText;
}

}

int main()
{
	const CrashResult nullWrite = Crash(TalesCrashTestFault);
	CheckFatalSegv(nullWrite, "null write");
	Check(Contains(nullWrite.stderrText, "TalesCrashTestFault"), "null write", "stderr holds a stack trace through the faulting function");
	Check(Contains(nullWrite.crashLog, "TalesCrashTestFault"), "null write", "log/crash.log holds the stack trace");

	const CrashResult garbageJump = Crash(TalesCrashTestJumpToGarbage);
	CheckFatalSegv(garbageJump, "jump to garbage");
	Check(Contains(garbageJump.stderrText, "fault address 0x244"), "jump to garbage", "stderr names the fault address");

	if (failures != 0)
		return 1;
	std::cout << "crash handler: traces logged, cores dumped\n";
	return 0;
}
