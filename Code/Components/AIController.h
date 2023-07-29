#pragma once

#include <DefaultComponents/Physics/CharacterControllerComponent.h>

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

public:
	
	Vec3 GetVelocity();
	bool IsWalking();
	bool IsOnGround();
	Cry::DefaultComponents::CCharacterControllerComponent* GetCharacterController();

	void LookAt(Vec3 position);
	void NavigateTo(Vec3 position);
	void MoveTo(Vec3 position);
	void MoveToAndLookAtWalkDirection(Vec3 position);
	Vec3 GetRandomPointOnNavmesh(float MaxDistance, Vec3 Around);
};