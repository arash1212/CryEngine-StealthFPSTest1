#pragma once

class InteractableComponent;

#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include <DefaultComponents/Physics/BoxPrimitiveComponent.h>

class AlarmButtonComponent final : public IEntityComponent
{
public:
	AlarmButtonComponent() = default;
	virtual ~AlarmButtonComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AlarmButtonComponent>& desc)
	{
		desc.SetGUID("{98C10821-B954-4E4E-B916-736DA3D5B27D}"_cry_guid);
	}

private :
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_animationComp = nullptr;
	Cry::DefaultComponents::CBoxPrimitiveComponent* m_boxComp = nullptr;
	IEntityAudioComponent* m_audioComp = nullptr;
	InteractableComponent* m_interactableComp = nullptr;

	IEntity* m_alaramManager = nullptr;

	CryAudio::ControlId m_pushButtonSound;

	FragmentID m_IdleFragmentId;
	FragmentID m_useFragmentId;
	FragmentID m_activeFragmentId;

	IActionPtr m_useAction;

private:
	void UpdateAnimation();
	void TriggerAlarams();
};