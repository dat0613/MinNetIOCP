#pragma once
#include "MinNetComponent.h"
#include "MinNet.h"
class PlayerMove :
	public MinNetComponent
{
public:
	PlayerMove();
	~PlayerMove();

	void InitRPC() override;

	void SyncPosition();
	void HitSync();

private:
	Vector3 chestRotation;

};

