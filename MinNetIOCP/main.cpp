#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#pragma comment (lib, "lua5.1.lib")
#pragma comment (lib, "lua51.lib")

#include "MinNet.h"

void main()
{

	//MinNetIOCP * iocp = new MinNetIOCP();
	//iocp->SetTickrate(20);
	//iocp->StartServer();
	//iocp->ServerLoop();

	MinNetPacket * packet = new MinNetPacket();
	packet->create_packet(0);
	string str = "123�ѱ��׽�Ʈasdf"; 
	packet->push(str);
	packet->buffer_position = 6;
	cout << packet->pop_string() << endl;

	//printf("Lua ��ũ��Ʈ �׽�Ʈ����\n");
	//lua_State * L = lua_open();
	//luaL_openlibs(L);
	//
	//int res = luaL_dofile(L, "test.lua");

	//lua_getglobal(L, "luaSub");
	//lua_pushnumber(L, 30);
	//lua_pushnumber(L, 100);
	//lua_call(L, 2, 1);
	//int num = lua_tointeger(L, lua_gettop(L));

	////lua_register(L, "DoSum", DoSum);

	//
	//
	_getch();
	//lua_close(L);
}