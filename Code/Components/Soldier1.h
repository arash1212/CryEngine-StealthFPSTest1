#pragma once

//#include "IAIActor.h"
#include <DefaultComponents/Physics/RagdollComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class AIControllerComponent;
class ActorStateComponent;
class AIDetectionComponent;
class IWeaponComponent;
class HealthComponent;

static constexpr f32 DEFAULT_SOLDIER_1_WALK_SPEED = 2.f;
static constexpr f32 DEFAULT_SOLDIER_1_RUN_SPEED = 3.1f;

class Soldier1Component final : public IAIActorComponent
{
public:
	Soldier1Component() = default;
	virtual ~Soldier1Component() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<Soldier1Component>& desc)
	{
		desc.SetGUID("{0FD34D98-C365-4D08-9858-3EB9529D2B64}"_cry_guid);
	}

private:

	Cry::DefaultComponents::CAdvancedAnimationComponent* m_animationComp = nullptr;

	AIControllerComponent* m_aiControllerComp = nullptr;
	ActorStateComponent* m_stateComp = nullptr;
	AIDetectionComponent* m_detectionComp = nullptr;
	ShootAccuracyComponent* m_shootAccuracyComp;

	IEntity* m_testTargetEntity = nullptr;
	Vec3 testMoveToPos = ZERO;
	IEntity* m_targetEntity = nullptr;
	IEntity* m_lastTargetPosition;

	//weapons
	IWeaponComponent* m_currentlySelectedWeapon;
	IWeaponComponent* m_primaryWeapon;

	IEntity* m_weaponBaseEntity = nullptr;
	bool bIsWeaponInitDone = false;

	IAttachment* m_gunAttachment;

	//health
	HealthComponent* m_healthComp;

	//audio
	IEntityAudioComponent* m_audioComp;

	//ragdol
	Cry::DefaultComponents::CRagdollComponent* m_ragdollComp;

private:
	bool bIsGameplayStarted = false;

	f32 m_walkSpeed = DEFAULT_SOLDIER_1_WALK_SPEED;
	f32 m_runSpeed = DEFAULT_SOLDIER_1_RUN_SPEED;
	f32 m_currentSpeed = DEFAULT_SOLDIER_1_WALK_SPEED;

	//animations
	FragmentID m_idleFragmentId;
	FragmentID m_cautiousFragmentId;
	FragmentID m_combatFragmentId;
	FragmentID m_closeAttackFragmentId;
	FragmentID m_activeFragmentId;
	FragmentID m_reactToHit1FragmentId;
	FragmentID m_reactToHit2FragmentId;

	IActionPtr m_closeAttackAction;
	IActionPtr m_reactToHit1Action;
	IActionPtr m_reactToHit2Action;
	//attack
	f32 m_maxAttackDistance = 40.f;
	f32 m_closeAttackDistance = 2.f;
	f32 m_timeBetweenCloseAttacks = 0.5f;
	f32 m_closeAttackTimePassed = 0.f;

	//patrol
	Schematyc::CSharedString m_patrolPathName = "aiPath-1";

	//cover
	Vec3 m_currentCoverPosition = ZERO;

	//shoot coolDown
	int32 m_currentShootCount = 0;
	int32 m_shootBeforeCoolDown = 13;
	f32 m_coolDownTimer = 0.4f;
	f32 m_coolDownTimePassed = 0.f;

	//hit reaction 
	f32 m_hitReactionTimer = 0.3f;
	f32 m_hitReactionTimePassed = 0.f;
	f32 m_timeBetweenPlayingGettingHitSound = 0.4f;
	f32 m_gettingHitSoundTimePassed = 0.4f;

	bool bIsAlive = true;

private:
	void InitLastTargetPositionEntity();
	Vec3 GetRandomPointToMoveTo(Vec3 Around, f32 distance);

protected:
	virtual void UpdateAnimation() override;
	virtual void MoveTo(Vec3 pos)  override;
	virtual void MoveAroundTarget(IEntity* target) override;
	virtual void UpdateCurrentSpeed()  override;
	virtual void UpdateLastTargetPositionEntity() override;
	virtual void Attack() override;
	virtual void CloseAttack() override;
	virtual bool CanMove() override;
	virtual bool CanUseWeapon() override;
	virtual void StopMoving() override;
	virtual void Die() override;

public:
	void SetPatrolPathName(Schematyc::CSharedString patrolPathName);
	virtual void ReactToHit() override;
};