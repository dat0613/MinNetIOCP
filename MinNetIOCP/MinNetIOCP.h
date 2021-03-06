#pragma once

#include <WinSock2.h>
#include <MSWSock.h>
#include <mstcpip.h>	
#include <unordered_map>

#include "MinNetOverlapped.h"
#include "MinNetRoom.h"
#include <list>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "libmysql.lib")

#pragma comment (lib, "DebugUtils.lib")
#pragma comment (lib, "Detour.lib")
#pragma comment (lib, "Recast.lib")
#pragma comment (lib, "DetourCrowd.lib")
#pragma comment (lib, "DetourTileCache.lib")

class MinNetUser;
class MinNetPacket;

struct KeyHash;

struct KeyEqual;

static class MinNetIOCP
{
public:

	MinNetIOCP();
	~MinNetIOCP();

	static void SetTickrate(int tick);
	static void StartServer(USHORT tcpPort, USHORT udpPort);
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

	static MinNetSpinLock userSpinLock;

	static std::list<MinNetUser *> user_list;
	static std::map<int, MinNetUser *> user_map;

	static std::unordered_map<SOCKADDR_IN, MinNetUser *, KeyHash, KeyEqual> userAddrMap;

	static void AddUser(MinNetUser * user);
	static void DelUser(MinNetUser * user);
	static MinNetUser * GetUser(int userID);

	static MinNetSpinLock userAddrSpinLock;

	static void RequestUdpFirst(MinNetUser * user);

	static void AddUserAddr(MinNetUser * user);
	static void DelUserAddr(MinNetUser * user);
	static MinNetUser * GetUser(SOCKADDR_IN & addr);

	static bool IsAlreadyAdded(MinNetUser * user);

	static SOCKET tcpSocket;
	static SOCKET udpSocket;

	static DWORD WINAPI WorkThread(LPVOID arg);

	static sockaddr_in * SOCKADDRtoSOCKADDR_IN(sockaddr * addr);

	static int userIDCount;
	static MinNetSpinLock idCountLock;

	static int GetNewID();

	static std::queue<std::pair<MinNetPacket *, MinNetUser *>> recvQ;
	static std::queue<std::pair<MinNetPacket *, MinNetUser *>> messageQ;

	static void SendIDtoNewUser(MinNetUser * newUser);
	static void NewUsertoMainRoom(MinNetUser * newUser);

	static void PacketHandler(MinNetUser * user, MinNetPacket * packet, MinNetRecvOverlapped * overlapped);

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

	static void AddMessageQueue(MinNetUser * user, MinNetPacket * packet);

	static void OnP2PACK(MinNetUser * user, MinNetPacket * packet);
	static void OnRPCP2P(MinNetUser * user, MinNetPacket * packet);
	static void OnPong(MinNetUser * user, MinNetPacket * packet);
	static void OnFirstUDP(MinNetPacket * packet, MinNetRecvOverlapped * overlapped);
};

struct KeyHash
{
	std::size_t operator()(const SOCKADDR_IN& k)const
	{
		return
			std::hash<ULONG>()(k.sin_addr.S_un.S_addr) ^

			std::hash<UCHAR>()(k.sin_addr.S_un.S_un_b.s_b1) ^
			std::hash<UCHAR>()(k.sin_addr.S_un.S_un_b.s_b2) ^
			std::hash<UCHAR>()(k.sin_addr.S_un.S_un_b.s_b3) ^
			std::hash<UCHAR>()(k.sin_addr.S_un.S_un_b.s_b4) ^

			std::hash<USHORT>()(k.sin_addr.S_un.S_un_w.s_w1) ^
			std::hash<USHORT>()(k.sin_addr.S_un.S_un_w.s_w2) ^

			std::hash<USHORT>()(k.sin_family) ^
			std::hash<USHORT>()(k.sin_port);
	}
};

struct KeyEqual
{
	bool operator()(const SOCKADDR_IN& k1, const SOCKADDR_IN& k2) const
	{
		return
			(
				k1.sin_addr.S_un.S_addr == k2.sin_addr.S_un.S_addr &&

				k1.sin_addr.S_un.S_un_b.s_b1 == k2.sin_addr.S_un.S_un_b.s_b1 &&
				k1.sin_addr.S_un.S_un_b.s_b2 == k2.sin_addr.S_un.S_un_b.s_b2 &&
				k1.sin_addr.S_un.S_un_b.s_b3 == k2.sin_addr.S_un.S_un_b.s_b3 &&
				k1.sin_addr.S_un.S_un_b.s_b4 == k2.sin_addr.S_un.S_un_b.s_b4 &&

				k1.sin_addr.S_un.S_un_w.s_w1 == k2.sin_addr.S_un.S_un_w.s_w1 &&
				k1.sin_addr.S_un.S_un_w.s_w2 == k2.sin_addr.S_un.S_un_w.s_w2 &&

				k1.sin_family == k2.sin_family &&
				k1.sin_port == k2.sin_port
				);
	}
};
