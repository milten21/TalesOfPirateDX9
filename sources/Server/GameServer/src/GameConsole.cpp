#include "stdafx.h"
#include "GameConsole.h"

#include <atomic>
#include <csignal>
#include <deque>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

#include "ConsoleInput.h"
#include "Config.h"
#include "GameApp.h"
#include "GameAppNet.h"
#include "GameDB.h"
#include "MapRes.h"
#include "Script.h"
#include "SubMap.h"

extern BOOL g_bGameEnd;

namespace
{
constexpr DWORD StatusRefreshInterval = 1000;
constexpr DWORD StatusLogInterval = 60 * 1000;
constexpr DWORD StatusStaleAfter = 5 * 1000;
constexpr unsigned long StopCountdownSeconds = 3;

std::atomic<bool> g_stopRequested{false};
static_assert(std::atomic<bool>::is_always_lock_free);

std::mutex g_luaMutex;
std::deque<std::string> g_luaChunks;

std::mutex g_statusMutex;
std::string g_status = "status: starting";
DWORD g_statusTick = GetTickCount();

void StopHandler(int sig)
{
	g_stopRequested = true;
	std::signal(sig, StopHandler);
}

std::string PKStatus()
{
	CMapRes* pCMap = g_pGameApp->FindMapByName("teampk");
	if (!pCMap)
	{
		return "none";
	}
	int copies = 0;
	long players = 0;
	pCMap->BeginGetUsedCopy();
	while (SubMap* pCopy = pCMap->GetNextUsedCopy())
	{
		copies++;
		players += pCopy->GetPlayerNum();
	}
	return std::to_string(copies) + " (" + std::to_string(players) + " players)";
}

std::string FormatStatus()
{
	const DWORD active = g_pGameApp->m_dwChaCnt;
	const DWORD players = g_pGameApp->m_dwPlayerCnt;
	std::ostringstream line;
	line << "status: loop " << g_pGameApp->m_dwFPS << "/s (" << g_pGameApp->m_dwRunCnt << " total)"
		<< ", db " << (game_db.m_bInitOK ? "ok" : "fail")
		<< ", objects " << active << " active (" << players << " players, "
		<< (active > players ? active - players : 0) << " other)"
		<< ", units " << g_pGameApp->m_dwActiveMgrUnit << " active"
		<< ", maps [";
	for (int i = 0; i < g_Config.m_nMapCnt; i++)
	{
		line << (i ? ", " : "") << g_Config.m_szMapList[i] << (g_Config.m_btMapOK[i] ? " ok" : " fail");
	}
	line << "], gates [";
	for (int i = 0; i < g_Config.m_nGateCnt; i++)
	{
		line << (i ? ", " : "") << g_Config.m_szGateIP[i] << (ISVALIDGATE(i) ? " connected" : " trying");
	}
	line << "], pk " << PKStatus()
		<< ", config " << szConfigFileN
		<< ", resource " << g_Config.m_szResDir
		<< ", log " << g_Config.m_szLogDir;
	return line.str();
}

void PrintStatus()
{
	std::string status;
	DWORD statusTick = 0;
	{
		auto const lock = std::lock_guard{g_statusMutex};
		status = g_status;
		statusTick = g_statusTick;
	}
	printf("%s\n", status.c_str());
	const DWORD age = GetTickCount() - statusTick;
	if (age >= StatusStaleAfter)
	{
		printf("status is %lu s old, game loop run step %ld\n", age / 1000, static_cast<long>(g_pGameApp->m_dwRunStep));
	}
	fflush(stdout);
}

void QueueLua(std::string chunk)
{
	auto const lock = std::lock_guard{g_luaMutex};
	g_luaChunks.push_back(std::move(chunk));
}
}

void InstallStopHandlers()
{
	std::signal(SIGINT, StopHandler);
	std::signal(SIGTERM, StopHandler);
}

void HandleStopRequest()
{
	if (!g_stopRequested.exchange(false))
	{
		return;
	}
	if (g_pGameApp->m_ulLeftSec > 0 && g_pGameApp->m_ulLeftSec <= StopCountdownSeconds)
	{
		return;
	}
	C_PRINT("Notify Logout to GameServer...\n");
	fflush(stdout);
	LG("init", "stop requested, notifying players and saving in %lu seconds\n", StopCountdownSeconds);
	g_pGameApp->m_CTimerReset.Begin(1000);
	g_pGameApp->m_ulLeftSec = StopCountdownSeconds;
}

void RunConsole()
{
	dbc::ConsoleInput console([] { return g_bGameEnd != FALSE; });
	std::string line;
	while (console.ReadLine(line))
	{
		if (line.empty())
		{
			continue;
		}
		if (line == "status")
		{
			PrintStatus();
			continue;
		}
		QueueLua(std::move(line));
	}
}

void ExecuteConsoleLua()
{
	std::deque<std::string> chunks;
	{
		auto const lock = std::lock_guard{g_luaMutex};
		chunks.swap(g_luaChunks);
	}
	for (auto const& chunk : chunks)
	{
		LG("console", "lua: %.1024s\n", chunk.c_str());
		if (luaL_loadbuffer(g_pLuaState, chunk.data(), chunk.size(), "=console") || lua_pcall(g_pLuaState, 0, 0, 0))
		{
			LG("console", "lua error: %.1024s\n", lua_tostring(g_pLuaState, -1));
			print_error(g_pLuaState);
		}
	}
}

void UpdateStatus(DWORD dwCurTick)
{
	static DWORD dwLastRefresh = dwCurTick - StatusRefreshInterval;
	static DWORD dwLastLog = dwCurTick - StatusLogInterval;
	if (dwCurTick - dwLastRefresh < StatusRefreshInterval)
	{
		return;
	}
	dwLastRefresh = dwCurTick;

	std::string status = FormatStatus();
	if (dwCurTick - dwLastLog >= StatusLogInterval)
	{
		dwLastLog = dwCurTick;
		LG("GameStatus", "%s\n", status.c_str());
	}

	auto const lock = std::lock_guard{g_statusMutex};
	g_status = std::move(status);
	g_statusTick = dwCurTick;
}
