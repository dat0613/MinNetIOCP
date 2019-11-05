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
	//std::cout << "ID " << gameObject->GetID() << " �� ���� ���� : " << chestRotation << std::endl;
}

void PlayerMove::HitSync()
{
	int hitObjectID = rpcPacket->pop_int();
	Vector3 hitPosition = rpcPacket->pop_vector3();

	auto hitObj = gameObject->GetNowRoom()->GetGameObject(hitObjectID);

	if (hitObj == nullptr)
		return;

	float distance = Vector3::distance(hitPosition, hitObj->position);// ���ð� ������ ��ġ ���� ���

	if (distance < 0.5f)// ��ġ ���̰� �ʹ� ���� ���� ����ȭ�� �����ų� ��ŷ�� Ŭ���̾�Ʈ��� �Ǵ�
	{// �¾Ҵٰ� �Ǵ�
		RPC("HitSuccess", gameObject->owner);// �����ϴµ� �����ߴٰ� �˷���
		RPC("HitCast", MinNetRpcTarget::All, hitObjectID);
	}
	else
	{// ���� �ʾҴٰ� �Ǵ�

	}
}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}