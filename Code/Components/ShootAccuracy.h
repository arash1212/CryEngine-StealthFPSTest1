#pragma once

class ActorStateComponent;

class ShootAccuracyComponent final : public IEntityComponent
{
public:

	ShootAccuracyComponent() = default;
	virtual ~ShootAccuracyComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<ShootAccuracyComponent>& desc)
	{
		desc.SetGUID("{6CF0B78B-64A9-4F8C-9DA0-F0FEDB0D5288}"_cry_guid);
	}

private:
	IEntity* m_ownerEntity = nullptr;
	ActorStateComponent* m_stateComp = nullptr;

private :
	f32 m_shootError = 0.f;

private:
	void UpdateShootError(f32 deltatime);

public:
	void AddShootError(f32 amount);
	void SetOwnerEntity(IEntity* ownerEntity);
	void SetStateComponent(ActorStateComponent* stateComp);
	f32 GetShootError();
};