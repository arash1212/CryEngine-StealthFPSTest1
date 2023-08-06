#include "StdAfx.h"
#include "AIController.h"
#include "ActorState.h"
#include "Player.h"
//#include "GamePlugin.h

#include <CryAISystem/ICoverSystem.h>
#include <CryAISystem/ITacticalPointSystem.h>
#include "CryAISystem/IAIActionSequence.h"
#include <CryAISystem/IAISystem.h>
#include <CryAISystem/INavigationSystem.h>
#include <CryAISystem/INavigation.h>
#include <CryAISystem/IMovementSystem.h>
#include <CryAISystem/NavigationSystem/INavMeshQueryManager.h>
#include <CryAISystem/NavigationSystem/MNMNavMesh.h>
#include <CryAISystem/Components/IEntityNavigationComponent.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterAIControllerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(AIControllerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterAIControllerComponent);
}


void AIControllerComponent::Initialize()
{
	m_characterControllerComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();

	//navigation component
	m_navigationComp = m_pEntity->GetOrCreateComponent<IEntityNavigationComponent>();
	m_navigationComp->SetNavigationAgentType("MediumSizedCharacters");
	//movementProperties
	IEntityNavigationComponent::SMovementProperties m_movementProps;
	m_movementProps.normalSpeed = 2.f;
	m_movementProps.minSpeed = 2;
	m_movementProps.maxSpeed = 3;
	m_movementProps.lookAheadDistance = 0.5f;
	m_navigationComp->SetMovementProperties(m_movementProps);

	//collision avoidance
	IEntityNavigationComponent::SCollisionAvoidanceProperties collisionAvoidanceProps;
	collisionAvoidanceProps.radius = 0.5f;
	m_navigationComp->SetCollisionAvoidanceProperties(collisionAvoidanceProps);

}

Cry::Entity::EventFlags AIControllerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void AIControllerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

f32 AIControllerComponent::GetRandomFloat(f32 min, f32 max)
{
	return (min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min)));
}

Vec3 AIControllerComponent::GetRandomPointInsideTriangle(Triangle t)
{
	f32 r1 = crymath::sqrt(GetRandomFloat(0.f, 1.f));
	f32 r2 = GetRandomFloat(0.f, 1.f);
	f32 m1 = 1 - r1;
	f32 m2 = r1 * (1 - r2);
	f32 m3 = r2 * r1;

	Vec3 p1 = t.v0;
	Vec3 p2 = t.v1;
	Vec3 p3 = t.v2;
	return (m1 * p1) + (m2 * p2) + (m3 * p3);
}

Vec3 AIControllerComponent::GetVelocity()
{
	return m_characterControllerComp->GetVelocity();
}

bool AIControllerComponent::IsWalking()
{
	return m_characterControllerComp != nullptr ? m_characterControllerComp->IsOnGround() : false;
}

bool AIControllerComponent::IsOnGround()
{
	return m_characterControllerComp != nullptr ? m_characterControllerComp->IsOnGround() : true;
}

Cry::DefaultComponents::CCharacterControllerComponent* AIControllerComponent::GetCharacterController()
{
	return m_characterControllerComp;
}

void AIControllerComponent::LookAt(Vec3 position)
{
	Vec3 dir = position - m_pEntity->GetWorldPos();
	dir.z = 0;
	m_pEntity->SetRotation(Quat::CreateRotationVDir(dir));
}

void AIControllerComponent::NavigateTo(Vec3 position)
{
	if (position != ZERO){
		m_navigationComp->NavigateTo(position);
	}
	else {
		m_navigationComp->NavigateTo(m_pEntity->GetWorldPos());
	}
}

void AIControllerComponent::MoveTo(Vec3 position)
{
	if (!m_stateComp) {
		CryLog("AIControllerComponent : (MoveTo) m_stateComp is null!");
		return;
	}
	if (!m_characterControllerComp) {
		CryLog("AIControllerComponent : (MoveTo) m_characterControllerComp is null!");
		return;
	}
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_stateComp->GetState() == EActorState::RUNNING ? m_navigationComp->GetRequestedVelocity() * 2 : m_navigationComp->GetRequestedVelocity());
}

void AIControllerComponent::MoveToAndLookAtWalkDirection(Vec3 position)
{
	if (!m_stateComp) {
		CryLog("AIControllerComponent : (MoveToAndLookAtWalkDirection) m_stateComp is null!");
		return;
	}
	if (!m_characterControllerComp) {
		CryLog("AIControllerComponent : (MoveToAndLookAtWalkDirection) m_characterControllerComp is null!");
		return;
	}
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_stateComp->GetState() == EActorState::RUNNING ? m_navigationComp->GetRequestedVelocity() * 2 : m_navigationComp->GetRequestedVelocity());
	m_pEntity->SetRotation(Quat::CreateRotationVDir(m_navigationComp->GetRequestedVelocity()));
}

