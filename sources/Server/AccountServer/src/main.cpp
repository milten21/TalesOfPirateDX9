// 经分析,此项目结构存在严重问题,有多处地方欠缺线程同步考虑,建议重写 - by Arcol

// main.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "stdafx.h"
#include "AccountServer2.h"
#include <signal.h>
#include <chrono>
#include <cstdio>
#include <string>
#include "GlobalVariable.h"

#include "IniFile.h"

ThreadPool* comm = NULL;
ThreadPool* proc = NULL;
AuthThreadPool atp;

//#pragma init_seg( lib )
pi_LeakReporter pi_leakReporter("accountmemleak.log");
CResourceBundleManage  g_ResourceBundleManage("AccountServer.loc");	//Add by lark.li 20080330

namespace
{
constexpr auto QueueWait = std::chrono::milliseconds(200);
constexpr auto StatusInterval = std::chrono::seconds(60);
}

void Ctrlc_Dispatch(int sig)
{
	g_MainQueue.RequestClose();
	signal(sig, Ctrlc_Dispatch);
}

void DispatchMainMessage(const MainMessage& message)
{
	const sUserLog& userLog = message.log;
	switch (message.type)
	{
	case MainMessageType::UserLog:
		if (userLog.bLogin)
		{
			g_MainDBHandle.UserLogin(userLog.nUserID, userLog.strUserName, userLog.strLoginIP);
		}
		else
		{
			g_MainDBHandle.UserLogout(userLog.nUserID);
		}
		break;

	case MainMessageType::UserLogMap:
		if (userLog.bLogin)
		{
			g_MainDBHandle.UserLoginMap(userLog.strUserName, "Enter_Map");
		}
		else
		{
			g_MainDBHandle.UserLogoutMap(userLog.strUserName);
		}
		break;
	}
}

void LogStatus()
{
	std::string threads;
	char buf[64];
	for (int i = 0; i < AuthThreadPool::AT_MAXNUM; ++i)
	{
		snprintf(buf, sizeof(buf), " #%02d %02d %04lu/%04lu", i + 1, AuthThreadPool::RunLabel[i],
			static_cast<unsigned long>(AuthThreadPool::RunLast[i]), static_cast<unsigned long>(AuthThreadPool::RunConsume[i]));
		threads += buf;
	}
	LG("AccountStatus", "queued packets %lu, concurrent tasks %ld, auth thread label last/max ms:%s\n",
		static_cast<unsigned long>(g_Auth.GetPkTotal()), static_cast<long>(proc->GetTaskCount()), threads.c_str());
	g_As2->LogGroups("AccountStatus");
}

void RunMainQueue()
{
	auto nextStatus = std::chrono::steady_clock::now() + StatusInterval;
	while (!g_MainQueue.IsCloseRequested())
	{
		if (auto message = g_MainQueue.Wait(QueueWait))
		{
			DispatchMainMessage(*message);
		}

		const auto now = std::chrono::steady_clock::now();
		if (now >= nextStatus)
		{
			LogStatus();
			nextStatus = now + StatusInterval;
		}
	}
	while (auto message = g_MainQueue.Wait(std::chrono::milliseconds(0)))
	{
		DispatchMainMessage(*message);
	}
}

#ifdef _WIN32
HANDLE hConsole = NULL;
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	C_TITLE("AccountServer.exe")
	C_PRINT("Loading AccountServer.cfg...\n");

	SEHTranslator translator;

	T_B

#ifdef _WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	signal(SIGINT, Ctrlc_Dispatch);
	signal(SIGTERM, Ctrlc_Dispatch);

	if (!g_MainDBHandle.CreateObject())
	{
		C_PRINT("failed\n");
		printf(RES_STRING(AS_MAIN_CPP_00003));
		LG("AccountServer", "%s", RES_STRING(AS_MAIN_CPP_00003));
		return -1;
	}

    // 启动认证计费线程
    atp.Launch();

    // 初始化网络
    TcpCommApp::WSAStartup();
    comm = ThreadPool::CreatePool(10, 10, 256);

    int nProcCnt = 2 * AuthThreadPool::AT_MAXNUM;
    proc = ThreadPool::CreatePool(nProcCnt, nProcCnt, 2048);
    try {
        g_As2 = new AccountServer2(proc, comm);
    } catch (excp& e) {
        printf("%s\n", e.what());
        LG("AccountServer", "%s\n", e.what());
        comm->DestroyPool();
        TcpCommApp::WSACleanup();
        return -1;
    } catch (...) {
        //printf("AccountServer初始化期间发生未知错误，请通知开发者\n");
   		printf(RES_STRING(AS_MAIN_CPP_00006));
		LG("AccountServer", "%s", RES_STRING(AS_MAIN_CPP_00006));
		comm->DestroyPool();
        TcpCommApp::WSACleanup();
        return -2;
    }

    RunMainQueue();
	C_PRINT(RES_STRING(AS_MAIN_CPP_00007));

    // 卸载网络
    delete g_As2;
    if (comm != NULL) comm->DestroyPool();
    if (proc != NULL) proc->DestroyPool();
    TcpCommApp::WSACleanup();

    // 认证、计费线程退出
    atp.NotifyToExit();

    atp.WaitForExit();

    // 记录最后状态
    LG("RunLabel", "\n");
    for (char i = 0; i < AuthThreadPool::AT_MAXNUM; ++ i) {
        LG("RunLabel", "%02d %04d\n", AuthThreadPool::RunLabel[i],
            AuthThreadPool::RunConsume[i]);
    }
    LG("RunLabel", "\n");

	g_MainDBHandle.ReleaseObject();

	T_FINAL

    return 0;
}
