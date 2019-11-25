#pragma once

#include <cstdio>
#include <WinSock2.h>
#include <iostream>
#include <queue>
#include <thread>
#include <process.h>
#include <map>
#include <list>

class MinNetRoom;
class MinNetGameObject;

enum class MinNetRpcTarget { All = -1000, Others, AllViaServer, Server, One, AllNotServer };

class Defines
{
public:
	static const short HEADERSIZE = 2 + 4;
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
		ID_CAST,
		CREATE_ROOM
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

	static float distance(Vector3 & v1, Vector3 & v2);
	static float sqrDistance(Vector3 & v1, Vector3 & v2);
};

class Vector2
{
public:

	float x;
	float y;

	Vector2();
	Vector2(float x, float y);
	friend std::ostream& operator<< (std::ostream& o, const Vector2& vector2);
};

class f	// Lua에서 int와 float을 구분하기 위한 Wrapping 클래스
{
public:

	f(float value)
	{
		this->value = value;
	}

	float value;
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
	void push(f data);
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
	//const char * pop_const_char();
	Vector2 pop_vector2();
	Vector3 pop_vector3();
	int Parse(byte * arr, int length);

private:
};

class MinNetUser
{
public:

	byte temporary_buffer[2048] = { '\0', };
	int buffer_position = 0;
	SOCKET sock;
	int ping = -1;

	int ID;
	
	bool isConnected = false;

	void ChangeRoom(MinNetRoom * room);
	void ChangeRoom(std::string roomName);

	MinNetRoom * GetRoom();
	clock_t last_ping = -1;
	clock_t last_pong = -1;

	std::list<std::shared_ptr<MinNetGameObject>> autoDeleteObjectList;

	MinNetUser();
	~MinNetUser();

protected:

private:

	MinNetRoom * now_room = nullptr;

	void PacketTypeClientAnswerId(MinNetPacket * packet);
};