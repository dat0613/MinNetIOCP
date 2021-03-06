#include "MinNetIOCP.h"
#include "MinNetMySQL.h"

#include "MinNet.h"
#include "MinNetPool.h"
#include "MinNetPool.h"
#include "MinNetRoom.h"
#include "MinNetTime.h"
#include "Debug.h"
#include "MinNetOptimizer.h"

LPFN_ACCEPTEX MinNetIOCP::lpfnAcceptEx = NULL;
GUID MinNetIOCP::guidAcceptEx = WSAID_ACCEPTEX;

LPFN_GETACCEPTEXSOCKADDRS MinNetIOCP::lpfnGetAcceptExSockaddrs = NULL;
GUID MinNetIOCP::guidGetAcceptSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

HANDLE MinNetIOCP::hPort = nullptr;
HANDLE MinNetIOCP::port = nullptr;

int MinNetIOCP::tick = 60;

MinNetRoomManager MinNetIOCP::room_manager;

MinNetSpinLock MinNetIOCP::messageQ_spin_lock;
MinNetSpinLock MinNetIOCP::userAddrSpinLock;


std::list<MinNetUser *> MinNetIOCP::user_list = std::list<MinNetUser *>();
std::map<int, MinNetUser *> MinNetIOCP::user_map = std::map<int, MinNetUser *>();

MinNetSpinLock MinNetIOCP::userSpinLock;

std::unordered_map<SOCKADDR_IN, MinNetUser *, KeyHash, KeyEqual> MinNetIOCP::userAddrMap = std::unordered_map<SOCKADDR_IN, MinNetUser *, KeyHash, KeyEqual>();

SOCKET MinNetIOCP::tcpSocket = SOCKET();
SOCKET MinNetIOCP::udpSocket = SOCKET();

int MinNetIOCP::userIDCount = 0;
MinNetSpinLock MinNetIOCP::idCountLock;


std::queue<std::pair<MinNetPacket *, MinNetUser *>> MinNetIOCP::recvQ = std::queue<std::pair<MinNetPacket *, MinNetUser *>>();
std::queue<std::pair<MinNetPacket *, MinNetUser *>> MinNetIOCP::messageQ = std::queue<std::pair<MinNetPacket *, MinNetUser *>>();

MinNetIOCP::MinNetIOCP()
{

}

MinNetIOCP::~MinNetIOCP()
{
	closesocket(tcpSocket);

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
	MinNetIOCP::tick = tick;
}

void MinNetIOCP::StartServer(USHORT tcpPort, USHORT udpPort)// 서버 시작
{	
	srand(static_cast<unsigned int>(time(NULL)));

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		Debug::Log("WSAStartup error");
		return;
	}

	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hPort == nullptr)
	{
		Debug::Log("CreateIoCompletionPort error");
		
		return;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
	{
		std::thread * hThread = new std::thread([&]() { WorkThread(hPort); });
		if (hThread == NULL)
			return;
	}

	tcpSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (tcpSocket == INVALID_SOCKET)
	{
		Debug::Log("WSASocket error");
		return;
	}

	SOCKADDR_IN udpServerAddr;
	ZeroMemory(&udpServerAddr, sizeof(udpServerAddr));
	udpServerAddr.sin_family = AF_INET;
	udpServerAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	udpServerAddr.sin_port = htons(udpPort);

	udpSocket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (bind(udpSocket, (sockaddr*)&udpServerAddr, sizeof(udpServerAddr)) == SOCKET_ERROR)
	{
		Debug::Log("bind error");
		return;
	}
	
	char iSockOpt = 1;
	setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &iSockOpt, sizeof(iSockOpt));

	BOOL on = TRUE;
	if (setsockopt(tcpSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char *)&on, sizeof(on)))
		return;

	SOCKADDR_IN tcpServerAddr;
	ZeroMemory(&tcpServerAddr, sizeof(tcpServerAddr));
	tcpServerAddr.sin_family = AF_INET;
	tcpServerAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	tcpServerAddr.sin_port = htons(tcpPort);

	int retval = bind(tcpSocket, (sockaddr*)&tcpServerAddr, sizeof(tcpServerAddr));
	if (retval == SOCKET_ERROR)
	{
		Debug::Log("bind error");
		return;
	}

	CreateIoCompletionPort((HANDLE)tcpSocket, hPort, tcpSocket, 0);
	CreateIoCompletionPort((HANDLE)udpSocket, hPort, udpSocket, 0);

	DWORD dwBytes;
	retval = WSAIoctl(tcpSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL);
	if (retval == SOCKET_ERROR)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Debug::Log("WSAIoctl error");
			return;
		}

	retval = WSAIoctl(tcpSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptSockAddrs, sizeof(guidGetAcceptSockAddrs), &lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs), &dwBytes, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Debug::Log("WSAIoctl error");
			return;
		}
	}

	retval = listen(tcpSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		Debug::Log("listen error");
		return;
	}

	MinNetPool::Init();

	StartAccept();

	Debug::Log("서버 시작");
	Debug::Log("IP ",GetIP().c_str());

	room_manager.GetPeacefulRoom("Main");// 기본적으로 있어야 하는 룸을 만듦

	StartRecv(nullptr, false);
}

