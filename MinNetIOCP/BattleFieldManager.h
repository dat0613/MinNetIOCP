#pragma once
#include "MinNetComponent.h"
#include <list>
#include <vector>
#include "PlayerMove.h"

class BattleFieldManager :
	public MinNetComponent
{
public:

	BattleFieldManager();
	~BattleFieldManager();

	enum class BattleFieldState { GameReady, GameStart, GameEnd, MAX }; // 게임 준비, 게임 시작, 게임 끝


	BattleFieldState state = BattleFieldState::MAX;

	void AddPlayer(std::weak_ptr<MinNetComponent> player);
	void DelPlayer(std::weak_ptr<MinNetComponent> player);

	void ChangeState(BattleFieldState state, clock_t time);

	PlayerMove * GetPlayer(int id);

	void InitRPC() override;
	void Awake() override;
	void Update() override;
	void OnInstantiate(MinNetUser * user) override;
	void OnDestroy() override;


	clock_t playerRespawnDelay = 3000;
private:


	clock_t readyStartTime = 0;
	clock_t readyTime = 5000;// 준비시간은 잘 모르겠음

	clock_t gameStartTime = 0;
	clock_t gameTime = 60000;// 몇ms 동안 게임을 할지
	
	clock_t gameEndTime = 0;
	clock_t endTime = 3000;// 몇초 후에 결과창을 보낼지


	std::list<std::weak_ptr<MinNetComponent>> playerList;

	clock_t getNowStateLeftTime();

	std::vector<std::vector<Vector3>> respawnPoints;
	std::vector<int> respawnPointCount;// 같은곳에 여러명이 리스폰 되지 않게 하기 위해 순차적으로 리스폰 시킴

	PlayerMove::Team getPeacefulTeam();// 플레이어가 적은 팀을 리턴함
	bool isPeacefulTeam(PlayerMove::Team team);

	void setRespawnPoints();// 플레이어들의 리스폰 지점을 정함
	Vector3 getRespawnPoint(PlayerMove::Team team);
	
	void PlayerRespawnUpdate(PlayerMove * player);
	void PlayerDieUpdate(PlayerMove * player);
	void SendChangeState(BattleFieldState state);
};