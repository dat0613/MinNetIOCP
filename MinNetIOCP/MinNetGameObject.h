#pragma once

#include <iostream>
#include "MinNet.h"
#include <map>
#include <list>

struct lua_State;
class MinNetRoom;
class MinNetComponent;

class MinNetGameObject
{
public:
	void SetID(int id);
	int GetID();
	void SetName(std::string name);

	std::string GetName();

	Vector3 position = { 0.0f, 0.0f, 0.0f };
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	MinNetUser * owner = nullptr;

	void ChangeRoom(MinNetRoom * room);
	MinNetRoom * GetNowRoom();
	void ObjectRPC(string componentName, string methodName, MinNetPacket * parameters);
	MinNetGameObject * Instantiate(const char * prefabName, Vector3 position, Vector3 euler);

	void AddComponent(string componentName);
	void AddComponent();
	void DelComponent(string componentName);
	void DelComponent();

	MinNetComponent * GetComponent(string componentName);

private:

	map<string, MinNetComponent *> componentMap;
	list<MinNetComponent *> componentList;

	string name = "";
	MinNetRoom * nowRoom = nullptr;
	int objectId = -1;
};