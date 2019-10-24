#pragma once

#include <lua.hpp>
#include <iostream>

class MinNetGameObject;
class MinNetComponent;
static class MinNetLua
{
public:

	MinNetLua();
	~MinNetLua();

public:
	
	static void ComponentInitializing(MinNetComponent * component, std::string componentName);

private:

};