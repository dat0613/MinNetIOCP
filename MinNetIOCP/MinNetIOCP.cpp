#include "MinNetIOCP.h"

#include "MinNet.h"
#include "MinNetPool.h"
#include "MinNetPool.h"

MinNetIOCP::MinNetIOCP() : room_manager(this)
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

void MinNetIOCP::SetTickrate(int tick)
{
	this->tick = tick;
}

void MinNetIOCP::StartServer()
{	
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
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
	serverAddr.sin_port = htons(8300);

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

	MinNetPool::Init();

	StartAccept();
}

void MinNetIOCP::ServerLoop()
{
	float sleep_time = 1000.0f / (float)tick;
	clock_t last_heart_beat = 0;
	clock_t cur_time = 0;

	while (true)
	{
		cur_time = clock();

		if (cur_time - last_heart_beat > 3000)
		{
			PingTest();
			last_heart_beat = cur_time;
		}

		messageQ_spin_lock.lock();

		while (!recvQ.empty())
		{
			auto packet_info = recvQ.front();
			auto packet = packet_info.first;
			auto user = packet_info.second;

			room_manager.PacketHandler(user, packet);

			MinNetPool::packetPool->push(packet);

			recvQ.pop();
		}

		messageQ_spin_lock.unlock();

		_sleep(sleep_time);
	}
}


string MinNetIOCP::GetIP()
{
	char name[255];
	char *ip;
	PHOSTENT host;

	if (gethostname(name, sizeof(name)) == 0)
	{
		if ((host = gethostbyname(name)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
		}
	}

	return ip;
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
				EndRecv((MinNetRecvOverlapped *)overlap, cbTransferred);
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
				StartClose(((MinNetRecvOverlapped *)overlap)->user);
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

void MinNetIOCP::PacketHandler(MinNetUser * user, MinNetPacket * packet)
{
}

void MinNetIOCP::JoinPeacefulRoom(MinNetUser * user)
{
	user->ChangeRoom(room_manager.GetPeacefulRoom());
}

void MinNetIOCP::PingTest()
{
	queue<MinNetUser *> removeQ;
	if (user_list.size() > 0)
	{
		for (auto it = user_list.begin(); it != user_list.end(); it++)
		{
			MinNetUser * user = *it;

			if (user->last_ping != -1)
			{
				if (user->ping > 500)
				{
					cout << user << " 이 응답하지 않아 연결 끊음" << endl;
					removeQ.push(user);
				}
				else
				{
					SendPing(user);
				}
			}
			else
			{
				SendPing(user);
			}
		}
	}

	while (!removeQ.empty())
	{
		MinNetUser * user = removeQ.front();
		user->ChangeRoom(nullptr);
		user_list.remove(user);
		StartClose(user);
		removeQ.pop();
	}
}

void MinNetIOCP::SendPing(MinNetUser * user)
{
	MinNetPacket * packet = MinNetPool::packetPool->pop();

	user->last_ping = clock();

	packet->create_packet(Defines::MinNetPacketType::PING);
	packet->create_header();

	StartSend(user, packet);

	MinNetPool::packetPool->push(packet);
}

void MinNetIOCP::StartAccept()
{
	MinNetAcceptOverlapped * overlap = MinNetPool::acceptOverlappedPool->pop();

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

		MinNetUser * user = MinNetPool::userPool->pop();
		user->sock = overlap->socket;

		StartClose(user);
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

	MinNetUser * user = MinNetPool::userPool->pop();
	user->sock = overlap->socket;

	user_list.push_back(user);

	user->isConnected = true;

	MinNetPacket * packet = MinNetPool::packetPool->pop();
	packet->create_packet(Defines::MinNetPacketType::USER_ENTER_ROOM);
	packet->create_header();

	messageQ_spin_lock.lock();
	recvQ.push(make_pair(packet, user));
	messageQ_spin_lock.unlock();

	MinNetPool::acceptOverlappedPool->push(overlap);

	StartRecv(user);

	StartAccept();
}

void MinNetIOCP::StartClose(MinNetUser * user)
{
	MinNetCloseOverlapped * overlap = MinNetPool::closeOverlappedPool->pop();
	overlap->user = user;

	if (TransmitFile(overlap->user->sock, 0, 0, 0, overlap, 0, TF_DISCONNECT | TF_REUSE_SOCKET) == FALSE)
	{
		DWORD error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			if (error == WSAENOTCONN || error == WSAENOTSOCK)
			{
				cout << "이미 닫힌 소켓임 : " << overlap->user->sock << endl;
				MinNetPool::closeOverlappedPool->push(overlap);
				return;
			}
			cout << "Transmitfile error : " << error << endl;
		}
	}
	else
	{
		cout << "성공적으로 호출 함" << endl;
	}
}

void MinNetIOCP::EndClose(MinNetCloseOverlapped * overlap)
{
	cout << "유저가 나감 : " << overlap->user->sock << endl;

	user_list.remove(overlap->user);
	MinNetPool::userPool->push(overlap->user);

	MinNetPool::closeOverlappedPool->push(overlap);
}

void MinNetIOCP::StartRecv(MinNetUser * user)
{
	MinNetRecvOverlapped * overlap = MinNetPool::recvOverlappedPool->pop();

	overlap->user = user;

	DWORD recvbytes;
	DWORD flags = 0;
	int retval = WSARecv(user->sock, &overlap->wsabuf, 1, &recvbytes, &flags, overlap, NULL);

	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			StartClose(user);
			return;
		}
	}
}

