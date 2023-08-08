#pragma once

#include <DefaultComponents/Geometry/StaticMeshComponent.h>

class AIDetectionComponent;

class SecurityCameraComponent final : public IEntityComponent
{
public:
	SecurityCameraComponent() = default;
	virtual ~SecurityCameraComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<SecurityCameraComponent>& desc)
	{
		desc.SetGUID("{5F957481-DCED-40E6-BE9D-46A77DBE4356}"_cry_guid);
	}

private:

	Cry::DefaultComponents::CStaticMeshComponent* m_meshComp;
	IEntityAudioComponent* m_audioComp;

	AIDetectionComponent* m_detectionComp;

	IEntity* m_targetEntity;

private:
	f32 m_maxZRotation = 250;
	bool bIsReachedRightEnd = false;
	f32 m_defaultZRotation = ZERO;

	f32 timeElapsed = 0.f;

	//detectioSound
	CryAudio::ControlId m_detectionSound = CryAudio::StringToId("security_camera_sound_1");
	bool bIsDetectionSoundPlayed = false;

	//detection timers
	f32 m_timeBetweenTriggeringAlarams = 1.4f;
	f32 m_triggeringAlaramsTimePassed = 0;
	bool bIsTriggeredAlarams = false;

private:
	void UpdateRotation(f32 DeltaTime);
	void TriggerAlarams();

};