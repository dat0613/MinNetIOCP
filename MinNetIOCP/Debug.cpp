#include "Debug.h"

MinNetSpinLock Debug::consoleLock;

Debug::Debug()
{
}


Debug::~Debug()
{
}

void Debug::Reader()
{
	std::cout << std::endl;
}
