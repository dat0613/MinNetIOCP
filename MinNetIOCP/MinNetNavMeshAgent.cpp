#include "MinNetNavMeshAgent.h"



MinNetNavMeshAgent::MinNetNavMeshAgent()
{
}


MinNetNavMeshAgent::~MinNetNavMeshAgent()
{
}

void MinNetNavMeshAgent::Awake()
{
	gameObject->GetNowRoom()->AddNavAgent(this);
}

void MinNetNavMeshAgent::OnDestroy()
{
	gameObject->GetNowRoom()->DelNavAgent(agentIdx);
}

void MinNetNavMeshAgent::SetDestination(Vector3 & dest)
{
	gameObject->GetNowRoom()->RequestMoveTarget(agentIdx, dest, halfExtends);
}

void MinNetNavMeshAgent::UpdateAgentParams()
{
	agentParams.userData = this;
	gameObject->GetNowRoom()->UpdateAgentParameters(agentIdx, &agentParams);
}

void MinNetNavMeshAgent::SetIdx(const int idx)
{
	if (idx < 0)
	{

		return;
	}
	agentIdx = idx;
}

void MinNetNavMeshAgent::SetDefaultParameter()
{
	memset(&agentParams, 0, sizeof(agentParams));

	agentParams.radius = 1.0f;
	agentParams.height = 1.0f;
	agentParams.maxAcceleration = 8.0f;
	agentParams.maxSpeed = 3.5f;
	agentParams.collisionQueryRange = agentParams.radius * 12.0f;
	agentParams.pathOptimizationRange = agentParams.radius * 30.0f;
	agentParams.updateFlags = 0;
	agentParams.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	agentParams.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	agentParams.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	agentParams.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	agentParams.updateFlags |= DT_CROWD_SEPARATION;
	agentParams.obstacleAvoidanceType = 3.0f;
	agentParams.separationWeight = 2.0f;

	agentParams.userData = this;
}

Vector3 MinNetNavMeshAgent::GetRandomPointAround()
{
	return Vector3();
}
