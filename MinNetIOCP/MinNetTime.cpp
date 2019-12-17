#include "MinNetTime.h"

float MinNetTime::deltaTime = 0.0f;
clock_t MinNetTime::lastFrameTime = 0.0f;


clock_t MinNetTime::curTime()
{
	return std::clock();
}

void MinNetTime::FrameEnd()
{
	clock_t nowTime = curTime();

	deltaTime = (nowTime - lastFrameTime) * 0.001f;

	lastFrameTime = nowTime;
}

MinNetTime::MinNetTime()
{
}


MinNetTime::~MinNetTime()
{
}
