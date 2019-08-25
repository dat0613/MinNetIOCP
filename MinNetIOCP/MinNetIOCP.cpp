#include "MinNetIOCP.h"

MinNetIOCP::MinNetIOCP()
{
}

MinNetIOCP::~MinNetIOCP()
{
	closesocket(listen_socket);
	
	auto it = user_list.begin();

	while (it != user_list.end())
	{
		auto user = *it;
		closesocket(user->sock);
		user_list.remove(user);
		it++;
	}
	user_list.clear();
}

void MinNetIOCP::StartServer()
{
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup error");
		return;
	}

	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hPort == nullptr)
	{
		cout << "CreateIoCompletionPort error" << endl;
		return;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
	{
		thread * hThread = new thread([&]() { WorkThread(hPort); });
		if (hThread == NULL)
			return;
	}

	listen_socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_socket == INVALID_SOCKET)
	{
		printf("WSASocket error");
		return;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	serverAddr.sin_port = htons(8200);

	char iSockOpt = 1;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &iSockOpt, sizeof(iSockOpt));

	CreateIoCompletionPort((HANDLE)listen_socket, hPort, listen_socket, 0);

	BOOL on = TRUE;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char *)&on, sizeof(on)))
		return;


	int retval = ::bind(listen_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
	{
		printf("bind error");
		return;
	}

	DWORD dwBytes;
	retval = WSAIoctl(listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL);
	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSAIoctl error");
			return;
		}
	
	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		printf("listen error");
		return;
	}

	CreatePool();

	StartAccept();
}

void MinNetIOCP::ServerLoop()
{
	int fps = 0;
	int frame = 0;
	int lasttime = 0;
	while (true)
	{
		if (lasttime != time(NULL))
		{
			fps = frame;
			frame = 0;
			lasttime = time(NULL);
		}

		frame++;

		recvq_spin_lock.lock();

		while (!recvQ.empty())
		{
			auto packet_info = recvQ.front();

			StartSend(packet_info.second, packet_info.first);

			recvQ.pop();
		}

		recvq_spin_lock.unlock();



		_sleep(0);
	}
}

DWORD WINAPI MinNetIOCP::WorkThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;
	DWORD cbTransferred;
	SOCKET sock;
	MinNetOverlapped * overlap;

	while (true)
	{
		retval = GetQueuedCompletionStatus
		(
			hcp, 
			&cbTransferred, 
			(LPDWORD)&sock,
			(LPOVERLAPPED *)&overlap,
			INFINITE
		);

		//if (retval == 0 || cbTransferred == 0)
		//	continue;

		if (retval > 0)
		{
			switch (overlap->type)
			{
			case MinNetOverlapped::ACCEPT:
				EndAccept((MinNetAcceptOverlapped *)overlap);
				break;

			case MinNetOverlapped::CLOSE:
				EndClose((MinNetCloseOverlapped *)overlap);
				break;

			case MinNetOverlapped::RECV:
				MinNetUser * user;
				user = ((MinNetRecvOverlapped *)overlap)->user;
				EndRecv((MinNetRecvOverlapped *)overlap, cbTransferred);
				StartRecv(user);
				break;
		
			case MinNetOverlapped::SEND:
				EndSend((MinNetSendOverlapped *)overlap);
				break;
			}
		}
		else
		{
			if (cbTransferred == 0)
			{
				StartClose(sock);
			}
		}
		_sleep(0);
	}
	return 0;
}

sockaddr_in * MinNetIOCP::SOCKADDRtoSOCKADDR_IN(sockaddr * addr)
{
	if (addr->sa_family == AF_INET)
	{
		sockaddr_in * sin = reinterpret_cast<sockaddr_in *>(addr);
		return sin;
	}
	return nullptr;
}

void MinNetIOCP::CreatePool()
{
	user_pool.AddObject(100);

	accept_overlapped_pool.SetOnPush(
		[](MinNetAcceptOverlapped * overlap)
		{
			ZeroMemory(overlap, sizeof(MinNetAcceptOverlapped));
			overlap->type = MinNetOverlapped::TYPE::ACCEPT;
			overlap->socket = INVALID_SOCKET;
			overlap->socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		}
	);
	accept_overlapped_pool.AddObject(20);

	close_overlapped_pool.SetOnPush(
		[](MinNetCloseOverlapped * overlap)
		{
			ZeroMemory(overlap, sizeof(MinNetCloseOverlapped));
			overlap->type = MinNetOverlapped::TYPE::CLOSE;
		}
	);
	close_overlapped_pool.AddObject(20);

	send_overlapped_pool.SetOnPush(
		[](MinNetSendOverlapped * overlap)
		{
			ZeroMemory(overlap, sizeof(MinNetSendOverlapped));
			overlap->type = MinNetOverlapped::TYPE::SEND;
		}
	);
	send_overlapped_pool.AddObject(100);

	recv_overlapped_pool.SetConstructor(
		[](MinNetRecvOverlapped * overlap)
		{
			overlap->wsabuf.buf = new char[1024];
			overlap->wsabuf.len = 1024;
		}
	);
	recv_overlapped_pool.SetDestructor(
		[](MinNetRecvOverlapped * overlap)
		{
			delete[] overlap->wsabuf.buf;
		}
	);
	recv_overlapped_pool.SetOnPush(
		[](MinNetRecvOverlapped * overlap)
		{
			ZeroMemory(overlap, sizeof(MinNetOverlapped));
			overlap->type = MinNetOverlapped::TYPE::RECV;

			ZeroMemory(overlap->wsabuf.buf, 1024);
			overlap->wsabuf.len = 1024;
		}
	);
	recv_overlapped_pool.AddObject(100);

	packet_pool.SetOnPush(
		[](MinNetPacket * packet)
		{
			ZeroMemory(packet->buffer, 1024);
		}
	);
	packet_pool.AddObject(0);
}

