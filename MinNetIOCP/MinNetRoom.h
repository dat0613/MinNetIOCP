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

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

private:
	string name = "";
	int room_number = 0;
	int max_user = 0;

	list<MinNetUser *> user_list;
	map<MinNetObject *, int> object_map;
};

class MinNetRoomManager
{
public:
	MinNetRoomManager(MinNetIOCP * minnet);
	MinNetRoom * GetPeacefulRoom();

private:
	MinNetObjectPool<MinNetRoom> room_pool;
	list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;

};
