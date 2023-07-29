#include "StdAfx.h"
#include "AIController.h"
#include "GamePlugin.h"

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

	m_navigationComp = m_pEntity->GetOrCreateComponent<IEntityNavigationComponent>();
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
