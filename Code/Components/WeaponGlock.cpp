#include "StdAfx.h"
#include "WeaponGlock.h"
#include "IWeapon.h"
#include "Player.h"
#include "NoiseMaker.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterWeaponGlockComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(WeaponGlockComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterWeaponGlockComponent);
}

void WeaponGlockComponent::Initialize()
{
	m_animationComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	m_animationComp->SetTransformMatrix(Matrix34::Create(m_defaultSize, m_defaultRotation, m_defaultPosition));
	m_animationComp->SetCharacterFile("Objects/Weapons/glock/1p/pistol_glock.cdf");
	m_animationComp->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/glock.adb");
	m_animationComp->SetControllerDefinitionFile("Animations/Mannequin/ADB/WeaponControllerDefinition.xml");
	m_animationComp->SetDefaultScopeContextName("FirstPersonCharacter");
	m_animationComp->SetDefaultFragmentName("Idle");
	m_animationComp->SetAnimationDrivenMotion(true);
	m_animationComp->LoadFromDisk();
	m_animationComp->ResetCharacter();

	//recoil
	m_meshefaultRotaion = m_defaultRotation;
	m_targetRotation = m_defaultRotation;

	//Fragments
	m_idleFragmentId = m_animationComp->GetFragmentId("Idle");
	m_walkFragmentId = m_animationComp->GetFragmentId("Walk");
	m_fireFragmentId = m_animationComp->GetFragmentId("Fire");
	m_runFragmentId = m_animationComp->GetFragmentId("Run");

	m_fireAction = new TAction<SAnimationContext>(30U, m_fireFragmentId);

	m_defaultAnimationCompRotation = m_animationComp->GetTransform().get()->GetRotation().ToQuat();
	m_defaultAnimationCompPosition = m_animationComp->GetTransform().get()->GetTranslation();

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	//Get Muzzle Attachment
	m_muzzleAttachment = m_animationComp->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("muzzle");

	//muzzleflashes
	m_muzzleFlash1Attachment = m_animationComp->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("muzzleflash1");
	m_muzzleFlash1Attachment->HideAttachment(1);
	m_muzzleFlash2Attachment = m_animationComp->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("muzzleflash2");
	m_muzzleFlash2Attachment->HideAttachment(1);
	m_muzzleFlash3Attachment = m_animationComp->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("muzzleflash3");
	m_muzzleFlash3Attachment->HideAttachment(1);

	InitShootSounds();

	//m_muzzleFlashMesh = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CStaticMeshComponent>();
	//m_muzzleFlashMesh->SetTransformMatrix(Matrix34::Create(Vec3(0.1f), Quat::CreateRotationZ(RAD2DEG(90)), m_muzzleAttachment->GetAttAbsoluteDefault().t + m_pEntity->GetForwardDir() * 2.5f));
	//m_muzzleFlashMesh->SetFilePath("Objects/effects/muzzleflash/muzzleflash1.cgf");
	//m_muzzleFlashMesh->LoadFromDisk();
	//m_muzzleFlashMesh->ResetObject();

	//noiseMaker comp
	m_noiseMakerComp = m_pEntity->GetOrCreateComponent<NoiseMakerComponent>();
	m_noiseMakerComp->SetOwner(m_ownerEntity);
}

Cry::Entity::EventFlags WeaponGlockComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void WeaponGlockComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		float deltatime = event.fParam[0];
		UpdateAnimation();
		KickBack();
		Recoil();
		UpdateMuzzleFlashes();
		Aim();

		//apply sway if owner is player
		if (m_ownerEntity && m_ownerEntity->GetComponent<PlayerComponent>()) {
			Vec2 rotationDelta = m_ownerEntity->GetComponent<PlayerComponent>()->GetRotationDelta();
			Sway(rotationDelta.y, rotationDelta.x);
		}

		//timers 
		if (m_shotTimePassed < m_timeBetweenShots) {
			m_shotTimePassed += 0.5f * deltatime;
		}
		if (m_MuzzleFlashDeActivationTimePassed < m_timeBetweenMuzzleFlashDeActivation) {
			m_MuzzleFlashDeActivationTimePassed += 0.5f * deltatime;
		}

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void WeaponGlockComponent::InitShootSounds()
{
	m_shootSounds.Insert(0, CryAudio::StringToId("glock_fire_sound_1"));
}
