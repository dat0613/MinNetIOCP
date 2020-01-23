#pragma once
#include "MinNetComponent.h"
class MinNetNavMeshAgent :
	public MinNetComponent
{
public:

	MinNetNavMeshAgent();
	~MinNetNavMeshAgent();

	void Awake() override;
	void OnDestroy() override;
	Vector3 halfExtends = Vector3::half;

	void SetDestination(Vector3 & dest);

	dtCrowdAgentParams agentParams;

	void UpdateAgentParams();

	void SetIdx(const int idx);

private:

	void SetDefaultParameter();

	int agentIdx = -1;
};