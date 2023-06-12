#pragma once
#include "BehaviorTree.h"
#include "SteeringBehavior.h"
#include <IExamInterface.h>
#include "Helpers.h"

//--------------//
//	Conditions	//
//--------------//
//TARGET
#pragma region Target
bool HasTarget(Blackboard* pB)
{
	bool hasTarget;
	auto dataAvailable = pB->GetData("ToTarget", hasTarget);
	if (!dataAvailable || !hasTarget)
		return false;
	return true;
}

bool HasReachedTarget(Blackboard* pB)
{
	Vector2 target;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Target", target) && pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return false;
	
	AgentInfo agent = pInterface->Agent_GetInfo();
	if (Distance(agent.Position, target) < agent.GrabRange)
		return true;
	return false;
}
#pragma endregion
//MAP
#pragma region Map
bool HasNotCompletelyExploredMap(Blackboard* pB)
{
	double nrOfPoints;
	unsigned int currentPoint;
	auto dataAvailable = pB->GetData("MaxExplorationPoints", nrOfPoints)
		&& pB->GetData("CurrentExplorationPoint", currentPoint);
	if (!dataAvailable)
		return true;

	if (nrOfPoints == currentPoint)
		return false;
	return true;
}
#pragma endregion
//HOUSES
#pragma region Houses
bool AreThereUnexploredHouses(Blackboard* pB)
{
	vector<House>* houses;
	auto dataAvailable = pB->GetData("ListOfHouses", houses);
	if (!dataAvailable || houses->size() == 0)
		return false;

	for (int i = 0; i < houses->size(); i++)
		if (!houses->at(i).explored)
			return true;
	return false;
}

bool IsInCurrentHouseIndex(Blackboard* pB)
{
	vector<House>* houses;
	int houseIndex = 0;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfHouses", houses)
		&& pB->GetData("HouseIndex", houseIndex)
		&& pB->GetData("Interface", pInterface);

	if (!dataAvailable || houses->size() == 0)
		return false;

	AgentInfo agent = pInterface->Agent_GetInfo();
	HouseInfo house = houses->at(houseIndex).houseInfo;
	if (agent.IsInHouse)
		if (agent.Position > (house.Center - house.Size / 2) && agent.Position < (house.Center + house.Size / 2))
			return true;
	return false;
}

bool IsInUnexploredHouse(Blackboard* pB)
{
	vector<House>* houses;
	int houseIndex = 0;
	IExamInterface* pInterface = nullptr;
	unsigned int currentPoint;
	vector<Vector2> explorationPoints;
	auto dataAvailable = pB->GetData("ListOfHouses", houses)
		&& pB->GetData("HouseIndex", houseIndex)
		&& pB->GetData("Interface", pInterface)
		&& pB->GetData("CurrentExplorationPoint", currentPoint)
		&& pB->GetData("ExplorationPoints", explorationPoints);

	if (!dataAvailable || houses->size() == 0)
		return false;

	AgentInfo agent = pInterface->Agent_GetInfo();
	for (int i = 0; i < houses->size(); i++)
	{
		House* house = &houses->at(i);
		if (!house->explored)
		{
			HouseInfo houseInfo = house->houseInfo;
			if (Distance(agent.Position, houseInfo.Center) < agent.GrabRange)
			{
				pB->ChangeData("CurrentHouseIndex", i);
				return true;
			}
		}
	}
	return false;
}
#pragma endregion
//ENEMIES
#pragma region Enemies
bool AreThereEnemiesInSight(Blackboard* pB)
{
	vector<EntityInfo> entities;
	auto dataAvailable = pB->GetData("ListOfEntities", entities);
	if (!dataAvailable || entities.empty())
		return false;

	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ENEMY)
			return true;
	return false;
}

bool FacesEnemy(Blackboard* pB)
{
	EnemyInfo enemy;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("EnemyTarget", enemy)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return false;

	AgentInfo agent = pInterface->Agent_GetInfo();
	Vector2 toTargetVector{ enemy.Location - agent.Position };
	float currentOrientation = agent.Orientation - float(_Pi / 2);
	//agent orientation is 180deg off
	float offset = float(_Pi / 2);
	float targetOrientation = GetOrientationFromVelocity(toTargetVector) - offset;

	float angleBetween = targetOrientation - currentOrientation;
	float y = sin(angleBetween) * toTargetVector.Magnitude();
	y = sqrt(pow(y, 2));
	if (y <= enemy.Size / 2)
		return true;
	return false;
}
#pragma endregion
//ITEMS
#pragma region Items
bool AreThereItemsInSight(Blackboard* pB)
{
	vector<EntityInfo> entities;
	auto dataAvailable = pB->GetData("ListOfEntities", entities);
	if (!dataAvailable || entities.empty())
		return false;

	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ITEM)
			return true;
	return false;
}

