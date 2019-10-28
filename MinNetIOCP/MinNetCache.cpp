#include "MinNetCache.h"
#include "MinNetGameObject.h"

ComponentCache MinNetCache::componentCache = ComponentCache();

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
