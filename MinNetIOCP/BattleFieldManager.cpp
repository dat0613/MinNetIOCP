#include "BattleFieldManager.h"
#include "PlayerMove.h"


void BattleFieldManager::AddPlayer(PlayerMove * player)
{
	playerList.push_back(player);
}

void BattleFieldManager::DelPlayer(PlayerMove * player)
{
	playerList.remove(player);
}

void BattleFieldManager::Update()
{
	for (auto player : playerList)
	{
		std::cout << "�÷��̾� : " << player << std::endl;
	}
}

BattleFieldManager::BattleFieldManager()
{
}


BattleFieldManager::~BattleFieldManager()
{
}