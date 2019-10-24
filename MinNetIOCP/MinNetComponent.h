#pragma once
#include <iostream>

struct lua_State;
class MinNetGameObject;
class MinNetPacket;
enum class MinNetRpcTarget;

class MinNetComponent
{
public:

	MinNetComponent();
	~MinNetComponent();

	void SetName(const char * name);
	const char * GetName();

	void Update();

	MinNetGameObject * gameObject;
	lua_State * state = nullptr;

	void RPC(const char * methodName, MinNetRpcTarget target, MinNetPacket * parameters);// RPC�� ȣ���Ҷ�
	void objectRPC(const char * methodName, MinNetPacket * parameters);// RPC�� ȣ�� �޾�����

private:

	std::string name;

};