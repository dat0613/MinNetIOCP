#pragma once
#include "MinNetComponent.h"
#include "MinNetGameObject.h"
#include "MinNet.h"

class FirstPersonController :
	public MinNetComponent
{
public:

	~FirstPersonController();
	
	void InitRPC() override;

	void SyncPosition(MinNetPacket * rpcPacket);

private:



};

