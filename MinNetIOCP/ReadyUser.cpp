#include "ReadyUser.h"

#include "MinNetGameObject.h"
#include "ReadyRoomManager.h"

void ReadyUser::InitRPC()
{
	DefRPC("SendChat", [this](MinNetPacket * packet) 
	{
		auto chat = packet->pop_string();

		this->SendChat(chat);
	});

	DefRPC("SetTeam", [this](MinNetPacket * packet) 
	{
		auto team = static_cast<PlayerMove::Team>(packet->pop_int());
		this->SetTeam(team);
	});

	DefRPC("SetMaster", [this](MinNetPacket * packet) {
		auto master = packet->pop_bool();
		this->SetMaster(master);
	});

	DefRPC("ChangeTeam", [this](MinNetPacket * packet) 
	{
		this->ChangeTeam();
	});

	DefRPC("GameStart", [this](MinNetPacket * packet)
	{
		this->GameStart();
	});
}

void ReadyUser::Awake()
{
	nickName = gameObject->owner->userValue.GetValueString("NickName");
	RPC("SetNickName", MinNetRpcTarget::AllNotServer, nickName);

	manager = gameObject->GetNowRoom()->GetGameObject("ReadyRoomManager")->GetComponent<ReadyRoomManager>();

	manager->AddUser(shared_from_this());
}

void ReadyUser::OnDestroy()
{
	if (manager != nullptr)
	{
		manager->DelUser(shared_from_this());
	}
}

void ReadyUser::OnInstantiate(MinNetUser * user)
{
	RPC("SetTeam", user, static_cast<int>(team));// 새로온 플레이어에게 기존 정보들을 알림
	RPC("SetNickName", user, nickName);

	if (isMaster)
		RPC("SetMaster", user, isMaster);
}

ReadyUser::ReadyUser()
{
}

ReadyUser::~ReadyUser()
{
}

void ReadyUser::SendChat(std::string chat)
{
	auto str2 = nickName + " : " + chat;

	RPC("ChatCast", MinNetRpcTarget::AllNotServer, str2);
}

void ReadyUser::SetMaster(bool isMaster)
{
	this->isMaster = isMaster;
}

void ReadyUser::GameStart()
{
	if (!isMaster)
		return;

	manager->GameStart(shared_from_this());
}

void ReadyUser::ChangeTeam()
{
	PlayerMove::Team changeTeam = PlayerMove::Team::None;

	if (team == PlayerMove::Team::Red)
		changeTeam = PlayerMove::Team::Blue;
	else
		changeTeam =  PlayerMove::Team::Red;

	RPC("SetTeam", MinNetRpcTarget::All, static_cast<int>(changeTeam));
}

PlayerMove::Team ReadyUser::GetTeam() const
{
	return team;
}

void ReadyUser::SetTeam(PlayerMove::Team team)
{
	orderCount = manager->GetOrderCount();
	manager->OrderSort();

	this->team = team;
}

bool ReadyUser::GetIsMaster()
{
	return isMaster;
}