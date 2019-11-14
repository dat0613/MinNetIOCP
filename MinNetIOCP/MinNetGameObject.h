#pragma once

#include <iostream>
#include "MinNet.h"
#include <map>
#include <list>
#include <typeinfo>

#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

struct lua_State;
class MinNetRoom;
class MinNetComponent;
class MinNetPacket;

class MinNetGameObject
{
private:

	std::map<std::string, std::shared_ptr<MinNetComponent>> componentMap;
	std::list<std::shared_ptr<MinNetComponent>> componentList;

	std::string name = "";
	MinNetRoom * nowRoom = nullptr;
	int objectId = -1;

public:


	MinNetGameObject();
	~MinNetGameObject();
	void SetID(int id);
	int GetID();
	void SetName(std::string name);

	std::string GetName();

	Vector3 position = { 0.0f, 0.0f, 0.0f };
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	bool isNetworkObject = true;// 클라이언트와 동기화 할지
	MinNetUser * owner = nullptr;

	void ChangeRoom(MinNetRoom * room);
	MinNetRoom * GetNowRoom();
	void ObjectRPC(std::string componentName, std::string methodName, MinNetPacket * parameters);
	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler);

	void AddComponent();
	void DelComponent(std::string componentName);
	void DelComponent();

	void Awake();
	void Update();

	std::shared_ptr<MinNetComponent> GetComponent(std::string componentName);

	template <typename T>
	T * GetComponent();

public:

	template <typename T>
	void AddComponent()
	{
		std::string name = typeid(T*).name();

		std::vector<std::string> vec;

		std::istringstream iss(name);
		std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(vec));

		std::string typeName = vec[1];

		T* temp = new T();
		MinNetComponent * comp = dynamic_cast<MinNetComponent *>(temp);
		comp->ComponentName = typeName;
		if (comp == nullptr)
		{
			std::cout << typeName.c_str() << "은 MinNetComponent를 상속하지 않습니다." << std::endl;
			delete temp;
		}
		else
		{
			temp->SetName(typeName.c_str());
			temp->SetParent(this);

			std::shared_ptr<MinNetComponent> shared(temp);

			this->componentList.push_back(shared);
			this->componentMap.insert(make_pair(typeName, shared));

			shared->InitRPC();

			if(this->nowRoom != nullptr)// 이미 한번 초기화된 게임 오브젝트에 추가로 컴포넌트를 붙힌 것임
				shared->Awake();
		}
	}
};

template<typename T>
inline T * MinNetGameObject::GetComponent()
{
	std::string name = typeid(T*).name();

	std::vector<std::string> vec;

	std::istringstream iss(name);
	std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(vec));

	std::string typeName = vec[1];

	GetComponent(typeName);

	auto set = componentMap.find(typeName);

	if (set == componentMap.end())
	{// 찾는 컴포넌트가 없음
		return nullptr;
	}
	else
	{
		return static_cast<T *>(set->second.get());
	}
}