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


using namespace std;
using namespace MinNet;

struct MinNetOverlapped : OVERLAPPED
{
	enum TYPE
	{
		RECV, SEND
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

class MinNetIOCP
{
public:
	MinNetIOCP();
	~MinNetIOCP();
	
	void ServerStart();
	void ServerLoop();

private:
	CRITICAL_SECTION user_list_section;
	list<MinNetUser *> user_list;
	DWORD WINAPI WorkThread(LPVOID arg);
	DWORD WINAPI AcceptThread(LPVOID arg);

	SOCKET listen_socket;

	CRITICAL_SECTION recvQ_section;
	queue<pair<MinNetPacket *, MinNetUser *>> recvQ;

	void StartRecv(MinNetUser * user);
	void EndRecv(MinNetRecvOverlapped * overlap, int len);

	void StartSend(MinNetUser * user, MinNetPacket * packet);
	void EndSend(MinNetSendOverlapped * overlap);

	HANDLE hPort = nullptr;
	HANDLE port = nullptr;	
};