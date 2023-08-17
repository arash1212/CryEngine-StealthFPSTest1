#pragma once

class InteractableComponent final : public IEntityComponent
{
public:
	InteractableComponent() = default;
	virtual ~InteractableComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<InteractableComponent>& desc)
	{
		desc.SetGUID("{F5F95219-D7B9-472E-A55B-BE76E37F62AD}"_cry_guid);
	}

private:
	bool bIsUsed = false;

public :
	void Use(bool isUsed);
	bool IsUsed();
};