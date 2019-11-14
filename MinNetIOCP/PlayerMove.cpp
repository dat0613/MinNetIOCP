#include "PlayerMove.h"
 
#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"
#include "BattleFieldManager.h"

void PlayerMove::InitRPC()
{
	DefRPC("SyncPosition", std::bind(&PlayerMove::SyncPosition, this, std::placeholders::_1));
	DefRPC("HitSync", std::bind(&PlayerMove::HitSync, this, std::placeholders::_1));
}

void PlayerMove::SyncPosition(MinNetPacket * rpcPacket)
{
	gameObject->position = rpcPacket->pop_vector3();
	chestRotation = rpcPacket->pop_vector3();
}

void PlayerMove::HitSync(MinNetPacket * rpcPacket)
{
	int hitObjectID = rpcPacket->pop_int();
	Vector3 hitPosition = rpcPacket->pop_vector3();
	bool isHead = rpcPacket->pop_bool();

	auto hitObj = gameObject->GetNowRoom()->GetGameObject(hitObjectID);

	if (hitObj == nullptr)
		return;

	float distance = Vector3::distance(hitPosition, hitObj->position);// 로컬과 서버의 위치 차이 계산

	if (distance < 0.5f)// 위치 차이가 너무 많이 나면 동기화가 깨졌거나 해킹된 클라이언트라고 판단
	{// 맞았다고 판단
		int hitDamage = damage + static_cast<int>(isHead) * damage;
		RPC("HitSuccess", gameObject->owner, isHead, hitDamage);// 적중하는데 성공했다고 알려줌
		auto component = hitObj->GetComponent<PlayerMove>();

		component->Hit(hitDamage, this);
		component->RPC("HitCast", MinNetRpcTarget::All, hitObjectID, hitDamage, isHead);
	}
	else
	{// 맞지 않았다고 판단

	}
}

void PlayerMove::Hit(int damage, PlayerMove * shooter)
{
	nowHP -= damage;

	if (nowHP <= 0)
	{
		nowHP = 0;
		// 플레이어 죽음

		RPC("PlayerDie", MinNetRpcTarget::All, shooter->gameObject->GetID());
		ChangeState(State::Die);
	}
}



void PlayerMove::ChangeState(State state)
{
	this->state = state;
}

void PlayerMove::Awake()
{
	battleFieldManager = gameObject->GetNowRoom()->GetGameObject("BattleFieldManager")->GetComponent<BattleFieldManager>();
	std::cout << battleFieldManager << std::endl;
}

void PlayerMove::Update()
{

}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}