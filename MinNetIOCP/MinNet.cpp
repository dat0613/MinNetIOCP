#include "MinNet.h"
#include "MinNetIOCP.h"
#include "MinNetRoom.h"

//////////////////////
//BitConverter class//
//////////////////////


byte * BitConverter::GetBytes(int data)				//int�� �����͸� ����Ʈ �迭�� �ٲٴ� �Լ�
{
	byte* temp_buffer = new byte[sizeof(int)];

	for (int i = 0; i < sizeof(int); i++)
	{
		temp_buffer[i] = (byte)((data >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

byte * BitConverter::GetBytes(bool data)			//bool�� �����͸� ����Ʈ �迭�� �ٲٴ� �Լ�
{
	byte* temp_buffer = new byte(sizeof(bool));

	*temp_buffer = (byte)(data & 0xFF);

	return temp_buffer;
}

byte * BitConverter::GetBytes(short data)			//short�� �����͸� ����Ʈ �迭�� �ٲٴ� �Լ�
{
	byte* temp_buffer = new byte[sizeof(short)];

	for (int i = 0; i < sizeof(short); i++)
	{
		temp_buffer[i] = (byte)((data >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

byte * BitConverter::GetBytes(float data)			//float�� �����͸� ����Ʈ �迭�� �ٲٴ� �Լ�
{
	byte* temp_buffer = new byte[sizeof(float)];

	for (int i = 0; i < sizeof(float); i++)
	{
		temp_buffer[i] = (byte)((*((unsigned long*)&data) >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

int BitConverter::ToInt(byte * byte_array, int start_position)		//����Ʈ �迭���� int�� �����͸� ������ �Լ�
{
	int temp_number = 0;

	for (int i = start_position; i < start_position + sizeof(int); i++)
	{
		temp_number += byte_array[i] << (8 * (i - start_position));
	}

	return temp_number;
}

bool BitConverter::ToBool(byte * byte_array, int start_position)	//����Ʈ �迭���� bool�� �����͸� ������ �Լ�
{
	bool temp_bool = byte_array[start_position] << 0;

	return temp_bool;
}

short BitConverter::ToShort(byte * byte_array, int start_position)	//����Ʈ �迭���� short�� �����͸� ������ �Լ�
{
	short temp_short = 0;

	for (int i = start_position; i < start_position + sizeof(short); i++)
	{
		temp_short += byte_array[i] << (8 * (i - start_position));
	}

	return temp_short;
}

float BitConverter::ToFloat(byte * byte_array, int start_position)	//����Ʈ �迭���� float�� �����͸� ������ �Լ�
{
	unsigned long temp_long = 0.0f;

	for (int i = start_position; i < start_position + sizeof(float); i++)
	{
		temp_long += ((unsigned long)(byte_array[i])) << ((8 * (i - start_position)));
	}

	float temp_float = *((float*)&temp_long);

	return temp_float;
}

void BitConverter::ByteCopy(byte * dst, int dst_position, byte * src, int src_length)	//drc����Ʈ �迭�� dst�迭 �ڿ� ������ �Լ�
{
	for (int i = 0; i < src_length; i++)
	{
		dst[dst_position + i] = ((src)[i]);
	}

	delete[] src;
}


//////////////////////
//MinNetPacket class//
//////////////////////


MinNetPacket::MinNetPacket()				//��Ŷ�� ������
{
	buffer = new byte[1024]();
	//buffer = { 0, };
	buffer_position = 0;
}

MinNetPacket::~MinNetPacket()				//��Ŷ�� �Ҹ���
{
	delete[] buffer;
}

int MinNetPacket::size()
{
	return body_size + 6;
}

void MinNetPacket::create_packet(int packet_type)	//��Ŷ�� ������� �Ҷ� ������������ ȣ���Ͽ��� �ϴ� �Լ�
{
	buffer_position = Defines::HEADERSIZE;
	this->packet_type = packet_type;
	//ZeroMemory(&buffer, sizeof(buffer));
	//memset(&buffer, 0, 1024);
}

void MinNetPacket::set_buffer_position(unsigned int pos)
{
	buffer_position = pos;
}

void MinNetPacket::create_header()					//��Ŷ�� �����Ҷ� �ڵ����� ȣ��Ǵ� �Լ�
{
	body_size = (short)(buffer_position - Defines::HEADERSIZE);
	int header_position = 0;

	byte* header_size = BitConverter::GetBytes(body_size);
	BitConverter::ByteCopy(buffer, header_position, header_size, sizeof(short));
	header_position += sizeof(short);

	byte* header_type = BitConverter::GetBytes(packet_type);
	BitConverter::ByteCopy(buffer, header_position, header_type, sizeof(int));
}

void MinNetPacket::push(int data)					//int�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(bool data)					//bool�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(short data)					//short�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(float data)					//float�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(string& str)
{
	int len = str.length();
	push(len);
	memcpy(&buffer[buffer_position], (byte *)str.c_str(), len);
	buffer_position += len;
}

void MinNetPacket::push(Vector2 data)				//Vector2�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	push(data.x);
	push(data.y);
}

void MinNetPacket::push(Vector3 data)				//Vector3�� �����͸� ��Ŷ�� �ִ� �Լ�
{
	push(data.x);
	push(data.y);
	push(data.z);
}

int MinNetPacket::pop_int()							//int�� �����͸� ��Ŷ���� ������ �Լ�
{
	int temp_num = 0;
	temp_num = BitConverter::ToInt(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

bool MinNetPacket::pop_bool()						//bool�� �����͸� ��Ŷ���� ������ �Լ�
{
	bool temp_bool = 0;
	temp_bool = BitConverter::ToBool(buffer, buffer_position);
	buffer_position += sizeof(temp_bool);

	return temp_bool;
}

short MinNetPacket::pop_short()						//short�� �����͸� ��Ŷ���� ������ �Լ�
{
	short temp_num = 0;
	temp_num = BitConverter::ToShort(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

float MinNetPacket::pop_float()						//float�� �����͸� ��Ŷ���� ������ �Լ�
{
	float temp_num = 0.0f;
	temp_num = BitConverter::ToFloat(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

string MinNetPacket::pop_string()
{
	int len = pop_int();
	string str;
	str.assign((char *)&buffer[buffer_position], len);
	buffer_position += len;
	return str;
}

Vector2 MinNetPacket::pop_vector2()			//vector2�� �����͸� ��Ŷ���� ������ �Լ�
{
	Vector2 temp_vector2;
	temp_vector2.x = pop_float();
	temp_vector2.y = pop_float();

	return temp_vector2;
}

Vector3 MinNetPacket::pop_vector3()			//vector3�� �����͸� ��Ŷ���� ������ �Լ�
{
	Vector3 temp_vector3;
	temp_vector3.x = pop_float();
	temp_vector3.y = pop_float();
	temp_vector3.z = pop_float();

	return temp_vector3;
}

int MinNetPacket::Parse(byte * arr, int length)
{
	buffer_position = 0;// ���� ��ġ�� 0���� �ʱ�ȭ ��Ŵ

	if (length < 6 - 1)// �ּ��� �����ŭ �ִ��� Ȯ��
		return 0;

	byte * real_buffer = buffer;// ���� �Ҵ� �Ǿ��ִ� ���۸� ��� �����ص�

	buffer = arr;// �޸��� �̵��� �ּ�ȭ �ϱ� ���� ��Ŷ�� ���۸� ��õ��� arr�� ������ �� �˻���

	body_size = pop_short();// ��ü ũ�� ����
	packet_type = pop_int();// ��Ŷ Ÿ�� ����

	buffer = real_buffer;// arr�� �ʿ��� �����Ͱ� ���� �ִ°����� �ǴܵǾ� ���� ���۷� ���ƿ�

	if (length < 6 + body_size - 1 - 1)// ��ü�� ���� �ִ��� üũ
		return 0;

	memcpy(real_buffer, arr, length + body_size);// ��ü�� �޾ƿ�

	byte * arr_cpy = new byte[length];
	memcpy(arr_cpy, arr, length);// arr�� �ӽ� �����ص�

	ZeroMemory(arr, length);// arr�� ���

	memcpy(arr, &arr_cpy[body_size + 6], length - body_size - 6);// arr�� ������ ó���� ��ŭ�� �����͸� ��ä�� ����

	delete[] arr_cpy;

	return body_size + 6;
}

////////////////
//Vector class//
////////////////

Vector3::Vector3()
{
	x = y = z = 0.0f;
}

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

ostream & operator<<(ostream & o, const Vector3 & vector3)
{
	cout << "[" << vector3.x << ", " << vector3.y << ", " << vector3.z << "]";

	return o;
}


Vector2::Vector2()
{
	x = y = 0.0f;
}

Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

ostream & operator<<(ostream & o, const Vector2 & vector2)
{
	cout << "[" << vector2.x << ", " << vector2.y << "]";

	return o;
}


void MinNetUser::ChangeRoom(MinNetRoom * room)
{
	if (now_room != nullptr)// �̹� �뿡 �� �ִٸ�
	{
		now_room->RemoveUser(this);// �뿡�� ����
	}
	if (room != nullptr)
	{
		now_room = room;// ���ο� �� ����
		now_room->AddUser(this);// ���ο� �뿡 ��
	}
}

MinNetUser::MinNetUser()
{
}

MinNetUser::~MinNetUser()
{
}


void MinNetUser::PacketTypeClientAnswerId(MinNetPacket * packet)
{
	ID = packet->pop_int();
}