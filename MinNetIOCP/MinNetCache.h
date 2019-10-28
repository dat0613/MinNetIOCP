#pragma once

#include <map>
#include <list>
#include <functional>
#include <iostream>

#include <memory>

class MinNetGameObject;

using FileCache = std::map<std::string, std::list<std::string>>;
using ComponentCache = std::map<std::string, std::function<void(MinNetGameObject *)>>;

static class MinNetCache
{
public:

	MinNetCache();
	~MinNetCache();
	
	static ComponentCache componentCache;

	static void SetComponentCache(std::string prefabName, std::function<void(MinNetGameObject *)> f);
	static void AddComponent(MinNetGameObject * object);
};

