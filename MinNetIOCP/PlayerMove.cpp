#include "PlayerMove.h"
 
#include "MinNetGameObject.h"


void PlayerMove::InitRPC()
{
	DefRPC("SyncPosition", std::bind(&PlayerMove::SyncPosition, this));
}

void PlayerMove::SyncPosition()
{
	gameObject->position = rpcPacket->pop_vector3();
	chestRotation = rpcPacket->pop_vector3();

	//std::cout << gameObject->position << std::endl;
	//std::cout << "ID " << gameObject->GetID() << " �� ���� ���� : " << chestRotation << std::endl;
}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}