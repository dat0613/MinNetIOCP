#pragma once

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <process.h>
#include <thread>
#include "MinNet.h"

using namespace std;
using namespace MinNet;

class MinNetIOCP
{
public:
	MinNetIOCP();
	~MinNetIOCP();
	
	void ServerStart();

private:
	list<MinNetUser *> user_list;
	DWORD WINAPI TestThread(LPVOID arg);

	SOCKET listen_socket;

	CRITICAL_SECTION section;

	HANDLE hPort = nullptr;
	HANDLE port = nullptr;

};

