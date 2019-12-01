#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"
#include "MinNetCache.h"
#include "MinNetGameObject.h"
#include "FirstPersonController.h"
#include "PlayerMove.h"
#include "BattleFieldManager.h"
#include "ComponentTest.h"
#include "LobbyUser.h"
#include "ReadyRoomManager.h"

void main()
{
	MinNetCache::SetComponentCache("ThirdPersonPlayer", [](MinNetGameObject * object)
	{
		object->AddComponent<PlayerMove>();
		//object->AddComponent<ComponentTest>();
	});

	MinNetCache::SetComponentCache("BattleFieldManager", [](MinNetGameObject * object)
	{
		object->AddComponent<BattleFieldManager>();
	});

	MinNetCache::SetRoomCache("Main", [](MinNetRoom * room, MinNetPacket * packet)
	{
		room->SetMaxUser(100);
	});

	MinNetCache::SetRoomCache("BattleField", [](MinNetRoom * room, MinNetPacket * packet)
	{
		room->SetMaxUser(10);
		room->Instantiate("BattleFieldManager");
	});
	MinNetCache::SetSceneCache("BattleField", "GameScene");

	MinNetCache::SetComponentCache("ReadyRoomManager", [](MinNetGameObject * object) 
	{
		object->AddComponent<ReadyRoomManager>();
	});

	MinNetCache::SetRoomCache("ReadyRoom", [](MinNetRoom * room, MinNetPacket * packet) 
	{
		std::string roomName = packet->pop_string();

		int TeamNumber = packet->pop_int();
		bool CanBargeIn = packet->pop_bool();
		bool OnlyHeadShot = packet->pop_bool();
		int TicketCount = packet->pop_int();
		float RespawnTime = packet->pop_float();
		int DefaultDamage = packet->pop_int();
		float HeadShotDamageMultiple = packet->pop_float();
		int PlayerMaxHP = packet->pop_int();

		room->SetMaxUser((TeamNumber + 1) * 2);

		room->roomOption.SetValue("RoomName", roomName);
		room->roomOption.SetValue("TeamNumber", TeamNumber);
		room->roomOption.SetValue("CanBargeIn", CanBargeIn);
		room->roomOption.SetValue("OnlyHeadShot", OnlyHeadShot);
		room->roomOption.SetValue("TicketCount", TicketCount);
		room->roomOption.SetValue("RespawnTime", RespawnTime);
		room->roomOption.SetValue("DefaultDamage", DefaultDamage);
		room->roomOption.SetValue("HeadShotDamageMultiple", HeadShotDamageMultiple);
		room->roomOption.SetValue("PlayerMaxHP", PlayerMaxHP);

		auto gameObject = room->Instantiate("ReadyRoomManager");
		auto roomManager = gameObject->GetComponent<ReadyRoomManager>();

		roomManager->SetOption(roomName, TeamNumber, CanBargeIn, OnlyHeadShot, TicketCount, RespawnTime, DefaultDamage, HeadShotDamageMultiple, PlayerMaxHP);
	});

	MinNetCache::SetSceneCache("ReadyRoom", "ReadyScene");

	MinNetCache::SetComponentCache("LobbyUser", [](MinNetGameObject * object) 
	{
		object->isSyncingObject = false;
		object->AddComponent<LobbyUser>();
	});

	MinNetCache::SetRoomCache("Lobby", [](MinNetRoom * room, MinNetPacket * packet) {
		room->SetMaxUser(100);
	});

	MinNetCache::SetSceneCache("Lobby", "Lobby");


	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();

	_getch();
}