bool IsAnItemInGrabRange(Blackboard* pB)
{
	vector<EntityInfo> entities;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfEntities", entities)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return false;

	AgentInfo agent = pInterface->Agent_GetInfo();
	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ITEM)
			if (Distance(agent.Position, entities.at(i).Location) < agent.GrabRange)
			{
				pB->ChangeData("Item", entities.at(i));
				return true;
			}
	return false;
}

bool HasDepletedItems(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	ItemInfo item;
	int amount = 0;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, item))
		{
			switch (item.Type)
			{
			case eItemType::FOOD:
				amount = pInterface->Food_GetEnergy(item);
				break;
			case eItemType::MEDKIT:
				amount = pInterface->Medkit_GetHealth(item);
				break;
			case eItemType::PISTOL:
				amount = pInterface->Weapon_GetAmmo(item);
				break;
			default:
				return true;
				break;
			}
			if (amount <= 0)
				return true;
		}
	}
	return false;
}

bool HasNoSpace(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	ItemInfo item;
	int amount = 0;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (!pInterface->Inventory_GetItem(i, item))
		{
			return false;
		}
	}
	return true;
}

bool HasMedkit(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::MEDKIT)
				return true;
	return false;
}

bool HasFood(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::FOOD)
				return true;
	return false;
}

bool IsInNeedOfFood(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	if (pInterface->Agent_GetInfo().Energy <= 3.f)
		return true;
	return false;
}

bool IsInNeedOfMedkit(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	if (pInterface->Agent_GetInfo().Health <= 8.f)
		return true;
	return false;
}
#pragma endregion
//PURGEZONE
#pragma region PurgeZone
bool IsThereAPurgeZoneInSight(Blackboard* pB)
{
	vector<EntityInfo> entities;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface)
		&& pB->GetData("ListOfEntities", entities);

	if (!dataAvailable)
		return false;

	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo purgeZone;
			if (pInterface->PurgeZone_GetInfo(entities[i], purgeZone))
			{
				pB->ChangeData("PurgeZone", purgeZone);
				return true;
			}
		}
	}
	return false;
}
#pragma endregion
//PISTOL
#pragma region Pistol
bool HasPistolWithAmmo(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return false;

	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::PISTOL)
				if (pInterface->Weapon_GetAmmo(item) > 0)
					return true;

	return false;
}
#pragma endregion
//PLAYER
#pragma region Player
bool IsBeenBitten(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	if (pInterface->Agent_GetInfo().WasBitten)
		return true;
	return false;
}

bool StaminaMax(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	if (pInterface->Agent_GetInfo().Stamina == 10.f)
		return true;
	return false;
}

bool StaminaLowerThen50Percent(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return false;

	if (pInterface->Agent_GetInfo().Stamina <= 5.f)
		return true;
	return false;
}

bool IsEnemyToClose(Blackboard* pB)
{
	EnemyInfo enemy;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("EnemyTarget", enemy)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return false;

	if (Distance(pInterface->Agent_GetInfo().Position, enemy.Location) < pInterface->Agent_GetInfo().GrabRange)
		return true;
	return false;
}
#pragma endregion

//------------------//
//	BehaviorStates	//
//------------------//
//MOVEMENT
#pragma region Movement
BehaviorState ChangeToSeek(Blackboard* pBlackboard)
{
	PlayerAgent* pAgent = nullptr;
	IExamInterface* pIExamInterface = nullptr;
	Vector2 targetPosition = {};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Interface", pIExamInterface)
		&& pBlackboard->GetData("Target", targetPosition);

	if (!dataAvailable)
		return Failure;

	targetPosition = pIExamInterface->NavMesh_GetClosestPathPoint(targetPosition);
	pAgent->SetToSeek(targetPosition);
	pBlackboard->ChangeData("AutoOrient", true);
	pBlackboard->ChangeData("Face", false);
	return Success;
}

