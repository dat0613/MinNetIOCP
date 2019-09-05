#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
#include "MinNet.h"
#include "MinNetOptimizer.h"

using namespace std;

class MinNetIOCP;
class MinNetUser;
class MinNetPacket;
class MinNetRoomManager;

class MinNetGameObject
{
public:
	void SetID(int id);
	int GetID();
	void SetName(string name);
	string GetName();

	Vector3 position;
	Vector3 rotation;
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	MinNetUser * owner;

private:
	string name;
	int id = -1;
};

class MinNetRoom
{
public:
	MinNetRoom();
	~MinNetRoom();

	MinNetObjectPool<MinNetGameObject> gameobject_pool;

	void SetName(string name);
	string GetName();
	void SetNumber(int number);
	void SetMaxUser(int max);
	int UserCount();
	int GetNumber();
	bool IsPeaceful();

	list<MinNetUser *> * GetUserList();

	MinNetGameObject * Instantiate(string prefabName, Vector3 position, Vector3 euler, int id, bool casting = false, MinNetUser * except = nullptr, bool autoDelete = false);
	void Destroy(string prefabName, int id, bool casting = false, MinNetUser * except = nullptr);

	void ObjectInstantiate(MinNetUser * user, MinNetPacket * packet);
	void ObjectDestroy(MinNetUser * user, MinNetPacket * packet);

	void SetManager(MinNetRoomManager * manager);

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

	void AddObject(MinNetGameObject * object);
	void RemoveObject(MinNetGameObject * object);
	void RemoveObject(int id);

	int GetNewID();

	void ObjectRPC(MinNetUser * user, MinNetPacket * packet);

private:
	string name = "";
	int room_number = 0;
	int max_user = 10;

	int id_count = 0;

	list<MinNetUser *> user_list;
	map<int, MinNetGameObject *> object_map;
	list<MinNetGameObject *> object_list;
	MinNetRoomManager * manager = nullptr;

};


class MinNetRoomManager
{
public:
	MinNetRoomManager(MinNetIOCP * minnet);
	MinNetRoom * GetPeacefulRoom();

	MinNetPacket * PopPacket();
	void PushPacket(MinNetPacket * packet);

	void Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except = nullptr);
	void Send(MinNetUser * user, MinNetPacket * packet);

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);

private:
	MinNetObjectPool<MinNetRoom> room_pool;
	list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;

};
