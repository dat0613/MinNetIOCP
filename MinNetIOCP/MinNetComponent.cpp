#include "MinNetComponent.h"

#include "MinNet.h"
#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"


MinNetComponent::MinNetComponent()
{
}

MinNetComponent::~MinNetComponent()
{
}

void MinNetComponent::DefRPC(std::string functionName, std::function<void(void)> function)
{
	auto f = RpcMap.find(functionName);
	if (f == RpcMap.end())
	{//
		RpcMap.insert(std::make_pair(functionName, function));
	}
	else
	{// 중복임
		std::cout << functionName.c_str() << "함수는 이미 추가되어 있습니다." << std::endl;
	}
}

void MinNetComponent::SetName(std::string name)
{
	this->name = name;

	if (name == "")
	{
		
		return;
	}
}

std::string MinNetComponent::GetName()
{
	return name;
}

void MinNetComponent::InitRPC()
{

}

void MinNetComponent::Awake()
{

}

void MinNetComponent::Update()
{

}

void MinNetComponent::LateUpdate()
{

}

void MinNetComponent::SetParent(MinNetGameObject * parent)
{
	this->gameObject = parent;
}

void MinNetComponent::PushRpcPacket(MinNetPacket * packet)
{
	this->rpcPacket = packet;
}




void MinNetComponent::VariableArgumentReader(MinNetPacket * packet)
{
}

//void MinNetComponent::RPC(std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters)
//{
//	if (parameters != nullptr)
//	{
//		switch (target)
//		{
//		case MinNetRpcTarget::All:
//		case MinNetRpcTarget::AllViaServer:
//			parameters->set_buffer_position(6);
//			break;
//
//		case MinNetRpcTarget::Others:
//			break;
//
//		case MinNetRpcTarget::Server:
//			return;// RPC대상이 서버 이므로 브로드캐스트 하지 않음
//		}
//	}
//
//	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, parameters);
//}
//
//void MinNetComponent::RPC(std::string methodName, MinNetUser * target, MinNetPacket * parameters)
//{
//	if (target == nullptr)
//	{
//		if (parameters != nullptr)
//			MinNetPool::packetPool->push(parameters);
//		return;
//	}
//
//	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, parameters);
//}

void MinNetComponent::CallRPC(std::string functionName, MinNetPacket * packet)
{
	auto value = RpcMap.find(functionName);
	if (value == RpcMap.end())
	{// 함수가 없음
		std::cout << functionName.c_str() << " 함수는 아직 추가되지 않아 호출할 수 없습니다." << std::endl;
		DefRPC(functionName, nullptr);
	}
	else
	{// 
		auto rpcFunction = value->second;
		if (rpcFunction != nullptr)
		{
			this->PushRpcPacket(packet);
			rpcFunction();
			this->PushRpcPacket(nullptr);
		}
	}
}
