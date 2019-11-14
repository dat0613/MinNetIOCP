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

	float distance = Vector3::distance(hitPosition, hitObj->position);// ���ð� ������ ��ġ ���� ���

	if (distance < 0.5f)// ��ġ ���̰� �ʹ� ���� ���� ����ȭ�� �����ų� ��ŷ�� Ŭ���̾�Ʈ��� �Ǵ�
	{// �¾Ҵٰ� �Ǵ�
		int hitDamage = damage + static_cast<int>(isHead) * damage;
		RPC("HitSuccess", gameObject->owner, isHead, hitDamage);// �����ϴµ� �����ߴٰ� �˷���
		auto component = hitObj->GetComponent<PlayerMove>();

		component->Hit(hitDamage, this);
		component->RPC("HitCast", MinNetRpcTarget::All, hitObjectID, hitDamage, isHead);
	}
	else
	{// ���� �ʾҴٰ� �Ǵ�

	}
}

void PlayerMove::Hit(int damage, PlayerMove * shooter)
{
	nowHP -= damage;

	if (nowHP <= 0)
	{
		nowHP = 0;
		// �÷��̾� ����

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