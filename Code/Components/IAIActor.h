#pragma once

class IAIActorComponent : public IEntityComponent
{
public:
	IAIActorComponent() = default;
	virtual ~IAIActorComponent() = default;

	virtual void Initialize() override = 0;

protected:
	virtual void UpdateAnimation() = 0;
	virtual void MoveTo(Vec3 pos) = 0;
	virtual void Attack() = 0;
	virtual void CloseAttack() = 0;
	virtual void MoveAroundTarget(IEntity* target) = 0;
	virtual void UpdateCurrentSpeed() = 0;
	virtual void UpdateLastTargetPositionEntity() = 0;
	virtual bool CanMove() = 0;
	virtual bool CanUseWeapon() = 0;
	virtual void StopMoving() = 0;
	virtual void Die() = 0;
	f32 GetRandomValue(f32 min, f32 max);
	int32 GetRandomInt(int32 min, int32 max);
public:
	virtual void ReactToHit(IEntity* attacker) = 0;
};