void MinNetIOCP::ServerLoop()
{
	float sleep_time = 1.0f / (float)tick;
	float last_heart_beat = 0;
	float lastUpdate = 0;
	float cur_time = 0;

	while (true)
	{
		cur_time = MinNetTime::time();
		
		MinNetMySQL::IOprocessing();// 데이터베이스와의 작업은 싱글 스레드에서만 함 멀티 스레드 로 하기에는 아직 경험이 없음

		if (cur_time - lastUpdate <= sleep_time)
		{
			_sleep(sleep_time * 1000.0f);
			continue;
		}

		lastUpdate = cur_time;

		if (cur_time - last_heart_beat > 3.0f)
		{
			PingTest();
			last_heart_beat = cur_time;
		}

		messageQ_spin_lock.lock();// 오랫동안 lock 하는걸 막기위해 한번에 큐를 전부 비움
		while (!recvQ.empty())
		{
			messageQ.push(recvQ.front());
			recvQ.pop();
		}
		messageQ_spin_lock.unlock();

		while (!messageQ.empty())
		{
			auto packet_info = messageQ.front();
			auto packet = packet_info.first;
			auto user = packet_info.second;

			room_manager.PacketHandler(user, packet);

			MinNetPool::packetPool->push(packet);

			messageQ.pop();
		}

		room_manager.Update(sleep_time);
		MinNetTime::FrameEnd();
	}
}


std::string MinNetIOCP::GetIP()
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

void MinNetIOCP::AddUser(MinNetUser * user)
{
	if (IsAlreadyAdded(user))
	{
		Debug::Log(user->ID, "는 이미 추가되어 있습니다.");
		return;
	}

	userSpinLock.lock();
	user_list.push_back(user);
	user_map.insert(std::make_pair(user->ID, user));
	userSpinLock.unlock();
}

void MinNetIOCP::DelUser(MinNetUser * user)
{
	if (!IsAlreadyAdded(user))
	{
		Debug::Log(user->ID, "는 존재하지 않습니다.");
		return;
	}

	DelUserAddr(user);

	userSpinLock.lock();
	user_list.remove(user);
	user_map.erase(user->ID);
	userSpinLock.unlock();
}

MinNetUser * MinNetIOCP::GetUser(int userID)
{
	MinNetUser * retval = nullptr;

	userSpinLock.lock();

	auto set = user_map.find(userID);
	if (set != user_map.end())// 해당 id를 갖는 유저가 있음
		retval = set->second;

	userSpinLock.unlock();

	return retval;
}

