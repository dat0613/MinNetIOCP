#pragma once

#include "MinNet.h"

class MinNetUser;
class MinNetPacket;

struct MinNetOverlapped : OVERLAPPED
{
	enum TYPE
	{
		ACCEPT, CLOSE, RECV, SEND
	};

	TYPE type;
};

struct MinNetAcceptOverlapped : MinNetOverlapped
{
	SOCKET socket;
	char buf[Defines::BUFFERSIZE] = { '\0' };
	DWORD dwBytes;
};

struct MinNetCloseOverlapped : MinNetOverlapped
{
	MinNetUser * user;
};
struct MinNetSendOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	WSABUF wsabuf;
	bool isTcp;
};

struct MinNetRecvOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	WSABUF wsabuf;
	SOCKADDR_IN addr;

	bool isTcp;
};
