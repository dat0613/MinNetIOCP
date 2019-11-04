#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
#include "MinNet.h"
#include "MinNetOptimizer.h"

class MinNetIOCP;
class MinNetUser;
class MinNetPacket;
class MinNetRoomManager;
typedef struct lua_State lua_State;

class MinNetGameObject;

class MinNetRoom
{
public:

	MinNetRoom();
	~MinNetRoom();

	void SetName(std::string name);
	std::string GetName();
	void SetNumber(int number);
	void SetMaxUser(int max);
	int UserCount();
	int GetNumber();
	bool IsPeaceful();

	std::list<MinNetUser *> * GetUserList();

	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler, int id, bool casting = false, MinNetUser * except = nullptr, bool autoDelete = false);
	void Destroy(std::string prefabName, int id, bool casting = false, MinNetUser * except = nullptr);

	void ObjectInstantiate(MinNetUser * user, MinNetPacket * packet);
	void ObjectDestroy(MinNetUser * user, MinNetPacket * packet);

	void SetManager(MinNetRoomManager * manager);

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

	void RemoveUsers();

	void AddObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(int id);
	void RemoveObjects();
	std::shared_ptr<MinNetGameObject> GetGameObject(int id);

	int GetUserCount();

	int GetNewID();

	MinNetUser * GetUser(int id);

	void ObjectRPC(MinNetUser * user, MinNetPacket * packet);
	void SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters, MinNetUser * user = nullptr);

private:

	std::string name = "";
	int room_number = 0;
	int max_user = 10;

	int id_count = 0;

	std::map<int, MinNetUser *> user_map;
	std::list<MinNetUser *> user_list;
	std::map<int, std::shared_ptr<MinNetGameObject>> object_map;
	std::list<std::shared_ptr<MinNetGameObject>> object_list;
	MinNetRoomManager * manager = nullptr;

};

class MinNetRoomManager
{
public:
	MinNetRoomManager(MinNetIOCP * minnet);
	MinNetRoom * GetPeacefulRoom();

	void Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except = nullptr);
	void Send(MinNetUser * user, MinNetPacket * packet);

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);
private:

	std::list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;
};
