#pragma once
#include "MinNetComponent.h"
#include "MinNet.h"

class BattleFieldManager;
class PlayerMove :
	public MinNetComponent
{
public:

	enum class Team { Red, Blue, Spectator, Individual };
	enum class State { Alive, Die };

	BattleFieldManager * battleFieldManager = nullptr;
	Team team = Team::Spectator;

	int maxHP = 100;
	int nowHP = maxHP;

	int damage = 20;

	State state = State::Alive;

	PlayerMove();
	~PlayerMove();

	void InitRPC() override;

	void SyncPosition(MinNetPacket * rpcPacket);
	void HitSync(MinNetPacket * rpcPacket);
	
	void Hit(int damage, PlayerMove * shooter);

	void ChangeState(State state);
	void Awake() override;
	void Update() override;

private:

	Vector3 chestRotation;
};