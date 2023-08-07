#include "StdAfx.h"
#include "SecurityCamera.h"
#include "Player.h"
#include "AIDetection.h"
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

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	m_detectionComp = m_pEntity->GetOrCreateComponent<AIDetectionComponent>();
	m_detectionComp->SetMaxDetectionDegree(160);
	m_detectionComp->SetMinDetectionDegree(118);
	m_detectionComp->SetDetectionHeight(0.8f);

	m_defaultZRotation = m_pEntity->GetRotation().GetRotZ();

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

	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltaTime = event.fParam[0];

		UpdateRotation(deltaTime);

		if (!m_targetEntity) {
			m_targetEntity = gEnv->pEntitySystem->FindEntityByName("playerEntity");
		}

	}break;
	case Cry::Entity::EEvent::Reset: {
		m_targetEntity = nullptr;

	}break;
	default:
		break;
	}
}

void SecurityCameraComponent::UpdateRotation(f32 DeltaTime)
{
	//rotate normaly if target is not visible
	if (!m_targetEntity || !m_detectionComp->IsTargetCanBeSeen(m_targetEntity)) {
		Quat currentRotation = m_pEntity->GetRotation();
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

		m_pEntity->SetRotation(Quat::CreateSlerp(currentRotation, rotation, timeElapsed / 350));
		timeElapsed += DeltaTime;

		//stop detection sound
		bIsDetectionSoundPlayed = false;
		m_audioComp->StopTrigger(m_detectionSound);
	}

	//if target is visible
	else if (m_detectionComp->IsTargetCanBeSeen(m_targetEntity)) {
		Vec3 dir = m_targetEntity->GetWorldPos() - m_pEntity->GetWorldPos();
		dir.z += 3.5f;
		m_pEntity->SetRotation(Quat::CreateSlerp(m_pEntity->GetRotation(), Quat::CreateRotationVDir(-dir), timeElapsed / 250));
		timeElapsed += DeltaTime;

		//play sound
		if (!bIsDetectionSoundPlayed) {
			bIsDetectionSoundPlayed = true;
			m_audioComp->ExecuteTrigger(m_detectionSound);
		}
	}
}
