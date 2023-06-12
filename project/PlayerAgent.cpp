#include "stdafx.h"
#include "PlayerAgent.h"
#include "CombinedBehaviors.h"

PlayerAgent::PlayerAgent(AgentInfo* pAgentInfo)
	: SteeringAgent{pAgentInfo}
{
	m_pWander = new Wander();
	m_pSeek = new Seek();
	m_pFlee = new Flee();
	m_pEvade = new Evade();
	m_pPursue = new Pursue();
	m_pFace = new Face();
	m_pAvoid = new CollisionAvoidance();

	m_pToSeek = new BlendedSteering(
		{ 
			{m_pWander, defaultWander},
			{m_pSeek, defaultSeek},
			{m_pFlee, defaultFlee},
			{m_pEvade, defaultEvade}, 
			{m_pPursue, defaultPursue},
			{m_pFace, defaultFace}
		});
	m_pToEvade = new BlendedSteering(
		{
			{m_pWander, evadeWanderValue},
			{m_pSeek, defaultSeek},
			{m_pFlee, defaultFlee},
			{m_pEvade, evadeValue},
			{m_pPursue, defaultPursue},
			{m_pFace, evadeFaceValue}
		});
	m_pToFace = new BlendedSteering(
		{
			{m_pWander, faceWanderValue},
			{m_pSeek, faceSeekValue},
			{m_pFlee, defaultFlee},
			{m_pEvade, faceEvadeValue},
			{m_pPursue, defaultPursue},
			{m_pFace, faceValue}
		});
	m_pToFlee = new BlendedSteering(
		{
			{m_pWander, fleeWanderValue},
			{m_pSeek, fleeSeekValue},
			{m_pFlee, fleeValue},
			{m_pEvade, fleeEvadeValue},
			{m_pPursue, defaultPursue},
			{m_pFace, fleeFaceValue}
		});
	m_pToAvoid = new BlendedSteering(
		{
			{m_pWander, 0.f},
			{m_pSeek, 0.f},
			{m_pFlee, 0.f},
			{m_pEvade, 0.f},
			{m_pPursue, 0.f},
			{m_pFace, 0.f},
			{m_pAvoid, 1.0f}
		}
	);
	m_pSteeringBehavior = m_pToSeek;
}

void PlayerAgent::Update(float deltaT, AgentInfo* pAgentInfo)
{
	m_pAgentInfo = pAgentInfo;

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaT);
}

SteeringPlugin_Output PlayerAgent::CalculateSteering(float deltaT)
{
	return m_pSteeringBehavior->CalculateSteering(deltaT, m_pAgentInfo);
}

void PlayerAgent::SetDecisionMaking(DecisionMaking* pDecisionMaking)
{
	m_pDecisionMaking = pDecisionMaking;
}

void PlayerAgent::SetToSeek(Vector2 position)
{
	m_pSeek->SetTarget(position);

	m_pSeek->SetValid(true);
	m_pWander->SetValid(true);
	m_pFlee->SetValid(false);
	m_pEvade->SetValid(false);
	m_pPursue->SetValid(false);
	m_pFace->SetValid(false);

	m_pSteeringBehavior = m_pToSeek;
}

void PlayerAgent::SetToFlee(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity)
{
	m_pFlee->SetTarget(positionEnemies);
	//m_pFace->SetTarget(positionEnemies);
	m_pEvade->SetTarget(positionEnemies, enemiesVelocity);

	m_pSeek->SetValid(false);
	m_pWander->SetValid(false);
	m_pFlee->SetValid(true);
	m_pEvade->SetValid(true);
	m_pPursue->SetValid(false);
	m_pFace->SetValid(false);

	m_pSteeringBehavior = m_pToFlee;
}

void PlayerAgent::SetToEvade(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity)
{
	m_pSeek->SetTarget(positionOriginalTarget);
	m_pEvade->SetTarget(positionEnemies, enemiesVelocity);

	m_pSeek->SetValid(true);
	m_pWander->SetValid(false);
	m_pFlee->SetValid(false);
	m_pEvade->SetValid(true);
	m_pPursue->SetValid(false);
	m_pFace->SetValid(true);

	m_pSteeringBehavior = m_pToEvade;

}

void PlayerAgent::SetToFace(Vector2 positionOriginalTarget, Vector2 positionEnemies, Vector2 enemiesVelocity)
{
	m_pSeek->SetTarget(positionOriginalTarget);
	m_pFace->SetTarget(positionEnemies);

	m_pSeek->SetValid(true);
	m_pWander->SetValid(false);
	m_pFlee->SetValid(false);
	m_pEvade->SetValid(false);
	m_pPursue->SetValid(false);
	m_pFace->SetValid(true);

	m_pSteeringBehavior = m_pToFace;
}

void PlayerAgent::SetToAvoid(Vector2 positionOriginalTarget, std::vector<EnemyInfo> enemies)
{
	m_pAvoid->SetTarget(positionOriginalTarget); //for the seek funtion
	m_pAvoid->SetMultipleTarget(enemies);

	m_pSeek->SetValid(false);
	m_pWander->SetValid(false);
	m_pFlee->SetValid(false);
	m_pEvade->SetValid(false);
	m_pPursue->SetValid(false);
	m_pFace->SetValid(false);
	m_pAvoid->SetValid(true);

	m_pSteeringBehavior = m_pToAvoid;
}

Vector2 PlayerAgent::DebugTarget() const
{
	return m_pSteeringBehavior->m_Target.position;
}
