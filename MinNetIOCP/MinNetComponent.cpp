#include "MinNetComponent.h"
#include "MinNetGameObject.h"
#include "MinNetLua.h"
#include "MinNet.h"
#include "MinNetRoom.h"
#include <luabind\luabind.hpp>

MinNetComponent::MinNetComponent()
{

}


MinNetComponent::~MinNetComponent()
{

}

void MinNetComponent::SetName(const char * name)
{
	this->name = name;

	if (name == "")
	{// state ÃÊ±âÈ­
		
		return;
	}

	MinNetLua::ComponentInitializing(this, name);
}

const char * MinNetComponent::GetName()
{
	return name.c_str();
}

void MinNetComponent::Update()
{

}

void MinNetComponent::RPC(const char * methodName, MinNetRpcTarget target, MinNetPacket * parameters)
{
	switch (target)
	{
	case MinNetRpcTarget::All:
	case MinNetRpcTarget::AllViaServer:
		parameters->set_buffer_position(6);
		luabind::call_function<void>(state, methodName, parameters);
		break;

	case MinNetRpcTarget::Others:
		break;

	case MinNetRpcTarget::Server:
		return;
	}

	gameObject->GetNowRoom()->SendRPC(gameObject->GetID(), name, methodName, target, parameters);
}

void MinNetComponent::objectRPC(const char * methodName, MinNetPacket * parameters)
{
	luabind::call_function<void>(state, methodName, parameters);
}
