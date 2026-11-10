#pragma once

#include "SocketTypes.h"

#ifdef _WIN32

#include <ws2tcpip.h>

#define DBC_SOCKERR(name) WSA##name

#else

#include <cerrno>
#include <fcntl.h>
#include <poll.h>

#define DBC_SOCKERR(name) name

#endif

#include <cstddef>

namespace dbc::net
{
inline int Startup()
{
#ifdef _WIN32
	WSADATA data;
	if (::WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		return -1;
	}
	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)
	{
		::WSACleanup();
		return -1;
	}
#endif
	return 0;
}

inline void Cleanup()
{
#ifdef _WIN32
	::WSACleanup();
#endif
}

inline int LastError()
{
#ifdef _WIN32
	return ::WSAGetLastError();
#else
	return errno;
#endif
}

inline bool WouldBlock(int error)
{
#ifdef _WIN32
	return error == WSAEWOULDBLOCK;
#else
	return error == EWOULDBLOCK || error == EAGAIN;
#endif
}

inline bool IsNetworkUnusable(int error)
{
#ifdef _WIN32
	return error == WSAENETDOWN || error == WSANOTINITIALISED;
#else
	static_cast<void>(error);
	return false;
#endif
}

inline int CloseSocket(SOCKET socket)
{
#ifdef _WIN32
	return ::closesocket(socket);
#else
	return ::close(socket);
#endif
}

inline int SetNonBlocking(SOCKET socket)
{
#ifdef _WIN32
	u_long enable = 1;
	return ::ioctlsocket(socket, FIONBIO, &enable);
#else
	const int flags = ::fcntl(socket, F_GETFL, 0);
	return flags < 0 ? -1 : ::fcntl(socket, F_SETFL, flags | O_NONBLOCK);
#endif
}

inline int SetOption(SOCKET socket, int level, int name, int value)
{
	return ::setsockopt(socket, level, name, reinterpret_cast<const char*>(&value), sizeof(value));
}

inline int GetOption(SOCKET socket, int level, int name, int& value)
{
	socklen_t length = sizeof(value);
	return ::getsockopt(socket, level, name, reinterpret_cast<char*>(&value), &length);
}

inline int AllowAddressReuse(SOCKET socket)
{
#ifdef _WIN32
	static_cast<void>(socket);
	return 0;
#else
	return SetOption(socket, SOL_SOCKET, SO_REUSEADDR, 1);
#endif
}

inline int Send(SOCKET socket, const char* data, int length)
{
#ifdef _WIN32
	return ::send(socket, data, length, 0);
#else
	return static_cast<int>(::send(socket, data, static_cast<std::size_t>(length), MSG_NOSIGNAL));
#endif
}

inline int Poll(pollfd* sockets, std::size_t count, int timeoutMilliseconds)
{
#ifdef _WIN32
	return ::WSAPoll(sockets, static_cast<ULONG>(count), timeoutMilliseconds);
#else
	return ::poll(sockets, static_cast<nfds_t>(count), timeoutMilliseconds);
#endif
}

inline bool ResolveIPv4(const char* host, in_addr& address)
{
	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	addrinfo* found = nullptr;
	if (::getaddrinfo(host, nullptr, &hints, &found) != 0 || !found)
	{
		return false;
	}
	address = reinterpret_cast<sockaddr_in*>(found->ai_addr)->sin_addr;
	::freeaddrinfo(found);
	return true;
}
}
