#include "MinNetCache.h"
#include "MinNetGameObject.h"
#include "MinNetRoom.h"

ComponentCache MinNetCache::componentCache = ComponentCache();
RoomCache MinNetCache::roomCache = RoomCache();
SceneCache MinNetCache::sceneCache = SceneCache();

MinNetCache::MinNetCache()
{
}


MinNetCache::~MinNetCache()
{
}

void MinNetCache::SetComponentCache(std::string prefabName, std::function<void(MinNetGameObject *)> f)
{
	if (componentCache.find(prefabName) == componentCache.end())
	{// 중복 키값이 없음
		componentCache.insert(std::make_pair(prefabName, f));
	}
	else
	{// 키값이 중복됨
		std::cout << prefabName.c_str() << " 오브젝트는 이미 캐시에 있습니다." << std::endl;
	}
}

void MinNetCache::AddComponent(MinNetGameObject * object)
{
	//object->AddComponent<FirstPersonController>();
	auto prefabName = object->GetName();

	auto cache = componentCache.find(prefabName);

	if (cache == componentCache.end())
	{// 캐시가 없음
		std::cout << prefabName.c_str() << " 오브젝트는 캐시에 없습니다." << std::endl;
	}
	else
	{
		auto function = cache->second;
		function(object);
	}
}

void MinNetCache::SetRoomCache(std::string prefabName, std::function<void(MinNetRoom*)> f)
{
	if (roomCache.find(prefabName) == roomCache.end())
	{// 중복 키값이 없음
		roomCache.insert(std::make_pair(prefabName, f));
	}
	else
	{// 키값이 중복됨
		std::cout << prefabName.c_str() << " 오브젝트는 이미 캐시에 있습니다." << std::endl;
	}
}

void MinNetCache::AddRoom(MinNetRoom * room)
{
	//object->AddComponent<FirstPersonController>();
	auto roomName = room->GetName();

	auto cache = roomCache.find(roomName);

	if (cache == roomCache.end())
	{// 캐시가 없음
		std::cout << roomName.c_str() << " 오브젝트는 캐시에 없습니다." << std::endl;
	}
	else
	{
		auto function = cache->second;
		function(room);
	}
}

void MinNetCache::SetSceneCache(std::string roomName, std::string sceneName)
{
	sceneCache.insert(std::make_pair(roomName, sceneName));
}

std::string MinNetCache::GetSceneCache(std::string roomName)
{
	std::string retval = "";

	auto cache = sceneCache.find(roomName);
	if (cache == sceneCache.end())
	{// 저장된 캐시가 없음
		SetSceneCache(roomName, retval);
	}
	else
	{// 저장된 캐시가 있음
		retval = cache->second;
	}

	return retval;
}