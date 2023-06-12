#include "stdafx.h"
#include "CombinedBehaviors.h"

BlendedSteering::BlendedSteering(vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringPlugin_Output BlendedSteering::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output blendedSteering = {};
	auto totalWeight = 0.f;

	for (auto weightedBehavior : m_WeightedBehaviors)
	{
		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, pAgent);
		blendedSteering.LinearVelocity += weightedBehavior.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehavior.weight * steering.AngularVelocity;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;
		blendedSteering.LinearVelocity *= scale;
		blendedSteering.AngularVelocity *= scale;
	}
	return blendedSteering;
}