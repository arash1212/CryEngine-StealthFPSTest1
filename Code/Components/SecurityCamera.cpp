#include "StdAfx.h"
#include "SecurityCamera.h"
#include "Player.h"
#include "AlarmSpeaker.h"
#include "Soldier1.h"
#include "AIDetection.h"
#include "AlarmManager.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterSecurityCameraComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(SecurityCameraComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterSecurityCameraComponent);
}


void SecurityCameraComponent::Initialize()
{
	m_meshComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CStaticMeshComponent>();
	m_meshComp->SetFilePath("Objects/securitycamera/security-camera-1.cgf");
	m_meshComp->LoadFromDisk();
	m_meshComp->ResetObject();

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	m_detectionComp = m_pEntity->GetOrCreateComponent<AIDetectionComponent>();
	m_detectionComp->SetMaxDetectionDegree(180);
	m_detectionComp->SetMinDetectionDegree(118);
	m_detectionComp->SetDetectionHeight(0.8f);
	m_detectionComp->SetTargetNormalHeight(0.8f);
	m_detectionComp->SetTargetCrouchHeight(-0.07f);

	m_defaultZRotation = m_pEntity->GetRotation().GetRotZ();

	//info
	m_info = m_pEntity->GetOrCreateComponent<ActorInfoComponent>();

	//health
	m_healthComp = m_pEntity->GetOrCreateComponent<HealthComponent>();
	m_healthComp->SetMaxHealth(40.f);

	//physicalize camera
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_STATIC;
	physParams.mass = 20000.f;
	m_pEntity->Physicalize(physParams);
}

Cry::Entity::EventFlags SecurityCameraComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void SecurityCameraComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {
		m_defaultX = m_pEntity->GetRotation().v.x;

	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltaTime = event.fParam[0];

		if (!bIsDestroyed) {
			UpdateRotation(deltaTime);

			if (!m_targetEntity) {
				m_targetEntity = gEnv->pEntitySystem->FindEntityByName("playerEntity");
			}

			if (!m_alaramManager) {
				m_alaramManager = gEnv->pEntitySystem->FindEntityByName("alarmManager");
			}


			Destroy();
		}
		else {
			m_pEntity->SetLocalTM(Matrix34::Create(m_pEntity->GetScale(), Quat::CreateSlerp(m_pEntity->GetRotation(), Quat::CreateRotationXYZ(Ang3(m_defaultX - 5, 0, m_pEntity->GetRotation().GetRotZ())), 0.4f * gEnv->pTimer->GetFrameTime()), m_pEntity->GetPos()));
		}


	}break;
	case Cry::Entity::EEvent::Reset: {
		m_targetEntity = nullptr;
		bIsDestroyed = false;

	}break;
	default:
		break;
	}
}

void SecurityCameraComponent::UpdateRotation(f32 DeltaTime)
{
	Quat currentRotation = m_pEntity->GetRotation();
	//rotate normaly if target is not visible
	if (!m_targetEntity || !m_detectionComp->IsTargetCanBeSeen(m_targetEntity)) {
		Quat rotation;

		Quat minRot = Quat::CreateRotationZ(m_defaultZRotation - m_maxZRotation);
		Quat maxRot = Quat::CreateRotationZ(m_defaultZRotation + m_maxZRotation);
		f32 currentRot = currentRotation.GetRotZ();
		//check if rotation reached right end
		if (currentRot == minRot.GetRotZ()) {
			bIsReachedRightEnd = true;
			timeElapsed = 0.06f;
		}
		if (currentRot == maxRot.GetRotZ()) {
			bIsReachedRightEnd = false;
			timeElapsed = 0.06f;
		}

		//rotation (left/right)
		if (!bIsReachedRightEnd) {
			rotation = minRot;
		}
		else {
			rotation = maxRot;
		}

		if (m_returningBackToNormalTimePassed >= m_timeBetweenReturningBackToNormal) {
			m_pEntity->SetRotation(Quat::CreateSlerp(currentRotation, rotation, timeElapsed / 350));
			timeElapsed += DeltaTime;
		}
		else {
			m_returningBackToNormalTimePassed += 0.5f * DeltaTime;
		}

		//stop detection sound
		bIsDetectionSoundPlayed = false;
		m_audioComp->StopTrigger(m_detectionSound);

		m_triggeringAlaramsTimePassed = 0;
		bIsTriggeredAlarams = false;
	}

	//if target is visible
	else if ( m_detectionComp->IsTargetCanBeSeen(m_targetEntity)) {
		f32 distanceToTarget = m_pEntity->GetWorldPos().GetDistance(m_targetEntity->GetWorldPos());
		if (distanceToTarget <= m_maxDetectionDistance) {
			m_returningBackToNormalTimePassed = 0;

			Vec3 targetPos = Vec3(m_targetEntity->GetWorldPos().x, m_targetEntity->GetWorldPos().y, m_targetEntity->GetWorldPos().z);
			Vec3 dir = targetPos - m_pEntity->GetWorldPos();
			dir.z += 3.5f;

			m_pEntity->SetRotation(Quat::CreateSlerp(m_pEntity->GetRotation(), Quat::CreateRotationVDir(-dir), timeElapsed / 250));
			timeElapsed += DeltaTime;

			//play sound
			if (!bIsDetectionSoundPlayed) {
				bIsDetectionSoundPlayed = true;
				m_audioComp->ExecuteTrigger(m_detectionSound);
			}

			m_triggeringAlaramsTimePassed += 0.5f * DeltaTime;
			if (m_triggeringAlaramsTimePassed >= m_timeBetweenTriggeringAlarams && !bIsTriggeredAlarams) {
				TriggerAlarams();
				bIsTriggeredAlarams = true;

				SetSoldiersLastTargetPosition();
			}
		}
	}
}

void SecurityCameraComponent::TriggerAlarams()
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
	m_alaramManager->GetComponent<AlarmManagerComponent>()->SetEnabled(true);
}

void SecurityCameraComponent::Destroy()
{
	if (!m_healthComp || bIsDestroyed) {
		return;
	}

	if (m_healthComp->GetHealth() <= 0) {
		//m_pEntity->SetRotation(Quat::CreateSlerp(m_pEntity->GetRotation(), Quat::CreateRotationY(m_defaultY - 40), 0.1f * gEnv->pTimer->GetFrameTime()));
		bIsDestroyed = true;

		//
		bIsDetectionSoundPlayed = false;
		m_audioComp->StopTrigger(m_detectionSound);

		m_audioComp->ExecuteTrigger(m_destroySound);
		m_triggeringAlaramsTimePassed = 0;
		bIsTriggeredAlarams = false;
	}
}

void SecurityCameraComponent::SetSoldiersLastTargetPosition()
{
	//todo
	int32 num = 0;

	IEntityItPtr entityItPtr = gEnv->pEntitySystem->GetEntityIterator();
	entityItPtr->MoveFirst();
	while (!entityItPtr->IsEnd()) {
		IEntity* entity = entityItPtr->Next();
		if (entity->GetComponent<Soldier1Component>()) {
			f32 distanceToSoldier = entity->GetWorldPos().GetDistance(m_pEntity->GetWorldPos());

			if (num < 3) {
				if (distanceToSoldier < 40) {
					entity->GetComponent<Soldier1Component>()->SetLastCameraReportedPos(m_targetEntity->GetWorldPos());
					entity->GetComponent<Soldier1Component>()->SetDetectionToCautious();
				}
				else {
					continue;
				}
			}
			else {
				break;
			}
		}
	}
}