Vec3 AIControllerComponent::GetRandomPointOnNavmesh(float MaxDistance, IEntity* Around)
{
	MNM::TriangleIDArray resultArray;
	DynArray<Vec3> resultPositions;

	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	NavigationMeshID navMeshId = gEnv->pAISystem->GetNavigationSystem()->FindEnclosingMeshID(agentTypeId, Around->GetWorldPos());

	//get Triangles
	const Vec3 triggerBoxSize = Vec3(20, 20, 20);
    MNM::aabb_t	 aabb = MNM::aabb_t(triggerBoxSize * -15.5f, triggerBoxSize * 15.5f);
	MNM::TriangleIDArray triangleIDArray = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->QueryTriangles(aabb);
	if (triangleIDArray.size() <= 0 || !triangleIDArray[0].IsValid() || !navMeshId.IsValid() || !agentTypeId.IsValid()) {
		return m_pEntity->GetWorldPos();
	}

	for (int32 i = 0; i < triangleIDArray.size(); i++) {
		int32 j = 0;
		Triangle triangle;
		gEnv->pAISystem->GetNavigationSystem()->GetTriangleVertices(navMeshId, triangleIDArray[i], triangle);
		while (j <= 20) {
			j++;
			Vec3 point = GetRandomPointInsideTriangle(triangle);
			if (IsPointVisibleFrom(agentTypeId, point, Around->GetWorldPos())) {
				IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
				pd->Begin("testPoint", true);
				pd->AddSphere(point, 0.5f, ColorF(1, 0, 0), 10);
				resultPositions.append(point);
			}
		}
	}


	/*
	//check distances
	MNM::TriangleIDArray iDArray;
	for (int32 i = 0; i < triangleIDArray.size(); i++) {
		iDArray.push_back(triangleIDArray[i]);
		MNM::SClosestTriangle closestTriangle = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->FindClosestTriangle(Around, iDArray);
		if (closestTriangle.position.GetVec3().GetDistance(Around) < MaxDistance) {
			resultArray.push_back(triangleIDArray[i]);
		}
	}

	//select random id
	int32 max = resultArray.size();
	int32 min = 0;
	int32 range = max - min + 1;
	int random = rand() % range + min;
	MNM::TriangleID selectedId = resultArray[random];

	triangleIDArray.clear();
	triangleIDArray.append(selectedId);


	MNM::SClosestTriangle closestTriangle = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->FindClosestTriangle(Around, triangleIDArray);

	*/

	//old
	//Vec3 resultPos = closestTriangle.position.GetVec3();
	
	//new 
	int32 max = resultPositions.size();
	int32 min = 0;
	int32 range = max - min + 1;
	int random = rand() % range + min;
	Vec3 resultPos = resultPositions[random];

	f32 resultMax = MaxDistance - 2;
	f32 resultMin = 3;
	Vec3 Dir = resultPos - Around->GetWorldPos();

	resultPos = Around->GetWorldPos() + Dir.normalize() * (resultMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / resultMax - resultMin)));

	MNM::SOrderedSnappingMetrics snappingMetrics;
	snappingMetrics.CreateDefault();
	SAcceptAllQueryTrianglesFilter filter;
	MNM::SPointOnNavMesh pointOnNavMesh = gEnv->pAISystem->GetNavigationSystem()->SnapToNavMesh(agentTypeId, resultPos, snappingMetrics, &filter, &navMeshId);
	return pointOnNavMesh.GetWorldPosition();


	/*
	//Test new ?
	MNM::TriangleIDArray zeroArray;
	zeroArray.append(triangleIDArray[0]);
	MNM::SClosestTriangle zerotTriangle = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->FindClosestTriangle(Around, zeroArray);

	Vec3 resultPos = zerotTriangle.position.GetVec3() * (1 - crymath::sqrt(GetRandomFloat(0, 10)));
	for (int32 i = 0; i < triangleIDArray.size(); i++) {
		MNM::TriangleIDArray iDArray;
		iDArray.append(triangleIDArray[i]);
		resultPos += gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->FindClosestTriangle(Around, iDArray).position.GetVec3() * (1 - crymath::sqrt(GetRandomFloat(0, 10)));
	}
	return resultPos;
	*/
}

bool AIControllerComponent::IsPointVisibleFrom(NavigationAgentTypeID agentTypeId, Vec3 from, Vec3 endPos)
{
	MNM::SRayHitOutput rayHitOut;
	SAcceptAllQueryTrianglesFilter filter;
	MNM::ERayCastResult result = gEnv->pAISystem->GetNavigationSystem()->NavMeshRayCast(agentTypeId, from, endPos, &filter, &rayHitOut);
	return result != MNM::ERayCastResult::DisconnectedLocations ? true : false;
}

void AIControllerComponent::SetActorStateComponent(ActorStateComponent* stateComp)
{
	this->m_stateComp = stateComp;
}

