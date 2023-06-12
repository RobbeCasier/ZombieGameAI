#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Zombie Survival AI";
	info.Student_FirstName = "Robbe";
	info.Student_LastName = "Casier";
	info.Student_Class = "2DAE07";

	//Initialize Exploration points
	Vector2 minPos, maxPos;
	WorldInfo worldInfo = m_pInterface->World_GetInfo();
	minPos = worldInfo.Center - worldInfo.Dimensions / 2.f;
	maxPos = worldInfo.Center + worldInfo.Dimensions / 2.f;


	unsigned int nrOfRowAndCols = sqrt(m_NrOfExplorationPoints);
	float offSet = worldInfo.Dimensions.x / (nrOfRowAndCols + 1);

	vector<Vector2> temp;
	for (unsigned int r = 1; r < nrOfRowAndCols + 1; r++)
		for (unsigned int c = 1; c < nrOfRowAndCols + 1; c++)
			temp.push_back({ minPos.x + c * offSet, minPos.y + r * offSet });

	//Get outer points
	int sqrtPoints = sqrt(m_NrOfExplorationPoints);
	m_ExplorationPoints.push_back(temp[0]); //first
	int vLocation = sqrtPoints - 1; // second
	m_ExplorationPoints.push_back(temp[vLocation]);
	vLocation *= sqrtPoints + 1; //fourth
	m_ExplorationPoints.push_back(temp[vLocation]);
	vLocation -= sqrtPoints - 1; //third
	m_ExplorationPoints.push_back(temp[vLocation]);
	

	//initialize behavior
	AgentInfo* agent = &m_pInterface->Agent_GetInfo();
	m_pPlayerAgent = new PlayerAgent(agent);
	m_pHouses = new vector<House>{};
	m_pBlackboard = CreateBlackboard();
	//start values blackboard
	m_Target = m_ExplorationPoints[m_CurrentExplorationPoint];
	m_pBlackboard->ChangeData("ToTarget", true);
	m_pBlackboard->ChangeData("Target", m_Target);
	BehaviorTree* pBehavior = new BehaviorTree
	(
		m_pBlackboard,
		new BehaviorSelector(
		{
			new BehaviorSequence //if has target
			{{
				new BehaviorConditional{HasTarget},
				new BehaviorSelector
				{{
					new BehaviorSequence // if it has reached the target, set it to have no target
					{{
						new BehaviorConditional{HasReachedTarget},
						new BehaviorAction{TargetReached}
					}},
					new BehaviorSequence //remove depleted items
					{{
						new BehaviorConditional{HasDepletedItems},
						new BehaviorAction{RemoveDepletedItems}
					}},
					new BehaviorSequence //Use Medkits
					{{
						new BehaviorConditional{HasMedkit},
						new BehaviorConditional{IsInNeedOfMedkit},
						new BehaviorAction{UseMedkit}
					}},
					new BehaviorSequence //Use Food
					{{
						new BehaviorConditional{HasFood},
						new BehaviorConditional{IsInNeedOfFood},
						new BehaviorAction{UseFood}
					}},
					new BehaviorSequence
					{{
						new BehaviorConditional{IsBeenBitten},
						new BehaviorAction{Run}
					}},
					new BehaviorSequence
					{{
						new BehaviorConditional{StaminaLowerThen50Percent},
						new BehaviorAction{StopRun}
					}},
					new BehaviorSequence //actions to take when there is an enemy
					{{
						new BehaviorConditional{AreThereEnemiesInSight},
						new BehaviorAction{GetClosestEnemy},
						new BehaviorSelector
						{{
							new BehaviorSequence
							{{
								new BehaviorConditional{HasPistolWithAmmo},
								new BehaviorSelector
								{{
									new BehaviorSequence
									{{
										new BehaviorConditional{FacesEnemy},
										new BehaviorAction{Shoot}
									}},
									new BehaviorAction{ChangeToFace}
								}}
							}},
							new BehaviorAction{GetEnemies},
							new BehaviorAction{ChangeToAvoid}
						}}
					}},
					new BehaviorSequence
					{{
						new BehaviorConditional{IsThereAPurgeZoneInSight},
						new BehaviorAction{SeekSafeSpot}
					}},
					new BehaviorSequence // if there is an item, go there
					{{
						new BehaviorConditional{AreThereItemsInSight},
						new BehaviorSequence // if it can grab it, grab it else go to the closest
						{{
							new BehaviorConditional{IsAnItemInGrabRange},
							new BehaviorAction{GrabItems}
						}},
						new BehaviorAction{GoToClosestItem}
					}},
					new BehaviorSequence //if unexplored house in sight, go there
					{{
						new BehaviorConditional{AreThereUnexploredHouses},
						new BehaviorAction{GoToUnexploredHouse},
						new BehaviorSequence
						{{
							new BehaviorConditional{IsInUnexploredHouse},
							new BehaviorAction{IsExplored}
						}},
						new BehaviorAction{ChangeToSeek}
					}},
					new BehaviorAction{ChangeToSeek}
				}},
			}},
			new BehaviorSequence //if no target, look for next exploration point
			{{
				new BehaviorConditional{HasNotCompletelyExploredMap},
				new BehaviorAction{GetNextExplorationPoint},
			}},
			new BehaviorSequence //if all exploration points have been reached, look for every home
			{{
				new BehaviorConditional{IsInCurrentHouseIndex},
				new BehaviorAction{IncrementHouseIndex},
				new BehaviorAction{TargetHouse}
			}},
			new BehaviorAction{TargetHouse},
		}
	));


	m_pPlayerAgent->SetDecisionMaking(pBehavior);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	std::cout << "Zombie Survival AI succesfully pluged in" << std::endl;
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{

}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();


	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	auto nextTargetPos = m_Target; //To start you can use the mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	m_TimeInHouse += dt;
	m_pBlackboard->ChangeData("TimeInHouse", m_TimeInHouse);

	//Update list of houses
	if (m_pHouses->empty())
		for (auto& housesInFOV : vHousesInFOV)
				m_pHouses->push_back(House{ housesInFOV, false });
	else
		for (auto& housesinFOV : vHousesInFOV)
		{
			auto houseIt = find_if(m_pHouses->begin(), m_pHouses->end(), [&housesinFOV](House h) {return h.houseInfo.Center == housesinFOV.Center; });
			if (houseIt == m_pHouses->end())
				m_pHouses->push_back(House{ housesinFOV, false });
		}

	//Update Visible entities
	m_pBlackboard->ChangeData("ListOfEntities", vEntitiesInFOV);

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y <<  " ---EntityHash: " << e.EntityHash << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
	}

	m_pPlayerAgent->Update(dt, &agentInfo);
	steering = m_pPlayerAgent->CalculateSteering(dt);
	m_Target = m_pPlayerAgent->DebugTarget();

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	m_pBlackboard->GetData("AutoOrient", steering.AutoOrient); //Setting AutoOrientate to TRue overrides the AngularVelocity

	m_pBlackboard->GetData("CanRun", steering.RunMode); //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
	/*for (int i = 0; i < m_NrOfExplorationPoints; i++)
	{
		m_pInterface->Draw_SolidCircle(m_ExplorationPoints[i], 1.f, { 0,0 }, { 0,0,0 });
	}*/
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