BehaviorState SeekSafeSpot(Blackboard* pBlackboard)
{
	PlayerAgent* pAgent = nullptr;
	IExamInterface* pIExamInterface = nullptr;
	PurgeZoneInfo purgeZone = {};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Interface", pIExamInterface)
		&& pBlackboard->GetData("PurgeZone", purgeZone);

	if (!dataAvailable)
		return Failure;

	AgentInfo agent = pIExamInterface->Agent_GetInfo();
	Vector2 targetDirection = agent.Position - purgeZone.Center;
	Vector2 targetDirectionNorm = GetNormalized(agent.Position - purgeZone.Center);
	Vector2 targetPosition = agent.Position + ((targetDirectionNorm * purgeZone.Radius) - targetDirection) + targetDirectionNorm * 10.f;
	targetPosition = pIExamInterface->NavMesh_GetClosestPathPoint(targetPosition);
	pAgent->SetToSeek(targetPosition);
	pBlackboard->ChangeData("AutoOrient", true);
	pBlackboard->ChangeData("Face", false);
	return Success;
}

BehaviorState ChangeToFlee(Blackboard* pBlackboard)
{
	PlayerAgent* pAgent = nullptr;
	IExamInterface* pIExamInterface = nullptr;
	Vector2 targetPosition = {};
	EnemyInfo enemy = {};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) 
		&& pBlackboard->GetData("Interface", pIExamInterface) 
		&& pBlackboard->GetData("Target", targetPosition)
		&& pBlackboard->GetData("EnemyTarget", enemy);
	if (!dataAvailable)
		return Failure;

	targetPosition = pIExamInterface->NavMesh_GetClosestPathPoint(targetPosition);
	pAgent->SetToFlee(targetPosition, enemy.Location, enemy.LinearVelocity);
	pBlackboard->ChangeData("AutoOrient", false);
	pBlackboard->ChangeData("Face", false);
	return Success;
}

BehaviorState ChangeToAvoid(Blackboard* pBlackboard)
{
	PlayerAgent* pAgent = nullptr;
	IExamInterface* pIExamInterface = nullptr;
	Vector2 targetPosition = {};
	EnemyInfo enemy;
	std::vector<EnemyInfo> enemies;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Interface", pIExamInterface)
		&& pBlackboard->GetData("Target", targetPosition)
		&& pBlackboard->GetData("EnemyTarget", enemy)
		&& pBlackboard->GetData("ListOfEnemies", enemies);
	if (!dataAvailable)
		return Failure;

	targetPosition = pIExamInterface->NavMesh_GetClosestPathPoint(targetPosition);
	//pAgent->SetToEvade(targetPosition, enemy.Location, enemy.LinearVelocity);
	pAgent->SetToAvoid(targetPosition, enemies);
	pBlackboard->ChangeData("AutoOrient", true);
	pBlackboard->ChangeData("Face", false);
	return Success;
}

BehaviorState ChangeToFace(Blackboard* pB)
{
	PlayerAgent* pAgent = nullptr;
	IExamInterface* pIExamInterface = nullptr;
	Vector2 targetPosition = {};
	EnemyInfo enemy;
	auto dataAvailable = pB->GetData("Agent", pAgent)
		&& pB->GetData("Interface", pIExamInterface)
		&& pB->GetData("Target", targetPosition)
		&& pB->GetData("EnemyTarget", enemy);
	if (!dataAvailable)
		return Failure;

	targetPosition = pIExamInterface->NavMesh_GetClosestPathPoint(targetPosition);
	pAgent->SetToFace(targetPosition, enemy.Location, enemy.LinearVelocity);
	pB->ChangeData("AutoOrient", false);
	pB->ChangeData("Face", true);
	return Success;
}
#pragma endregion

//TARGET
#pragma region Target
BehaviorState TargetReached(Blackboard* pB)
{
	pB->ChangeData("ToTarget", false);
	return Success;
}
#pragma endregion

//MAP
#pragma region Map
BehaviorState GetNextExplorationPoint(Blackboard* pB)
{
	unsigned int currentPoint;
	vector<Vector2> explorationPoints;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("CurrentExplorationPoint", currentPoint)
		&& pB->GetData("ExplorationPoints", explorationPoints)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	//check if the exploration target has been reached
	//if not keep point
	AgentInfo agent = pInterface->Agent_GetInfo();
	if (Distance(agent.Position, explorationPoints[currentPoint]) < agent.GrabRange)
		++currentPoint;
	if (currentPoint < explorationPoints.size())
	{
		Vector2 target = explorationPoints[currentPoint];
		pB->ChangeData("Target", target);
		pB->ChangeData("ToTarget", true);
	}
	pB->ChangeData("CurrentExplorationPoint", currentPoint);
	return Success;
}
#pragma endregion

