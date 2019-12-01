#pragma once

#include <map>
#include <list>
#include <functional>
#include <iostream>

#include <memory>

class MinNetGameObject;
class MinNetRoom;
class MinNetPacket;

using FileCache = std::map<std::string, std::list<std::string>>;
using ComponentCache = std::map<std::string, std::function<void(MinNetGameObject *)>>;
using RoomCache = std::map<std::string, std::function<void(MinNetRoom *, MinNetPacket *)>>;
using SceneCache = std::map<std::string, std::string>;

static class MinNetCache
{
public:

	MinNetCache();
	~MinNetCache();
	
	static ComponentCache componentCache;
	static RoomCache roomCache;
	static SceneCache sceneCache;


	static void SetComponentCache(std::string prefabName, std::function<void(MinNetGameObject *)> f);
	static void AddComponent(MinNetGameObject * object);

	static void SetRoomCache(std::string prefabName, std::function<void(MinNetRoom *, MinNetPacket *)> f);
	static void AddRoom(MinNetRoom * room, MinNetPacket * packet);

	static void SetSceneCache(std::string roomName, std::string sceneName);
	static std::string GetSceneCache(std::string roomName);
};