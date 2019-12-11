#pragma once

#include <WinSock2.h>
#include <MSWSock.h>
#include <mstcpip.h>	

#include "MinNetOverlapped.h"
#include "MinNetRoom.h"
#include <list>

#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
//#pragma pack(1)


class MinNetUser;
class MinNetPacket;

class MinNetIOCP
{
public:
	MinNetIOCP();
	~MinNetIOCP();

	void SetTickrate(int tick);
	void StartServer();
	void ServerLoop();

	std::string GetIP();
	void StartSend(MinNetUser * user, MinNetPacket * packet, bool isTcp = true);

	MinNetSpinLock consoleLock;

private:

	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID guidAcceptEx = WSAID_ACCEPTEX;

	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
	GUID guidGetAcceptSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	HANDLE hPort = nullptr;
	HANDLE port = nullptr;
	
	int tick = 60;

	MinNetRoomManager room_manager;

	MinNetSpinLock messageQ_spin_lock;

	std::list<MinNetUser *> user_list;
	DWORD WINAPI WorkThread(LPVOID arg);

	sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	SOCKET tcpSocket;
	SOCKET udpSocket;

	std::queue<std::pair<MinNetPacket *, MinNetUser *>> recvQ;
	std::queue<std::pair<MinNetPacket *, MinNetUser *>> messageQ;

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);

	void PingTest();
	void SendPing(MinNetUser * user);
	void SyncTime(MinNetUser * user);

	void StartAccept();
	void EndAccept(MinNetAcceptOverlapped * overlap);

	void StartClose(MinNetUser * user);
	void EndClose(MinNetCloseOverlapped * overlap);

	void StartRecv(MinNetUser * user, bool isTcp = true);
	void EndRecv(MinNetRecvOverlapped * overlap, int len);

	void EndSend(MinNetSendOverlapped * overlap);

	void OnPong(MinNetUser * user, MinNetPacket * packet);
};
