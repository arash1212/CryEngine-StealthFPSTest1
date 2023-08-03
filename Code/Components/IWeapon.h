#pragma once

#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Geometry/StaticMeshComponent.h>

static constexpr f32 WEAPON_DEFAULT_DAMAGE = 20.f;
static constexpr f32 WEAPON_DEFAULT_TIME_BETWEEN_SHOTS = 0.1f;

class IWeaponComponent : public IEntityComponent
{
public:
	IWeaponComponent() = default;
	virtual ~IWeaponComponent() = default;

	virtual void Initialize() override = 0;

	virtual Cry::Entity::EventFlags GetEventMask() const override = 0;
	virtual void ProcessEvent(const SEntityEvent& event) override = 0;

	/*
	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<IWeaponComponent>& desc)
	{
		desc.SetGUID("{30E252EC-C63E-4036-8A44-8D42671BF019}"_cry_guid);
		desc.AddMember(&IWeaponComponent::m_damage, 'wd', "weapondamage", "Weapon Damage", "Set Weapon Damage", WEAPON_DEFAULT_DAMAGE);
		desc.AddMember(&IWeaponComponent::m_timeBetweenShots, 'tbs', "weapontimebetweenshots", "Weapon Time Between Shots", "Set Weapon Time Between Shots", WEAPON_DEFAULT_TIME_BETWEEN_SHOTS);
	}
	*/

protected:
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_animationComp = nullptr;
	Cry::DefaultComponents::CCameraComponent* m_cameraComp = nullptr;
	Cry::DefaultComponents::CCharacterControllerComponent* m_characteControllerrComp = nullptr;
	IEntityAudioComponent* m_audioComp = nullptr;
	IEntity* m_ownerEntity = nullptr;
	IEntity* m_cameraBaseEntity = nullptr;


protected:
	Vec3 m_muzzlePos = ZERO;
	Vec3 m_attackDir = ZERO;

	f32 m_damage = WEAPON_DEFAULT_DAMAGE;

	//timers
	f32 m_timeBetweenShots = WEAPON_DEFAULT_TIME_BETWEEN_SHOTS;
	f32 m_shotTimePassed = 0.f;

	//default & aim position/rations
	Vec3 m_defaultSize = Vec3(1);
	Quat m_defaultRotation = Quat::CreateRotationXYZ(Ang3(0.12f, 0, 160.11f));
	Vec3 m_defaultPosition = Vec3(0, 0, -0.6f);

	//animations
	FragmentID m_walkFragmentId;
	FragmentID m_idleFragmentId;
	FragmentID m_runFragmentId;
	FragmentID m_fireFragmentId;
	FragmentID m_activeFragmentId;

	IActionPtr m_fireAction;

	//sway
	float m_swaySpeed = 1.4f;
	Quat m_defaultAnimationCompRotation = IDENTITY;
	Vec3 m_defaultAnimationCompPosition = ZERO;

	//kickback
	Vec3 m_kickBackAmount = Vec3(0.f, 0.f, 0.f);
	float m_kickBackSpeed = 45.f;

	//sounds
	int32 m_currentShootSoundNumber = 0;
	Schematyc::CArray<CryAudio::ControlId> m_shootSounds;

	//recoil
	f32 m_snapiness = 50.2f;
	f32 m_returnSpeed = 30.1f;
	Quat m_meshefaultRotaion = IDENTITY;
	Quat m_currentRotation = IDENTITY;
	Quat m_targetRotation = IDENTITY;
	int32 recoilInv = 1;

	//shootError
	f32 m_shootError = 0.01f;

	//Attachments
	IAttachment* m_muzzleAttachment;

	//muzzleflash
	f32 m_timeBetweenMuzzleFlashDeActivation = 0.04f;
	f32 m_MuzzleFlashDeActivationTimePassed = 0.f;
	IAttachment* m_muzzleFlash1Attachment;
	IAttachment* m_muzzleFlash2Attachment;
	IAttachment* m_muzzleFlash3Attachment;
	bool bCanChangeMuzzleFlash = false;

	bool bIsInitMeshDone = false;

	//aiming
	bool bIsAiming = false;
	Vec3 m_aimPosition = Vec3(-0.842f, -0.05f, -0.09f);
	Quat m_aimRotation = Quat::CreateRotationZ(160.234f);

protected:
	f32 GetRandomValue(f32 min, f32 max);
	int32 GetRandomInt(int32 min, int32 max);
	virtual void UpdateAnimation();
	virtual void KickBack();
	virtual void Recoil();
	virtual f32 GetShootError(f32 error);
	virtual void SpawnBulletTracer(Vec3 error, Vec3 pos, Quat dir);
	virtual void UpdateMuzzleFlashes();
	virtual void Aim();
	virtual CryAudio::ControlId GetRandomShootSound();
	virtual void InitShootSounds() = 0;

public :
	IEntity* Raycast(Vec3 from, Vec3 to, Vec3 error);
	virtual bool Fire();
	virtual void Sway(f32 mouseX, f32 mouseY);

	virtual void AddKickBack(Vec3 amount);

	//Getter & Setters

	void SetCameraComponent(Cry::DefaultComponents::CCameraComponent* cameraComp);
	void SetMuzzlePos(Vec3 muzzlePos);
	void SetOwnerEntity(IEntity* ownerEntity);
	void SetCharacterController(Cry::DefaultComponents::CCharacterControllerComponent* characterControllerComp);
	void SetCameraBaseEntity(IEntity* cameraBaseEntity);
	void SetIsAiming(bool isAiming);
	f32 GetDamage();
	f32 GetShootError();
	virtual Vec3 GetCameraRecoilAmount();
	virtual Vec3 GetMeshRecoilAmount();
	virtual void ResetShootSoundNumber();
};