void MinNetIOCP::EndRecv(MinNetRecvOverlapped * overlap, int len)
{
	if (overlap->user == nullptr)
		return;

	if (len == 0)
	{
		StartClose(overlap->user);
		return;
	}

	MinNetUser * user = overlap->user;

	memcpy(&user->temporary_buffer[user->buffer_position], overlap->wsabuf.buf, len);
	user->buffer_position += len;

	while (true)
	{

		MinNetPacket * packet = MinNetPool::packetPool->pop();

		int checked = packet->Parse(user->temporary_buffer, user->buffer_position);
		user->buffer_position -= checked;

		if (checked == 0)// 패킷을 완성하지 못함
		{
			MinNetPool::packetPool->push(packet);
			break;
		}

		if (packet->packet_type == Defines::MinNetPacketType::PONG)
			OnPong(user, packet);
		else
		{
			messageQ_spin_lock.lock();
			recvQ.push(make_pair(packet, user));
			messageQ_spin_lock.unlock();
		}
	}

	StartRecv(user);

	MinNetPool::recvOverlappedPool->push(overlap);
}

void MinNetIOCP::StartSend(MinNetUser * user, MinNetPacket * packet)
{
	if (user == nullptr)
		return;

	MinNetSendOverlapped * overlap = MinNetPool::sendOverlappedPool->pop();
	memcpy(overlap->wsabuf.buf, packet->buffer, packet->size());
	overlap->wsabuf.len = packet->size();

	int retval = WSASend(user->sock, &overlap->wsabuf, 1, nullptr, 0, overlap, nullptr);

	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			StartClose(user);
			return;
		}
}

void MinNetIOCP::EndSend(MinNetSendOverlapped * overlap)
{
	MinNetPool::sendOverlappedPool->push(overlap);
}

void MinNetIOCP::OnPong(MinNetUser * user, MinNetPacket * packet)
{
	user->last_pong = clock();
	user->ping = user->last_pong - user->last_ping;
	MinNetPool::packetPool->push(packet);

	MinNetPacket * cast = MinNetPool::packetPool->pop();

	cast->create_packet(Defines::MinNetPacketType::PING_CAST);
	cast->push(user->ping);
	cast->create_header();

	StartSend(user, cast);

	MinNetPool::packetPool->push(cast);
}

void MinNetSpinLock::lock()
{
	while (locker.test_and_set(memory_order_acquire));
}

void MinNetSpinLock::unlock()
{
	locker.clear(memory_order_release);
}