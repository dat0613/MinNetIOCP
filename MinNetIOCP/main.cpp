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
#include "ReadyUser.h"

#include "Debug.h"

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
		room->SetLock(false);

		auto gameObject = room->Instantiate("ReadyRoomManager");
		auto roomManager = gameObject->GetComponent<ReadyRoomManager>();
		
		std::string roomName = "";
		int TeamNumber = 0;
		bool CanBargeIn = false;
		bool OnlyHeadShot = false;
		int TicketCount = 0;
		float RespawnTime = 0.0f;
		int DefaultDamage = 0;
		float HeadShotDamageMultiple = 0.0f;
		int PlayerMaxHP = 0;

		if (packet != nullptr)// 처음으로 이 방을 만들었을때, nullptr 일때는 게임이 끝난 후 다시 대기실로 돌아왔을때 임
		{
			room->destroyWhenEmpty = true;

			roomName = packet->pop_string();
			TeamNumber = packet->pop_int();
			CanBargeIn = packet->pop_bool();
			OnlyHeadShot = packet->pop_bool();
			TicketCount = packet->pop_int();
			RespawnTime = packet->pop_float();
			DefaultDamage = packet->pop_int();
			HeadShotDamageMultiple = packet->pop_float();
			PlayerMaxHP = packet->pop_int();

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
		}
		else
		{
			roomName = room->roomOption.GetValueString("RoomName");
			TeamNumber = room->roomOption.GetValueInt("TeamNumber");
			CanBargeIn = room->roomOption.GetValueBool("CanBargeIn");
			OnlyHeadShot = room->roomOption.GetValueBool("OnlyHeadShot");
			TicketCount = room->roomOption.GetValueInt("TicketCount");
			RespawnTime = room->roomOption.GetValueFloat("RespawnTime");
			DefaultDamage = room->roomOption.GetValueInt("DefaultDamage");
			HeadShotDamageMultiple = room->roomOption.GetValueFloat("HeadShotDamageMultiple");
			PlayerMaxHP = room->roomOption.GetValueInt("PlayerMaxHP");
		}
		
		roomManager->SetOption(roomName, TeamNumber, CanBargeIn, OnlyHeadShot, TicketCount, RespawnTime, DefaultDamage, HeadShotDamageMultiple, PlayerMaxHP);
	});

	MinNetCache::SetSceneCache("ReadyRoom", "ReadyScene");

	MinNetCache::SetComponentCache("ReadyUser", [](MinNetGameObject * object) 
	{
		object->AddComponent<ReadyUser>();
	});

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