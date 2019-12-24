#pragma once

#include <WinSock2.h>
#include <MSWSock.h>
#include <mstcpip.h>	

#include "MinNetOverlapped.h"
#include "MinNetRoom.h"
#include <list>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "libmysql.lib")

class MinNetUser;
class MinNetPacket;

static class MinNetIOCP
{
public:
	MinNetIOCP();
	~MinNetIOCP();

	static void SetTickrate(int tick);
	static void StartServer();
	static void ServerLoop();

	static std::string GetIP();
	static void StartSend(MinNetUser * user, MinNetPacket * packet, bool isTcp = true);

private:

	static LPFN_ACCEPTEX lpfnAcceptEx;
	static GUID guidAcceptEx;

	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
	static GUID guidGetAcceptSockAddrs;

	static HANDLE hPort;
	static HANDLE port;
	
	static int tick;

	static MinNetRoomManager room_manager;

	static MinNetSpinLock messageQ_spin_lock;

	static std::list<MinNetUser *> user_list;

	static SOCKET tcpSocket;
	static SOCKET udpSocket;

	static DWORD WINAPI WorkThread(LPVOID arg);

	static sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	static int userIDcount;
	static MinNetSpinLock idCountLock;

	static std::queue<std::pair<MinNetPacket *, MinNetUser *>> recvQ;
	static std::queue<std::pair<MinNetPacket *, MinNetUser *>> messageQ;

	static void PacketHandler(MinNetUser * user, MinNetPacket * packet);

	static void PingTest();
	static void SendPing(MinNetUser * user);
	static void SyncTime(MinNetUser * user);

	static void StartAccept();
	static void EndAccept(MinNetAcceptOverlapped * overlap);

	static void StartClose(MinNetUser * user);
	static void EndClose(MinNetCloseOverlapped * overlap);

	static void StartRecv(MinNetUser * user, bool isTcp = true);
	static void EndRecv(MinNetRecvOverlapped * overlap, int len);

	static void EndSend(MinNetSendOverlapped * overlap);

	static void OnPong(MinNetUser * user, MinNetPacket * packet);

	static int GetUserID();
};