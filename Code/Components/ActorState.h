#pragma once

#include <DefaultComponents/Physics/CharacterControllerComponent.h>

enum class EActorState {
	IDLE,
	WALKING,
	RUNNING,
	FALLING
};

class ActorStateComponent final : public IEntityComponent
{
public:

	ActorStateComponent() = default;
	virtual ~ActorStateComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<ActorStateComponent>& desc)
	{
		desc.SetGUID("{E8C2EB53-CD41-4F8E-B635-770C53567F0B}"_cry_guid);
	}

private :
	Cry::DefaultComponents::CCharacterControllerComponent* m_characterController = nullptr;
	EActorState m_state;

private:

	//Speed
	f32 m_walkSpeed = 0.f;
	f32 m_runSpeed = 0.f;
	f32 m_currentSpeed = 0.f;

private :
	void UpdateState();

public :
	void SetCharacterController(Cry::DefaultComponents::CCharacterControllerComponent* characterController);
	EActorState GetState();

	//Getter and setter

	void SetWalkSpeed(f32& walkSpeed);
	void SetRunSpeed(f32& runSpeed);
	void SetCurrentSpeed(f32& currentSpeed);
};