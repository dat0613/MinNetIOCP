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
	static MinNetSpinLock consoleLock;// 다수의 스레드가 콘솔창에 접근하기 때문에 가끔 문장이 합처지는 경우가 있어 lock한 후 프린트 함

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
