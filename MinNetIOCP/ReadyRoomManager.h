#pragma once
#include "MinNetComponent.h"
#include "PlayerMove.h"

class ReadyUser;
class ReadyRoomManager :
	public MinNetComponent
{
public:
	ReadyRoomManager();
	~ReadyRoomManager();
	
	void OnInstantiate(MinNetUser * user) override;

	void SetOption(std::string roomName, int TeamNumber, bool CanBargeIn, bool OnlyHeadShot, int TicketCount, float RespawnTime, int DefaultDamage, float HeadShotDamageMultiple, int PlayerMaxHP);

	void AddUser(std::weak_ptr<MinNetComponent> user);
	void DelUser(std::weak_ptr<MinNetComponent> user);

	void GameStart(std::weak_ptr<MinNetComponent> component);
	int GetOrderCount();// 대기실에서 플레이어들의 순서 보장을 위해 사용함
	void OrderSort();

private:

	std::string roomName = "";
	int TeamNumber = 0;
	bool CanBargeIn = false;
	bool OnlyHeadShot = false;
	int TicketCount = 0;
	float RespawnTime = 0.0f;
	int DefaultDamage = 0;
	float HeadShotDamageMultiple = 0.0f;
	int PlayerMaxHP = 0;
	int orderCount = 0;

	std::list<std::weak_ptr<MinNetComponent>> userList;

	PlayerMove::Team GetPeacefulTeam();
};