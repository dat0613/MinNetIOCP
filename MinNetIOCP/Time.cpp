#include "Time.h"

float Time::deltaTime = 0.0f;
clock_t Time::lastFrameTime = 0.0f;


clock_t Time::curTime()
{
	return clock();
}

void Time::FrameEnd()
{
	clock_t nowTime = curTime();

	deltaTime = (nowTime - lastFrameTime) * 0.001f;

	lastFrameTime = nowTime;
}

Time::Time()
{
}


Time::~Time()
{
}
