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
	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();

	_getch();
}