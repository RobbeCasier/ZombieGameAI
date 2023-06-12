#pragma once
#include <Exam_HelperStructs.h>
#include "SteeringAgent.h"
#include "SteeringBehavior.h"
#include "DecisionMaking.h"
class BlendedSteering;

class PlayerAgent : public SteeringAgent
{
public:
	PlayerAgent(AgentInfo* pAgentInfo);
	virtual ~PlayerAgent() = default;
	void Update(float deltaT, AgentInfo* pAgentInfo);

	virtual SteeringPlugin_Output CalculateSteering(float deltaT) override;

	void SetDecisionMaking(DecisionMaking* pDecisionMaking);

	void SetToSeek(Vector2 position);
	void SetToFlee(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity);
	void SetToEvade(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity);
	void SetToFace(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity);
	void SetToAvoid(Vector2 positionOriginalTarget, std::vector<EnemyInfo> enemies);

	Vector2 DebugTarget() const;
private:
	DecisionMaking* m_pDecisionMaking = nullptr;

	SteeringBehavior* m_pWander = nullptr;
	SteeringBehavior* m_pSeek = nullptr;
	SteeringBehavior* m_pFlee = nullptr;
	SteeringBehavior* m_pEvade = nullptr;
	SteeringBehavior* m_pPursue = nullptr;
	SteeringBehavior* m_pFace = nullptr;
	SteeringBehavior* m_pAvoid = nullptr;

	BlendedSteering* m_pToSeek = nullptr;
	BlendedSteering* m_pToEvade = nullptr;
	BlendedSteering* m_pToFlee = nullptr;
	BlendedSteering* m_pToFace = nullptr;
	BlendedSteering* m_pToAvoid = nullptr;

	//hardcoded behavior weights
	//wander
	const float defaultWander = 0.f;				//default value
	const float evadeWanderValue = 0.f;				//evade
	const float fleeWanderValue = 0.f;				//flee
	const float faceWanderValue = 0.f;				//face

	//seek
	const float defaultSeek = 1.f;					//default value
	const float faceSeekValue = 0.f;				//face
	const float fleeSeekValue = 0.f;				//flee

	//flee
	const float defaultFlee = 0.f;					//default value
	const float fleeValue = 1.0f;

	//evade
	const float defaultEvade = 0.f;					//default value
	const float evadeValue = 1.f;					//evade
	const float fleeEvadeValue = 0.5f;				//flee
	const float faceEvadeValue = 0.f;				//face
		
	//pursue
	const float defaultPursue = 0.f;				//default value

	//face
	const float defaultFace = 0.f;					//default value
	const float faceValue = 1.f;					//face
	const float evadeFaceValue = 0.f;				//evade
	const float fleeFaceValue = 1.f;				//flee
};

