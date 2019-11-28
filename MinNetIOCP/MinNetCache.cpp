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
	{// �ߺ� Ű���� ����
		componentCache.insert(std::make_pair(prefabName, f));
	}
	else
	{// Ű���� �ߺ���
		std::cout << prefabName.c_str() << " ������Ʈ�� �̹� ĳ�ÿ� �ֽ��ϴ�." << std::endl;
	}
}

void MinNetCache::AddComponent(MinNetGameObject * object)
{
	//object->AddComponent<FirstPersonController>();
	auto prefabName = object->GetName();

	auto cache = componentCache.find(prefabName);

	if (cache == componentCache.end())
	{// ĳ�ð� ����
		std::cout << prefabName.c_str() << " ������Ʈ�� ĳ�ÿ� �����ϴ�." << std::endl;
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
	{// �ߺ� Ű���� ����
		roomCache.insert(std::make_pair(prefabName, f));
	}
	else
	{// Ű���� �ߺ���
		std::cout << prefabName.c_str() << " ������Ʈ�� �̹� ĳ�ÿ� �ֽ��ϴ�." << std::endl;
	}
}

void MinNetCache::AddRoom(MinNetRoom * room)
{
	//object->AddComponent<FirstPersonController>();
	auto roomName = room->GetName();

	auto cache = roomCache.find(roomName);

	if (cache == roomCache.end())
	{// ĳ�ð� ����
		std::cout << roomName.c_str() << " ������Ʈ�� ĳ�ÿ� �����ϴ�." << std::endl;
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
	{// ����� ĳ�ð� ����
		SetSceneCache(roomName, retval);
	}
	else
	{// ����� ĳ�ð� ����
		retval = cache->second;
	}

	return retval;
}