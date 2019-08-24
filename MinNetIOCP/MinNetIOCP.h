#pragma once

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <process.h>
#include <thread>
#include "MinNet.h"
#include <queue>
#include <MSWSock.h>

#pragma comment (lib, "mswsock.lib")

using namespace std;
using namespace MinNet;

template <class T>
class MinNetMemoryPool
{
public:

};

struct MinNetOverlapped : OVERLAPPED
{
	enum TYPE
	{
		ACCEPT, CLOSE, RECV, SEND
	};

	TYPE type;
};

struct MinNetSendOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	MinNetPacket * packet;
	WSABUF wsabuf;
};

struct MinNetRecvOverlapped : MinNetOverlapped
{
	MinNetUser * user;
	WSABUF wsabuf;
};

struct MinNetAcceptOverlapped : MinNetOverlapped
{
	SOCKET socket;
	char buf[1024] = { '\0' };
	DWORD dwBytes;
};

struct MinNetCloseOverlapped : MinNetOverlapped
{
	SOCKET socket;
};

class MinNetIOCP
{
public:
	MinNetIOCP();
	~MinNetIOCP();
	
	void StartServer();
	void ServerLoop();

private:

	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID guidAcceptEx = WSAID_ACCEPTEX;

	CRITICAL_SECTION user_list_section;
	list<MinNetUser *> user_list;
	DWORD WINAPI WorkThread(LPVOID arg);

	sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	SOCKET listen_socket;

	CRITICAL_SECTION recvQ_section;
	queue<pair<MinNetPacket *, MinNetUser *>> recvQ;

	void StartAccept();
	void EndAccept(MinNetAcceptOverlapped * overlap);

	void StartClose(SOCKET socket);
	void EndClose(MinNetCloseOverlapped * overlap);

	void StartRecv(MinNetUser * user);
	void EndRecv(MinNetRecvOverlapped * overlap, int len);

	void StartSend(MinNetUser * user, MinNetPacket * packet);
	void EndSend(MinNetSendOverlapped * overlap);

	HANDLE hPort = nullptr;
	HANDLE port = nullptr;
};