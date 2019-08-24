#pragma once

#include <cstdio>
#include <WinSock2.h>
#include <iostream>
#include <queue>
#include <thread>
#include <process.h>

#pragma comment (lib,"ws2_32.lib")
#pragma pack(1)

using namespace std;

namespace MinNet
{
	class Defines
	{
	public:
		static const short HEADERSIZE = 2 + 4;
		static const short MAXCONN = 64 - 1;
		enum MinNetPacketType { CLIENT_ACCEPT_ALLOW = -4, CLIENT_ACCEPT_DISALLOW, CLIENT_REQUEST_ID, CLIENT_ANSWER_ID };
	};

	class BitConverter
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

	class Vector3
	{
	public:

		float x;
		float y;
		float z;

		Vector3();
		Vector3(float x, float y, float z);
		friend ostream& operator<< (ostream& o, const Vector3& vector3);

	};

	class Vector2
	{
	public:

		float x;
		float y;

		Vector2();
		Vector2(float x, float y);
		friend ostream& operator<< (ostream& o, const Vector2& vector2);
	};

	class MinNetPacket
	{
		friend class MinNetUser;
	public:

		MinNetPacket();
		MinNetPacket(char * buffer);
		~MinNetPacket();

		byte *buffer;
		int buffer_position;
		int packet_type;
		int body_size = 0;
		int size();

		void create_packet(int packet_type);
		void create_header();
		void set_buffer_position(unsigned int pos);

		void push(int data);
		void push(bool data);
		void push(short data);
		void push(float data);
		void push(Vector2 data);
		void push(Vector3 data);

		int pop_int();
		bool pop_bool();
		short pop_short();
		float pop_float();
		Vector2 pop_vector2();
		Vector3 pop_vector3();
		int Parse(byte * arr, int length);


	private:


	};

	class MinNetPacketEvent
	{
	public:
		MinNetPacketEvent();
		MinNetPacketEvent(MinNetUser * user, MinNetPacket * packet);
		~MinNetPacketEvent();

		MinNetUser * user;
		MinNetPacket * packet;
	};

	class MinNetUser
	{
	public:

		byte temporary_buffer[2048] = { '\0', };
		int buffer_position = 0;
		SOCKET sock;
		bool isConnected = false;

		int ID;

		MinNetUser();
		~MinNetUser();

		bool ConnectToServer(char* IP, short PORT);


	protected:

	private:
		CRITICAL_SECTION recv_CS;
		CRITICAL_SECTION send_CS;

		void PacketTypeClientAnswerId(MinNetPacket * packet);


	};
}