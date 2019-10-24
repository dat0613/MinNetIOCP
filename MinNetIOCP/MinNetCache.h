#pragma once

#include <map>
#include <list>

using FileCache = std::map<std::string, std::list<std::string>>;

static class MinNetCache
{
public:

	MinNetCache();
	~MinNetCache();

	static FileCache ComponentNameCache;
};