Blackboard* Plugin::CreateBlackboard()
{
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", m_pPlayerAgent);
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("Target", Vector2{});
	pBlackboard->AddData("EnemyTarget", EnemyInfo{});
	pBlackboard->AddData("ListOfEnemies", vector<EnemyInfo>{});
	pBlackboard->AddData("ToTarget", false);
	pBlackboard->AddData("ExplorationPoints", m_ExplorationPoints);
	pBlackboard->AddData("CurrentExplorationPoint", m_CurrentExplorationPoint);
	pBlackboard->AddData("MaxExplorationPoints", sqrt(m_NrOfExplorationPoints));
	pBlackboard->AddData("ListOfHouses", m_pHouses);
	pBlackboard->AddData("HouseIndex", 0);
	pBlackboard->AddData("CurrentHouseIndex", 0);
	pBlackboard->AddData("TimeInHouse", 0.f);
	pBlackboard->AddData("MaxTimeInHouse", m_MaxTimeInHouse);
	pBlackboard->AddData("ListOfEntities", vector<EntityInfo>{});
	pBlackboard->AddData("PurgeZone", PurgeZoneInfo{});
	pBlackboard->AddData("Item", EntityInfo{});
	pBlackboard->AddData("AutoOrient", true);
	pBlackboard->AddData("Face", false);
	pBlackboard->AddData("CanRun", false);
	return pBlackboard;
}
