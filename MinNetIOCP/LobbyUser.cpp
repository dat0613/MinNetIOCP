#include "LobbyUser.h"
#include "MinNetGameObject.h"

LobbyUser::LobbyUser()
{
}


LobbyUser::~LobbyUser()
{
}

void LobbyUser::InitRPC()
{
	DefRPC("GetRoomList", [this](MinNetPacket * packet)
	{
		GetRoomList();
	});
}

void LobbyUser::Awake()
{
}

void LobbyUser::GetRoomList()
{
	auto roomList = gameObject->GetNowRoom()->GetManager()->GetRoomList();

	for (auto room : roomList)
	{
		auto roomName = room->GetName();

		auto readyRoom = roomName == "ReadyRoom";
		auto battlefieldRoom = roomName == "BattleField";

		if (!(readyRoom || battlefieldRoom))
			continue;

		if (!(room->IsPeaceful()))
			continue;
		
		auto managerName = roomName + "Manager";
		auto managerObject = room->GetGameObject(managerName);

		if (managerObject == nullptr)
		{
			std::cout << "여기 인거 가태" << std::endl;
			continue;
		}

		std::string roomTitle = room->roomOption.GetValueString("RoomName");
		std::string roomState = "";
		int roomId = room->GetNumber();
		int nowUser = room->UserCount();
		int maxUser = room->GetMaxUser();

		if (readyRoom)
		{// 게임 대기중인 룸
			roomState = "대기중";
		}
		
		if (battlefieldRoom)
		{// 게임 중인 룸
			roomState = "게임중";
		}

		RPC("AddRoom", gameObject->owner, roomTitle, roomState, roomId, nowUser, maxUser);
	}
}
