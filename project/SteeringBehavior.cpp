#include "stdafx.h"
#include "SteeringBehavior.h"

Vector2 SteeringBehavior::DebugGetTarget() const
{
	return m_Target.position;
}

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	if (m_IsValid)
	{
		steering.LinearVelocity = m_Target.position - pAgent->Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}
	else
	{
		steering.LinearVelocity = { 0.f, 0.f };
		steering.AngularVelocity = 0.f;
	}
	return steering;
}

//WANDER
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	if (m_IsValid)
	{
		Elite::Vector2 wanderCircle{};
		wanderCircle = pAgent->LinearVelocity;
		wanderCircle.Normalize();
		wanderCircle *= m_Offset;
		wanderCircle.x += cos(m_WanderAngle) * m_Radius;
		wanderCircle.y += sin(m_WanderAngle) * m_Radius;
		//set random angle in FOV
		float nextAngleCHange = (randomFloat() * (pAgent->FOV_Angle/2)- (pAgent->FOV_Angle / 4));
		if (nextAngleCHange < m_MaxAngelChange)
			m_WanderAngle += nextAngleCHange;
	
		//set the new location
		m_Target.position = pAgent->Position + wanderCircle;
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

//FLEE
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	if (m_IsValid)
	{
		steering.LinearVelocity = pAgent->Position - m_Target.position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}
	else
	{
		steering.LinearVelocity = { 0.f, 0.f };
		steering.AngularVelocity = 0.f;
	}
	return steering;
}

//EVADE
//*****
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	if (m_IsValid)
	{
		auto distanceToTarget = Distance(pAgent->Position, m_Target.position);

		if (distanceToTarget < pAgent->FOV_Range);
		{
			float timeToTarget = distanceToTarget / pAgent->MaxLinearSpeed;
			auto velocityTarget = m_Target.linearVelocity;

			m_Target.position += (velocityTarget * timeToTarget);
			cout << m_Target.position.x << endl;
		}
	}
	return Flee::CalculateSteering(deltaT, pAgent);
}

//PURSUE
//******
SteeringPlugin_Output Pursue::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	if (m_IsValid)
	{
		m_Target.position += m_Target.linearVelocity;
		cout << m_Target.position.x << endl;
	}
	return Seek::CalculateSteering(deltaT, pAgent);
}
//FACE
//****
SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	if (m_IsValid)
	{
		Vector2 toTargetVector{ m_Target.position - pAgent->Position };
		float currentOrientation = pAgent->Orientation;
		Vector2 currentVector = { cos(currentOrientation), sin(currentOrientation) };
		float targetOrientation = GetOrientationFromVelocity(toTargetVector);
		Vector2 targetVector = { cos(targetOrientation), sin(targetOrientation) };
		float sign = Cross(currentVector, targetVector);
		if (sign >= 0.f)
			steering.AngularVelocity = 1 * pAgent->MaxAngularSpeed;
		else
			steering.AngularVelocity = -1 * pAgent->MaxAngularSpeed;
	}
	else
	{
		steering.LinearVelocity = { 0.f, 0.f };
		steering.AngularVelocity = 0.f;
	}

	return steering;
}

//COLLISION AVOIDANCE
//*******************
SteeringPlugin_Output CollisionAvoidance::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	steering.LinearVelocity += Seek::CalculateSteering(deltaT, pAgent).LinearVelocity;
	steering.LinearVelocity += CalculateAvoidSteering(pAgent);
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	return steering;
}

Elite::Vector2 CollisionAvoidance::CalculateAvoidSteering(AgentInfo* pAgent)
{
	Elite::Vector2 collisionAvoidSteering, leftPos, rightPos;

	Elite::Vector2 direction = pAgent->LinearVelocity.GetNormalized();

	EnemyInfo enemy;
	if (findObstacleToAvoid(pAgent, enemy))
	{
		Elite::Vector2 aheadVector = direction * pAgent->FOV_Range;
		collisionAvoidSteering = m_AntiClock * direction;
	
		Elite::Vector2 vectorToCenter = pAgent->Position - enemy.Location;
		float sign = Cross(aheadVector, vectorToCenter);
		collisionAvoidSteering *= sign;
		Elite::Normalize(collisionAvoidSteering);
		collisionAvoidSteering *= pAgent->MaxLinearSpeed * 22;
	}
	return collisionAvoidSteering;
}

bool CollisionAvoidance::findObstacleToAvoid(AgentInfo* pAgent, EnemyInfo& closestEnemy)
{
	float closestDistance = pAgent->FOV_Range, distance;
	Elite::Vector2 direction = pAgent->LinearVelocity.GetNormalized();

	for (EnemyInfo enemy : m_MultipleTargets)
	{
		distance = Distance(pAgent->Position, enemy.Location);
		if (distance < closestDistance)
		{
			closestDistance = distance;
			closestEnemy = enemy;
		}
	}

	if (closestDistance < m_MaxAhead)
		return true;
	else
		return false;
}

