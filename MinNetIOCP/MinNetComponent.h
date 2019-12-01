#pragma once

#include <memory>
#include <iostream>
#include <functional>
#include <map>
#include <ctime>
#include "Time.h"
#include "MinNetPool.h"
#include "MinNet.h"
#include "MinNetRoom.h"

class MinNetGameObject;
class MinNetPacket;
class MinNetUser;
enum class MinNetRpcTarget;

class MinNetComponent : public std::enable_shared_from_this<MinNetComponent>
{
public:

	MinNetComponent();
	virtual ~MinNetComponent();

	void DefRPC(std::string functionName, std::function<void(MinNetPacket *)> function);

	void SetName(std::string name);
	std::string GetName();

	virtual void InitRPC();
	virtual void OnInstantiate(MinNetUser * user);
	virtual void Awake();
	virtual void Update();
	virtual void LateUpdate();
	virtual void OnDestroy();

	void SetParent(MinNetGameObject * parent);

	MinNetGameObject * gameObject;
	std::string ComponentName = "";

	template <typename ... args>
	void RPC(std::string methodName, MinNetRpcTarget target, args&&... parameters);// RPC를 호출할때

	template <typename ... args>
	void RPC(std::string methodName, MinNetUser * target, args&&... parameters);// RPC를 호출할때
	
	template <typename first, typename ... args>
	void VariableArgumentReader(MinNetPacket * packet, first f, args&&... parameters);
	void VariableArgumentReader(MinNetPacket * packet);

	void CallRPC(std::string functionName, MinNetPacket * parameters);

private:

	std::map<std::string, std::function<void(MinNetPacket *)>> RpcMap;
	std::string name;

};

template<typename ...args>
inline void MinNetComponent::RPC(std::string methodName, MinNetRpcTarget target, args && ...parameters)
{
	if (!gameObject->isSyncingObject)
	{// 다른 클라이언트와 동기화 되지 않는 오브젝트는 오직 자신의 클라이언트 에게만 RPC호출을 할 수 있음
		if (gameObject->owner != nullptr)
		{
			std::cout << "isSyncingObject가 false인 오브젝트는 자신의 owner에게만 RPC호출을 할 수 있습니다. 오브젝트 이름 : " << gameObject->GetName() << std::endl;
		}
		else
		{
			std::cout << "isSyncingObject가 false이며 owner가 없는 오브젝트의 RPC호출은 혀용되지 않습니다. 오브젝트 이름 : " << gameObject->GetName() << std::endl;
		}
		return;
	}

	MinNetPacket * parametersPacket = nullptr;
	
	if (sizeof...(parameters) > 0)
	{
		parametersPacket = MinNetPool::packetPool->pop();
		parametersPacket->create_packet();
		VariableArgumentReader(parametersPacket, parameters...);
		parametersPacket->create_header();
	}

	switch (target)
	{
	case MinNetRpcTarget::All:
	case MinNetRpcTarget::AllViaServer:
	case MinNetRpcTarget::Server:// RPC대상이 서버 이므로 브로드캐스트 하지 않음
		if (parametersPacket != nullptr)
		{
			parametersPacket->set_buffer_position(Defines::HEADERSIZE);
			this->CallRPC(methodName, parametersPacket);
			parametersPacket->set_buffer_position(Defines::HEADERSIZE);
		}
		else
		{
			this->CallRPC(methodName, nullptr);
		}
		break;

	case MinNetRpcTarget::Others:
		break;

	case MinNetRpcTarget::AllNotServer:
		target = MinNetRpcTarget::All;
		break;
	}

	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, parametersPacket);
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
