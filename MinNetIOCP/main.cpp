#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"
#include "MinNetCache.h"
#include "MinNetGameObject.h"
#include "FirstPersonController.h"
#include "PlayerMove.h"

void main()
{
	MinNetCache::SetComponentCache("ThirdPersonPlayer", [](MinNetGameObject * object)
	{
		object->AddComponent<PlayerMove>();
	});

	MinNetCache::SetRoomCache("Main", [](MinNetRoom * room)
	{
		room->SetMaxUser(100);
	});

	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();

	_getch();
}