Vec3 AIControllerComponent::Patrol(Schematyc::CSharedString pathName)
{
	float distanceToPoint = m_currentPatrolPoint.GetDistance(m_pEntity->GetWorldPos());
	if (distanceToPoint <= 1 || m_currentPatrolPoint == ZERO) {
		//update progress
		if (m_patrolProgress >= 100) {
			bIsAtEnd = true;
		}
		else if (m_patrolProgress <= 0) {
			bIsAtEnd = false;
		}

		//reverse
		if (bIsAtEnd) {
			m_patrolProgress = CLAMP(m_patrolProgress - 1, 0, 100);
		}
		else {
			m_patrolProgress = CLAMP(m_patrolProgress + 1, 0, 100);;
		}

		gEnv->pAISystem->GetINavigation()->GetPointOnPathBySegNo(pathName.c_str(), m_currentPatrolPoint, m_patrolProgress);
	}

	return m_currentPatrolPoint;
	/*
	if (m_currentPatrolPoint != ZERO) {
		MoveToAndLookAtWalkDirection(m_currentPatrolPoint);
	}
	*/
}

Vec3 AIControllerComponent::FindCover(IEntity* target)
{
	Vec3 eyes = Vec3(0, 0, 0.25f);
	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	std::array<Vec3, 400> locations;

	int32 count = gEnv->pAISystem->GetCoverSystem()->GetCover(m_pEntity->GetWorldPos(), 900.f, &eyes, 1, 0.9f, locations.data(), 30, 3);
	CryLog("count %i", count);
	for (int32 i = 0; i < count; i++) {
		if (IsCoverPointSafe(locations[i], target) && IsCoverUsable(locations[i], target)) {
			return locations[i];
		}
	}
	return ZERO;
}

bool AIControllerComponent::IsCoverPointSafe(Vec3 point, IEntity* target)
{
	/*
	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	MNM::SRayHitOutput rayHitOut;
	SAcceptAllQueryTrianglesFilter filter;
	MNM::ERayCastResult result = gEnv->pAISystem->GetNavigationSystem()->NavMeshRayCast(agentTypeId, point, target->GetWorldPos(), &filter, &rayHitOut);
	*/

	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();
	//height target estefade beshe
	Vec3 targetPos = Vec3(target->GetWorldPos().x, target->GetWorldPos().y, target->GetWorldPos().z + 2.1f);
	if (target->GetComponent<PlayerComponent>()) {
		if (target->GetComponent<PlayerComponent>()->IsCrouching()) {
			targetPos = Vec3(target->GetWorldPos().x, target->GetWorldPos().y, target->GetWorldPos().z + 1.0f);
		}
	}

	Vec3 dir = targetPos - point;
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(point, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {
			//Debug
			if (pd) {
				pd->Begin("CoverRaycast", true);
				pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 1, 0), 2);
			}

			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				if (hitEntity == target) {
					return false;
				}
				else
				{
					return true;
				}
			}
			else {
				return true;
			}
		}
	}
	else {
		return true;
	}

	/*
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	pd->Begin("testPoi456nt7123", true);
	pd->AddSphere(rayHitOut.position, 0.8f, ColorF(0, 0, 1), 10);
	CryLog("safe result : %i", result);
	return result == MNM::ERayCastResult::Hit ? true : false;
	*/
	return true;
}

bool AIControllerComponent::IsCoverUsable(Vec3 point, IEntity* target)
{
	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();
	//height target estefade beshe
	Vec3 targetPos = Vec3(target->GetWorldPos().x, target->GetWorldPos().y, target->GetWorldPos().z + 2.1f);
	if (target->GetComponent<PlayerComponent>()) {
		if (target->GetComponent<PlayerComponent>()->IsCrouching()) {
			targetPos = Vec3(target->GetWorldPos().x, target->GetWorldPos().y, target->GetWorldPos().z + 1.0f);
		}
	}

	point.z += 1.5f;
	Vec3 dir = targetPos - point;
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(point, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {
			//Debug
			if (pd) {
				pd->Begin("CoverRaycast", true);
				pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 1, 0), 2);
			}

			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				if (hitEntity == target) {
					return true;
				}
				else
				{
					return false;
				}
			}
			else {
				return false;
			}
		}
	}
	else {
		return false;
	}
	return false;
}

Vec3 AIControllerComponent::snapToNavmesh(Vec3 point)
{
	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	NavigationMeshID navMeshId = gEnv->pAISystem->GetNavigationSystem()->FindEnclosingMeshID(agentTypeId, point);
	MNM::SOrderedSnappingMetrics snappingMetrics;
	snappingMetrics.CreateDefault();
	SAcceptAllQueryTrianglesFilter filter;
	MNM::SPointOnNavMesh pointOnNavMesh = gEnv->pAISystem->GetNavigationSystem()->SnapToNavMesh(agentTypeId, point, snappingMetrics, &filter, &navMeshId);
	return pointOnNavMesh.GetWorldPosition();
}
