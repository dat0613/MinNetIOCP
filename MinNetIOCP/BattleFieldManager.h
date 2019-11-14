#pragma once
#include "MinNetComponent.h"
#include <list>

class PlayerMove;

class BattleFieldManager :
	public MinNetComponent
{
public:

	BattleFieldManager();
	~BattleFieldManager();

	void AddPlayer(PlayerMove * player);
	void DelPlayer(PlayerMove * player);

	void Update() override;

private:

	std::list<PlayerMove *> playerList;

};