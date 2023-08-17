#include "StdAfx.h"
#include "DestroyTimer.h"
#include "GamePlugin.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterDestroyTimerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(DestroyTimerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterDestroyTimerComponent);
}


void DestroyTimerComponent::Initialize()
{
}

Cry::Entity::EventFlags DestroyTimerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void DestroyTimerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {


	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltaTime = event.fParam[0];

		DestroyEntity(deltaTime);

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void DestroyTimerComponent::DestroyEntity(f32 deltaTime)
{
	if (m_timePassed < m_maxTimeAlive) {
		m_timePassed += 0.5f * deltaTime;
	}
	else {
		gEnv->pEntitySystem->RemoveEntity(m_pEntity->GetId());
	}
}

void DestroyTimerComponent::SetMaxTimeAlive(f32 maxTimeAlive)
{
	this->m_maxTimeAlive = maxTimeAlive;
}