void MinNetIOCP::RequestUdpFirst(MinNetUser * user)
{
	auto requestPacket = MinNetPool::packetPool->pop();
	requestPacket->create_packet(Defines::MinNetPacketType::SEND_UDP_FIRST);
	requestPacket->create_header();
	StartSend(user, requestPacket);

	MinNetPool::packetPool->push(requestPacket);
}

void MinNetIOCP::AddUserAddr(MinNetUser * user)
{
	userAddrSpinLock.lock();
	auto set = userAddrMap.find(user->addr);
	if (set == userAddrMap.end())
	{
		userAddrMap.insert(std::make_pair(user->addr, user));
	}
	else
	{
		Debug::Log(user->ID, " 는 이미 userAddrMap에 존재 합니다");
	}
	userAddrSpinLock.unlock();
}

void MinNetIOCP::DelUserAddr(MinNetUser * user)
{
	userAddrSpinLock.lock();
	auto set = userAddrMap.find(user->addr);
	if (set == userAddrMap.end())
	{
		Debug::Log(user->ID, " 는 userAddrMap에 존재하지 않습니다");
	}
	else
	{
		userAddrMap.erase(user->addr);
	}
	userAddrSpinLock.unlock();
}

MinNetUser * MinNetIOCP::GetUser(SOCKADDR_IN & addr)
{
	MinNetUser * retval = nullptr;

	userAddrSpinLock.lock();
	auto set = userAddrMap.find(addr);
	if (set != userAddrMap.end())
	{
		retval = set->second;
	}
	userAddrSpinLock.unlock();

	return retval;
}

bool MinNetIOCP::IsAlreadyAdded(MinNetUser * user)
{
	bool retval = false;
	
	userSpinLock.lock();
	auto set = user_map.find(user->ID);
	retval = !(set == user_map.end());
	userSpinLock.unlock();

	return retval;
}

DWORD WINAPI MinNetIOCP::WorkThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	BOOL retval;
	DWORD cbTransferred;
	SOCKET sock;
	MinNetOverlapped * overlap;

	while (true)
	{
		retval = GetQueuedCompletionStatus
		(
			hcp,
			&cbTransferred,
			(PULONG_PTR)&sock,
			(LPOVERLAPPED *)&overlap,
			INFINITE
		);

		if (retval)
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
		//else
		//{
			//if (cbTransferred == 0)
			//{
			//	Debug::Log("WorkThread cbTransferred = 0");
			//	StartClose(((MinNetRecvOverlapped *)overlap)->user);
			//}
		//}
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

int MinNetIOCP::GetNewID()
{
	idCountLock.lock();
	int retval = userIDCount++;
	idCountLock.unlock();

	return retval;
}

void MinNetIOCP::SendIDtoNewUser(MinNetUser * newUser)
{
	MinNetPacket * idCastPacket = MinNetPool::packetPool->pop();
	idCastPacket->create_packet(Defines::MinNetPacketType::USER_ID_CAST);
	idCastPacket->push(newUser->ID);
	idCastPacket->create_header();

	StartSend(newUser, idCastPacket);

	MinNetPool::packetPool->push(idCastPacket);
}

void MinNetIOCP::NewUsertoMainRoom(MinNetUser * newUser)
{
	MinNetPacket * packet = MinNetPool::packetPool->pop();
	packet->create_packet(Defines::MinNetPacketType::USER_ENTER_ROOM);
	packet->push(-2);
	packet->push("Main");
	packet->create_header();

	messageQ_spin_lock.lock();
	recvQ.push(std::make_pair(packet, newUser));
	messageQ_spin_lock.unlock();
}

void MinNetIOCP::PacketHandler(MinNetUser * user, MinNetPacket * packet, MinNetRecvOverlapped * overlapped)
{
	switch (packet->packet_type)
	{
	case Defines::MinNetPacketType::IS_CAN_P2P_ACK:
		OnP2PACK(user, packet);
		break;

	case Defines::MinNetPacketType::PONG:
		OnPong(user, packet);
		break;	

	case Defines::MinNetPacketType::READY_TO_ENTER:
		NewUsertoMainRoom(user);// udp성공 인증을 받은 후 게임에 입장 시킴
		break;

	case Defines::MinNetPacketType::RPC_P2P:
		OnRPCP2P(user, packet);
		AddMessageQueue(user, packet);
		break;

	default:
		AddMessageQueue(user, packet);
		break;
	}
}

void MinNetIOCP::PingTest()
{
	std::queue<MinNetUser *> removeQ;

	auto curTime = MinNetTime::time();

	if (user_list.size() > 0)
	{
		for (auto it = user_list.begin(); it != user_list.end(); it++)
		{
			MinNetUser * user = *it;

			if (user->last_ping > 0)
			{
				if (curTime - user->last_pong > 5.0f)
				{
					std::cout << user << " 이 응답하지 않아 연결 끊음" << std::endl;
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
				SyncTime(user);
			}
		}
	}

	while (!removeQ.empty())
	{
		MinNetUser * user = removeQ.front();
		StartClose(user);
		removeQ.pop();
	}
}

void MinNetIOCP::SendPing(MinNetUser * user)// 클라이언트 에게 핑을 알려줌
{
	MinNetPacket * packet = MinNetPool::packetPool->pop();

	user->last_ping = MinNetTime::time();

	packet->create_packet(Defines::MinNetPacketType::PING);
	packet->create_header();

	StartSend(user, packet);

	MinNetPool::packetPool->push(packet);
}

void MinNetIOCP::SyncTime(MinNetUser * user)// 클라이언트의 시간을 맞춤
{
	MinNetPacket * packet = MinNetPool::packetPool->pop();

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
		MinNetIOCP::tcpSocket,
		overlap->socket,
		(LPVOID)&overlap->buf,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&overlap->dwBytes,
		overlap
	);

	if (error)
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "accept 실패 : " << WSAGetLastError() << std::endl;
			return;
		}
}

