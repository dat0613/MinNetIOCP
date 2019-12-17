#pragma once

#include <ctime>

static class MinNetTime
{
public:

	static float deltaTime;
	static clock_t curTime();

	static void FrameEnd();

private:

	static clock_t lastFrameTime;

	MinNetTime();
	~MinNetTime();
};

