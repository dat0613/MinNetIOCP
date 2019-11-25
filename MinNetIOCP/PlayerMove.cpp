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

	float distance = Vector3::distance(hitPosition, hitObj->position);// ���ð� ������ ��ġ ���� ���

	if (distance < 0.5f)// ��ġ ���̰� �ʹ� ���� ���� ����ȭ�� �����ų� ��ŷ�� Ŭ���̾�Ʈ��� �Ǵ�
	{// �¾Ҵٰ� �Ǵ�
		int hitDamage = damage + static_cast<int>(isHead) * damage;
		RPC("HitSuccess", gameObject->owner, isHead, hitDamage);// �����ϴµ� �����ߴٰ� �˷���
		auto component = hitObj->GetComponent<PlayerMove>();

		component->Hit(hitDamage, this);
		component->lastHead = isHead;
		component->RPC("HitCast", MinNetRpcTarget::AllNotServer, shotPosition, hitDamage, isHead);
	}
	else
	{// ���� �ʾҴٰ� �Ǵ�
		std::cout << "�Ÿ� ���̰� �ʹ� ŭ : " << hitPosition << ", " << hitObj->position << ", " << distance << std::endl;
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
		// �÷��̾� ����
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
		RPC("PlayerDie", MinNetRpcTarget::All, lastHitPlayerID, lastHitPlayerName, lastHead);
		dieTime = Time::curTime();
		break;

	default:
		break;
	}

	this->state = state;
}

void PlayerMove::Awake()
{
	battleFieldManager = gameObject->GetNowRoom()->GetGameObject("BattleFieldManager")->GetComponent<BattleFieldManager>();
	battleFieldManager->AddPlayer(shared_from_this());
}

void PlayerMove::Update()
{
	if (lastHitPlayerID != -1)
	{
		if (Time::curTime() - dieTime > hitResetTime)
		{
			HitInformationReset();
		}
	}

	if (state == State::Alive)
	{
		if (gameObject->position.y < -25.0f)
		{// ���� �հ� �������� ���
			Hit(9000, this);
			RPC("HitCast", MinNetRpcTarget::AllNotServer, Vector3(0.0f, 0.0f, 0.0f), gameObject->GetID(), 9000, true);
		}
	}
}

void PlayerMove::OnInstantiate(MinNetUser * user)
{
	RPC("OnInstantiate", user, static_cast<int>(team), static_cast<int>(state), nowHP);
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
	std::cout << "���� ������ ���� �ٲ� : " << static_cast<int>(team) << std::endl;
	if (this->team != team) // ����� ���������� �ٲٴ°� �ʿ����� ����
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