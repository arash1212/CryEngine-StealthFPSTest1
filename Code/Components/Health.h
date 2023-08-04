#pragma once

class HealthComponent final : public IEntityComponent
{
public:
	HealthComponent() = default;
	virtual ~HealthComponent() = default;

	virtual void Initialize() override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<HealthComponent>& desc)
	{
		desc.SetGUID("{32474862-E055-4F79-BC5D-9397E14C8D76}"_cry_guid);
	}

private:
	f32 m_maxHealth = 100.f;
	f32 m_health = m_maxHealth;

public:
	void ApplyDamage(f32 damage);
	f32 GetHealth();
};