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
			std::cout << "���� �ΰ� ����" << std::endl;
			continue;
		}

		std::string roomTitle = room->roomOption.GetValueString("RoomName");
		std::string roomState = "";
		int roomId = room->GetNumber();
		int nowUser = room->UserCount();
		int maxUser = room->GetMaxUser();

		if (readyRoom)
		{// ���� ������� ��
			roomState = "�����";
		}
		
		if (battlefieldRoom)
		{// ���� ���� ��
			roomState = "������";
		}

		RPC("AddRoom", gameObject->owner, roomTitle, roomState, roomId, nowUser, maxUser);
	}
}
