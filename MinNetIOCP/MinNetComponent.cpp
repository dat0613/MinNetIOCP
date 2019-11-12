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
	{// �ߺ���
		std::cout << functionName.c_str() << "�Լ��� �̹� �߰��Ǿ� �ֽ��ϴ�." << std::endl;
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
{// �� �Լ�
}

void MinNetComponent::CallRPC(std::string functionName, MinNetPacket * packet)
{
	auto value = RpcMap.find(functionName);
	if (value == RpcMap.end())
	{// �Լ��� ����
		std::cout << functionName.c_str() << " �Լ��� ���� �߰����� �ʾ� ȣ���� �� �����ϴ�." << std::endl;
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
