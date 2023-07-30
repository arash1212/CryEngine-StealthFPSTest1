#pragma once

class AIDetectionComponent final : public IEntityComponent
{
public:
	AIDetectionComponent() = default;
	virtual ~AIDetectionComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AIDetectionComponent>& desc)
	{
		desc.SetGUID("{7E235301-7B22-4B96-981B-8A4A5A0C7828}"_cry_guid);
	}

private:

public:
	bool IsInView(IEntity* target);
	bool IsVisible(IEntity* target);
};