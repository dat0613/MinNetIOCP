#pragma once

#include <string>
#include <iostream>
#include <list>
#include <map>
#include "MinNet.h"
#include "MinNetOptimizer.h"
#include "EasyContainer.h"

class MinNetIOCP;
class MinNetUser;
class MinNetPacket;
class MinNetRoomManager;

class MinNetGameObject;

class MinNetRoom
{
public:

	MinNetRoom();
	~MinNetRoom();

	void SetName(std::string name);
	std::string GetName();
	void SetNumber(int number);
	int GetNumber();
	void SetMaxUser(int max);
	int GetMaxUser();
	int UserCount();
	bool IsPeaceful();

	std::list<MinNetUser *> * GetUserList();

	bool destroyWhenEmpty = false;
	bool changeRoom = false; // 룸이 실행되는 도중 룸 옵션을 바꿀때 true로
	std::string changeRoomName = "";

	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName);
	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler);
	std::shared_ptr<MinNetGameObject> Instantiate(std::string prefabName, Vector3 position, Vector3 euler, int id, bool casting = false, MinNetUser * except = nullptr, bool autoDelete = false);

	void Destroy(std::string prefabName, int id, bool casting = false, MinNetUser * except = nullptr);
	void Destroy();

	void UserLoadingComplete(MinNetUser * user);

	void ObjectInstantiate(MinNetUser * user, MinNetPacket * packet);
	void ObjectDestroy(MinNetUser * user, MinNetPacket * packet);

	void SetManager(MinNetRoomManager * manager);
	MinNetRoomManager * GetManager();

	void ObjectSyncing(MinNetUser * user);

	void AddUser(MinNetUser * user);
	void RemoveUser(MinNetUser * user);

	EasyContainer roomOption;

	void RemoveUsers();

	void AddObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(std::shared_ptr<MinNetGameObject> object);
	void RemoveObject(int id);
	void RemoveObjects();
	std::shared_ptr<MinNetGameObject> GetGameObject(int id);

	std::shared_ptr<MinNetGameObject> GetGameObject(std::string prefabName);
	std::vector<std::shared_ptr<MinNetGameObject>> & GetGameObjects(std::string prefabName);

	void ChangeRoom(std::string roomName);

	void Update();

	int GetNewID();

	MinNetUser * GetUser(int id);

	void ObjectRPC(MinNetUser * user, MinNetPacket * packet);
	void SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters);
	void SendRPC(int objectId, std::string componentName, std::string methodName, MinNetUser * target, MinNetPacket * parameters);

	void SetSceneName(std::string sceneName);

private:

	std::string nowSceneName = "";

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
	MinNetRoom * GetPeacefulRoom(std::string roomName);
	MinNetRoom * GetRoom(int roomId);

	void Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except = nullptr);
	void Send(MinNetUser * user, MinNetPacket * packet);

	void PacketHandler(MinNetUser * user, MinNetPacket * packet);
	
	void Update();

	std::list<MinNetRoom *>& GetRoomList();

private:

	int roomNumberCount = -1;
	MinNetRoom * CreateRoom(std::string roomName, MinNetPacket * packet);
	void DestroyRoom(MinNetRoom * room);

	std::list<MinNetRoom *> room_list;
	MinNetIOCP * minnet;
};
