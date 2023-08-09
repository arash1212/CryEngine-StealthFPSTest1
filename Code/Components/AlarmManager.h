#pragma once

class AlarmManagerComponent final : public IEntityComponent
{
public:
	AlarmManagerComponent() = default;
	virtual ~AlarmManagerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AlarmManagerComponent>& desc)
	{
		desc.SetGUID("{56CAC271-3562-4B69-8A7E-F712234C3A51}"_cry_guid);
		desc.AddMember(&AlarmManagerComponent::bIsAlarmEnabled, 'aien', "isEnabled", "Is Alarm Enabled", "Set Is Alarm Enabled", false);
	}

private:
	bool bIsAlarmEnabled = false;

	Schematyc::CArray<IEntity*> m_speakerEntities;

private:
	void FillSpeakers();

public:
	void SetEnabled(bool isEnable);
};