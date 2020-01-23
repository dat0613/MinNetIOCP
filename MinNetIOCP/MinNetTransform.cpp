#include "MinNetTransform.h"
#include "Debug.h"


MinNetTransform::MinNetTransform()
{
}


MinNetTransform::~MinNetTransform()
{
}

float MinNetTransform::SyncRateAvg()
{
	if (syncCount < 1 || syncTimeSum < 1)
	{
		return 1;
	}
	else
	{
		return syncTimeSum / syncCount;
	}
}

void MinNetTransform::SyncPosition(Vector3 position, Vector3 rotation)
{
	if (serverIsOriginal)
		return;

	gameObject->position = position;
	gameObject->rotation = rotation;

	syncCount++;
}

void MinNetTransform::InitRPC()
{
	DefRPC("SyncPosition", [this](MinNetPacket * packet)
	{
		auto position = packet->pop_vector3();
		auto rotation = packet->pop_vector3();

		SyncPosition(position, rotation);
	});
}

void MinNetTransform::Update()
{
	if (!serverIsOriginal)
		return;

	float nowTime = MinNetTime::time();
	if (nowTime - lastSyncTime > 1.0f / syncPerSecond)
	{
		RPC("SyncPosition", MinNetRpcTarget::AllNotServer, gameObject->position, gameObject->rotation);
		lastSyncTime = nowTime;
	}
}
