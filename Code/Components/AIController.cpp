#include "StdAfx.h"
#include "AIController.h"
#include "ActorState.h"
#include "GamePlugin.h"

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
	m_navigationComp->NavigateTo(position);
}

void AIControllerComponent::MoveTo(Vec3 position)
{
	if (!m_stateComp) {
		CryLog("AIControllerComponent : (MoveTo) m_stateComp is null!");
		return;
	}
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_stateComp->GetState() == EActorState::RUNNING ? m_navigationComp->GetRequestedVelocity() * 2 : m_navigationComp->GetRequestedVelocity());
}

void AIControllerComponent::MoveToAndLookAtWalkDirection(Vec3 position)
{
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_navigationComp->GetRequestedVelocity());
	m_pEntity->SetRotation(Quat::CreateRotationVDir(m_navigationComp->GetRequestedVelocity()));
}

Vec3 AIControllerComponent::GetRandomPointOnNavmesh(float MaxDistance, IEntity* Around)
{
	MNM::TriangleIDArray resultArray;
	DynArray<Vec3> resultPositions;

	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	NavigationMeshID navMeshId = gEnv->pAISystem->GetNavigationSystem()->FindEnclosingMeshID(agentTypeId, Around->GetWorldPos());

	//get Triangles
	const Vec3 triggerBoxSize = Vec3(50, 50, 50);
    MNM::aabb_t	 aabb = MNM::aabb_t(triggerBoxSize * -25.5f, triggerBoxSize * 25.5f);
	MNM::TriangleIDArray triangleIDArray = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->QueryTriangles(aabb);
	if (triangleIDArray.size() <= 0 || !triangleIDArray[0].IsValid() || !navMeshId.IsValid() || !agentTypeId.IsValid()) {
		return m_pEntity->GetWorldPos();
	}

	for (int32 i = 0; i < triangleIDArray.size(); i++) {
		int32 j = 0;
		Triangle triangle;
		gEnv->pAISystem->GetNavigationSystem()->GetTriangleVertices(navMeshId, triangleIDArray[i], triangle);
		while (j < 12) {
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

	f32 resultMax = MaxDistance;
	f32 resultMin = 0;
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

void AIControllerComponent::Patrol(INavPath* path)
{
	//PathFollowerParams params;
	//IPathFollower* follower;
	//follower->SetParams(params);
	//follower->AttachToPath(path);
	//follower->Draw(m_pEntity->GetWorldPos());

	NavigationVolumeID vId =gEnv->pAISystem->GetNavigationSystem()->GetAreaId("1");
	IEntity* pathTest = gEnv->pEntitySystem->FindEntityByName("ai-path1");
	if (pathTest) {
		CryLog("Path Found");
	}
	else {
		CryLog("Path Not Found");
	}
	//INavPathPtr pt =gEnv->pAISystem->GetNavigationSystem()
	// gEnv->pAISystem->GetMNMPathfinder();
}

