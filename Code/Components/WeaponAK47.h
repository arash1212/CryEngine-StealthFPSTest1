#pragma once

class IWeaponComponent;
class WeaponAK47Component;

class WeaponAK47Component final : public IWeaponComponent
{
public:
	WeaponAK47Component() = default;
	virtual ~WeaponAK47Component() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<WeaponAK47Component>& desc)
	{
		desc.SetGUID("{E9FB6A07-27B4-48E4-8571-D84C68B6D849}"_cry_guid);
	}

protected:
	virtual void InitShootSounds() override;
};