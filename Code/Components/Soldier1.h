#pragma once

#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class AIControllerComponent;
class ActorStateComponent;
class AIDetectionComponent;
class IAIActorComponent;

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

	IEntity* m_testTargetEntity = nullptr;
	Vec3 testMoveToPos = ZERO;
	IEntity* m_targetEntity = nullptr;
	IEntity* m_lastTargetPosition;

private:
	f32 m_walkSpeed = DEFAULT_PLAYER_WALK_SPEED;
	f32 m_runSpeed = DEFAULT_PLAYER_RUN_SPEED;
	f32 m_currentSpeed = DEFAULT_PLAYER_WALK_SPEED;

	//animations
	FragmentID m_idleFragmentId;
	FragmentID m_cautiousFragmentId;
	FragmentID m_combatFragmentId;
	FragmentID m_closeAttackFragmentId;
	FragmentID m_activeFragmentId;

	IActionPtr m_closeAttackAction;
	//attack
	f32 m_maxAttackDistance = 40.f;
	f32 m_closeAttackDistance = 2.f;
	f32 m_timeBetweenCloseAttacks = 0.5f;
	f32 m_closeAttackTimePassed = 0.f;

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
	virtual void StopMoving() override;
};