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
#include <functional>
#include <typeinfo>

#pragma comment (lib, "mswsock.lib")

using namespace std;
using namespace MinNet;


class MinNetSpinLock
{
public:

	MinNetSpinLock();
	~MinNetSpinLock();

	void lock();
	void unlock();

private:
	
	CRITICAL_SECTION section;
};

template <class T>
class MinNetMemoryPool
{
public:

};

template <class T>
class MinNetObjectPool
{
public:
	~MinNetObjectPool()
	{
		while (!pool.empty())
		{
			T* obj = pool.front();
			pool.pop();
			if (destructor != nullptr)
				destructor(obj);
			delete obj;
		}
	}

	void SetConstructor(function<void(T *)> constructor)
	{
		this->constructor = constructor;
	}

	void SetDestructor(function<void(T *)> destructor)
	{
		this->destructor = destructor;
	}

	void SetOnPush(function<void(T *)> onPush)
	{
		this->onPush = onPush;
	}

	void AddObject(int size)
	{
		for (int i = 0; i < size; i++)
			push(CreateNewObject());
	}

	T* pop()
	{
		if (pool.empty())
		{// 풀이 비어있음
			cout << typeid(T).name() << " 풀의 객체가 고갈되어 새로운 객체를 생성합니다" << endl;
			return CreateNewObject();
		}
		else
		{
			T* obj = pool.front();
			spinLock.lock();
			pool.pop();
			spinLock.unlock();
			return obj;
		}
	}

	void push(T* obj)
	{
		if (onPush != nullptr)
		{
			onPush(obj);
		}

		spinLock.lock();
		pool.push(obj);
		spinLock.unlock();
	}

private:
	queue<T*> pool;
	function<void(T*)> constructor = nullptr;
	function<void(T*)> destructor = nullptr;
	function<void(T*)> onPush = nullptr;

	MinNetSpinLock spinLock;

	T* CreateNewObject()
	{
		T* obj = new T();

		if (constructor != nullptr)
			constructor(obj);

		return obj;
	}
};

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
	char buf[1024] = { '\0' };
	DWORD dwBytes;
};

struct MinNetCloseOverlapped : MinNetOverlapped
{
	SOCKET socket;
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

class MinNetObject {
public:

private:

};

class MinNetRoom
{
public:
	string name;


private:

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

	MinNetObjectPool<MinNetUser> user_pool;
	MinNetObjectPool<MinNetPacket> packet_pool;

	MinNetObjectPool<MinNetAcceptOverlapped> accept_overlapped_pool;
	MinNetObjectPool<MinNetCloseOverlapped> close_overlapped_pool;
	MinNetObjectPool<MinNetSendOverlapped> send_overlapped_pool;
	MinNetObjectPool<MinNetRecvOverlapped> recv_overlapped_pool;

	CRITICAL_SECTION user_list_section;
	MinNetSpinLock recvq_spin_lock;
	MinNetSpinLock user_list_spin_lock;
	list<MinNetUser *> user_list;
	DWORD WINAPI WorkThread(LPVOID arg);

	sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	SOCKET listen_socket;

	CRITICAL_SECTION recvQ_section;
	queue<pair<MinNetPacket *, MinNetUser *>> recvQ;

	void CreatePool();

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
