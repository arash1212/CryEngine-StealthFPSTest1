#include "StdAfx.h"
#include "Player.h"
#include "AlarmSpeaker.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterAlarmSpeakerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(AlarmSpeakerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterAlarmSpeakerComponent);
}


void AlarmSpeakerComponent::Initialize()
{
	m_meshComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CStaticMeshComponent>();
	m_meshComp->SetFilePath("Objects/alarm1/alarm_1.cgf");
	m_meshComp->LoadFromDisk();
	m_meshComp->ResetObject();

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	m_alarmSoundId = CryAudio::StringToId("alarm_sound_1");

	//physicalize speaker
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_STATIC;
	physParams.mass = 20000.f;
	m_pEntity->Physicalize(physParams);
}

Cry::Entity::EventFlags AlarmSpeakerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void AlarmSpeakerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		//f32 deltaTime = event.fParam[0];

		UpdateAlarm();
	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void AlarmSpeakerComponent::UpdateAlarm()
{
	if (bIsEnabled) {
		if (!bIsSoundPlaying) {
			m_audioComp->ExecuteTrigger(m_alarmSoundId);
			bIsSoundPlaying = true;
		}
	}
	else
	{
		bIsSoundPlaying = false;
		m_audioComp->StopTrigger(m_alarmSoundId);
	}
}

void AlarmSpeakerComponent::SetEnabled(bool isEnable)
{
	this->bIsEnabled = isEnable;
}
