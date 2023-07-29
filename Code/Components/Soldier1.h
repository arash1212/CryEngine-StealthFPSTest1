#pragma once

#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class AIControllerComponent;

class Soldier1Component final : public IEntityComponent
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

	AIControllerComponent* m_aiController;
};