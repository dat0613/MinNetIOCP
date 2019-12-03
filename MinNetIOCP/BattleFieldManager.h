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

	enum class BattleFieldState { GameReady, GameStart, GameEnd, MAX }; // ���� �غ�, ���� ����, ���� ��


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
	clock_t readyTime = 5000;// �غ�ð��� �� �𸣰���

	clock_t gameStartTime = 0;
	clock_t gameTime = 60000;// ��ms ���� ������ ����
	
	clock_t gameEndTime = 0;
	clock_t endTime = 3000;// ���� �Ŀ� ���â�� ������


	std::list<std::weak_ptr<MinNetComponent>> playerList;

	clock_t getNowStateLeftTime();

	std::vector<std::vector<Vector3>> respawnPoints;
	std::vector<int> respawnPointCount;// �������� �������� ������ ���� �ʰ� �ϱ� ���� ���������� ������ ��Ŵ

	PlayerMove::Team getPeacefulTeam();// �÷��̾ ���� ���� ������
	bool isPeacefulTeam(PlayerMove::Team team);

	void setRespawnPoints();// �÷��̾���� ������ ������ ����
	Vector3 getRespawnPoint(PlayerMove::Team team);
	
	void PlayerRespawnUpdate(PlayerMove * player);
	void PlayerDieUpdate(PlayerMove * player);
	void SendChangeState(BattleFieldState state);
};