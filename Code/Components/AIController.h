#pragma once

class Soldier1Component;

#include <CryAISystem/IPathfinder.h>
#include <CryAISystem/INavigationSystem.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>

class ActorStateComponent;

class AIControllerComponent final : public IEntityComponent
{
public:
	AIControllerComponent() = default;
	virtual ~AIControllerComponent() = default;

	virtual void Initialize() override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AIControllerComponent>& desc)
	{
		desc.SetGUID("{3A6FA260-D52B-4EE4-AC56-62546D134350}"_cry_guid);
	}

private:

	Cry::DefaultComponents::CCharacterControllerComponent* m_characterControllerComp = nullptr;
	struct IEntityNavigationComponent* m_navigationComp = nullptr;

	ActorStateComponent* m_stateComp;

private :
	Vec3 m_currentPatrolPoint = ZERO;
	f32 m_patrolProgress = 0.f;
	bool bIsAtEnd = false;

protected:
	f32 GetRandomFloat(f32 min, f32 max);
	int32 GetRandomInt(int32 min, int32 max);
	Vec3 GetRandomPointInsideTriangle(Triangle t);
	INavPath* path;

public:
	
	Vec3 GetVelocity();
	bool IsWalking();
	bool IsOnGround();
	Cry::DefaultComponents::CCharacterControllerComponent* GetCharacterController();

	void LookAt(Vec3 position);
	void NavigateTo(Vec3 position);
	void MoveTo(Vec3 position);
	void MoveToAndLookAtWalkDirection(Vec3 position);
	Vec3 GetRandomPointOnNavmesh(float MaxDistance, IEntity* Around);
	bool IsPointVisibleFrom(NavigationAgentTypeID agentTypeId, Vec3 from, Vec3 endPos);
	void SetActorStateComponent(ActorStateComponent* stateComp);
	Vec3 Patrol(Schematyc::CSharedString pathName);
	Vec3 FindCover(IEntity* target);
	bool IsCoverPointSafe(Vec3 point, IEntity* target);
	bool IsCoverUsable(Vec3 point, IEntity* target);
	bool isCoverAvailable(Vec3 point);
	Vec3 snapToNavmesh(Vec3 point);
	void SortLocationsByDistance(std::array<Vec3, 400> &locations, int32 size);
};