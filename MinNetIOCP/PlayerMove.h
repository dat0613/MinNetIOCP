#pragma once
#include "MinNetComponent.h"
#include "MinNet.h"

class BattleFieldManager;
class PlayerMove :
	public MinNetComponent
{
public:
	
	std::string playerName = "";

	int lastHitPlayerID = -1; // 가장 최근에 이 플레이어를 공격한 플레이어의 ID
	std::string lastHitPlayerName = ""; // 킬로그가 뜨기 전에 공격자가 나가면 ID를 통해 플레이어를 찾을 수 없기때문에 미리 이름을 저장해 둠

	clock_t lastHit = 0; // 가장 최근에 맞은 시간
	clock_t hitResetTime = 3000; // 맞은 상태를 리셋하는 시간

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
	Team nextSpawnTeam = Team::None; // 한번 죽고난 다음 바꿀 팀
	
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
	std::map<int, int> killCount;// 키값을 id로 갖는 플레이어를 얼마나 사살 하였는지
	void SyncScore();

	void SetMaxHP(int maxHP);

};