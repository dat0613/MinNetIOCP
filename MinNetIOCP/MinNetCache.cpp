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
