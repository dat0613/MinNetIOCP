#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"
#include "MinNetCache.h"
#include "MinNetGameObject.h"
#include "FirstPersonController.h"
#include "PlayerMove.h"
#include "BattleFieldManager.h"

void main()
{
	MinNetCache::SetComponentCache("ThirdPersonPlayer", [](MinNetGameObject * object)
	{
		object->AddComponent<PlayerMove>();
	});

	MinNetCache::SetComponentCache("BattleFieldManager", [](MinNetGameObject * object)
	{
		object->AddComponent<BattleFieldManager>();
	});

	MinNetCache::SetRoomCache("Main", [](MinNetRoom * room)
	{
		room->SetMaxUser(100);
	});

	MinNetCache::SetRoomCache("BattleField", [](MinNetRoom * room)
	{
		room->SetMaxUser(10);
		room->Instantiate("BattleFieldManager");
	});


	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();

	_getch();
}