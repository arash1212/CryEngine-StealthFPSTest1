#pragma once

class DestroyTimerComponent final : public IEntityComponent
{
public:
	DestroyTimerComponent() = default;
	virtual ~DestroyTimerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<DestroyTimerComponent>& desc)
	{
		desc.SetGUID("{85BC8335-CF0E-446F-9183-CF00C84D9CF8}"_cry_guid);
	}

private:
	f32 m_maxTimeAlive = 3.f;
	f32 m_timePassed = 0.f;

private:
	void DestroyEntity(f32 deltaTime);

public:
	void SetMaxTimeAlive(f32 maxTimeAlive);
};