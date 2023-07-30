#include "StdAfx.h"
#include "Soldier1.h"
#include "AIController.h"
#include "AIDetection.h"
#include "ActorState.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterSoldier1Component(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(Soldier1Component));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterSoldier1Component);
}


void Soldier1Component::Initialize()
{
	m_animationComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	m_animationComp->SetTransformMatrix(Matrix34::Create(Vec3(1), Quat::CreateRotationXYZ(Ang3(DEG2RAD(90), 0, DEG2RAD(180))), Vec3(0)));
	m_animationComp->SetCharacterFile("Objects/Characters/soldier1/soldier_1.cdf");
	m_animationComp->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/soldier1.adb");
	m_animationComp->SetControllerDefinitionFile("Animations/Mannequin/ADB/ThirdPersonControllerDefinition.xml");
	m_animationComp->SetDefaultScopeContextName("ThirdPersonCharacter");
	m_animationComp->SetDefaultFragmentName("Idle");
	m_animationComp->SetAnimationDrivenMotion(true);
	m_animationComp->LoadFromDisk();
	m_animationComp->ResetCharacter();

	//ai controller
	m_aiControllerComp = m_pEntity->GetOrCreateComponent<AIControllerComponent>();

	//state
	m_stateComp = m_pEntity->GetOrCreateComponent<ActorStateComponent>();
	m_stateComp->SetCharacterController(m_aiControllerComp->GetCharacterController());
	m_stateComp->SetWalkSpeed(m_walkSpeed);
	m_stateComp->SetRunSpeed(m_runSpeed);
	m_stateComp->SetCurrentSpeed(m_currentSpeed);

	//detection
	m_detectionComp = m_pEntity->GetOrCreateComponent<AIDetectionComponent>();
}

Cry::Entity::EventFlags Soldier1Component::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void Soldier1Component::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {
		//todo : test target hazf beshe
		if (!m_testTargetEntity) {
			m_testTargetEntity = gEnv->pEntitySystem->FindEntityByName("AiTestTargetEntity");
		}

		if (!m_targetEntity) {
			m_targetEntity = gEnv->pEntitySystem->FindEntityByName("playerEntity");
		}


	}break;
	case Cry::Entity::EEvent::Update: {
		UpdateAnimation();

		//move to target
		if (m_targetEntity) {
			//MoveTo(m_targetEntity->GetWorldPos());

			m_detectionComp->IsInView(m_targetEntity);
			if (m_detectionComp->IsVisible(m_targetEntity)) {
				CryLog("player visible !");
			}
		}
	}break;
	case Cry::Entity::EEvent::Reset: {
		m_targetEntity = nullptr;
		m_testTargetEntity = nullptr;

	}break;
	default:
		break;
	}
}

void Soldier1Component::UpdateAnimation()
{
	if (!m_aiControllerComp) {
		return;
	}

	Vec3 forwardVector = m_pEntity->GetForwardDir().normalized();
	Vec3 rightVector = m_pEntity->GetRightDir().normalized();
	Vec3 velocity = m_aiControllerComp->GetVelocity().normalized();

	float forwardDot = velocity.dot(forwardVector);
	float rightDot = velocity.dot(rightVector);
	if (m_stateComp->GetState() == EActorState::WALKING) {
		m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelSpeed, 2.f);
	}
	else if (m_stateComp->GetState() == EActorState::RUNNING) {
		m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelSpeed, 3.1f);
	}

	int32 inv = rightDot < 0 ? 1 : -1;
	m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelAngle, crymath::acos(forwardDot) * inv);
}

void Soldier1Component::MoveTo(Vec3 pos)
{
	if (!m_aiControllerComp) {
		CryLog("Soldier1Component : (MoveTo) m_aiControllerComp is not assigned !");
		return;
	}

	//testing
	if (testMoveToPos == ZERO || m_pEntity->GetWorldPos().GetDistance(testMoveToPos) < 2) {
		testMoveToPos = m_aiControllerComp->GetRandomPointOnNavmesh(15, m_targetEntity != nullptr ? m_targetEntity->GetWorldPos() : m_pEntity->GetWorldPos());

		f32 max = 10;
		f32 min = 0;
		f32 random = (min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min)));

		Vec3 Dir = testMoveToPos - m_pEntity->GetWorldPos();
		testMoveToPos = m_targetEntity->GetWorldPos() + Dir.normalize() * random;
	}

	m_aiControllerComp->MoveTo(testMoveToPos);
	if (m_targetEntity) {
		m_aiControllerComp->LookAt(m_targetEntity->GetWorldPos());
	}
}
