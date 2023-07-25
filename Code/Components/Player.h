#pragma once

#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Physics/CapsulePrimitiveComponent.h>

class ActorStateComponent;
class IWeaponComponent;

static constexpr f32 DEFAULT_PLAYER_WALK_SPEED = 4.f;
static constexpr f32 DEFAULT_PLAYER_RUN_SPEED = 6.f;
static constexpr f32 DEFAULT_PLAYER_ROTATION_SPEED = 0.001f;
static constexpr f32 DEFAULT_PLAYER_JUMP_FORCE = 6.f;

////////////////////////////////////////////////////////
// Represents a player participating in gameplay
////////////////////////////////////////////////////////
class PlayerComponent final : public IEntityComponent
{
public:

	PlayerComponent() = default;
	virtual ~PlayerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	
	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<PlayerComponent>& desc)
	{
		desc.SetGUID("{63F4C0C6-32AF-4ACB-8FB0-57D45DD14725}"_cry_guid);
		desc.AddMember(&PlayerComponent::m_walkSpeed, 'pws', "playerwalkspeed", "Player Walk Speed", "Set Player Walk Speed", DEFAULT_PLAYER_WALK_SPEED);
		desc.AddMember(&PlayerComponent::m_runSpeed, 'prs', "playerrunspeed", "Player Run Speed", "Set Player Run Speed", DEFAULT_PLAYER_RUN_SPEED);
		desc.AddMember(&PlayerComponent::m_rotationSpeed, 'prs', "playerrotationspeed", "Player Rotation Speed", "Set Player Rotation Speed", DEFAULT_PLAYER_ROTATION_SPEED);
		desc.AddMember(&PlayerComponent::m_jumpForce, 'pjf', "playerjumpforce", "Player Jump Force", "Set Player Jump Force", DEFAULT_PLAYER_JUMP_FORCE);
	}

private:
	IEntity* m_cameraRoot = nullptr;
	IEntity* m_cameraBase = nullptr;

	Cry::DefaultComponents::CCameraComponent* m_cameraComp = nullptr;
	Cry::DefaultComponents::CCharacterControllerComponent* m_characterControllerComp = nullptr;
	Cry::DefaultComponents::CInputComponent* m_inputComp = nullptr;
	Cry::DefaultComponents::CCapsulePrimitiveComponent* m_capsuleComp = nullptr;


	//weapons
	IWeaponComponent* m_currentlySelectedWeapon;
	IWeaponComponent* m_primaryWeapon;

	//State
	ActorStateComponent* m_stateComp;

private:
	//movement infos
	Vec2 m_movementOffset = ZERO;
	Vec2 m_rotationDelta = ZERO;
	f32 m_walkSpeed = DEFAULT_PLAYER_WALK_SPEED;
	f32 m_runSpeed = DEFAULT_PLAYER_RUN_SPEED;
	f32 m_currentSpeed = DEFAULT_PLAYER_WALK_SPEED;
	f32 m_rotationSpeed = DEFAULT_PLAYER_ROTATION_SPEED;
	f32 m_jumpForce = DEFAULT_PLAYER_JUMP_FORCE;
	f32 m_headBobTimer = 0.f;
	f32 m_defaultPosZ = 0;

	//
	f32 m_rotationX = 0.f;
	f32 m_rotationY = 0.f;
	f32 m_minRoationX = -1000;
	f32 m_maxRoationX = 1000;

	//recoil
	f32 m_snapiness = 15.2f;
	f32 m_returnSpeed = 5.1f;
	Quat m_cameraDefaultRotaion = IDENTITY;
	Quat m_currentRotation = IDENTITY;
	Quat m_targetRotation = IDENTITY;

private:
	//inits
	void InitCamera();
	void InitInputs();

	//movement
	void Move();
	void Rotate();
	void RotateCamera();
	void HeadBob(float deltatime);

	void RecoilUpdate();

	void UpdateState();

public:
	Vec2 GetRotationDelta();
	void AddRecoil(Vec3 Amount);

};
