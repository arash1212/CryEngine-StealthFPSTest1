#include "StdAfx.h"
#include "ActorInfo.h"
#include "Health.h"
#include "GamePlugin.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterActorInfoComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(ActorInfoComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterActorInfoComponent);
}


void ActorInfoComponent::Initialize()
{

}

Cry::Entity::EventFlags ActorInfoComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void ActorInfoComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {
		bIsGameStarted = true;


	}break;
	case Cry::Entity::EEvent::Update: {
		//f32 deltaTime = event.fParam[0];

	}break;
	case Cry::Entity::EEvent::Reset: {
		bIsGameStarted = false;

	}break;
	default:
		break;
	}
}

void ActorInfoComponent::FillEntities()
{
	Schematyc::CArray<IEntity*> allies;
	Schematyc::CArray<IEntity*> hostiles;

	IEntityItPtr entityItPtr = gEnv->pEntitySystem->GetEntityIterator();
	entityItPtr->MoveFirst();
	while (!entityItPtr->IsEnd())
	{
		IEntity* entity = entityItPtr->Next();
		if (entity->GetComponent<ActorInfoComponent>()) {
			if (entity->GetComponent<ActorInfoComponent>()->GetFaction() == m_faction) {
				allies.PushBack(entity);
			}
			else {
				hostiles.PushBack(entity);
			}
		}
	}

	m_allieEntities = allies;
	m_hostileEntities = hostiles;
}

EFaction ActorInfoComponent::GetFaction()
{
	return m_faction;
}

void ActorInfoComponent::SetFaction(EFaction faction)
{
	this->m_faction = faction;
}

IEntity* ActorInfoComponent::GetHostileEntity()
{
	FillEntities();
	IEntity* resultTarget = nullptr;
	f32 closest = gEnv->p3DEngine->GetMaxViewDistance();
	for (int32 i = 0; i < m_hostileEntities.Size(); i++) {
		f32 distance = m_pEntity->GetWorldPos().GetDistance(m_hostileEntities.At(i)->GetWorldPos());
		if (distance < closest) {
			closest = distance;
			if (m_hostileEntities.At(i)->GetComponent<HealthComponent>() && m_hostileEntities.At(i)->GetComponent<HealthComponent>()->GetHealth() > 0) {
				resultTarget = m_hostileEntities.At(i);
			}
		}
	}
	return resultTarget;
}
