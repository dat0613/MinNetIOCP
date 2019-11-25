#pragma once

#include <ctime>

static class Time
{
public:

	static float deltaTime;
	static clock_t curTime();

	static void FrameEnd();

private:

	static clock_t lastFrameTime;

	Time();
	~Time();
};

