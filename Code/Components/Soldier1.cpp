#include "StdAfx.h"
#include "Soldier1.h"
#include "Player.h"
#include "IAIActor.h"
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
	m_animationComp->SetAnimationDrivenMotion(false);
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
	m_aiControllerComp->SetActorStateComponent(m_stateComp);

	//detection
	m_detectionComp = m_pEntity->GetOrCreateComponent<AIDetectionComponent>();

	//animations
	m_idleFragmentId = m_animationComp->GetFragmentId("Idle");
	m_cautiousFragmentId = m_animationComp->GetFragmentId("Cautious");
	m_combatFragmentId = m_animationComp->GetFragmentId("Combat");
	m_closeAttackFragmentId = m_animationComp->GetFragmentId("CloseAttack");
	m_closeAttackAction = new TAction<SAnimationContext>(30U, m_closeAttackFragmentId);

	//last target position
	InitLastTargetPositionEntity();
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
		f32 deltatime = event.fParam[0];

		UpdateAnimation();
		UpdateCurrentSpeed();
		UpdateLastTargetPositionEntity();

		//test
		m_aiControllerComp->Patrol(nullptr);

		//todo ?
		if (!m_stateComp->GetCharacterController()) {
			m_stateComp->SetCharacterController(m_aiControllerComp->GetCharacterController());
		}

		//set target for detection object
		if (m_targetEntity) {
			Attack();
		}
		else {
			m_detectionComp->SetCurrentTarget(nullptr);
		}

		//timers 
		if (m_closeAttackTimePassed < m_timeBetweenCloseAttacks) {
			m_closeAttackTimePassed += 0.5f * deltatime;
		}

	}break;
	case Cry::Entity::EEvent::Reset: {
		m_targetEntity = nullptr;
		m_testTargetEntity = nullptr;
		m_currentSpeed = DEFAULT_SOLDIER_1_WALK_SPEED;

	}break;
	default:
		break;
	}
}

void Soldier1Component::InitLastTargetPositionEntity()
{
	SEntitySpawnParams targetPositionpawnParams;
	targetPositionpawnParams.vPosition = m_pEntity->GetWorldPos();
	m_lastTargetPosition = gEnv->pEntitySystem->SpawnEntity(targetPositionpawnParams);
}

Vec3 Soldier1Component::GetRandomPointToMoveTo(Vec3 Around, f32 distance)
{
	Vec3 point = m_aiControllerComp->GetRandomPointOnNavmesh(15, m_lastTargetPosition != nullptr ? m_lastTargetPosition : m_pEntity);
	//if (m_detectionComp->IsTargetFound()) {
	//	while (!m_detectionComp->IsVisibleFrom(point, m_targetEntity)) {
	//		point = m_aiControllerComp->GetRandomPointOnNavmesh(15, m_lastTargetPosition != nullptr ? m_lastTargetPosition->GetWorldPos() : m_pEntity->GetWorldPos());
	//	}
	//}
	return point;
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

	//Set travel speed
	if (m_stateComp->GetState() == EActorState::IDLE) {
		m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelSpeed, 0.f);
	}
	else if (m_stateComp->GetState() == EActorState::WALKING) {
		m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelSpeed, 2.f);
	}
	else if (m_stateComp->GetState() == EActorState::RUNNING) {
		m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelSpeed, 3.1f);
	}
	

	int32 inv = rightDot < 0 ? 1 : -1;
	m_animationComp->SetMotionParameter(EMotionParamID::eMotionParamID_TravelAngle, crymath::acos(forwardDot) * inv);

	//update animation fragment
	FragmentID currentFragmentId;
	if (m_detectionComp->GetDetectionState() == EDetectionState::IDLE) {
		currentFragmentId = m_idleFragmentId;
	}
	else if (m_detectionComp->GetDetectionState() == EDetectionState::CAUTIOUS) {
		currentFragmentId = m_cautiousFragmentId;
	}
	else if (m_detectionComp->GetDetectionState() == EDetectionState::COMBAT) {
		currentFragmentId = m_combatFragmentId;
	}

	if (m_activeFragmentId != currentFragmentId) {
		m_activeFragmentId = currentFragmentId;

		m_animationComp->QueueFragmentWithId(m_activeFragmentId);
	}
}

