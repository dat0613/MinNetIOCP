#pragma once

#include <iostream>
#include "MinNetOptimizer.h"

static class Debug
{
public:
	Debug();
	~Debug();

	template<typename ... args>
	static void Log(args&&...params);

	template<typename first, typename ... args>
	static void Reader(first f, args&&... params);

	static void Reader();

private:
	static MinNetSpinLock consoleLock;

};

template<typename ... args>
inline void Debug::Log(args && ...params)
{
	consoleLock.lock();
	Reader(params...);
	consoleLock.unlock();
}

template<typename first, typename ... args>
inline void Debug::Reader(first f, args && ...params)
{
	if (sizeof...(params) > 0)
		std::cout << f << ", ";
	else
		std::cout << f;
	Reader(params...);
}
