#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
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

private:
	string name;
	int id = -1;
};

class MinNetRoom
{
public:
	MinNetRoom();
	~MinNetRoom();

	void SetName(string name);
	string GetName();
	void SetNumber(int number);
	void SetMaxUser(int max);
	int UserCount();
	int GetNumber();
	bool IsPeaceful();

	list<MinNetUser *> * GetUserList();

	void SetManager(MinNetRoomManager * manager);

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

	void AddObject(MinNetGameObject * object, string name, int id, MinNetUser * spawner);
	void RemoveObject(MinNetGameObject * object);
	void RemoveObject(int id);

	int GetNewID();

	void lock();
	void unlock();

private:
	string name = "";
	int room_number = 0;
	int max_user = 10;

	int id_count = 0;

	MinNetSpinLock user_lock;
	MinNetSpinLock object_lock;
	list<MinNetUser *> user_list;
	map<int, MinNetGameObject *> object_map;
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

private:
	MinNetObjectPool<MinNetRoom> room_pool;
	list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;

};
