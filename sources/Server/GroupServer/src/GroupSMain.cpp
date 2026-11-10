// GroupServerApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GroupServerApp.h"
#include "ConsoleInput.h"

#ifdef _WIN32
HANDLE hConsole = NULL;
#endif

pi_LeakReporter pi_leakReporter("groupememleak.log");
CResourceBundleManage g_ResourceBundleManage("GroupServer.loc"); //Add by alfred.shi 20080130

// binary compatible
int main(int argc, char* argv[])
{
#ifdef _WIN32
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	C_TITLE("GroupServer.exe")
	C_PRINT("Loading GroupServer.cfg...\n");

	//SEHTranslator translator;

	T_B

	TcpCommApp::WSAStartup();
	ThreadPool	*l_proc = ThreadPool::CreatePool(8,8,2048);
	ThreadPool	*l_comm = ThreadPool::CreatePool(4,4,512,THREAD_PRIORITY_ABOVE_NORMAL);

	try{
		g_gpsvr	=new GroupServerApp(l_proc,l_comm);
	}catch(...)
	{
		l_comm->DestroyPool();
		l_proc->DestroyPool();
		TcpCommApp::WSACleanup();
		return -1;
	}
	ConsoleInput console([] { return g_exit != 0; });
	std::string str;
	while(!g_exit)
	{
        std::cout<<RES_STRING(GP_MAIN_CPP_00001);
		if(!console.ReadLine(str) || str =="exit")
		{
			std::cout<<RES_STRING(GP_MAIN_CPP_00002)<<std::endl;
			break;
		}
		else if(str.empty())
		{
			continue;
		}
		else if(str =="logbak")
		{
			LogStream::Backup();
		}
		else
		{
			std::cout<<RES_STRING(GP_MAIN_CPP_00003)<<std::endl;
		}
	}
	//if(!g_exit)
	{
		g_exit	=1;
		while(g_ref)
		{
			Sleep(1);
		}
		delete g_gpsvr;

		l_comm->DestroyPool();
		l_proc->DestroyPool();
		TcpCommApp::WSACleanup();
		g_exit	=2;
		Sleep(2000);
	}
	while(g_exit !=2)
	{
		Sleep(1);
	}

	T_FINAL
	return 0;
}
