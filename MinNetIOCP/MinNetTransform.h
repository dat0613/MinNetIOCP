#pragma once
#include "MinNetComponent.h"
#include "MinNetTime.h"

class MinNetTransform :
	public MinNetComponent
{
public:
	MinNetTransform();
	~MinNetTransform();

	int syncPerSecond = 1;
	float teleportDistance = 5.0f;
	float rotationLerp = 1.0f;
	bool serverIsOriginal = false;

	float SyncRateAvg();

	void SyncPosition(Vector3 position, Vector3 rotation);

	void InitRPC() override;
	void Update() override;

private:

	float lastSendTime = 0.0f;

	float lastSyncTime = 0;// 가장 최근에 클라이언트로 부터 동기화 패킷을 받은 시간
	float syncTimeSum = 0;

	unsigned int syncCount = 0;

	Vector3 lastPosition = Vector3::zero;
	Vector3 lastRotation = Vector3::zero;

	Vector3 targetPosition = Vector3::zero;
	Vector3 targetRotation = Vector3::zero;
};