//HOUSES
#pragma region Houses
BehaviorState GoToUnexploredHouse(Blackboard* pB)
{
	vector<House>* houses;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfHouses", houses)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	float maxDistance = FLT_MAX;
	for (int i = 0; i < houses->size(); i++)
		if (!houses->at(i).explored)
		{
			float distance = Distance(pInterface->Agent_GetInfo().Position, houses->at(i).houseInfo.Center);
			if (distance < maxDistance)
			{
				maxDistance = distance;
				pB->ChangeData("Target", houses->at(i).houseInfo.Center);
				pB->ChangeData("ToTarget", true);
			}
		}
	return Success;
}
BehaviorState IsExplored(Blackboard* pB)
{
	int currentHouseIndex;
	vector<House>* houses = nullptr;
	auto dataAvailable = pB->GetData("CurrentHouseIndex", currentHouseIndex)
		&& pB->GetData("ListOfHouses", houses);

	if (!dataAvailable)
		return Failure;

	houses->at(currentHouseIndex).explored = true;
	return Success;
}

BehaviorState IncrementHouseIndex(Blackboard* pB)
{
	int houseIndex = 0;
	vector<House>* houses = nullptr;
	auto dataAvailable = pB->GetData("HouseIndex", houseIndex)
		&& pB->GetData("ListOfHouses", houses);
	if (!dataAvailable)
		return Failure;

	houseIndex = (++houseIndex) % houses->size();
	pB->ChangeData("HouseIndex", houseIndex);
	return Success;
}

BehaviorState TargetHouse(Blackboard* pB)
{
	int houseIndex = 0;
	vector<House>* houses = nullptr;
	auto dataAvailable = pB->GetData("HouseIndex", houseIndex)
		&& pB->GetData("ListOfHouses", houses);
	if (!dataAvailable)
		return Failure;

	pB->ChangeData("Target", houses->at(houseIndex).houseInfo.Center);
	pB->ChangeData("ToTarget", true);
	return Success;
}
#pragma endregion

//ENEMIES
#pragma region Enemies
BehaviorState GetClosestEnemy(Blackboard* pB)
{
	vector<EntityInfo> entities;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfEntities", entities)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	AgentInfo agent = pInterface->Agent_GetInfo();
	float maxDistance = FLT_MAX;
	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ENEMY)
		{
			float distance = Distance(agent.Position, entities.at(i).Location);
			if (distance < maxDistance)
			{
				maxDistance = distance;
				EnemyInfo enemy;
				pInterface->Enemy_GetInfo(entities.at(i), enemy);
				pB->ChangeData("EnemyTarget", enemy);
			}
		}
	return Success;
}

BehaviorState GetEnemies(Blackboard* pB)
{
	vector<EntityInfo> entities;
	vector<EnemyInfo> enemies;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfEntities", entities) 
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	AgentInfo agent = pInterface->Agent_GetInfo();
	float maxDistance = FLT_MAX;
	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ENEMY)
		{
			EnemyInfo enemy;
			pInterface->Enemy_GetInfo(entities.at(i), enemy);
			enemies.push_back(enemy);
		}
	pB->ChangeData("ListOfEnemies", enemies);
	return Failure;
}
#pragma endregion

//ITEMS
#pragma region Items
BehaviorState UseMedkit(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return Failure;

	int index, maxAmount = INT_MAX, amount;
	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::MEDKIT)
			{
				amount = pInterface->Medkit_GetHealth(item);
				if (amount < maxAmount)
				{
					maxAmount = amount;
					index = i;
				}
			}
	pInterface->Inventory_UseItem(index);
	return Success;
}

