#include "MinNetLua.h"
#include "MinNetGameObject.h"
#include "MinNetPool.h"
#include "MinNetComponent.h"
#include <luabind/luabind.hpp>

MinNetPacket * PopPacket()
{
	return MinNetPool::packetPool->pop();
}

void ReturnPacket(MinNetPacket * packet)
{
	MinNetPool::packetPool->push(packet);
}

MinNetLua::MinNetLua()
{

}

MinNetLua::~MinNetLua()
{

}

void MinNetLua::ComponentInitializing(MinNetComponent * component, std::string componentName)
{
	component->state = lua_open();
	luaL_openlibs(component->state);
	luabind::open(component->state);

	luaJIT_setmode(component->state, 0, LUAJIT_MODE_ON);

	if (luaL_dofile(component->state, (componentName + ".lua").c_str()))
	{
		cout << componentName << ".lua 파일을 찾을 수 없습니다." << endl;
		return;
	}

	luabind::module(component->state)// Vector3
	[
		luabind::class_<Vector3>("Vector3").
		def(luabind::constructor<>()).
		def(luabind::constructor<float, float, float>()).
		def_readwrite("x", &Vector3::x).
		def_readwrite("y", &Vector3::y).
		def_readwrite("z", &Vector3::z)
	];

	luabind::module(component->state)// Vector2
	[
		luabind::class_<Vector2>("Vector2").
		def(luabind::constructor<>()).
		def(luabind::constructor<float, float>()).
		def_readwrite("x", &Vector2::x).
		def_readwrite("y", &Vector2::y)
	];

	//luabind::module(component->state, "Time")// 시간 관련
	//[
	//	luabind::def("deltaTime", 0.1f)
	//];

	luabind::module(component->state)// GameObject 
	[
		luabind::class_<MinNetGameObject>("GameObject").
		def("SetID", &MinNetGameObject::SetID).
		def("GetID", &MinNetGameObject::GetID).
		def_readwrite("position", &MinNetGameObject::position).
		def_readwrite("rotation", &MinNetGameObject::rotation).
		def_readwrite("scale", &MinNetGameObject::scale).
		def("Instantiate", &MinNetGameObject::Instantiate)
	];

	luabind::module(component->state)
	[
		luabind::class_<MinNetComponent>("Component").
		def("RPC", &MinNetComponent::RPC).
		def("GetName", &MinNetComponent::GetName).
		def_readonly("gameObject", &MinNetComponent::gameObject)
	];

	luabind::module(component->state)// float과 int를 구분하기 위한 float Wrapping 클래스
	[
		luabind::class_<f>("f").
		def(luabind::constructor<float>()).
		def_readwrite("value", &f::value)
	];

	luabind::module(component->state, "MinNetPool")
	[
		luabind::def("PopPacket", &PopPacket),
		luabind::def("ReturnPacket", &ReturnPacket)
	];

	luabind::module(component->state)// MinNetPacket
	[
		luabind::class_<MinNetPacket>("MinNetPacket").
		def(luabind::constructor<>()).
		def("pop_int", &MinNetPacket::pop_int).
		def("pop_bool", &MinNetPacket::pop_bool).
		def("pop_short", &MinNetPacket::pop_short).
		def("pop_float", &MinNetPacket::pop_float).
		def("pop_string", &MinNetPacket::pop_const_char).
		def("pop_vector2", &MinNetPacket::pop_vector2).
		def("pop_vector3", &MinNetPacket::pop_vector3).
		def("push", (void(MinNetPacket::*)(int)) &MinNetPacket::push).
		def("push", (void(MinNetPacket::*)(bool)) &MinNetPacket::push).
		//def("push_short", (void(MinNetPacket::*)(short)) &MinNetPacket::push).
		def("push", (void(MinNetPacket::*)(f)) &MinNetPacket::push).
		def("push", (void(MinNetPacket::*)(const char *)) &MinNetPacket::push).
		def("push", (void(MinNetPacket::*)(Vector2)) &MinNetPacket::push).
		def("push", (void(MinNetPacket::*)(Vector3)) &MinNetPacket::push).
		def("create_packet", &MinNetPacket::create_packet).
		def("create_header", &MinNetPacket::create_header)
	];

	luabind::module(component->state)
	[
		luabind::class_<MinNetRpcTarget>("MinNetRpcTarget").
		enum_("RpcTarget")
		[
			luabind::value("All", 0),
			luabind::value("Others", 1),
			luabind::value("AllViaServer", 2),
			luabind::value("Server", 3)
		]
	];

	luabind::module(component->state)
	[
		luabind::class_<Defines::MinNetPacketType>("MinNetPacketType").
		enum_("PacketType")
		[
			luabind::value("RPC", -8191)
		]
	];

	luabind::call_function<void>(component->state, "SetComponent", component);
	cout << "여기" << endl;
}