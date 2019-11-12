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

void MinNetComponent::DefRPC(std::string functionName, std::function<void(MinNetPacket *)> function)
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

void MinNetComponent::VariableArgumentReader(MinNetPacket * packet)
{// 빈 함수
}

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
			rpcFunction(packet);
		}
	}
}