void MinNetIOCP::EndAccept(MinNetAcceptOverlapped * overlap)
{
	if (setsockopt(overlap->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&tcpSocket, sizeof(SOCKET)) == SOCKET_ERROR)
	{
		// 대충 오류니까 overlap->socket 닫음
		std::cout << "EndAccept error : " << WSAGetLastError() << std::endl;

		MinNetUser * user = MinNetPool::userPool->pop();
		user->sock = overlap->socket;
		StartClose(user);
		return;
	}

	SOCKADDR *local_addr, *remote_addr;
	int l_len = 0, r_len = 0;
	lpfnGetAcceptExSockaddrs
	(
		(LPVOID)&overlap->buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&local_addr,
		&l_len,
		&remote_addr,
		&r_len
	);

	sockaddr_in * sin = SOCKADDRtoSOCKADDR_IN(remote_addr);
	std::string remoteIP = inet_ntoa(sin->sin_addr);
	int remotePort = ntohs(sin->sin_port);
	Debug::Log("새로운 유저 접속", overlap->socket, remoteIP);

	HANDLE hResult = CreateIoCompletionPort((HANDLE)overlap->socket, hPort, (DWORD)overlap->socket, 0);
	if (hResult == NULL)
		return;

	MinNetUser * user = MinNetPool::userPool->pop();
	user->sock = overlap->socket;

	user->isConnected = true;
	user->ID = GetNewID();

	MinNetPool::acceptOverlappedPool->push(overlap);

	AddUser(user);
	SendIDtoNewUser(user);

	StartRecv(user, true);

	StartAccept();// 다음 클라이언트를 받을 준비를 함
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
				//std::cout << "이미 닫힌 소켓임 : " << overlap->user->sock << std::endl;
				MinNetPool::closeOverlappedPool->push(overlap);
				return;
			}
			std::cout << "Transmitfile error : " << error << std::endl;
		}
	}
	else
	{
		std::cout << "성공적으로 호출 함" << std::endl;
	}
}

