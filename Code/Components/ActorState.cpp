#include "StdAfx.h"
#include "ActorState.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterActorStateComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(ActorStateComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterActorStateComponent);
}

void ActorStateComponent::Initialize()
{

}

Cry::Entity::EventFlags ActorStateComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void ActorStateComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		UpdateState();

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void ActorStateComponent::UpdateState()
{
	if (m_characterController->IsWalking() && m_currentSpeed == m_walkSpeed) {
		m_state = EActorState::WALKING;
	}else if (m_characterController->IsWalking() && m_currentSpeed == m_runSpeed) {
		m_state = EActorState::RUNNING;
	}
	else if (!m_characterController->IsWalking()) {
		m_state = EActorState::IDLE;
	}
}

void ActorStateComponent::SetCharacterController(Cry::DefaultComponents::CCharacterControllerComponent* characterController)
{
	this->m_characterController = characterController;
}

EActorState ActorStateComponent::GetState()
{
	return m_state;
}

void ActorStateComponent::SetWalkSpeed(f32& walkSpeed)
{
	this->m_walkSpeed = walkSpeed;
}

void ActorStateComponent::SetRunSpeed(f32& runSpeed)
{
	this->m_runSpeed = runSpeed;
}

void ActorStateComponent::SetCurrentSpeed(f32& currentSpeed)
{
	m_currentSpeed = currentSpeed;
}
