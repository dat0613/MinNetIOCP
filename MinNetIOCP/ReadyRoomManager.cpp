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
	//std::cout << roomName.c_str() << " �� �̸��� ���� ���ӹ��� ����" << std::endl;
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
	{// �� â���� ���������� ���� ����Ʈ�� �ִ� ������� ��Ŷ�� ������
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
	{// ������ �������Ƿ� ���� �������� ���� ���� �÷��̾ �������� ������
		if (userList.size() > 0)
		{
			userList.front().lock()->RPC("SetMaster", MinNetRpcTarget::All, true);
		}
		else
		{// �̶��� ���� ��Ʈ��

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
	{// ����� �ʹ� ���� ���ӽ��� �Ұ���
		reason = "�÷��̾ �ʹ� ���� ������ ������ �� �����ϴ�.";
	}
	else if (abs(blueTeamCount - redTeamCount) > 1)
	{// ������ �յ����� ����
		reason = "���� ���� �÷��̾ ���� �ֽ��ϴ�.";
	}

	if (reason == "")
	{// �׽���
		std::cout << "���� ����" << std::endl;
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
	{// ����� ������ �������� ��
		if (rand() % 2 == 0)
			return PlayerMove::Team::Red;
		else
			return PlayerMove::Team::Blue;
	}
	else
	{
		if (blueTeamCount > redTeamCount)// �������� ����� �� ����
			return PlayerMove::Team::Red;
		else// ������� ����� �� ����
			return PlayerMove::Team::Blue;
	}
	return PlayerMove::Team();
}
