#include "StdAfx.h"
#include "ShootAccuracy.h"
#include "ActorState.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterShootAccuracyComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(ShootAccuracyComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterShootAccuracyComponent);
}

void ShootAccuracyComponent::Initialize()
{
}

Cry::Entity::EventFlags ShootAccuracyComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void ShootAccuracyComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltatime = event.fParam[0];

		UpdateShootError(deltatime);
	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void ShootAccuracyComponent::UpdateShootError(f32 deltatime)
{
	if (!m_stateComp) {
		return;
	}

	//add to m_shootError if condition exist
	if (m_stateComp->GetState() == EActorState::WALKING || m_stateComp->GetState() == EActorState::RUNNING || m_stateComp->GetState() == EActorState::FALLING) {
		if (m_shootError <= m_maxError) {
			m_shootError = CLAMP(m_shootError += deltatime, 0, m_maxError);
		}
		else {
			m_shootError = m_maxError;
		}
	}
	//otherwise return m_shootError back to zero
	else {
		if (m_shootError > 0) {
			m_shootError = CLAMP(m_shootError -= deltatime, 0, m_maxError);
		}
	}

	//CryLog("m_error : %f", m_shootError);
}

void ShootAccuracyComponent::AddShootError(f32 amount)
{
	m_shootError += CLAMP(amount, 0, m_maxError);
}

void ShootAccuracyComponent::SetOwnerEntity(IEntity* ownerEntity)
{
	m_ownerEntity = ownerEntity;
}

void ShootAccuracyComponent::SetStateComponent(ActorStateComponent* stateComp)
{
	m_stateComp = stateComp;
}

f32 ShootAccuracyComponent::GetShootError()
{
	return m_shootError;
}
