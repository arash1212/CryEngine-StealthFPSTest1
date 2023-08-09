#pragma once

static constexpr f32 MAX_HEALTH = 100.f;

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
		desc.AddMember(&HealthComponent::m_maxHealth, 'mh', "maxhealth", "Max Health", "Set Max Health", MAX_HEALTH);
	}

private:
	f32 m_maxHealth = MAX_HEALTH;
	f32 m_health = m_maxHealth;

public:
	void ApplyDamage(f32 damage);
	f32 GetHealth();
};