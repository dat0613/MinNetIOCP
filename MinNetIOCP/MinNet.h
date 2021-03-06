#pragma once

#include <cstdio>
#include <WinSock2.h>
#include <iostream>
#include <queue>
#include <thread>
#include <process.h>
#include <map>
#include <list>
#include "EasyContainer.h"
#include "MinNetOptimizer.h"
#include "MinNetTime.h"

class MinNetRoom;
class MinNetGameObject;

enum class MinNetRpcTarget 
{
	All = -1000,
	Others,
	AllViaServer, 
	Server, 
	P2Pgroup, 
	P2PgroupAndServer, 
	One, 
	AllNotServer 
};

class Defines
{
public:
	static const short HEADERSIZE = 2 + 4;
	static const short BUFFERSIZE = 4096;
	static const short PACKETSIZE = 1024;
	static const short TEMPORARYBUFFERSIZE = 10240;
	static const short BOOSTTHREADCOUNT = 4;

	enum MinNetPacketType 
	{
		OTHER_USER_ENTER_ROOM = -8200, 
		OTHER_USER_LEAVE_ROOM, 
		USER_ENTER_ROOM, 
		USER_LEAVE_ROOM,
		OBJECT_INSTANTIATE, 
		OBJECT_DESTROY, 
		PING,
		PONG, 
		PING_CAST,
		RPC, 
		RPC_P2P,
		USER_ID_CAST,
		ID_CAST,
		CREATE_ROOM,
		CHANGE_SCENE,
		USER_ENTER_ROOM_FAIL,
		CHANGE_SCENE_COMPLETE,
		SET_USER_VALUE,
		GET_USER_VALUE,
		OTHER_JOIN_P2P_GROUP,
		OTHER_LEAVE_P2P_GROUP,
		JOIN_P2P_GROUP,
		LEAVE_P2P_GROUP,
		P2P_MEMBER_CAST,
		SEND_UDP_FIRST,
		SEND_UDP_FIRST_ACK,
		READY_TO_ENTER,
		IS_CAN_P2P,
		IS_CAN_P2P_ACK,
	};
};

static class BitConverter
{
public:
	static byte* GetBytes(int data);
	static byte* GetBytes(bool data);
	static byte* GetBytes(short data);
	static byte* GetBytes(float data);

	static int ToInt(byte* byte_array, int start_position);
	static bool ToBool(byte* byte_array, int start_position);
	static short ToShort(byte* byte_array, int start_position);
	static float ToFloat(byte* byte_array, int start_position);

	static void ByteCopy(byte* dst, int dst_position, byte *src, int src_length);
};

static class StringConverter
{
public:
	static std::wstring MultibyteToUnicode(std::string multibyte);
	static std::string UnicodeToMultibyte(std::wstring unicode);
	static std::string UnicodeToUTF8(std::wstring unicode);
	static std::wstring UTF8ToUnicode(std::string utf8);
	static std::string MultibyteToUTF8(std::string multibyte);
	static std::string UTF8ToMultibyte(std::string utf8);

};

class Vector3
{
public:

	float x;
	float y;
	float z;

	Vector3();
	Vector3(float x, float y, float z);
	friend std::ostream& operator<< (std::ostream& o, const Vector3& vector3);

	inline float magnitude();
	inline float sqrMagnitude();
	Vector3 Normalize();

	Vector3 operator+(Vector3 & v);
	Vector3 operator+=(Vector3 & v);
	Vector3 operator-(Vector3 & v);
	Vector3 operator-=(Vector3 & v);
	Vector3 operator*(float f);
	Vector3 operator/(float f);

	void ToArray(float * arr);

	static const Vector3 zero;
	static const Vector3 half;

	static const Vector3 Parse(float * floatArray);

	static float distance(Vector3 & v1, Vector3 & v2);
	static float sqrDistance(Vector3 & v1, Vector3 & v2);
	static Vector3 Lerp(Vector3 & v1, Vector3 & v2, float t);
	static float Dot(Vector3 & v1, Vector3 & v2);
	static Vector3 Cross(Vector3 & v1, Vector3 & v2);
};

class Vector2
{
public:

	float x;
	float y;

	Vector2();
	Vector2(float x, float y);
	friend std::ostream& operator << (std::ostream& o, const Vector2& vector2);

	inline float magnitude();
	inline float sqrMagnitude();
	Vector2 Normalize();

	Vector2 operator+(Vector2 & v);
	Vector2 operator+=(Vector2 & v);
	Vector2 operator-(Vector2 & v);
	Vector2 operator-=(Vector2 & v);
	Vector2 operator*(float f);
	Vector2 operator/(float f);

	void ToArray(float * arr);

	static const Vector2 zero;
	static const Vector2 half;

	static float distance(Vector2 & v1, Vector2 & v2);
	static float sqrDistance(Vector2 & v1, Vector2 & v2);
	static Vector2 Lerp(Vector2 & v1, Vector2 & v2, float t);
	static float Dot(Vector2 & v1, Vector2 & v2);
};

class MinNetPacket
{
	friend class MinNetUser;

public:
	MinNetPacket();
	~MinNetPacket();

	byte *buffer;
	int buffer_position;
	int packet_type;
	int body_size = 0;
	int size();

	void create_packet(int packet_type);
	void create_packet();
	void create_header();
	void set_buffer_position(unsigned int pos);

	void push(int data);
	void push(bool data);
	void push(short data);
	void push(float data);
	void push(std::string str);
	void push(const char * str);
	void push(Vector2 data);
	void push(Vector3 data);

	int pop_int();
	bool pop_bool();
	short pop_short();
	float pop_float();
	std::string pop_string();
	Vector2 pop_vector2();
	Vector3 pop_vector3();
	int Parse(byte * arr, int length);

	bool isTcpCasting = true;

private:
};


class MinNetp2pGroup;
class MinNetUser
{
public:

	MinNetSpinLock tcpBufferLock;
	byte tcpBuffer[Defines::TEMPORARYBUFFERSIZE] = { '\0', };
	MinNetSpinLock udpBufferLock;
	byte udpBuffer[Defines::TEMPORARYBUFFERSIZE] = { '\0', };

	int tcpBufferPosition = 0;
	int udpBufferPosition = 0;

	SOCKET sock;
	int ping = -1;

	int ID;
	
	bool isConnected = false;
	bool loadingEnd = true;

	SOCKADDR_IN addr;

	std::string remoteIP = "";
	int remotePort = -1;

	void ChangeRoom(MinNetRoom * room);

	MinNetRoom * GetRoom();
	float last_ping = -1.0f;
	float last_pong = -1.0f;

	std::list<std::shared_ptr<MinNetGameObject>> autoDeleteObjectList;

	MinNetUser();
	~MinNetUser();

	EasyContainer userValue;

	MinNetp2pGroup * nowp2pGroup = nullptr;
	std::list<MinNetUser *> relayUserList;// p2p통신에 실패해 서버를 경유해서 통신 해야할 클라이언트의 리스트

protected:

private:

	MinNetRoom * now_room = nullptr;

	void PacketTypeClientAnswerId(MinNetPacket * packet);
};