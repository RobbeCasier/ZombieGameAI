#pragma once
#include <Exam_HelperStructs.h>
#include "SteeringBehavior.h"

//****************
//BLENDED STEERING
class BlendedSteering final : public SteeringBehavior
{
	struct WeightedBehavior
	{
		SteeringBehavior* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(SteeringBehavior* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

	friend class PlayerAgent;
public:
	BlendedSteering(vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

private:
	vector<WeightedBehavior> m_WeightedBehaviors = {};
};

