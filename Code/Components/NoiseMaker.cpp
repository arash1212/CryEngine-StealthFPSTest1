#include "StdAfx.h"
#include "NoiseMaker.h"
#include "Soldier1.h"
#include "ActorInfo.h"
#include "GamePlugin.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterNoiseMakerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(NoiseMakerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterNoiseMakerComponent);
}

void NoiseMakerComponent::Initialize()
{

}

Cry::Entity::EventFlags NoiseMakerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void NoiseMakerComponent::ProcessEvent(const SEntityEvent& event)
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

void NoiseMakerComponent::MakeNoise()
{
	/*
	DynArray<IPhysicalEntity**> pEntities;
	CryLog("noise entity size : %i", gEnv->pEntitySystem->GetPhysicalEntitiesInBox(m_pEntity->GetWorldPos(), m_noiseDistance, *pEntities.data()));
	*/
	IEntityItPtr entityItPtr = gEnv->pEntitySystem->GetEntityIterator();
	entityItPtr->MoveFirst();

	while (!entityItPtr->IsEnd()) {
		IEntity* currentEntity = entityItPtr->Next();

		f32 distance = m_pEntity->GetWorldPos().GetDistance(currentEntity->GetWorldPos());
		if (distance <= m_noiseDistance && currentEntity->GetComponent<Soldier1Component>()) {
			//if (m_owner->GetComponent<ActorInfoComponent>() && currentEntity->GetComponent<ActorInfoComponent>() && m_owner->GetComponent<ActorInfoComponent>()->GetFaction() != currentEntity->GetComponent<ActorInfoComponent>()->GetFaction()) {
				currentEntity->GetComponent<Soldier1Component>()->SetLastTargetPosition(m_pEntity->GetWorldPos());
				currentEntity->GetComponent<Soldier1Component>()->SetDetectionToMax();
				currentEntity->GetComponent<Soldier1Component>()->SetFoundTarget(true);
		//	}
		}
	}
}

void NoiseMakerComponent::SetNoiseDistance(f32 noiseDistance)
{
	this->m_noiseDistance = noiseDistance;
}

void NoiseMakerComponent::SetOwner(IEntity* owner)
{
	this->m_owner = owner;
}
