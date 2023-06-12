#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "SteeringBehavior.h"
#include "PlayerAgent.h"
#include "Behavior.h"
#include "Helpers.h"

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	vector<House>* m_pHouses = nullptr;

	Elite::Vector2 m_Target = {};
	Blackboard* m_pBlackboard = nullptr;
	PlayerAgent* m_pPlayerAgent = nullptr;

	const unsigned int m_NrOfExplorationPoints = 16; //give nr that returns a natural number after sqrt
	vector<Vector2> m_ExplorationPoints;
	unsigned int m_CurrentExplorationPoint = 0;
	float m_TimeInHouse = 0.f;
	float m_MaxTimeInHouse = 5.f;

	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 2.f; //Demo purpose

	Blackboard* CreateBlackboard();
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}