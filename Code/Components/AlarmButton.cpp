#include "StdAfx.h"
#include "AlarmButton.h"
#include "Interactable.h"
#include "AlarmManager.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterAlarmButtonComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(AlarmButtonComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterAlarmButtonComponent);
}


void AlarmButtonComponent::Initialize()
{
	m_animationComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	m_animationComp->SetCharacterFile("Objects/alarmButton/alarmButton.cdf");
	m_animationComp->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/alarmButton.adb");
	m_animationComp->SetControllerDefinitionFile("Animations/Mannequin/ADB/InteractablesControllerDefinition.xml");
	m_animationComp->SetDefaultScopeContextName("FirstPersonCharacter");
	m_animationComp->SetDefaultFragmentName("Idle");
	m_animationComp->SetAnimationDrivenMotion(true);
	m_animationComp->LoadFromDisk();
	m_animationComp->ResetCharacter();

	m_boxComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CBoxPrimitiveComponent>();
	m_boxComp->SetTransformMatrix(Matrix34::Create(Vec3(0.3f), IDENTITY, Vec3(0)));

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	m_interactableComp = m_pEntity->GetOrCreateComponent<InteractableComponent>();

	m_pushButtonSound = CryAudio::StringToId("alarmButton_push_sound_1");

	m_useFragmentId = m_animationComp->GetFragmentId("Use");

	//physicalize button
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_STATIC;
	physParams.mass = 20000.f;
	m_pEntity->Physicalize(physParams);

}

Cry::Entity::EventFlags AlarmButtonComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void AlarmButtonComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		//f32 deltaTime = event.fParam[0];

		if (!m_alaramManager) {
			m_alaramManager = gEnv->pEntitySystem->FindEntityByName("alarmManager");
		}


		if (m_interactableComp && m_interactableComp->IsUsed()) {
			TriggerAlarams();
		}
	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void AlarmButtonComponent::TriggerAlarams()
{
	if (!m_alaramManager) {
		CryLog("SecurityCameraComponent : (TriggerAlarams) m_alaramManager is null.");
		return;
	}
	/*
	IEntityItPtr entityItPtr = gEnv->pEntitySystem->GetEntityIterator();
	entityItPtr.get()->MoveFirst();

	while (!entityItPtr.get()->IsEnd())
	{
		IEntity* entity = entityItPtr.get()->Next();
		if (entity->GetComponent<AlaramSpeakerComponent>()) {
			entity->GetComponent<AlaramSpeakerComponent>()->SetEnabled(true);
		}
	}
	*/
	m_alaramManager->GetComponent<AlarmManagerComponent>()->SetEnabled(!m_alaramManager->GetComponent<AlarmManagerComponent>()->IsAlarmEnable());
	m_animationComp->QueueFragmentWithId(m_useFragmentId);
	m_audioComp->ExecuteTrigger(m_pushButtonSound);
	m_interactableComp->Use(false);
}