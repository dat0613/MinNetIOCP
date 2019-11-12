#include "MinNetGameObject.h"

#include "MinNetComponent.h"
#include "MinNetCache.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"

MinNetGameObject::MinNetGameObject()
{
}

MinNetGameObject::~MinNetGameObject()
{
}

void MinNetGameObject::SetID(int id)
{
	this->objectId = id;
}

int MinNetGameObject::GetID()
{
	return objectId;
}

void MinNetGameObject::SetName(std::string name)
{
	this->name = name;

	if (name == "")
	{
		return;
	}
	AddComponent();
}

std::string MinNetGameObject::GetName()
{
	return name;
}

void MinNetGameObject::ChangeRoom(MinNetRoom * room)
{
	nowRoom = room;

	//if (nowRoom == nullptr)
	//{
	//	nowRoom = room;
	//}
	//else
	//{
	//
	//}
}

MinNetRoom * MinNetGameObject::GetNowRoom()
{
	return nowRoom;
}

void MinNetGameObject::ObjectRPC(std::string componentName, std::string methodName, MinNetPacket * parameters)
{
	auto com = GetComponent(componentName);

	if (com == nullptr)
	{// 컴포넌트를 추가 시켜줄지 패킷 손상으로 할것인지 고민중,..

		return;
	}
	com->CallRPC(methodName, parameters);
}

std::shared_ptr<MinNetGameObject>  MinNetGameObject::Instantiate(std::string prefabName, Vector3 position, Vector3 euler)
{
	return nowRoom->Instantiate(prefabName, position, euler, nowRoom->GetNewID(), true);
}

void MinNetGameObject::PrintSomeThing()
{
	std::cout << "시발" << std::endl;
}

void MinNetGameObject::AddComponent()
{
	MinNetCache::AddComponent(static_cast<MinNetGameObject *>(this));
}

void MinNetGameObject::DelComponent(std::string componentName)
{
	auto com = GetComponent(componentName);
	if (com == nullptr)
	{// 대충 예외처리

		return;
	}

	componentList.remove(com);
	componentMap.erase(componentName);
}

void MinNetGameObject::DelComponent()
{
	componentList.clear();
	componentMap.clear();
}

void MinNetGameObject::Update()
{
	for (auto component : componentList)
	{
		component->Update();
	}
}

std::shared_ptr<MinNetComponent> MinNetGameObject::GetComponent(std::string componentName)
{
	auto set = componentMap.find(componentName);

	if (set == componentMap.end())
	{// 찾는 컴포넌트가 없음
		return nullptr;
	}
	else
	{
		return set->second;
	}
}