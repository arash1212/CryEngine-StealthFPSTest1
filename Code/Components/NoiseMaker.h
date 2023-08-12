#pragma once

class NoiseMakerComponent final : public IEntityComponent
{
public:

	NoiseMakerComponent() = default;
	virtual ~NoiseMakerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<NoiseMakerComponent>& desc)
	{
		desc.SetGUID("{AC213592-0AC1-429A-9BC3-24C31C2C4C35}"_cry_guid);
	}

	f32 m_noiseDistance = 20.f;

	IEntity* m_owner;
public:
	void MakeNoise();
	void SetNoiseDistance(f32 noiseDistance);
	void SetOwner(IEntity* owner);
};