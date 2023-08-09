#pragma once

#include <DefaultComponents/Geometry/StaticMeshComponent.h>

class AlarmSpeakerComponent final : public IEntityComponent
{
public:
	AlarmSpeakerComponent() = default;
	virtual ~AlarmSpeakerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AlarmSpeakerComponent>& desc)
	{
		desc.SetGUID("{6596A191-FF21-413E-B843-A781078FE3F6}"_cry_guid);
		desc.AddMember(&AlarmSpeakerComponent::bIsEnabled, 'ien', "isEnabled", "Is Enabled", "Set Is Enabled", false);
	}

private:
	Cry::DefaultComponents::CStaticMeshComponent* m_meshComp = nullptr;
	IEntityAudioComponent* m_audioComp = nullptr;


private:
	bool bIsEnabled = false;
	bool bIsSoundPlaying = false;

	CryAudio::ControlId m_alarmSoundId;
private:
	void UpdateAlarm();

public:
	void SetEnabled(bool isEnable);
};