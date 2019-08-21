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
		thread * hThread = new thread([&]() { TestThread(hPort); });
		if (hThread == NULL)
			return;
	}
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
			return;

		MinNetUser * user = new MinNet::MinNetUser();
		user_list.push_back(user);

		ZeroMemory(&(user->overlapped), sizeof(user->overlapped));

		user->sock = clientSocket;
		user->wsabuf.buf = user->recv_buffer;
		user->wsabuf.len = 1024;

		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(clientSocket, &(user->wsabuf), 1, &recvbytes, &flags, &(user->overlapped), NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("WSARecv error");
				return;
			}
			continue;
		}
	}
}

DWORD WINAPI MinNetIOCP::TestThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;
	DWORD cbTransferred;
	SOCKET clientSocket;
	MinNetUser * user = nullptr;

	while (true)
	{
		retval = GetQueuedCompletionStatus
		(
			hcp, 
			&cbTransferred, 
			(LPDWORD)&clientSocket, 
			(LPOVERLAPPED *)&user, 
			INFINITE
		);

		if (retval == 0 || cbTransferred == 0)
		{
			if (retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(user->sock, &(user->overlapped), &temp1, FALSE, &temp2);
				return 0;
			}

			EnterCriticalSection(&section);
			user_list.remove((MinNetUser*)user);
			LeaveCriticalSection(&section);

			closesocket(user->sock);
			printf("나감\n");
			delete user;
			continue;
		}

		if (user)
		{
			cout << "받은 데이터 : " << cbTransferred << endl;
			
			memcpy(&user->temporary_buffer[user->buffer_position], &user->recv_buffer, cbTransferred);
			user->buffer_position += cbTransferred;

			cout << user->buffer_position << endl;

			while (true)
			{
			
				MinNetPacket * packet = new MinNetPacket();
				int checked = packet->Parse(user->temporary_buffer, user->buffer_position);
				user->buffer_position -= checked;
				if (checked == 0)
				{
					break;
				}
				packet->user = user;
			}
		}

		ZeroMemory(&(user->overlapped), sizeof(user->overlapped));
		user->wsabuf.buf = user->recv_buffer;
		user->wsabuf.len = 1024;

		ZeroMemory(&(user->recv_buffer), sizeof(user->recv_buffer));

		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(user->sock, &(user->wsabuf), 1, &recvbytes, &flags, &(user->overlapped), NULL);

		if (retval == SOCKET_ERROR)
			if (WSAGetLastError() != WSA_IO_PENDING)
				return 0;

		_sleep(1000);

		continue;
	}
	return 0;
}
