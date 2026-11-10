#include "SocketApi.h"
#include "UdpManage.h"

#include <system_error>

CUdpManage::CUdpManage() {}

CUdpManage::~CUdpManage()
{
	Exit();
}

bool CUdpManage::Init( int nPort, ReceiveManageEvent evtReceive )
{
	_evtReceive = evtReceive;
	_nPort = nPort;
	_IsRun = true;

	if( !_Socket.Init( _nPort ) )
	{
		_Socket.Close();
		return false;
	}

	// 设置缓接收冲区大小
	dbc::net::SetOption( _Socket.GetSocket(), SOL_SOCKET, SO_RCVBUF, 1024 );

	try
	{
		_thread = std::thread( &CUdpManage::Run, this );
	}
	catch( const std::system_error& )
	{
		_Socket.Close();
		return false;
	}
	return true;
}

void CUdpManage::Run()
{
	while( _IsRun )
	{
		pollfd socket{ _Socket.GetSocket(), POLLIN, 0 };
		if( dbc::net::Poll( &socket, 1, 200 ) > 0 )
		{
			_Socket.Read( ReceiveEvent, this );
		}
	}
}

void CUdpManage::ReceiveEvent( CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len, void* pParam )
{
	CUdpManage* pThis = ((CUdpManage*)pParam);
	if( pThis->_evtReceive ) pThis->_evtReceive( pThis, pUdpServer, szClientIP, nClientPort, pData, len );
}

void CUdpManage::Exit()
{
	_IsRun = false;
	if( _thread.joinable() )
	{
		_thread.join();
	}
	_Socket.Close();
}
