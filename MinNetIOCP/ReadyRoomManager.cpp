#include "ReadyRoomManager.h"

#include "ReadyUser.h"

ReadyRoomManager::ReadyRoomManager()
{
}


ReadyRoomManager::~ReadyRoomManager()
{
}

void ReadyRoomManager::OnInstantiate(MinNetUser * user)
{
	RPC("LoadRoomSetting", user, roomName, TeamNumber, CanBargeIn, OnlyHeadShot, TicketCount, RespawnTime, DefaultDamage, HeadShotDamageMultiple, PlayerMaxHP);
}

void ReadyRoomManager::SetOption(std::string roomName, int TeamNumber, bool CanBargeIn, bool OnlyHeadShot, int TicketCount, float RespawnTime, int DefaultDamage, float HeadShotDamageMultiple, int PlayerMaxHP)
{
	//std::cout << roomName.c_str() << " 의 이름을 가진 게임방을 만듦" << std::endl;
	this->roomName = roomName;
	this->TeamNumber = TeamNumber;
	this->CanBargeIn = CanBargeIn;
	this->OnlyHeadShot = OnlyHeadShot;
	this->TicketCount = TicketCount;
	this->RespawnTime = RespawnTime;
	this->DefaultDamage = DefaultDamage;
	this->HeadShotDamageMultiple = HeadShotDamageMultiple;
	this->PlayerMaxHP = PlayerMaxHP;
}

void ReadyRoomManager::AddUser(std::weak_ptr<MinNetComponent> user)
{
	auto team = GetPeacefulTeam();
	auto sharedUser = user.lock();

	if (userList.size() == 0)
	{
		sharedUser->RPC("SetMaster", MinNetRpcTarget::All, true);
	}

	for (auto component : userList)
	{// 팀 창에서 순서보장을 위해 리스트에 있는 순서대로 패킷을 보내줌
		auto oldUser = static_cast<ReadyUser *>(component.lock().get());
		sharedUser->RPC("SetTeam", sharedUser->gameObject->owner, static_cast<int>(oldUser->GetTeam()));
	}

	user.lock()->RPC("SetTeam", MinNetRpcTarget::All, static_cast<int>(team));
	userList.push_back(user);
}

void ReadyRoomManager::DelUser(std::weak_ptr<MinNetComponent> user)
{
	userList.remove_if([user](std::weak_ptr<MinNetComponent> p)
	{
		auto srcPlayer = user.lock();
		auto dstPlayer = p.lock();

		if (srcPlayer && dstPlayer)
			return srcPlayer == dstPlayer;

		return false;
	});

	auto readyUser = static_cast<ReadyUser *>(user.lock().get());
	if (readyUser->GetIsMaster())
	{// 방장이 나갔으므로 방장 다음으로 먼저 들어온 플레이어를 방장으로 지정함
		if (userList.size() > 0)
		{
			userList.front().lock()->RPC("SetMaster", MinNetRpcTarget::All, true);
		}
		else
		{// 이때는 방을 터트림

		}
	}
}

void ReadyRoomManager::GameStart(std::weak_ptr<MinNetComponent> component)
{
	auto user = static_cast<ReadyUser *>(component.lock().get());
	if (!user->GetIsMaster())
		return;

	int blueTeamCount = 0, redTeamCount = 0;

	for (auto comp : userList)
	{
		if (comp.expired())
			continue;

		auto user = static_cast<ReadyUser *> (comp.lock().get());
		auto team = user->GetTeam();
		if (team == PlayerMove::Team::Blue)
		{
			blueTeamCount++;
		}
		else if (team == PlayerMove::Team::Red)
		{
			redTeamCount++;
		}
	}

	std::string reason = "";

	if (blueTeamCount + redTeamCount < 2)
	{// 사람이 너무 적어 게임시작 불가능
		reason = "플레이어가 너무 적어 게임을 시작할 수 없습니다.";
	}
	else if (abs(blueTeamCount - redTeamCount) > 1)
	{// 팀원이 균등하지 않음
		reason = "한쪽 팀에 플레이어가 몰려 있습니다.";
	}

	if (reason == "")
	{// 겜시작
		std::cout << "게임 시작" << std::endl;
		gameObject->GetNowRoom()->ChangeRoom("BattleField");
	}
	else
	{
		user->RPC("CantStartGame", MinNetRpcTarget::AllNotServer, reason);
	}
}

int ReadyRoomManager::GetOrderCount()
{
	return orderCount++;
}

void ReadyRoomManager::OrderSort()
{
	userList.sort([](std::weak_ptr<MinNetComponent> c1, std::weak_ptr<MinNetComponent> c2)
	{
		auto u1 = static_cast<ReadyUser *>(c1.lock().get());
		auto u2 = static_cast<ReadyUser *>(c2.lock().get());

		if (u1->orderCount > u2->orderCount)
			return true;
		return false;
	});
}

PlayerMove::Team ReadyRoomManager::GetPeacefulTeam()
{
	int blueTeamCount = 0, redTeamCount = 0;

	for (auto component : userList)
	{
		if (component.expired())
			continue;

		auto user = static_cast<ReadyUser *> (component.lock().get());
		auto team = user->GetTeam();
		if (team == PlayerMove::Team::Blue)
		{
			blueTeamCount++;
		}
		else if (team == PlayerMove::Team::Red)
		{
			redTeamCount++;
		}
	}

	if (blueTeamCount == redTeamCount)
	{// 사람수 같으면 랜덤으로 함
		if (rand() % 2 == 0)
			return PlayerMove::Team::Red;
		else
			return PlayerMove::Team::Blue;
	}
	else
	{
		if (blueTeamCount > redTeamCount)// 레드팀에 사람이 더 적음
			return PlayerMove::Team::Red;
		else// 블루팀에 사람이 더 적음
			return PlayerMove::Team::Blue;
	}
	return PlayerMove::Team();
}
