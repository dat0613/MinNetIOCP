#pragma once

#include <WinSock2.h>
#include <MSWSock.h>
#include <mstcpip.h>	

#include "MinNetOverlapped.h"
#include "MinNetRoom.h"
#include <list>

#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma pack(1)

using namespace std;


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

	MinNetPacket * PopPacket();
	void PushPacket(MinNetPacket * packet);
	void StartSend(MinNetUser * user, MinNetPacket * packet);

private:

	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID guidAcceptEx = WSAID_ACCEPTEX;

	HANDLE hPort = nullptr;
	HANDLE port = nullptr;
	
	int tick = 60;

	MinNetObjectPool<MinNetUser> user_pool;
	MinNetObjectPool<MinNetPacket> packet_pool;

	MinNetObjectPool<MinNetAcceptOverlapped> accept_overlapped_pool;
	MinNetObjectPool<MinNetCloseOverlapped> close_overlapped_pool;
	MinNetObjectPool<MinNetSendOverlapped> send_overlapped_pool;
	MinNetObjectPool<MinNetRecvOverlapped> recv_overlapped_pool;

	MinNetRoomManager room_manager;

	MinNetSpinLock recvq_spin_lock;
	MinNetSpinLock user_list_spin_lock;
	list<MinNetUser *> user_list;
	DWORD WINAPI WorkThread(LPVOID arg);

	sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	SOCKET listen_socket;

	CRITICAL_SECTION recvQ_section;
	queue<pair<MinNetPacket *, MinNetUser *>> recvQ;

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);

	void JoinPeacefulRoom(MinNetUser * user);


	void PingTest();
	void SendPing(MinNetUser * user);

	void CreatePool();

	void StartAccept();
	void EndAccept(MinNetAcceptOverlapped * overlap);

	void StartClose(MinNetUser * user);
	void EndClose(MinNetCloseOverlapped * overlap);

	void StartRecv(MinNetUser * user);
	void EndRecv(MinNetRecvOverlapped * overlap, int len);

	void EndSend(MinNetSendOverlapped * overlap);

	void OnPong(MinNetUser * user, MinNetPacket * packet);
};
