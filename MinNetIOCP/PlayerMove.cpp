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
	//std::cout << "ID " << gameObject->GetID() << " 가 받은 각도 : " << chestRotation << std::endl;
}

PlayerMove::PlayerMove()
{
}

PlayerMove::~PlayerMove()
{
}