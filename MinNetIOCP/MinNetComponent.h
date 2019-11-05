#pragma once

#include <memory>
#include <iostream>
#include <functional>
#include <map>

class MinNetGameObject;
class MinNetPacket;
class MinNetUser;
enum class MinNetRpcTarget;

class MinNetComponent
{
public:

	MinNetComponent();
	~MinNetComponent();

	void DefRPC(std::string functionName, std::function<void(void)> function);

	void SetName(std::string name);
	std::string GetName();

	virtual void InitRPC();
	virtual void Awake();
	virtual void Update();
	virtual void LateUpdate();

	void SetParent(MinNetGameObject * parent);

	MinNetGameObject * gameObject;

	void PushRpcPacket(MinNetPacket * packet);// ���÷��ǵ� �ȵǰ� �ڵ� �ٿ� ĳ���õ� �ȵǼ� �׳� ��Ŷ ��ü�� �ְ� ���� ������ ��

	template <typename ... args>
	void RPC(std::string methodName, MinNetRpcTarget target, args&&... parameters);// RPC�� ȣ���Ҷ�

	template <typename ... args>
	void RPC(std::string methodName, MinNetUser * target, args&&... parameters);// RPC�� ȣ���Ҷ�
	
	template <typename first, typename ... args>
	void VariableArgumentReader(MinNetPacket * packet, first f, args&&... parameters);
	void VariableArgumentReader(MinNetPacket * packet);

	//void RPC(std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters);// RPC�� ȣ���Ҷ�
	//void RPC(std::string methodName, MinNetUser * target, MinNetPacket * parameters);
	void CallRPC(std::string functionName, MinNetPacket * parameters);

protected:

	MinNetPacket * rpcPacket = nullptr;

private:

	std::map<std::string, std::function<void(void)>> RpcMap;
	std::string name;

};

template<typename ...args>
inline void MinNetComponent::RPC(std::string methodName, MinNetRpcTarget target, args && ...parameters)
{
	MinNetPacket * packet = nullptr;
	
	if (sizeof...(parameters) > 0)
	{
		packet = MinNetPool::packetPool->pop();
		packet->create_packet();
		VariableArgumentReader(packet, parameters...);
		packet->create_header();
	}

	if (packet != nullptr)
	{
		switch (target)
		{
		case MinNetRpcTarget::All:
		case MinNetRpcTarget::AllViaServer:
			packet->set_buffer_position(6);
			break;

		case MinNetRpcTarget::Others:
			break;

		case MinNetRpcTarget::Server:
			return;// RPC����� ���� �̹Ƿ� ��ε�ĳ��Ʈ ���� ����
		}
	}

	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, packet);
}

template<typename ...args>
inline void MinNetComponent::RPC(std::string methodName, MinNetUser * target, args && ...parameters)
{
	MinNetPacket * packet = nullptr;

	if (sizeof...(parameters) > 0)
	{
		packet = MinNetPool::packetPool->pop();
		packet->create_packet();
		VariableArgumentReader(packet, parameters...);
		packet->create_header();
	}

	if (target == nullptr)
	{
		if (packet != nullptr)
			MinNetPool::packetPool->push(packet);
		return;
	}

	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, packet);
}

template<typename first, typename ...args>
inline void MinNetComponent::VariableArgumentReader(MinNetPacket * packet, first f, args && ...parameters)
{
	packet->push(f);
	VariableArgumentReader(packet, parameters...);
}
