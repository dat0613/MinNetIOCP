#include "PlayerMove.h"
 
#include "MinNetGameObject.h"
#include "MinNetRoom.h"
#include "MinNetPool.h"

void PlayerMove::InitRPC()
{
	DefRPC("SyncPosition", std::bind(&PlayerMove::SyncPosition, this));
	DefRPC("HitSync", std::bind(&PlayerMove::HitSync, this));
}

void PlayerMove::SyncPosition()
{
	gameObject->position = rpcPacket->pop_vector3();
	chestRotation = rpcPacket->pop_vector3();

	//std::cout << gameObject->position << std::endl;
	//std::cout << "ID " << gameObject->GetID() << " 가 받은 각도 : " << chestRotation << std::endl;
}

void PlayerMove::HitSync()
{
	int hitObjectID = rpcPacket->pop_int();
	Vector3 hitPosition = rpcPacket->pop_vector3();

	auto hitObj = gameObject->GetNowRoom()->GetGameObject(hitObjectID);

	if (hitObj == nullptr)
		return;

	float distance = Vector3::distance(hitPosition, hitObj->position);// 로컬과 서버의 위치 차이 계산

	if (distance < 0.5f)// 위치 차이가 너무 많이 나면 동기화가 깨졌거나 해킹된 클라이언트라고 판단
	{// 맞았다고 판단
		RPC("HitSuccess", gameObject->owner);// 적중하는데 성공했다고 알려줌
		RPC("HitCast", MinNetRpcTarget::All, hitObjectID);
	}
	else
	{// 맞지 않았다고 판단

	}
}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}