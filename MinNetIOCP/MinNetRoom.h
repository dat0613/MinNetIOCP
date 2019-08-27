#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
#include "MinNetOptimizer.h"

using namespace std;

class MinNetIOCP;
class MinNetObject;
class MinNetUser;
class MinNetRoomManager;

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

	void lock();
	void unlock();

private:
	string name = "";
	int room_number = 0;
	int max_user = 0;

	MinNetSpinLock user_lock;
	list<MinNetUser *> user_list;
	map<MinNetObject *, int> object_map;
	MinNetRoomManager * manager = nullptr;
};

class MinNetRoomManager
{
public:
	MinNetRoomManager(MinNetIOCP * minnet);
	MinNetRoom * GetPeacefulRoom();

	MinNetPacket * PopPacket();
	void PushPacket(MinNetPacket * packet);

	void Send(MinNetRoom * room, MinNetPacket * packet);
	void Send(MinNetUser * user, MinNetPacket * packet);

private:
	MinNetObjectPool<MinNetRoom> room_pool;
	list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;

};