void MinNetIOCP::EndClose(MinNetCloseOverlapped * overlap)
{
	std::cout << "유저가 나감 : " << overlap->user->sock << std::endl;

	DelUser(overlap->user);

	MinNetPool::userPool->push(overlap->user);

	MinNetPool::closeOverlappedPool->push(overlap);
}

void MinNetIOCP::StartRecv(MinNetUser * user, bool isTcp)
{
	MinNetRecvOverlapped * overlap = MinNetPool::recvOverlappedPool->pop();

	overlap->user = user;

	DWORD recvbytes;
	DWORD flags = 0;

	int retval = 0;
	overlap->isTcp = isTcp;

	if (isTcp)
	{
		retval = WSARecv(user->sock, &overlap->wsabuf, 1, &recvbytes, &flags, overlap, NULL);
	}
	else
	{
		int lenSize = sizeof(SOCKADDR_IN);
		retval = WSARecvFrom(udpSocket, &overlap->wsabuf, 1, &recvbytes, &flags, (sockaddr *)&overlap->addr, &lenSize, overlap, NULL);
	}
	
	if (retval == SOCKET_ERROR && user != nullptr)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			Debug::Log("StartRecv error", user->ID, error);
			StartClose(user);
			return;
		}
	}
}

void MinNetIOCP::EndRecv(MinNetRecvOverlapped * overlap, int len)
{
	auto isTcp = overlap->isTcp;

	if (len == 0 && isTcp)// udp에서는 받은 데이터의 길이가 0일 수 있음, tcp일 경우에는 close해야함
	{
		Debug::Log("EndRecv len = 0", overlap->user->ID);
		StartClose(overlap->user);
		MinNetPool::recvOverlappedPool->push(overlap);

		return;
	}

	MinNetUser * user = overlap->user;

	if (!overlap->isTcp && user == nullptr)
	{// 여기서 유저 찾아야함
		user = GetUser(overlap->addr);
	
		if (user == nullptr)
		{// 이때 
			MinNetPacket * packet = MinNetPool::packetPool->pop();
			packet->Parse((byte *)&overlap->wsabuf.buf[0], len);
			OnFirstUDP(packet, overlap);
		}
	}

	if (user != nullptr)
	{
		int * bufferPosition = nullptr;
		byte * buffer = nullptr;
		MinNetSpinLock * spinLock = nullptr;

		if (isTcp)
		{
			bufferPosition = &user->tcpBufferPosition;
			buffer = user->tcpBuffer;
			spinLock = &user->tcpBufferLock;
		}
		else
		{
			bufferPosition = &user->udpBufferPosition;
			buffer = user->udpBuffer;
			spinLock = &user->udpBufferLock;
		}

		spinLock->lock();
		memmove(&buffer[*bufferPosition], &overlap->wsabuf.buf[0], len);
		*bufferPosition += len;
		spinLock->unlock();

		while (true)
		{
			MinNetPacket * packet = MinNetPool::packetPool->pop();

			spinLock->lock();
			int checked = packet->Parse(buffer, *bufferPosition);// 잘린 패킷을 합치거나 이어진 패킷을 자름
			*bufferPosition -= checked;
			spinLock->unlock();

			if (checked == 0)// 패킷을 완성하지 못함
			{
				MinNetPool::packetPool->push(packet);
				break;
			}

			packet->isTcpCasting = isTcp;
			PacketHandler(user, packet, overlap);
		}
	}

	if (isTcp)
		StartRecv(user, isTcp);
	else
		StartRecv(nullptr, isTcp);

	MinNetPool::recvOverlappedPool->push(overlap);
}