void MinNetIOCP::StartAccept()
{
	MinNetAcceptOverlapped * overlap = accept_overlapped_pool.pop();

	bool error = !lpfnAcceptEx
	(
		this->listen_socket,
		overlap->socket,
		(LPVOID)&overlap->buf,
		0,
		sizeof(SOCKADDR_IN) + 16, 
		sizeof(SOCKADDR_IN) + 16,
		&overlap->dwBytes, 
		overlap
	);

	if (error)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "accept 실패 : " << WSAGetLastError() << endl;
			return;
		}
}

void MinNetIOCP::EndAccept(MinNetAcceptOverlapped * overlap)
{
	if (setsockopt(overlap->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&listen_socket, sizeof(SOCKET)) == SOCKET_ERROR)
	{
		// 대충 오류니까 overlap->socket 닫음
		cout << "EndAccept error" << endl;
		StartClose(overlap->socket);
		return;
	}
	
	//SOCKADDR *local_addr, *remote_addr;
	//int l_len = 0, r_len = 0;
	//GetAcceptExSockaddrs
	//(
	//	(LPVOID)&overlap->buf,
	//	(sizeof(SOCKADDR_IN) + 16) * 2,
	//	sizeof(SOCKADDR_IN) + 16,
	//	sizeof(SOCKADDR_IN) + 16,
	//	&local_addr,
	//	&l_len,
	//	&remote_addr,
	//	&r_len
	//);

	//sockaddr_in * sin = SOCKADDRtoSOCKADDR_IN(remote_addr);

	//cout << inet_ntoa(sin->sin_addr) << endl;

	cout << "새로운 유저 접속 : " << overlap->socket << endl;

	HANDLE hResult = CreateIoCompletionPort((HANDLE)overlap->socket, hPort, (DWORD)overlap->socket, 0);
	if (hResult == NULL)
		return;

	MinNetUser * user = user_pool.pop();
	user->sock = overlap->socket;
	user->isConnected = true;

	user_list_spin_lock.lock();
	user_list.push_back(user);
	user_list_spin_lock.unlock();

	accept_overlapped_pool.push(overlap);

	StartRecv(user);
	StartAccept();
}

void MinNetIOCP::StartClose(SOCKET socket)
{
	//if (!user->isConnected)
	//	return;

	//user->isConnected = false;

	MinNetCloseOverlapped * overlap = close_overlapped_pool.pop();
	overlap->socket = socket;


	if (TransmitFile(overlap->socket, 0, 0, 0, overlap, 0, TF_DISCONNECT | TF_REUSE_SOCKET) == FALSE)
	{
		DWORD error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			if (error == WSAENOTCONN)
			{
				cout << "이미 닫힌 소켓임 : " << overlap->socket << endl;
				return;
			}
			cout << "Transmitfile error : " << error << endl;
			return;
		}
	}
	else
	{
		cout << "성공적으로 호출 함" << endl;
	}
}

void MinNetIOCP::EndClose(MinNetCloseOverlapped * overlap)
{
	user_list_spin_lock.lock();

	// 대충 유저 지우는 코드
	user_list_spin_lock.unlock();

	close_overlapped_pool.push(overlap);
}

void MinNetIOCP::StartRecv(MinNetUser * user)
{
	MinNetRecvOverlapped * overlap = recv_overlapped_pool.pop();

	overlap->user = user;

	DWORD recvbytes;
	DWORD flags = 0;
	int retval = WSARecv(user->sock, &overlap->wsabuf, 1, &recvbytes, &flags, overlap, NULL);

	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			StartClose(user->sock);
			return;
		}
}

void MinNetIOCP::EndRecv(MinNetRecvOverlapped * overlap, int len)
{
	if (overlap->user == nullptr)
		return;

	if(len == 0)
	{
		StartClose(overlap->user->sock);
		return;
	}

	MinNetUser * user = overlap->user;

	memcpy(&user->temporary_buffer[user->buffer_position], overlap->wsabuf.buf, len);
	user->buffer_position += len;

	while (true)
	{

		MinNetPacket * packet = packet_pool.pop();
		int checked = packet->Parse(user->temporary_buffer, user->buffer_position);
		user->buffer_position -= checked;

		if (checked == 0)// 패킷을 완성하지 못함
		{
			packet_pool.push(packet);// 없앰
			break;
		}

		recvq_spin_lock.lock();
		recvQ.push(make_pair(packet, user));
		recvq_spin_lock.unlock();

	}
	recv_overlapped_pool.push(overlap);
}

void MinNetIOCP::StartSend(MinNetUser * user, MinNetPacket * packet)
{
	if (user == nullptr)
		return;

	MinNetSendOverlapped * overlap = send_overlapped_pool.pop();
	overlap->packet = packet;
	overlap->wsabuf.buf = (char *)packet->buffer;
	overlap->wsabuf.len = packet->size();

	int retval = WSASend(user->sock, &overlap->wsabuf, 1, nullptr, 0, overlap, nullptr);

	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			StartClose(user->sock);
			return;
		}

	float asdf = 10;
	
}

void MinNetIOCP::EndSend(MinNetSendOverlapped * overlap)
{
	packet_pool.push(overlap->packet);
	send_overlapped_pool.push(overlap);
}

MinNetSpinLock::MinNetSpinLock()
{
	InitializeCriticalSection(&section);
}

MinNetSpinLock::~MinNetSpinLock()
{
	DeleteCriticalSection(&section);
}

void MinNetSpinLock::lock()
{
	while (TryEnterCriticalSection(&section) == FALSE)
		_sleep(0);
}

void MinNetSpinLock::unlock()
{
	LeaveCriticalSection(&section);
}
