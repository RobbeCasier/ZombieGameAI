#pragma once
#include <Exam_HelperStructs.h>
class SteeringBehavior;
class BlendedSteering;

class SteeringAgent
{
public:
	SteeringAgent(AgentInfo* pAgentInfo) : m_pAgentInfo(pAgentInfo) {};
	virtual ~SteeringAgent() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT) = 0;

	virtual void SetSteeringBehavior(BlendedSteering* pBehavior) { m_pSteeringBehavior = pBehavior; }
protected:
	AgentInfo* m_pAgentInfo;
	BlendedSteering* m_pSteeringBehavior = nullptr;
};

