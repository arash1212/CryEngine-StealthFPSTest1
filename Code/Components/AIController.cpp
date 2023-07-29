#include "StdAfx.h"
#include "AIController.h"
#include "GamePlugin.h"

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
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_navigationComp->GetRequestedVelocity());
}

void AIControllerComponent::MoveToAndLookAtWalkDirection(Vec3 position)
{
	NavigateTo(position);
	m_characterControllerComp->SetVelocity(m_navigationComp->GetRequestedVelocity());
	m_pEntity->SetRotation(Quat::CreateRotationVDir(m_navigationComp->GetRequestedVelocity()));
}

Vec3 AIControllerComponent::GetRandomPointOnNavmesh(float MaxDistance, Vec3 Around)
{
	MNM::TriangleIDArray resultArray;

	NavigationAgentTypeID agentTypeId = NavigationAgentTypeID::TNavigationID(1);
	NavigationMeshID navMeshId = gEnv->pAISystem->GetNavigationSystem()->FindEnclosingMeshID(agentTypeId, Around);

	//get Triangles
	const Vec3 triggerBoxSize = Vec3(50, 50, 50);
    MNM::aabb_t	 aabb = MNM::aabb_t(triggerBoxSize * -25.5f, triggerBoxSize * 25.5f);
	MNM::TriangleIDArray triangleIDArray = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->QueryTriangles(aabb);
	if (triangleIDArray.size() <= 0 || !triangleIDArray[0].IsValid() || !navMeshId.IsValid() || !agentTypeId.IsValid()) {
		return m_pEntity->GetWorldPos();
	}

	//check distances
	for (int32 i = 0; i < triangleIDArray.size(); i++) {
		MNM::TriangleIDArray iDArray;
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
	triangleIDArray.push_back(selectedId);


	MNM::SClosestTriangle closestTriangle = gEnv->pAISystem->GetNavigationSystem()->GetMNMNavMesh(navMeshId)->FindClosestTriangle(Around, triangleIDArray);
	return closestTriangle.position.GetVec3();
}
