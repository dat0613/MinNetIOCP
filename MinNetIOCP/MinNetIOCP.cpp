#include "MinNetIOCP.h"

MinNetIOCP::MinNetIOCP()
{
	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	//port = CreateIoCompletionPort(socket, hPort, (ULONG_PTR)session, 0);
	
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	DWORD Threadid;
	// (int)si.dwNumberOfProcessors * 2
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
	{
		//hThread = CreateThread(NULL, 0, TestThread, hPort, 0, &Threadid);
		thread * hThread = new thread([&]() { WorkThread(hPort); });
		if (hThread == NULL)
			return;
	}

	InitializeCriticalSection(&user_list_section);
	InitializeCriticalSection(&recvQ_section);
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

void MinNetIOCP::ServerStart()
{
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup error");
		return;
	}

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_socket == INVALID_SOCKET)
	{
		printf("socket error");
		return;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	serverAddr.sin_port = htons(8200);

	char iSockOpt = 1;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &iSockOpt, sizeof(iSockOpt));

	int retval = ::bind(listen_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
	{
		printf("bind error");
		return;
	}

	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		printf("listen error");
		return;
	}

	thread * hThread = new thread([&]() { AcceptThread(nullptr); });
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
			cout << "fps : " << fps << endl;
		}

		frame++;


		EnterCriticalSection(&recvQ_section);

		while (!recvQ.empty())
		{
			auto packet_info = recvQ.front();

			StartSend(packet_info.second, packet_info.first);

			recvQ.pop();
		}

		LeaveCriticalSection(&recvQ_section);



		_sleep(0);
	}
}

DWORD WINAPI MinNetIOCP::WorkThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;
	DWORD cbTransferred;
	SOCKET clientSocket;
	MinNetOverlapped * overlap;

	while (true)
	{
		retval = GetQueuedCompletionStatus
		(
			hcp, 
			&cbTransferred, 
			(LPDWORD)&clientSocket,
			(LPOVERLAPPED *)&overlap,
			INFINITE
		);

		if (retval == 0 || cbTransferred == 0)
		{
		//	//if (retval == 0)
		//	//{
		//	//	DWORD temp1, temp2;
		//	//	WSAGetOverlappedResult(user->sock, (LPWSAOVERLAPPED)&overlap, &temp1, FALSE, &temp2);
		//	//	printf("???");
		//	//	return 0;
		//	//}

		//	EnterCriticalSection(&user_list_section);
		//	//user_list.remove((MinNetUser*)user);
		//	LeaveCriticalSection(&user_list_section);

		//	closesocket(user->sock);
		//	printf("나감\n");
		//	delete user;
			continue;
		}

		switch (overlap->type)
		{
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

		//if (user != nullptr)
		//{
		//	EndRecv((MinNetRecvOverlapped *)overlap, cbTransferred);
		//	StartRecv(user);
		//}
		printf("탈출\n");


		_sleep(0);
		continue;
	}
	return 0;
}

DWORD WINAPI MinNetIOCP::AcceptThread(LPVOID arg)
{
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrlen = sizeof(clientAddr);
		SOCKET clientSocket = accept(listen_socket, (SOCKADDR *)&clientAddr, &addrlen);
		if (clientSocket == INVALID_SOCKET)
			continue;
		printf("대충 클라이언트 접속 했다는 내용\n");
		printf("[ %s : %d ]\n\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		HANDLE hResult = CreateIoCompletionPort((HANDLE)clientSocket, hPort, (DWORD)clientSocket, 0);
		if (hResult == NULL)
			return 0;

		MinNetUser * user = new MinNet::MinNetUser();

		cout << "새로운 유저 : " << user << endl;

		EnterCriticalSection(&user_list_section);
		user_list.push_back(user);
		LeaveCriticalSection(&user_list_section);

		user->sock = clientSocket;

		StartRecv(user);

		_sleep(0);
	}

	return 0;
}

void MinNetIOCP::StartRecv(MinNetUser * user)
{
	MinNetRecvOverlapped * overlap = new MinNetRecvOverlapped();
	//ZeroMemory(overlap, sizeof(MinNetRecvOverlapped));
	overlap->type = MinNetOverlapped::TYPE::RECV;
	overlap->user = user;
	char * buf = new char[1024];
	overlap->wsabuf.buf = buf;
	ZeroMemory(overlap->wsabuf.buf, 1024);
	overlap->wsabuf.len = 1024;

	DWORD recvbytes;
	DWORD flags = 0;
	int retval = WSARecv(user->sock, &overlap->wsabuf, 1, &recvbytes, &flags, overlap, NULL);

	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "에러" << endl;
			return;
		}

}

void MinNetIOCP::EndRecv(MinNetRecvOverlapped * overlap, int len)
{
	if (overlap->user == nullptr)
		return;

	MinNetUser * user = overlap->user;

	memcpy(&user->temporary_buffer[user->buffer_position], overlap->wsabuf.buf, len);
	user->buffer_position += len;

	cout << user->buffer_position << endl;

	while (true)
	{

		MinNetPacket * packet = new MinNetPacket();
		int checked = packet->Parse(user->temporary_buffer, user->buffer_position);
		user->buffer_position -= checked;

		if (checked == 0)// 패킷을 완성하지 못함
		{
			delete packet;// 없앰
			break;
		}

		EnterCriticalSection(&recvQ_section);
		recvQ.push(make_pair(packet, user));
		LeaveCriticalSection(&recvQ_section);
	}

	delete[] overlap->wsabuf.buf;
	delete overlap;
}

void MinNetIOCP::StartSend(MinNetUser * user, MinNetPacket * packet)
{
	if (user == nullptr)
		return;

	MinNetSendOverlapped * overlap = new MinNetSendOverlapped();
	ZeroMemory(overlap, sizeof(MinNetSendOverlapped));
	overlap->type = MinNetOverlapped::TYPE::SEND;
	overlap->packet = packet;
	overlap->wsabuf.buf = (char *)packet->buffer;
	overlap->wsabuf.len = packet->size();

	int retval = WSASend(user->sock, &overlap->wsabuf, 1, nullptr, 0, overlap, nullptr);
	cout << retval << endl;
	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
			return;
}

void MinNetIOCP::EndSend(MinNetSendOverlapped * overlap)
{
	delete overlap->packet;
	delete overlap;
}