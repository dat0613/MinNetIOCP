#pragma once
#include "MinNetComponent.h"
#include "MinNet.h"

class BattleFieldManager;
class PlayerMove :
	public MinNetComponent
{
public:
	
	std::string playerName = "";

	int lastHitPlayerID = -1; // ���� �ֱٿ� �� �÷��̾ ������ �÷��̾��� ID
	std::string lastHitPlayerName = ""; // ų�αװ� �߱� ���� �����ڰ� ������ ID�� ���� �÷��̾ ã�� �� ���⶧���� �̸� �̸��� ������ ��

	clock_t lastHit = 0; // ���� �ֱٿ� ���� �ð�
	clock_t hitResetTime = 3000; // ���� ���¸� �����ϴ� �ð�

	bool lastHead = false;

	int kill = 0;
	int death = 0;

	void AddScore(int kill, int death);

	int GetKillCount(int victimId);
	void AddKillCount(int victimId);

	enum class Team { Red, Blue, Spectator, Individual, None };
	enum class State { Alive, Die, None };

	BattleFieldManager * battleFieldManager = nullptr;

	Team team = Team::Spectator;
	Team nextSpawnTeam = Team::None; // �ѹ� �װ� ���� �ٲ� ��
	
	State state = State::Die;

	int maxHP = 150;
	int nowHP = maxHP;

	int damage = 20;

	clock_t dieTime = 0;

	bool IsCanRespawn(clock_t playerRespawnDelay);
	bool IsDie();

	PlayerMove();
	~PlayerMove();

	void InitRPC() override;

	void SyncPosition(Vector3 position, Vector3 chestRotation);
	void HitSync(int hitObjectID, Vector3 hitPosition, Vector3 shotPosition, bool isHead);

	void Hit(int damage, PlayerMove * shooter);

	void ChangeTeam(Team team);
	void ChangeState(State state);
	void Awake() override;
	void Update() override;
	void OnInstantiate(MinNetUser * user) override;
	void OnDestroy() override;
	void HitInformationReset();

	void Respawn(Vector3 position, int hp, PlayerMove::Team team);

	void SelectTeam(Team team);

	void Chat(std::string chat);

private:

	Vector3 chestRotation;
	std::map<int, int> killCount;// Ű���� id�� ���� �÷��̾ �󸶳� ��� �Ͽ�����
	void SyncScore();

	void SetMaxHP(int maxHP);

};