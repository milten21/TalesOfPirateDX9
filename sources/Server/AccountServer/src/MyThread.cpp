#include "stdafx.h"
#include <stdlib.h>

#include "MyThread.h"

#include <system_error>


MyThread::MyThread() : m_bExitFlag(false), m_bExitThrd(true), m_nExitCode(0)
{
}

MyThread::~MyThread()
{
	NotifyToExit();
	WaitForExit(-1);
}

bool MyThread::Launch()
{
	if (m_Thread.joinable()) return false;

	m_bExitFlag = false;
	m_bExitThrd = false;
	try
	{
		m_Thread = std::thread([this] { m_nExitCode = Run(); });
	}
	catch (const std::system_error&)
	{
		m_bExitThrd = true;
		return false;
	}
	return true;
}

void MyThread::NotifyToExit()
{
	m_bExitFlag = true;
}

int MyThread::WaitForExit(int ms)
{
	while (!m_bExitThrd) {
		Sleep(1000);
	}

	if (m_Thread.joinable())
	{
		m_Thread.join();
	}
	return m_nExitCode;
}

int MyThread::Run()
{
	ExitThread();
	return 0;
}