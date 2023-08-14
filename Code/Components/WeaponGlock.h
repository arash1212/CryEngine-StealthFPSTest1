#pragma once

class IWeaponComponent;

class WeaponGlockComponent final : public IWeaponComponent
{
public:
	WeaponGlockComponent() = default;
	virtual ~WeaponGlockComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<WeaponGlockComponent>& desc)
	{
		desc.SetGUID("{F97248EA-84DE-4D3C-B1C7-D24425DD3CC8}"_cry_guid);
	}

protected:
	virtual void InitShootSounds() override;
public:
	virtual string GetAttachmentName() override;
};