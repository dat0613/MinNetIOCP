#include "PlayerMove.h"

#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"
#include "BattleFieldManager.h"

void PlayerMove::InitRPC()
{
	DefRPC("SyncPosition", [this](MinNetPacket * packet) {

		auto position = packet->pop_vector3();
		auto chestRotation = packet->pop_vector3();

		SyncPosition(position, chestRotation);
	});

	DefRPC("HitSync", [this](MinNetPacket * packet) {

		int hitObjectID = packet->pop_int();
		Vector3 hitPosition = packet->pop_vector3();
		Vector3 shotPosition = packet->pop_vector3();
		bool isHead = packet->pop_bool();

		HitSync(hitObjectID, hitPosition, shotPosition, isHead);
	});

	DefRPC("Respawn", [this](MinNetPacket * packet) {

		auto respawnPosition = packet->pop_vector3();
		auto repawnHp = packet->pop_int();
		auto team = static_cast<Team>(packet->pop_int());

		Respawn(respawnPosition, repawnHp, team);
	});

	DefRPC("SelectTeam", [this](MinNetPacket * packet) {

		auto team = static_cast<Team>(packet->pop_int());

		SelectTeam(team);
	});

	DefRPC("Chat", [this](MinNetPacket * packet) {

		auto chat = packet->pop_string();

		Chat(chat);
	});
}

void PlayerMove::SyncPosition(Vector3 position, Vector3 chestRotation)
{
	gameObject->position = position;
	this->chestRotation = chestRotation;
}

void PlayerMove::HitSync(int hitObjectID, Vector3 hitPosition, Vector3 shotPosition, bool isHead)
{
	auto hitObj = gameObject->GetNowRoom()->GetGameObject(hitObjectID);

	if (hitObj == nullptr)
		return;

	auto hitComponent = hitObj->GetComponent<PlayerMove>();

	if ((hitComponent->team == team) && (hitObjectID != gameObject->GetID()))
	{// 아군 사격은 불가능하지만 자신에게 피해를 입히는건 가능함
		return;
	}

	float distance = Vector3::distance(hitPosition, hitObj->position);// 로컬과 서버의 위치 차이 계산

	if (distance < 0.5f)// 위치 차이가 너무 많이 나면 동기화가 깨졌거나 해킹된 클라이언트라고 판단
	{// 맞았다고 판단
		int hitDamage = damage + static_cast<int>(isHead) * damage;
		RPC("HitSuccess", gameObject->owner, isHead, hitDamage);// 적중하는데 성공했다고 알려줌

		hitComponent->Hit(hitDamage, this);
		hitComponent->lastHead = isHead;
		hitComponent->RPC("HitCast", MinNetRpcTarget::AllNotServer, shotPosition, hitDamage, isHead);
	}
	else
	{// 맞지 않았다고 판단
		//std::cout << "거리 차이가 너무 큼 : " << hitPosition << ", " << hitObj->position << ", " << distance << std::endl;
	}
}

void PlayerMove::Hit(int damage, PlayerMove * shooter)
{
	nowHP -= damage;

	lastHitPlayerID = shooter->gameObject->GetID();
	lastHitPlayerName = shooter->playerName;

	lastHit = Time::curTime();

	if (nowHP <= 0)
	{
		nowHP = 0;
		// 플레이어 죽음
	}
}


void PlayerMove::ChangeTeam(Team team)
{
	if (this->team == team)
		return;

	switch (team)
	{
	case PlayerMove::Team::Red:

		break;

	case PlayerMove::Team::Blue:

		break;

	case PlayerMove::Team::Spectator:

		break;

	case PlayerMove::Team::Individual:

		break;

	case PlayerMove::Team::None:

		break;

	default:

		break;
	}

	this->team = team;
}

void PlayerMove::ChangeState(State state)
{
	if (this->state == state)
		return;

	switch (state)
	{
	case PlayerMove::State::Alive:
		HitInformationReset();
		break;

	case PlayerMove::State::Die:
	{
		if (battleFieldManager == nullptr)
			break;

		RPC("PlayerDie", MinNetRpcTarget::All, lastHitPlayerID, lastHitPlayerName, lastHead);

		auto killer = battleFieldManager->GetPlayer(lastHitPlayerID);
		death++;// 죽은 기록을 증가시킴
		killer->AddKillCount(gameObject->GetID());
		auto myKillCount = GetKillCount(lastHitPlayerID);// 내가 상대를 처치한 횟수
		auto killersKillCount = killer->GetKillCount(gameObject->GetID());// 상대가 나를 처치한 횟수

		RPC("DieInformation", gameObject->owner, lastHitPlayerID, myKillCount, killersKillCount, battleFieldManager->playerRespawnDelay, Time::curTime());// 죽고죽인 횟수를 보여주면 경쟁하는 재미가 생기지 않지만 넣음
		
		dieTime = Time::curTime();
		break;
	}
	default:
		break;
	}

	this->state = state;
}

