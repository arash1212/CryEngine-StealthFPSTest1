#pragma once

#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Physics/CapsulePrimitiveComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>

class ActorStateComponent;
class IWeaponComponent;
class CrosshairComponent;
class ShootAccuracyComponent;
class WeaponGlockComponent;
class HealthComponent;
class ActorInfoComponent;

struct IUIElement;

static constexpr f32 DEFAULT_PLAYER_WALK_SPEED = 4.f;
static constexpr f32 DEFAULT_PLAYER_RUN_SPEED = 6.f;
static constexpr f32 DEFAULT_PLAYER_ROTATION_SPEED = 0.001f;
static constexpr f32 DEFAULT_PLAYER_JUMP_FORCE = 6.f;

static string HEALTHBAR_UI_ELEMENT_NAME = "healthbar";

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
	Cry::Audio::DefaultComponents::CListenerComponent* m_listenerComp = nullptr;

	//audio
	IEntityAudioComponent* m_audioComp;

	//weapons
	IWeaponComponent* m_currentlySelectedWeapon;
	IWeaponComponent* m_primaryWeapon;
	ShootAccuracyComponent* m_shootAccuracyComp;

	//State
	ActorStateComponent* m_stateComp;

	//crosshair
	CrosshairComponent* m_crosshairComp;

	//health
	HealthComponent* m_health;
	IUIElement* m_healthbarUIElement = nullptr;

	//info
	ActorInfoComponent* m_info;

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
	bool bCanStand = false;

	//crouch
	bool bIsCrouching = false;
	f32 m_capsuleNormalHeight = ZERO;

	//rotation (camera/body)
	f32 m_rotationX = 0.f;
	f32 m_rotationY = 0.f;
	f32 m_minRoationX = -1300;
	f32 m_maxRoationX = 1300;

	//recoil
	f32 m_snapiness = 13.2f;
	f32 m_returnSpeed = 5.1f;
	Quat m_cameraDefaultRotaion = IDENTITY;
	Quat m_currentRotation = IDENTITY;
	Quat m_targetRotation = IDENTITY;

	//aiming
	bool bIsAiming = false;
	float m_deafultFov = 70.f;
	float m_currentFov = m_deafultFov;
	float m_fovChangeSpeed = 20.f;

	//health
	f32 m_timeBetweenPlayingGettingHitSound = 0.4f;
	f32 m_gettingHitSoundTimePassed = 0.4f;


private:
	//
	f32 GetRandomValue(f32 min, f32 max);
	int32 GetRandomInt(int32 min, int32 max);

	//inits
	void InitCamera();
	void InitInputs();

	//movement
	void Move();
	void Rotate();
	void RotateCamera();
	void HeadBob(float deltatime);
	bool CanStand();

	void RecoilUpdate();

	void UpdateCrosshair();

	void UpdateFOV();

	void UpdateCrouch(Quat Rotation);

	//healthbar
	void HideHealthbar();
	void ShowHealthbar();
	void UpdateHealthbar();

public:
	Vec2 GetRotationDelta();
	void AddRecoil(Vec3 Amount);
	bool IsCrouching();
	Cry::DefaultComponents::CCharacterControllerComponent* GetCharacterController();

	void ReactToHit(IEntity* attacker);
};
