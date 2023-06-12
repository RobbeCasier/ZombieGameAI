#pragma once
#include <Exam_HelperStructs.h>
#include <EliteMath\EMathUtilities.h>

using namespace Elite;
using namespace std;

class SteeringBehavior
{
public:
	struct TargetData
	{
		Vector2 position;
		Vector2 linearVelocity;
		float size;
	};
	SteeringBehavior() = default;
	virtual ~SteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	void SetTarget(const Vector2& position, const Vector2& linearVelocity = { 0.f, 0.f }) { m_Target = TargetData{ position, linearVelocity }; }
	void SetMultipleTarget(const std::vector<EnemyInfo> enemies) { m_MultipleTargets = enemies; }
	void SetValid(bool valid) { m_IsValid = valid; }
	Vector2 DebugGetTarget() const;
protected:
	TargetData m_Target;
	std::vector<EnemyInfo> m_MultipleTargets;
	bool m_IsValid = false;
};

//SEEK
//****
class Seek : public SteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

protected:
	float m_WanderAngle = 0.f;
	float m_Offset = 10.f;
	float m_Radius = 4.f;
	float m_MaxAngelChange = _Pi / 8.f;
};

//FLEE
//****
class Flee : public SteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

//EVADE
//*****
class Evade : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

//PURSUE
//******
class Pursue : public Seek
{
public:
	Pursue() = default;
	virtual ~Pursue() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};
//FACE
//****
class Face : public SteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

//COLLISION AVOIDANCE
//*******************
class CollisionAvoidance : public Seek
{
public:
	CollisionAvoidance() = default;
	virtual ~CollisionAvoidance() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
private:
	float m_MaxAhead = 10.f;
	Elite::Mat22 m_Clock = { cos(_Pi / 2.f), sin(_Pi / 2.f), -sin(_Pi / 2.f), cos(_Pi / 2.f) };
	Elite::Mat22 m_AntiClock = { cos(_Pi / 2.f), -sin(_Pi / 2.f), sin(_Pi / 2.f), cos(_Pi / 2.f) };

	Elite::Vector2 CalculateAvoidSteering(AgentInfo* pAgent);
	bool findObstacleToAvoid(AgentInfo* pAgent, EnemyInfo& enemy);
};