void MinNetIOCP::StartSend(MinNetUser * user, MinNetPacket * packet, bool isTcp)
{
	if (user == nullptr || user->loadingEnd == false)
		return;

	MinNetSendOverlapped * overlap = MinNetPool::sendOverlappedPool->pop();
	memcpy(overlap->wsabuf.buf, packet->buffer, packet->size());
	overlap->wsabuf.len = packet->size();
	overlap->isTcp = isTcp;

	int retval = 0;

	if (isTcp || user->addr.sin_port == -1)
	{
		retval = WSASend(user->sock, &overlap->wsabuf, 1, nullptr, 0, overlap, nullptr);
	}
	else
	{
		retval = WSASendTo(udpSocket, &overlap->wsabuf, 1,nullptr, 0, (sockaddr*)&user->addr, sizeof(SOCKADDR_IN), overlap, NULL);
	}


	if (retval == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		std::string remoteAddr = inet_ntoa(user->addr.sin_addr);
		int remotePort = ntohs(user->addr.sin_port);

		if (error != WSA_IO_PENDING)
		{
			if (!(error == WSAENOTCONN || error == WSAENOTSOCK))
			{
				Debug::Log("StartSend error", user->ID, error, remoteAddr, remotePort);
				StartClose(user);
				return;
			}
		}
	}
}

void MinNetIOCP::EndSend(MinNetSendOverlapped * overlap)
{
	MinNetPool::sendOverlappedPool->push(overlap);
}

void MinNetIOCP::AddMessageQueue(MinNetUser * user, MinNetPacket * packet)
{
	messageQ_spin_lock.lock();
	recvQ.push(std::make_pair(packet, user));
	messageQ_spin_lock.unlock();
}

void MinNetIOCP::OnP2PACK(MinNetUser * user, MinNetPacket * packet)
{
	int peerID = packet->pop_int();
	auto peer = GetUser(peerID);

	if (peer == nullptr)
	{
		Debug::Log(peerID, " 는 존재하지 않습니다.");
		return;
	}

	if (peer->nowp2pGroup == user->nowp2pGroup)
	{
		Debug::Log(user->ID,"가 ", peerID, " 와 p2p 통신 성공");
		user->relayUserList.remove(peer);
	}
}

void MinNetIOCP::OnRPCP2P(MinNetUser * user, MinNetPacket * packet)
{
	for (auto member : user->relayUserList)
	{
		Debug::Log(user->ID, "가 ", member->ID, " 에게 p2p 통신을 할 수 없어 서버가 대신 합니다.");
		StartSend(member, packet, false);
	}
}

void MinNetIOCP::OnPong(MinNetUser * user, MinNetPacket * packet)
{
	float curTime = MinNetTime::time();
	user->last_pong = curTime;
	user->ping = (user->last_pong - user->last_ping) * 1000;// 핑은 ms 단위를 사용해서 초단위에 1000을 곱함
	MinNetPool::packetPool->push(packet);

	MinNetPacket * cast = MinNetPool::packetPool->pop();

	cast->create_packet(Defines::MinNetPacketType::PING_CAST);
	cast->push(user->ping);
	cast->push(curTime);
	cast->create_header();

	StartSend(user, cast);

	MinNetPool::packetPool->push(cast);
}

void MinNetIOCP::OnFirstUDP(MinNetPacket * packet, MinNetRecvOverlapped * overlapped)
{
	if (packet->packet_type != Defines::MinNetPacketType::SEND_UDP_FIRST)
		return;

	auto userID = packet->pop_int();

	auto user = GetUser(userID);

	if (user != nullptr)
	{
		auto sin = overlapped->addr;

		user->remoteIP = inet_ntoa(sin.sin_addr);
		user->remotePort = ntohs(sin.sin_port);

		user->addr = overlapped->addr;
		user->addr.sin_family = AF_INET;

		AddUserAddr(user);

		auto ackPacket = MinNetPool::packetPool->pop();
		ackPacket->create_packet(Defines::MinNetPacketType::SEND_UDP_FIRST_ACK);
		ackPacket->push(user->remoteIP);
		ackPacket->push(user->remotePort);
		ackPacket->create_header();
		StartSend(user, ackPacket);

		MinNetPool::packetPool->push(ackPacket);
	}

	MinNetPool::packetPool->push(packet);
}
