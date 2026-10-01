#pragma once

#include "UdpSocket.h"

#include <atomic>
#include <thread>

// 服务器监听用

class CUdpManage;
typedef void (*ReceiveManageEvent) ( CUdpManage* pManage, CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len );

class CUdpManage
{
public:
	CUdpManage();
	~CUdpManage();

	bool			Init( int nPort, ReceiveManageEvent evtReceive );
	void			Exit();

private:
	static  void	ReceiveEvent( CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len, void* pParam );

	void			Run();

private:
	int					_nPort{0};
	std::atomic<bool>	_IsRun{true};
	ReceiveManageEvent	_evtReceive{ nullptr };
	std::thread			_thread;
	CUdpServer			_Socket;

};
