#include "MinNetIOCP.h"
#include "MinNet.h"
#include "MinNetRoom.h"
#include <atlstr.h>
#include <cmath>

//////////////////////
//BitConverter class//
//////////////////////


byte * BitConverter::GetBytes(int data)				//int형 데이터를 바이트 배열로 바꾸는 함수
{
	byte* temp_buffer = new byte[sizeof(int)];

	for (int i = 0; i < sizeof(int); i++)
	{
		temp_buffer[i] = (byte)((data >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

byte * BitConverter::GetBytes(bool data)			//bool형 데이터를 바이트 배열로 바꾸는 함수
{
	byte* temp_buffer = new byte(sizeof(bool));

	*temp_buffer = (byte)(data & 0xFF);

	return temp_buffer;
}

byte * BitConverter::GetBytes(short data)			//short형 데이터를 바이트 배열로 바꾸는 함수
{
	byte* temp_buffer = new byte[sizeof(short)];

	for (int i = 0; i < sizeof(short); i++)
	{
		temp_buffer[i] = (byte)((data >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

byte * BitConverter::GetBytes(float data)			//float형 데이터를 바이트 배열로 바꾸는 함수
{
	byte* temp_buffer = new byte[sizeof(float)];

	for (int i = 0; i < sizeof(float); i++)
	{
		temp_buffer[i] = (byte)((*((unsigned long*)&data) >> (8 * i) & 0xFF));
	}

	return temp_buffer;
}

int BitConverter::ToInt(byte * byte_array, int start_position)		//바이트 배열에서 int형 데이터를 빼오는 함수
{
	int temp_number = 0;

	for (int i = start_position; i < start_position + sizeof(int); i++)
	{
		temp_number += byte_array[i] << (8 * (i - start_position));
	}

	return temp_number;
}

bool BitConverter::ToBool(byte * byte_array, int start_position)	//바이트 배열에서 bool형 데이터를 빼오는 함수
{
	bool temp_bool = byte_array[start_position] << 0;

	return temp_bool;
}

short BitConverter::ToShort(byte * byte_array, int start_position)	//바이트 배열에서 short형 데이터를 빼오는 함수
{
	short temp_short = 0;

	for (int i = start_position; i < start_position + sizeof(short); i++)
	{
		temp_short += byte_array[i] << (8 * (i - start_position));
	}

	return temp_short;
}

float BitConverter::ToFloat(byte * byte_array, int start_position)	//바이트 배열에서 float형 데이터를 빼오는 함수
{
	unsigned long temp_long = 0.0f;

	for (int i = start_position; i < start_position + sizeof(float); i++)
	{
		temp_long += ((unsigned long)(byte_array[i])) << ((8 * (i - start_position)));
	}

	float temp_float = *((float*)&temp_long);

	return temp_float;
}

void BitConverter::ByteCopy(byte * dst, int dst_position, byte * src, int src_length)	//drc바이트 배열을 dst배열 뒤에 붙히는 함수
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


MinNetPacket::MinNetPacket()				//패킷의 생성자
{
	buffer = new byte[Defines::PACKETSIZE]();
	//buffer = { 0, };
	buffer_position = 0;
}

MinNetPacket::~MinNetPacket()				//패킷의 소멸자
{
	delete[] buffer;
}

int MinNetPacket::size()// body_size + head_size
{
	return body_size + 6;
}

void MinNetPacket::create_packet(int packet_type)	//패킷을 만들고자 할때 무조건적으로 호출하여야 하는 함수
{
	buffer_position = Defines::HEADERSIZE;
	this->packet_type = packet_type;
	//ZeroMemory(&buffer, sizeof(buffer));
	//memset(&buffer, 0, 1024);
}

void MinNetPacket::create_packet()
{
	create_packet(0);
}

void MinNetPacket::set_buffer_position(unsigned int pos)
{
	buffer_position = pos;
}

void MinNetPacket::create_header()					//패킷을 전송할때 자동으로 호출되는 함수
{
	body_size = (short)(buffer_position - Defines::HEADERSIZE);
	int header_position = 0;

	byte* header_size = BitConverter::GetBytes(body_size);
	BitConverter::ByteCopy(buffer, header_position, header_size, sizeof(short));
	header_position += sizeof(short);

	byte* header_type = BitConverter::GetBytes(packet_type);
	BitConverter::ByteCopy(buffer, header_position, header_type, sizeof(int));
}

void MinNetPacket::push(int data)					//int형 데이터를 패킷에 넣는 함수
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(bool data)					//bool형 데이터를 패킷에 넣는 함수
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(short data)					//short형 데이터를 패킷에 넣는 함수
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(float data)					//float형 데이터를 패킷에 넣는 함수
{
	byte* temp_buffer = BitConverter::GetBytes(data);
	BitConverter::ByteCopy(buffer, buffer_position, temp_buffer, sizeof(data));
	buffer_position += sizeof(data);
}

void MinNetPacket::push(std::string str)
{
	std::string utf8 = StringConverter::MultibyteToUTF8(str);
	int len = utf8.size();
	push(len);

	memcpy(&buffer[buffer_position], (byte *)utf8.c_str(), len);
	buffer_position += len;
}

void MinNetPacket::push(const char * str)
{
	auto str2 = std::string(str);
	push(str2);
}

void MinNetPacket::push(Vector2 data)				//Vector2형 데이터를 패킷에 넣는 함수
{
	push(data.x);
	push(data.y);
}

void MinNetPacket::push(Vector3 data)				//Vector3형 데이터를 패킷에 넣는 함수
{
	push(data.x);
	push(data.y);
	push(data.z);
}

int MinNetPacket::pop_int()							//int형 데이터를 패킷에서 빼오는 함수
{
	int temp_num = 0;
	temp_num = BitConverter::ToInt(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

bool MinNetPacket::pop_bool()						//bool형 데이터를 패킷에서 빼오는 함수
{
	bool temp_bool = 0;
	temp_bool = BitConverter::ToBool(buffer, buffer_position);
	buffer_position += sizeof(temp_bool);

	return temp_bool;
}

short MinNetPacket::pop_short()						//short형 데이터를 패킷에서 빼오는 함수
{
	short temp_num = 0;
	temp_num = BitConverter::ToShort(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

float MinNetPacket::pop_float()						//float형 데이터를 패킷에서 빼오는 함수
{
	float temp_num = 0.0f;
	temp_num = BitConverter::ToFloat(buffer, buffer_position);
	buffer_position += sizeof(temp_num);

	return temp_num;
}

std::string MinNetPacket::pop_string()
{ 
	int len = pop_int();

	std::string utf8((char *)&buffer[buffer_position], len);
	buffer_position += len;

	return StringConverter::UTF8ToMultibyte(utf8);
}

//const char * MinNetPacket::pop_const_char() // 생각보다 쓸모가 없어서 뺌
//{
//	auto str = pop_string();
//	return str.c_str();
//}

Vector2 MinNetPacket::pop_vector2()			//vector2형 데이터를 패킷에서 빼오는 함수
{
	Vector2 temp_vector2;
	temp_vector2.x = pop_float();
	temp_vector2.y = pop_float();

	return temp_vector2;
}

Vector3 MinNetPacket::pop_vector3()			//vector3형 데이터를 패킷에서 빼오는 함수
{
	Vector3 temp_vector3;
	temp_vector3.x = pop_float();
	temp_vector3.y = pop_float();
	temp_vector3.z = pop_float();

	return temp_vector3;
}

int MinNetPacket::Parse(byte * arr, int length)
{
	buffer_position = 0;// 버퍼 위치를 0으로 초기화 시킴

	if (length < Defines::HEADERSIZE)// 최소한 헤더만큼 있는지 확인
		return 0;

	byte * real_buffer = buffer;// 현재 할당 되어있는 버퍼의 주소를 기억해둠

	buffer = arr;// 메모리의 이동을 최소화 하기 위해 패킷의 버퍼를 잠시동안 arr로 변경한 후 검사함

	body_size = pop_short();// 몸체 크기 받음
	packet_type = pop_int();// 패킷 타입 받음

	buffer = real_buffer;// arr에 필요한 데이터가 전부 있는것으로 판단되어 원래 버퍼주소로 돌아옴

	if (length < Defines::HEADERSIZE + body_size)// 몸체가 전부 있는지 체크
		return 0;

	memcpy(buffer, arr, Defines::HEADERSIZE + body_size);// 몸체를 파싱하기위해 패킷에 넣음

	memmove(arr, &arr[body_size + Defines::HEADERSIZE], length - body_size - Defines::HEADERSIZE);// 빼낸 크기만큼 버퍼를 앞으로 당김

	return body_size + 6;
}

////////////////
//Vector class//
////////////////

const Vector3 Vector3::zero = Vector3();
const Vector3 Vector3::half = Vector3(0.5f, 0.5f, 0.5f);

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

float Vector3::magnitude()
{
	return sqrt(sqrMagnitude());
}

float Vector3::sqrMagnitude()
{
	return x * x + y * y + z * z;
}

Vector3 Vector3::Normalize()
{
	float len = magnitude();
	return *this / len;
}

Vector3 Vector3::operator+(Vector3 & v)
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator+=(Vector3 & v)
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

Vector3 Vector3::operator-(Vector3 & v)
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator-=(Vector3 & v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

Vector3 Vector3::operator*(float f)
{
	return Vector3(x * f, y * f, z * f);
}

Vector3 Vector3::operator/(float f)
{
	return Vector3(x / f, y / f, z / f);
}

void Vector3::ToArray(float * arr)
{
	arr[0] = -x;
	arr[1] = y;
	arr[2] = z;
} 

const Vector3 Vector3::Parse(float * floatArray)
{
	return Vector3(floatArray[0], floatArray[1], floatArray[2]);
}

float Vector3::distance(Vector3 & v1, Vector3 & v2)
{
	return (v1 - v2).magnitude();
}

float Vector3::sqrDistance(Vector3 & v1, Vector3 & v2)
{
	return (v1 - v2).sqrMagnitude();
}

Vector3 Vector3::Lerp(Vector3 & v1, Vector3 & v2, float t)
{
	Vector3 ret; 
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t); 
	ret.x = v1.x + t * (v2.x - v1.x); 
	ret.y = v1.y + t * (v2.y - v1.y); 
	ret.z = v1.z + t * (v2.z - v1.z); 
	return ret;
}

float Vector3::Dot(Vector3 & v1, Vector3 & v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Vector3::Cross(Vector3 & v1, Vector3 & v2)
{
	return Vector3(v1.y * v2.x - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

std::ostream & operator<<(std::ostream & o, const Vector3 & vector3)
{
	std::cout << "[" << vector3.x << ", " << vector3.y << ", " << vector3.z << "]";

	return o;
}

const Vector2 Vector2::zero = Vector2();
const Vector2 Vector2::half = Vector2(0.5f, 0.5f);

Vector2::Vector2()
{
	x = y = 0.0f;
}

Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

inline float Vector2::magnitude()
{
	return sqrt(sqrMagnitude());
}

inline float Vector2::sqrMagnitude()
{
	return x * x + y * y;
}

Vector2 Vector2::Normalize()
{
	return Vector2();
}

Vector2 Vector2::operator+(Vector2 & v)
{
	return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator+=(Vector2 & v)
{
	x += v.x;
	y += v.y;

	return *this;
}

Vector2 Vector2::operator-(Vector2 & v)
{
	return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator-=(Vector2 & v)
{
	x -= v.x;
	y -= v.y;

	return *this;
}

Vector2 Vector2::operator*(float f)
{
	return Vector2(x * f, y * f);
}

void Vector2::ToArray(float * arr)
{
	arr[0] = x;
	arr[1] = y;
}

float Vector2::distance(Vector2 & v1, Vector2 & v2)
{
	return (v1 - v2).magnitude();
}

float Vector2::sqrDistance(Vector2 & v1, Vector2 & v2)
{
	return (v1 - v2).sqrMagnitude();
}

Vector2 Vector2::Lerp(Vector2 & v1, Vector2 & v2, float t)
{
	Vector2 ret;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);
	ret.x = v1.x + t * (v2.x - v1.x);
	ret.y = v1.y + t * (v2.y - v1.y);
	return ret;
}

float Vector2::Dot(Vector2 & v1, Vector2 & v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

std::ostream & operator<<(std::ostream & o, const Vector2 & vector2)
{
	std::cout << "[" << vector2.x << ", " << vector2.y << "]";

	return o;
}

void MinNetUser::ChangeRoom(MinNetRoom * room)
{
	if (now_room != nullptr)// 이미 룸에 들어가 있다면
	{
		now_room->RemoveUser(this);// 룸에서 나옴
	}
	if (room != nullptr)
	{
		now_room = room;// 새로운 룸 갱신
		now_room->AddUser(this);// 새로운 룸에 들어감
	}
	else
	{
		now_room = nullptr;
	}
}

MinNetRoom * MinNetUser::GetRoom()
{
	return now_room;
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

std::wstring StringConverter::MultibyteToUnicode(std::string multibyte)
{
	return CA2W(multibyte.c_str());
}

std::string StringConverter::UnicodeToMultibyte(std::wstring unicode)
{
	return CW2A(unicode.c_str());
}

std::string StringConverter::UnicodeToUTF8(std::wstring unicode)
{
	return CW2A(unicode.c_str(), CP_UTF8);
}

std::wstring StringConverter::UTF8ToUnicode(std::string utf8)
{
	return CA2W(utf8.c_str(), CP_UTF8);
}

std::string StringConverter::MultibyteToUTF8(std::string multibyte)
{
	return UnicodeToUTF8(MultibyteToUnicode(multibyte));
}

std::string StringConverter::UTF8ToMultibyte(std::string utf8)
{
	return UnicodeToMultibyte(UTF8ToUnicode(utf8));
}