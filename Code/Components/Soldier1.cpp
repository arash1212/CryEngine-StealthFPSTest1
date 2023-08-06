#include "StdAfx.h"
#include "Soldier1.h"
#include "Player.h"
#include "IAIActor.h"
#include "AIController.h"
#include "WeaponAK47.h"
#include "AIDetection.h"
#include "ActorState.h"
#include "Health.h"
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
	m_reactToHit1FragmentId = m_animationComp->GetFragmentId("ReactToHit1");
	m_reactToHit2FragmentId = m_animationComp->GetFragmentId("ReactToHit2");

	m_closeAttackAction = new TAction<SAnimationContext>(30U, m_closeAttackFragmentId);
	m_reactToHit1Action = new TAction<SAnimationContext>(30U, m_reactToHit1FragmentId);
	m_reactToHit2Action = new TAction<SAnimationContext>(30U, m_reactToHit2FragmentId);

	//shootError
	m_shootAccuracyComp = m_pEntity->GetOrCreateComponent<ShootAccuracyComponent>();
	m_shootAccuracyComp->SetOwnerEntity(m_pEntity);
	m_shootAccuracyComp->SetStateComponent(m_stateComp);
	m_shootAccuracyComp->SetMaxShootError(0.4f);
	m_shootAccuracyComp->SetShootError(0.2f);

	//last target position
	InitLastTargetPositionEntity();

	//audio
	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	//health
	m_healthComp = m_pEntity->GetOrCreateComponent<HealthComponent>();

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
		bIsGameplayStarted = true;

	}break;
	case Cry::Entity::EEvent::Update: {
		//if (bIsGameplayStarted) {
		if (bIsAlive) {
			f32 deltatime = event.fParam[0];

			PlayDetectionSound();

			//todo function ?
			if (!bIsWeaponInitDone) {
				m_gunAttachment = m_animationComp->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("gun");

				SEntitySpawnParams spawnParams;
				spawnParams.vPosition = m_gunAttachment->GetAttAbsoluteDefault().t;
				m_weaponBaseEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
				m_pEntity->AttachChild(m_weaponBaseEntity);

				//primary weapon init
				m_primaryWeapon = m_weaponBaseEntity->GetOrCreateComponent<WeaponAK47Component>();
				m_primaryWeapon->SetCharacterController(m_aiControllerComp->GetCharacterController());
				m_primaryWeapon->SetCameraComponent(nullptr);
				m_primaryWeapon->SetOwnerEntity(m_pEntity);
				m_primaryWeapon->SetCameraBaseEntity(nullptr);


				//IAttachmentObject* object = m_gunAttachment->GetIAttachmentObject();
				//todo
				m_currentlySelectedWeapon = m_primaryWeapon;

				bIsWeaponInitDone = true;
			}

			if (m_weaponBaseEntity && m_gunAttachment) {
				m_weaponBaseEntity->SetPos(m_gunAttachment->GetAttModelRelative().t);
			}

			//test
			if (m_weaponBaseEntity) {
				IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
				if (pd) {
					pd->Begin("DetectionRaycast", true);
					pd->AddSphere(m_weaponBaseEntity->GetWorldPos(), 5.6f, ColorF(0, 1, 0), 2);
				}
			}

			UpdateAnimation();
			UpdateCurrentSpeed();
			UpdateLastTargetPositionEntity();

			//todo : test target hazf beshe
			if (!m_testTargetEntity) {
				m_testTargetEntity = gEnv->pEntitySystem->FindEntityByName("AiTestTargetEntity");
			}

			if (!m_targetEntity) {
				m_targetEntity = gEnv->pEntitySystem->FindEntityByName("playerEntity");
			}

			//patrol
			//todo : && !m_patrolPathName.empty() && m_patrolPathName != ""
			if (!m_detectionComp->IsTargetFound() && !m_patrolPathName.empty() && m_patrolPathName != "") {
				MoveTo(m_aiControllerComp->Patrol(m_patrolPathName));
			}

			//todo ?
			if (!m_stateComp->GetCharacterController()) {
				m_stateComp->SetCharacterController(m_aiControllerComp->GetCharacterController());
			}

			//set target for detection object
			if (m_targetEntity) {
				//m_detectionComp->IsInView(m_targetEntity);
				Attack();
			}
			else {
				m_detectionComp->SetCurrentTarget(nullptr);
			}

			//timers 
			if (m_closeAttackTimePassed < m_timeBetweenCloseAttacks) {
				m_closeAttackTimePassed += 0.5f * deltatime;
			}
			if (m_coolDownTimePassed < m_coolDownTimer) {
				m_coolDownTimePassed += 0.5f * deltatime;
				m_currentShootCount = 0;
				m_currentlySelectedWeapon->ResetShootSoundNumber();
			}
			if (m_hitReactionTimePassed < m_hitReactionTimer) {
				m_hitReactionTimePassed += 0.5f * deltatime;
			}
			if (m_gettingHitSoundTimePassed < m_timeBetweenPlayingGettingHitSound) {
				m_gettingHitSoundTimePassed += 0.5f * deltatime;
			}
			if (m_lastTargetPositionTimePassed < m_timeBetweenCheckLastTargetPosition) {
				m_lastTargetPositionTimePassed += 0.5f * deltatime;
			}
			if (m_findingCoverTimePassed < m_timeBetweenFindingCover) {
				m_findingCoverTimePassed += 0.5f * deltatime;
			}

			Die();

		}
	//	}

	}break;
	case Cry::Entity::EEvent::Reset: {
		m_targetEntity = nullptr;
		m_testTargetEntity = nullptr;
		m_currentSpeed = DEFAULT_SOLDIER_1_WALK_SPEED;
		bIsGameplayStarted = false;

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
	Vec3 point = m_aiControllerComp->GetRandomPointOnNavmesh(10, m_lastTargetPosition != nullptr ? m_lastTargetPosition : m_pEntity);
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

		f32 distanceTotarget = m_pEntity->GetWorldPos().GetDistance(m_lastTargetPosition->GetWorldPos());
		if (distanceTotarget > m_maxAttackDistance || !m_detectionComp->IsTargetFound()) {
			MoveTo(m_lastTargetPosition->GetWorldPos());
		}
		else {
			if (distanceTotarget > m_closeAttackDistance) {

				//fire weapon if target is visible
				if (m_detectionComp->IsVisible(m_targetEntity) && CanUseWeapon()) {
					if (m_currentlySelectedWeapon->Fire(m_targetEntity)) {

						//shoot coolDown
						m_currentShootCount++;
						if (m_currentShootCount >= m_shootBeforeCoolDown) {
							m_coolDownTimePassed = 0;
						}
					}
				}

				//move around target if it close and cover is not safe
				if (!m_aiControllerComp->IsCoverPointSafe(m_currentCoverPosition, m_targetEntity)) {
					MoveAroundTarget(m_targetEntity);
					m_currentCoverPosition = m_aiControllerComp->FindCover(m_targetEntity);
				}
				else
				{
					if (m_currentCoverPosition == ZERO || !m_aiControllerComp->IsCoverPointSafe(m_currentCoverPosition, m_targetEntity) || !m_aiControllerComp->IsCoverUsable(m_currentCoverPosition, m_targetEntity)) {
						m_currentCoverPosition = m_aiControllerComp->FindCover(m_targetEntity);
						//move arount target while looking for new cover
						MoveAroundTarget(m_targetEntity);
						m_aiControllerComp->LookAt(m_lastTargetPosition->GetWorldPos());
					}
					//move to cover
					if (m_findingCoverTimePassed >= m_timeBetweenFindingCover) {
						MoveTo(m_currentCoverPosition);
					}
					m_aiControllerComp->LookAt(m_lastTargetPosition->GetWorldPos());
				}
			}
			else {
				//close attack (kick)
				m_aiControllerComp->LookAt(m_targetEntity->GetWorldPos());
				CloseAttack();
			}
		}

		//check last target pos if target is not visible
		if (m_detectionComp->IsTargetFound() && !m_detectionComp->IsVisible(m_targetEntity) && m_lastTargetPositionTimePassed >= m_timeBetweenCheckLastTargetPosition) {
			//todo : move around
			MoveTo(m_lastTargetPosition->GetWorldPos());
		}

		//reset detection timer if target is visible
		if (m_detectionComp->IsVisible(m_targetEntity)) {
			m_lastTargetPositionTimePassed = 0;
		}
		else {
			//m_findingCoverTimePassed = m_timeBetweenFindingCover - 1.5f;
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
	return m_closeAttackTimePassed >= m_timeBetweenCloseAttacks 
		&& m_hitReactionTimePassed >= m_hitReactionTimer;
}

bool Soldier1Component::CanUseWeapon()
{
	return m_coolDownTimePassed >= m_coolDownTimer 
		&& m_closeAttackTimePassed >= m_timeBetweenCloseAttacks 
		&& m_hitReactionTimePassed >= m_hitReactionTimer;
}

void Soldier1Component::StopMoving()
{
	m_aiControllerComp->GetCharacterController()->ChangeVelocity(ZERO, Cry::DefaultComponents::CCharacterControllerComponent::EChangeVelocityMode::Jump);
	m_aiControllerComp->MoveTo(m_pEntity->GetWorldPos());
}

void Soldier1Component::ReactToHit(IEntity* attacker)
{
	if (!bIsAlive) {
		return;
	}
	if (!m_animationComp) {
		CryLog("Soldier1Component : (ReactToHit) animationComp is null.");
	}

	int32 randomNum = GetRandomInt(1, 2);
	IActionPtr selectedAction;
	if (randomNum == 1) {
		selectedAction = m_reactToHit1Action;
	}else if (randomNum == 2) {
		selectedAction = m_reactToHit2Action;
	}

	m_animationComp->GetActionController()->Flush();
	m_animationComp->QueueCustomFragment(*selectedAction);
	m_activeFragmentId = m_reactToHit1FragmentId;
	m_hitReactionTimePassed = 0;

	if (m_gettingHitSoundTimePassed >= m_timeBetweenPlayingGettingHitSound) {
		//play sound
		int32 randomInt = GetRandomInt(1, 3);
		const char* audioName;
		if (randomInt == 1) {
			audioName = "enemy_get_hit_sound_1";
		}
		else 	if (randomInt == 2) {
			audioName = "enemy_get_hit_sound_2";
		}
		else 	if (randomInt == 3) {
			audioName = "enemy_get_hit_sound_3";
		}
		m_audioComp->ExecuteTrigger(CryAudio::StringToId(audioName));
		m_gettingHitSoundTimePassed = 0;
	}

	//
	m_aiControllerComp->LookAt(attacker->GetWorldPos());
	m_detectionComp->SetDetectionToCatious();
}

void Soldier1Component::SetPatrolPathName(Schematyc::CSharedString patrolPathName)
{
	this->m_patrolPathName = patrolPathName;
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
	if (!m_detectionComp->IsVisible(m_targetEntity)) {
		testMoveToPos = m_pEntity->GetWorldPos();
	}

	if (testMoveToPos == ZERO || m_pEntity->GetWorldPos().GetDistance(testMoveToPos) < 2 ) {
		testMoveToPos = GetRandomPointToMoveTo(m_lastTargetPosition != nullptr ? m_lastTargetPosition->GetWorldPos() : m_pEntity->GetWorldPos(), 8);
	}

	/*
	//testing
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	pd->Begin("testPoint123", true);
	pd->AddSphere(testMoveToPos, 0.5f, ColorF(0, 0, 1), 10);
	*/

	if (m_detectionComp->IsTargetFound()) {
		m_aiControllerComp->MoveTo(testMoveToPos);
		m_aiControllerComp->LookAt(m_lastTargetPosition->GetWorldPos());
	}
	else {
		m_aiControllerComp->MoveToAndLookAtWalkDirection(testMoveToPos);
	}
}


void Soldier1Component::Die()
{
	if (m_healthComp->GetHealth() <= 0 && bIsAlive) {

		/*
		//play dead sound
		m_randomDeadSound = cry_random<int32>(1, 3);
		string idString;
		if (m_randomGetHitSound == 1) {
			idString = "ai_dead_sound_1";
		}
		else if (m_randomGetHitSound == 2) {
			idString = "ai_dead_sound_2";
		}
		else if (m_randomGetHitSound == 3) {
			idString = "ai_dead_sound_3";
		}
	

		CryAudio::ControlId deadSound = CryAudio::StringToId(idString);
		m_audioComp->ExecuteTrigger(deadSound);
		*/

		m_pEntity->RemoveComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
		m_pEntity->RemoveComponent<IEntityNavigationComponent>();
		m_pEntity->RemoveComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();

		m_animationComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
		m_animationComp->SetCharacterFile("Objects/Characters/soldier1/soldier_1.cdf");
		m_animationComp->SetTransformMatrix(Matrix34::Create(Vec3(1), Quat::CreateRotationXYZ(Ang3(0, 0, DEG2RAD(-45))), Vec3(0)));
		m_animationComp->ResetCharacter();

		m_ragdollComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CRagdollComponent>();
		//m_ragdollComp->SetTransformMatrix(Matrix34::Create(Vec3(1), Quat::CreateRotationXYZ(Ang3(0, 0, 179.1)), Vec3(0, 0, 0)));

		//detach gun
		m_weaponBaseEntity->SetPos(m_pEntity->GetWorldPos());
		m_pEntity->DetachAll();

		m_ragdollComp->Enable(true);

		PlatDeathSound();
		bIsAlive = false;
	}
}

void Soldier1Component::PlatDeathSound()
{
	int32 randomInt = GetRandomInt(1, 3);
	const char* audioName;
	if (randomInt == 1) {
		audioName = "ai_dead_sound_1";
	}
	else 	if (randomInt == 2) {
		audioName = "ai_dead_sound_2";
	}
	else 	if (randomInt == 3) {
		audioName = "ai_dead_sound_3";
	}
	m_audioComp->ExecuteTrigger(CryAudio::StringToId(audioName));
}

void Soldier1Component::PlayDetectionSound()
{
	if (m_detectionComp->IsTargetFound() && !bIsDetectionSoundPlayed) {
		int32 randomInt = GetRandomInt(1, 3);
		const char* audioName;
		if (randomInt == 1) {
			audioName = "ai_detection_sound_1";
		}
		else 	if (randomInt == 2) {
			audioName = "ai_detection_sound_2";
		}
		else 	if (randomInt == 3) {
			audioName = "ai_detection_sound_3";
		}
		m_audioComp->ExecuteTrigger(CryAudio::StringToId(audioName));
		bIsDetectionSoundPlayed = true;
		m_coolDownTimePassed = 0;
	}
}