void Soldier1Component::UpdateCurrentSpeed()
{
	if (m_detectionComp->GetDetectionState() == EDetectionState::IDLE || m_detectionComp->GetDetectionState() == EDetectionState::CAUTIOUS) {
		m_currentSpeed = m_walkSpeed;
	}else if(m_detectionComp->GetDetectionState() == EDetectionState::COMBAT) {
		m_currentSpeed = m_runSpeed;
	}
	m_stateComp->SetCurrentSpeed(m_currentSpeed);
}

void Soldier1Component::UpdateLastTargetPositionEntity()
{
	if (!m_lastTargetPosition) {
		return;
	}
	if (!m_targetEntity) {
		return;
	}


	if (m_detectionComp->IsVisible(m_targetEntity) && (m_detectionComp->GetDetectionState() == EDetectionState::CAUTIOUS || m_detectionComp->GetDetectionState() == EDetectionState::COMBAT)) {
		m_lastTargetPosition->SetPos(m_targetEntity->GetWorldPos());
	}
	
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (pd) {
		pd->Begin("DetectionRaycast", true);
		pd->AddSphere(m_lastTargetPosition->GetWorldPos(), 0.6f, ColorF(1, 0, 0), 2);
	}
}

void Soldier1Component::Attack()
{
	if (!m_targetEntity) {
		return;
	}

	m_detectionComp->SetCurrentTarget(m_targetEntity);
	if (m_detectionComp->IsTargetFound()) {
		f32 distanceTotarget = m_pEntity->GetWorldPos().GetDistance(m_targetEntity->GetWorldPos());
		if (distanceTotarget > m_maxAttackDistance || !m_detectionComp->IsTargetFound()) {
			MoveTo(m_lastTargetPosition->GetWorldPos());
		}
		else {
			if (distanceTotarget > m_closeAttackDistance) {
				MoveAroundTarget(m_lastTargetPosition);
			}
			else {
				m_aiControllerComp->LookAt(m_targetEntity->GetWorldPos());
				CloseAttack();
			}
		}
	}
}

void Soldier1Component::CloseAttack()
{
	if (m_closeAttackTimePassed >= m_timeBetweenCloseAttacks) {
		m_animationComp->GetActionController()->Flush();
		m_activeFragmentId = m_closeAttackFragmentId;
		m_animationComp->QueueCustomFragment(*m_closeAttackAction);
		m_closeAttackTimePassed = 0;

		if (m_targetEntity->GetComponent<PlayerComponent>()) {
			Vec3 dir = m_targetEntity->GetWorldPos() - m_pEntity->GetWorldPos();
			m_targetEntity->GetComponent<PlayerComponent>()->GetCharacterController()->AddVelocity(dir * 7);
		}
	}
}

bool Soldier1Component::CanMove()
{
	return m_closeAttackTimePassed >= m_timeBetweenCloseAttacks;
}

void Soldier1Component::StopMoving()
{
	m_aiControllerComp->GetCharacterController()->ChangeVelocity(ZERO, Cry::DefaultComponents::CCharacterControllerComponent::EChangeVelocityMode::Jump);
	m_aiControllerComp->MoveTo(m_pEntity->GetWorldPos());
}

void Soldier1Component::MoveTo(Vec3 pos)
{
	if (!m_aiControllerComp) {
		CryLog("Soldier1Component : (MoveTo) m_aiControllerComp is not assigned !");
		return;
	}
	//stop moving if can't move
	if (!CanMove()) {
		StopMoving();
		return;
	}

	m_aiControllerComp->MoveToAndLookAtWalkDirection(pos);
}

void Soldier1Component::MoveAroundTarget(IEntity* target)
{
	if (!m_targetEntity) {
		return;
	}
	//stop moving if can't move
	if (!CanMove()) {
		StopMoving();
		return;
	}

	if (testMoveToPos == ZERO || m_pEntity->GetWorldPos().GetDistance(testMoveToPos) < 2) {
		testMoveToPos = GetRandomPointToMoveTo(m_lastTargetPosition != nullptr ? m_lastTargetPosition->GetWorldPos() : m_pEntity->GetWorldPos(), 10);
	}

	//testing
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	pd->Begin("testPoint123", true);
	pd->AddSphere(testMoveToPos, 0.5f, ColorF(0, 0, 1), 10);

	if (m_detectionComp->IsTargetFound()) {
		m_aiControllerComp->MoveTo(testMoveToPos);
		m_aiControllerComp->LookAt(m_lastTargetPosition->GetWorldPos());
	}
	else {
		m_aiControllerComp->MoveToAndLookAtWalkDirection(testMoveToPos);
	}
}