BehaviorState UseFood(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return Failure;

	int index, maxAmount = INT_MAX, amount;
	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::FOOD)
			{
				amount = pInterface->Food_GetEnergy(item);
				if (amount < maxAmount)
				{
					maxAmount = amount;
					index = i;
				}
			}
	pInterface->Inventory_UseItem(index);
	return Success;
}
BehaviorState MakeSpace(Blackboard* pB)
{
	EntityInfo entity;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Item", entity)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	ItemInfo temp, item;

	float currentHealth = pInterface->Agent_GetInfo().Health;
	float currentEnergy = pInterface->Agent_GetInfo().Energy;
	
	pInterface->Item_GetInfo(entity, item);

	if (currentHealth < currentEnergy && HasMedkit(pB))
	{
		for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			pInterface->Inventory_GetItem(i, temp);
			if (temp.Type == item.Type)
			{
				switch (temp.Type)
				{
				case eItemType::MEDKIT:
					return UseMedkit(pB);
					break;
				default:
					break;
				}
			}
		}
	}
	else if (currentEnergy < currentHealth && HasFood(pB))
	{
		for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			pInterface->Inventory_GetItem(i, temp);
			if (temp.Type == item.Type)
			{
				switch (temp.Type)
				{
				case eItemType::FOOD:
					return UseFood(pB);
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		int index = 0;
		int maxAmount = INT_MAX, amount;
		for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			pInterface->Inventory_GetItem(i, temp);
			if (temp.Type == item.Type)
			{
				switch (temp.Type)
				{
				case eItemType::PISTOL:
					amount = pInterface->Weapon_GetAmmo(temp);
					if (amount < maxAmount)
					{
						maxAmount = amount;
						index = i;
					}
					break;
				default:
					break;
				}
			}
		}
		pInterface->Inventory_RemoveItem(index);
	}
}
BehaviorState GrabItems(Blackboard* pB)
{
	vector<EntityInfo> entities;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfEntities", entities)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	if (HasNoSpace(pB))
	{
		MakeSpace(pB);
	}
	AgentInfo agent = pInterface->Agent_GetInfo();
	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ITEM)
			if (Distance(agent.Position, entities.at(i).Location) < agent.GrabRange)
			{
				ItemInfo item;
				pInterface->Item_GetInfo(entities.at(i), item);
				if (item.Type == eItemType::GARBAGE)
					pInterface->Item_Destroy(entities.at(i));

				pInterface->Item_Grab(entities.at(i), item);

				for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
				{
					ItemInfo temp;
					switch (temp.Type)
					{
					case eItemType::FOOD:
						cout << "FOOD" << endl;
						break;
					case eItemType::MEDKIT:
						cout << "MEDKIT" << endl;
						break;
					case eItemType::PISTOL:
						cout << "PISTOL" << endl;
						break;
					case eItemType::GARBAGE:
						cout << "GARBAGE" << endl;
						break;
					default:
						break;
					}
					if (!pInterface->Inventory_GetItem(i, temp))
					{
						pInterface->Inventory_AddItem(i, item);
						pB->ChangeData("ToTarget", false);
						pB->ChangeData("AutoOrient", true);
						break;
					}
				}

			}
	return Success;
}


BehaviorState GoToClosestItem(Blackboard* pB)
{
	vector<EntityInfo> entities;
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("ListOfEntities", entities)
		&& pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	AgentInfo agent = pInterface->Agent_GetInfo();
	float maxDistance = FLT_MAX;
	for (int i = 0; i < entities.size(); i++)
		if (entities.at(i).Type == eEntityType::ITEM)
		{
			float distance = Distance(agent.Position, entities.at(i).Location);
			if (distance < maxDistance)
			{
				maxDistance = distance;
				ItemInfo item;
				pInterface->Item_GetInfo(entities.at(i), item);
				pB->ChangeData("Target", item.Location);
				pB->ChangeData("ToTarget", true);
				pB->ChangeData("AutoOrient", true);
			}
		}
	return Success;
}

BehaviorState RemoveDepletedItems(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);

	if (!dataAvailable)
		return Failure;

	ItemInfo item;
	int amount = 0;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, item))
		{
			switch (item.Type)
			{
			case eItemType::FOOD:
				amount = pInterface->Food_GetEnergy(item);
				break;
			case eItemType::MEDKIT:
				amount = pInterface->Medkit_GetHealth(item);
				break;
			case eItemType::PISTOL:
				amount = pInterface->Weapon_GetAmmo(item);
				break;
			default:
				pInterface->Inventory_RemoveItem(i);
				return Success;
				break;
			}
			if (amount <= 0)
			{
				pInterface->Inventory_RemoveItem(i);
				return Success;
			}
		}
	}
	return Failure;
}


#pragma endregion

//PISTOL
#pragma region Pistol
BehaviorState Shoot(Blackboard* pB)
{
	IExamInterface* pInterface = nullptr;
	auto dataAvailable = pB->GetData("Interface", pInterface);
	if (!dataAvailable)
		return Failure;

	int index, maxAmount = INT_MAX, amount;
	ItemInfo item;
	for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		if (pInterface->Inventory_GetItem(i, item))
			if (item.Type == eItemType::PISTOL)
				if (pInterface->Weapon_GetAmmo(item) > 0)
				{
					amount = pInterface->Weapon_GetAmmo(item);
					if (amount < maxAmount)
					{
						maxAmount = amount;
						index = i;
					}
				}

	pInterface->Inventory_UseItem(index);
	return Success;
}
#pragma endregion

//PLAYER
#pragma region Player
BehaviorState Run(Blackboard* pB)
{
	pB->ChangeData("CanRun", true);

	return Success;
}
BehaviorState StopRun(Blackboard* pB)
{
	pB->ChangeData("CanRun", false);

	return Failure; // keep on going in tree
}
#pragma endregion