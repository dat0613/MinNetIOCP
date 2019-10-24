#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"
#include "MinNetComponent.h"
#include "MinNetCache.h"

#include <lua.hpp>
#include <luabind/luabind.hpp>

void MinNetGameObject::SetID(int id)
{
	this->objectId = id;
}

int MinNetGameObject::GetID()
{
	return objectId;
}

void MinNetGameObject::SetName(string name)
{
	this->name = name;

	if (name == "")
	{
		return;
	}

	AddComponent();
}

string MinNetGameObject::GetName()
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

void MinNetGameObject::ObjectRPC(string componentName, string methodName, MinNetPacket * parameters)
{
	//luabind::call_function<void>(this->lua, methodName.c_str(), parameters);

	auto com = GetComponent(componentName);
	if (com == nullptr)
	{// ������Ʈ�� �߰� �������� ��Ŷ �ջ����� �Ұ����� �����,..

		
		return;
	}
	com->objectRPC(methodName.c_str(), parameters);
	

	//cout << position << endl;
}

MinNetGameObject * MinNetGameObject::Instantiate(const char * prefabName, Vector3 position, Vector3 euler)
{
	return nowRoom->Instantiate(prefabName, position, euler, nowRoom->GetNewID(), true);
}

void MinNetGameObject::AddComponent(string componentName)
{
	auto com = MinNetPool::componentPool->pop();

	com->gameObject = this;
	com->SetName(componentName.c_str());

	componentList.push_back(com);
	componentMap.insert(make_pair(componentName, com));
}

void MinNetGameObject::AddComponent()
{
	auto cache = MinNetCache::ComponentNameCache.find(name);// ���ʿ��� ��� ����� �������� ĳ�ø� �����
	list<string> componentNameList;

	if (cache == MinNetCache::ComponentNameCache.end())
	{// ĳ�ð� ����

		lua_State * state = lua_open();
		
		if (luaL_dofile(state, (name + ".lua").c_str()))
		{// ������ ã�� �� ����
			cout << name.c_str() << ".lua ������ ã�� �� �����ϴ�." << endl;
		}
		else
		{
			lua_getglobal(state, "ComponentTable");

			if (!lua_istable(state, -1))
			{//���̺��� �ƴ�
				cout << name.c_str() << ".lua ������ �߸� �Ǿ����ϴ�." << endl;
			}
			else
			{
				size_t len = lua_objlen(state, 1);

				for (int i = 1; i <= len; i++)
				{
					lua_rawgeti(state, -1, i);
					componentNameList.push_back(lua_tostring(state, -1));
					lua_pop(state, 1);
				}
			}
		}

		// ��� ������ ã�� ���Ͽ��ų� ComponentTable�� ã�� ���ߴٸ�, �ش� ���ӿ�����Ʈ�� ������Ʈ�� ���°����� ������
		MinNetCache::ComponentNameCache.insert(make_pair(name, componentNameList));

		lua_close(state);
	}
	else
	{
		componentNameList = cache->second;
	}


	for (auto componentName : componentNameList)
	{
		cout << componentName.c_str() << endl;
		AddComponent(componentName);
	}

}

void MinNetGameObject::DelComponent(string componentName)
{
	MinNetComponent * com = GetComponent(componentName);
	if (com == nullptr)
	{// ���� ����ó��

		return;
	}

	componentList.remove(com);

	MinNetPool::componentPool->push(com);
}

void MinNetGameObject::DelComponent()
{
	for (auto comp : componentList)
	{
		MinNetPool::componentPool->push(comp);
	}

	componentList.clear();
	componentMap.clear();
}

MinNetComponent * MinNetGameObject::GetComponent(string componentName)
{
	return componentMap[componentName];
}
