#include "StdAfx.h"
#include "Player.h"
#include "AlarmManager.h"
#include "AlarmSpeaker.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterAlarmManagerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(AlarmManagerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterAlarmManagerComponent);
}


void AlarmManagerComponent::Initialize()
{

	m_pEntity->SetName("alarmManager");
}

Cry::Entity::EventFlags AlarmManagerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void AlarmManagerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {
		m_pEntity->SetName("alarmManager");

		FillSpeakers();

	}break;
	case Cry::Entity::EEvent::Update: {
		//f32 deltaTime = event.fParam[0];

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void AlarmManagerComponent::FillSpeakers()
{
	IEntityItPtr entityItPtr = gEnv->pEntitySystem->GetEntityIterator();
	entityItPtr.get()->MoveFirst();
	while (!entityItPtr.get()->IsEnd())
	{
		IEntity* entity = entityItPtr.get()->Next();
		if (entity->GetComponent<AlarmSpeakerComponent>()) {
			m_speakerEntities.PushBack(entity);
		}
	}
}

void AlarmManagerComponent::SetEnabled(bool isEnable)
{
	this->bIsAlarmEnabled = isEnable;

	for (int32 i = 0; i < m_speakerEntities.Size(); i++) {
		m_speakerEntities.At(i)->GetComponent<AlarmSpeakerComponent>()->SetEnabled(bIsAlarmEnabled);
	}
}

bool AlarmManagerComponent::IsAlarmEnable()
{
	return bIsAlarmEnabled;
}
