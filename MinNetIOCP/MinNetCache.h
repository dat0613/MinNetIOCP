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
	
	static ComponentCache componentCache;// string의 이름을 갖는 게임오브젝트 에게 컴포넌트를 추가하는 용도로 쓸 람다의 집합
	static RoomCache roomCache;// string의 이름을 갖는 룸이 생성될 때 옵션을 주는 용도로 쓸 람다의 집합
	static SceneCache sceneCache;// string의 이름을 갖는 룸이 사용할 유니티 씬 이름


	static void SetComponentCache(std::string prefabName, std::function<void(MinNetGameObject *)> f);
	static void AddComponent(MinNetGameObject * object);

	static void SetRoomCache(std::string prefabName, std::function<void(MinNetRoom *, MinNetPacket *)> f);
	static void AddRoom(MinNetRoom * room, MinNetPacket * packet);

	static void SetSceneCache(std::string roomName, std::string sceneName);
	static std::string GetSceneCache(std::string roomName);
};