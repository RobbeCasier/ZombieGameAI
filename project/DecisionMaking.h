#pragma once

class DecisionMaking
{
public:
	DecisionMaking() = default;
	virtual ~DecisionMaking() = default;

	virtual void Update(float deltaT) = 0;

};