void PlayerMove::Awake()
{
	auto nowRoom = gameObject->GetNowRoom();
	if (nowRoom == nullptr)
		return;

	auto battleFieldManagerObject = nowRoom->GetGameObject("BattleFieldManager");
	if (battleFieldManagerObject == nullptr)
		return;

	battleFieldManager = battleFieldManagerObject->GetComponent<BattleFieldManager>();
	if (battleFieldManager == nullptr)
		return;

	battleFieldManager->AddPlayer(shared_from_this());
}

void PlayerMove::Update()
{
	if (lastHitPlayerID != -1)
	{
		if (Time::curTime() - lastHit > hitResetTime)
		{
			HitInformationReset();
		}
	}

	if (state == State::Alive)
	{
		if (gameObject->position.y < -25.0f)
		{// 땅을 뚫고 떨어저서 사망
			Hit(9000, this);
			RPC("HitCast", MinNetRpcTarget::AllNotServer, Vector3(0.0f, 0.0f, 0.0f), gameObject->GetID(), 9000, true);
		}
	}
}

void PlayerMove::OnInstantiate(MinNetUser * user)
{
	RPC("OnInstantiate", user, static_cast<int>(team), static_cast<int>(state), nowHP);
	SyncScore();
}

void PlayerMove::OnDestroy()
{
	if (battleFieldManager != nullptr)
	{
		battleFieldManager->DelPlayer(shared_from_this());
	}
}

void PlayerMove::HitInformationReset()
{
	lastHitPlayerID = -1;
	lastHitPlayerName = "";
	lastHead = false;
}

void PlayerMove::Respawn(Vector3 position, int hp, PlayerMove::Team team)
{
	gameObject->position = position;
	nowHP = hp;
	ChangeState(State::Alive);
	ChangeTeam(team);
}

void PlayerMove::SelectTeam(Team team)
{
	if (this->team != team) // 현재와 같은팀으로 바꾸는건 필요하지 않음
		nextSpawnTeam = team;
}

void PlayerMove::Chat(std::string chat)
{
	float r, g, b, a;
	r = g = b = a = 1.0f;

	switch (team)
	{
	case PlayerMove::Team::Red:
		r = 1.0f;
		g = 0.0f;
		b = 0.0f;
		break;

	case PlayerMove::Team::Blue:
		r = 0.0f;
		g = 0.0f;
		b = 1.0f;
		break;

	case PlayerMove::Team::Spectator:
	case PlayerMove::Team::Individual:
	case PlayerMove::Team::None:
		break;
	}

	std::string str = playerName + " : " + chat;

	RPC("Chat", MinNetRpcTarget::AllNotServer, str, r, g, b, a);
}

void PlayerMove::SyncScore()
{
	RPC("SyncScore", MinNetRpcTarget::AllNotServer, this->kill, this->death);
}

void PlayerMove::AddScore(int kill, int death)
{
	this->kill += kill;
	this->death += death;

	SyncScore();
}

int PlayerMove::GetKillCount(int victimId)
{
	auto set = killCount.find(victimId);
	auto retval = 0;

	if (set == killCount.end())
	{// 이 상대에 대한 기록이 없음
		killCount.insert(std::make_pair(victimId, 0));// 처치가 0인 기록을 미리 만들어 둠
	}
	else
	{// 기록이 있음
		retval = set->second;
	}

	return retval;
}

void PlayerMove::AddKillCount(int victimId)
{
	auto set = killCount.find(victimId);

	if (set == killCount.end())
	{// 이 상대에 대한 기록이 없음
		killCount.insert(std::make_pair(victimId, 1));// 첫 처치를 기록함
	}
	else
	{// 이 상대에 대한 처치 횟수를 올림
		killCount[victimId] = set->second + 1;
	}

	kill++;// 전체 처치 기록도 올림
}

bool PlayerMove::IsCanRespawn(clock_t playerRespawnDelay)
{
	if (state == State::Die)
	{
		if (Time::curTime() - dieTime > playerRespawnDelay)
			return true;
	}
	return false;
}

bool PlayerMove::IsDie()
{
	if (nowHP <= 0)
	{
		nowHP = 0;
		return true;
	}

	return false;
}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}