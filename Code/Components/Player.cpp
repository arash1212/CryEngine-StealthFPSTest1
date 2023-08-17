#include "StdAfx.h"
#include "Player.h"
#include "IWeapon.h"
#include "WeaponGlock.h"
#include "Crosshair.h"
#include "SpawnPoint.h"
#include "Health.h"
#include "ShootAccuracy.h"
#include "ActorInfo.h"
#include "Interactable.h"
#include "GamePlugin.h"

#include <FlashUI/FlashUIElement.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterPlayerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(PlayerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPlayerComponent);
}

void PlayerComponent::Initialize()
{
	m_characterControllerComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
	m_characterControllerComp->SetTransformMatrix(Matrix34::Create(Vec3(1, 1, 2.7f), IDENTITY, Vec3(0, 0, 1.f)));
	m_characterControllerComp->Physicalize();

	m_capsuleComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCapsulePrimitiveComponent>();
	m_capsuleComp->SetTransformMatrix(Matrix34::Create(Vec3(0.99f, 1.2f, 1.2f), IDENTITY, Vec3(0, 0, 1.2f)));

	m_capsuleNormalHeight = m_capsuleComp->m_height;

	m_inputComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();

	m_listenerComp = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();

	InitCamera();
	InitInputs();

	//primary weapon init
	m_primaryWeapon = m_cameraBase->GetOrCreateComponent<WeaponGlockComponent>();
	m_primaryWeapon->SetCharacterController(m_characterControllerComp);
	m_primaryWeapon->SetCameraComponent(m_cameraComp);
	m_primaryWeapon->SetOwnerEntity(m_pEntity);
	m_primaryWeapon->SetCameraBaseEntity(m_cameraBase);

	//todo
	m_currentlySelectedWeapon = m_primaryWeapon;

	//state
	m_stateComp = m_pEntity->GetOrCreateComponent<ActorStateComponent>();
	m_stateComp->SetCharacterController(m_characterControllerComp);
	m_stateComp->SetWalkSpeed(m_walkSpeed);
	m_stateComp->SetRunSpeed(m_runSpeed);
	m_stateComp->SetCurrentSpeed(m_currentSpeed);

	//crosshair
	m_crosshairComp = m_pEntity->GetOrCreateComponent<CrosshairComponent>();

	//shootError
	m_shootAccuracyComp = m_pEntity->GetOrCreateComponent<ShootAccuracyComponent>();
	m_shootAccuracyComp->SetOwnerEntity(m_pEntity);
	m_shootAccuracyComp->SetStateComponent(m_stateComp);

	//health
	m_health = m_pEntity->GetOrCreateComponent<HealthComponent>();
	m_healthbarUIElement = gEnv->pFlashUI->GetUIElement(HEALTHBAR_UI_ELEMENT_NAME);
	ShowHealthbar();

	m_audioComp = m_pEntity->GetOrCreateComponent<IEntityAudioComponent>();

	//info
	m_info = m_pEntity->GetOrCreateComponent<ActorInfoComponent>();
	m_info->SetFaction(EFaction::FACTION2);

	//Set player Entity name.
	m_pEntity->SetName("playerEntity");
}

Cry::Entity::EventFlags PlayerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize|
		Cry::Entity::EEvent::GameplayStarted|
		Cry::Entity::EEvent::Update|
		Cry::Entity::EEvent::Reset;
}

void PlayerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltatime = event.fParam[0];

		Move();
		Rotate();
		RotateCamera();
		HeadBob(deltatime);

		RecoilUpdate();

		UpdateCrosshair();

		UpdateFOV();

		UpdateHealthbar();

		InteractableCheckRayast();

		bCanStand = CanStand();

		//timers 
		if (m_gettingHitSoundTimePassed < m_timeBetweenPlayingGettingHitSound) {
			m_gettingHitSoundTimePassed += 0.5f * deltatime;
		}
	}break;
	case Cry::Entity::EEvent::Reset: {
		m_movementOffset = ZERO;
		m_rotationDelta = ZERO;
		m_currentSpeed = DEFAULT_PLAYER_WALK_SPEED;

	}break;
	default:
		break;
	}
}

void PlayerComponent::InitCamera()
{
	//Spawn m_cameraRoot Entity
	SEntitySpawnParams cameraRootSpawnParams;
	cameraRootSpawnParams.vPosition = Vec3(0);
	m_cameraRoot = gEnv->pEntitySystem->SpawnEntity(cameraRootSpawnParams);
	m_cameraRoot->SetLocalTM(Matrix34::Create(Vec3(1), IDENTITY, Vec3(0, 0, 1.9f)));

	//Spawn m_cameraBase Entity
	SEntitySpawnParams cameraBaseSpawnParams;
	cameraBaseSpawnParams.vPosition = Vec3(0);
	m_cameraBase = gEnv->pEntitySystem->SpawnEntity(cameraBaseSpawnParams);

	//Attach spawned entities to player
	m_pEntity->AttachChild(m_cameraRoot);
	m_cameraRoot->AttachChild(m_cameraBase);


	m_cameraComp = m_cameraBase->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();

	m_defaultPosZ = m_cameraRoot->GetPos().z;
}

void PlayerComponent::InitInputs()
{
	m_inputComp->RegisterAction("player", "forward", [this](int activationMode, float value) {m_movementOffset.y = value; });
	m_inputComp->BindAction("player", "forward", eAID_KeyboardMouse, eKI_W);

	m_inputComp->RegisterAction("player", "backward", [this](int activationMode, float value) {m_movementOffset.y = -value; });
	m_inputComp->BindAction("player", "backward", eAID_KeyboardMouse, eKI_S);

	m_inputComp->RegisterAction("player", "left", [this](int activationMode, float value) {m_movementOffset.x = -value; });
	m_inputComp->BindAction("player", "left", eAID_KeyboardMouse, eKI_A);

	m_inputComp->RegisterAction("player", "right", [this](int activationMode, float value) {m_movementOffset.x = value; });
	m_inputComp->BindAction("player", "right", eAID_KeyboardMouse, eKI_D);

	m_inputComp->RegisterAction("player", "mouseX", [this](int activationMode, float value)
		{
			m_rotationDelta.x = -value;
		}
	);
	m_inputComp->BindAction("player", "mouseX", eAID_KeyboardMouse, eKI_MouseX);

	m_inputComp->RegisterAction("player", "mouseY", [this](int activationMode, float value)
		{
			m_rotationDelta.y = -value;
		}
	);
	m_inputComp->BindAction("player", "mouseY", eAID_KeyboardMouse, eKI_MouseY);

	m_inputComp->RegisterAction("player", "jump", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnPress && m_characterControllerComp->IsOnGround()) {
				m_characterControllerComp->AddVelocity(Vec3(0, 0, m_jumpForce));
			}
		}
	);
	m_inputComp->BindAction("player", "jump", eAID_KeyboardMouse, eKI_Space);

	m_inputComp->RegisterAction("player", "fire", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnHold) {
				m_currentlySelectedWeapon->Fire(nullptr);
			}
		}
	);
	m_inputComp->BindAction("player", "fire", eAID_KeyboardMouse, eKI_Mouse1);

	m_inputComp->RegisterAction("player", "run", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnHold && m_characterControllerComp->IsWalking()) {
				m_currentSpeed = m_runSpeed;
				m_stateComp->SetCurrentSpeed(m_currentSpeed);
			}
			else {
				m_currentSpeed = m_walkSpeed;
				m_stateComp->SetCurrentSpeed(m_currentSpeed);
			}
		}
	);
	m_inputComp->BindAction("player", "run", eAID_KeyboardMouse, eKI_LShift);


	m_inputComp->RegisterAction("player", "aim", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnPress) {
				bIsAiming = true;
				m_currentlySelectedWeapon->SetIsAiming(true);
			}
			else if (activationMode == eAAM_OnRelease) {
				bIsAiming = false;
				m_currentlySelectedWeapon->SetIsAiming(false);
			}
		}
	);
	m_inputComp->BindAction("player", "aim", eAID_KeyboardMouse, eKI_Mouse2);

	m_inputComp->RegisterAction("player", "crouch", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnHold) {
				bIsCrouching = true;
			}
			else {
				bIsCrouching = false;
			}
		}
	);
	m_inputComp->BindAction("player", "crouch", eAID_KeyboardMouse, eKI_LCtrl);

	m_inputComp->RegisterAction("player", "interact", [this](int activationMode, float value)
		{
			if (activationMode == eAAM_OnPress) {
				if (m_currentInteractbleEntity) {
					CryLog("used");
					m_currentInteractbleEntity->GetComponent<InteractableComponent>()->Use(true);
				}
			}
		}
	);
	m_inputComp->BindAction("player", "interact", eAID_KeyboardMouse, eKI_E);
}

void PlayerComponent::Move()
{
	Vec3 velocity = Vec3(m_movementOffset.x, m_movementOffset.y, 0);
	velocity.NormalizeSafe();
	m_characterControllerComp->SetVelocity(m_pEntity->GetRotation() * velocity * m_currentSpeed);
}

void PlayerComponent::Rotate()
{
	m_rotationX += m_rotationDelta.x;
	Quat rotation = Quat::CreateRotationZ(m_rotationX * m_rotationSpeed);
	m_pEntity->SetRotation(rotation);
}

void PlayerComponent::RotateCamera()
{
	m_rotationY = CLAMP(m_rotationY += m_rotationDelta.y, m_minRoationX, m_maxRoationX);
	Quat rotation = Quat::CreateRotationX(m_rotationY * m_rotationSpeed);
	m_cameraRoot->SetRotation(rotation);

	//crouch
	UpdateCrouch(rotation);
}

void PlayerComponent::HeadBob(float deltatime)
{
	if (bIsCrouching || !bCanStand) {
		return;
	}

	//apply headbob if player is moving
	if (m_characterControllerComp->IsWalking()) {
		m_headBobTimer += deltatime * m_currentSpeed * 3;
		Vec3 currentPos = m_cameraBase->GetPos();
		m_cameraRoot->SetPos(Vec3(currentPos.x, currentPos.y, m_defaultPosZ + crymath::sin(m_headBobTimer) / 10.f));
	}
	//else return cameraRoot to default position
	else {
		m_headBobTimer = 0;
		Vec3 currentPos = m_cameraBase->GetPos();
		currentPos.z = m_defaultPosZ;
		m_cameraRoot->SetPos(Vec3::CreateLerp(m_cameraRoot->GetPos(), currentPos, 0.1f * 20 * deltatime));
	}
}

bool PlayerComponent::CanStand()
{
	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();

	Vec3 currentPos = m_pEntity->GetPos();
	Vec3 origin = Vec3(currentPos.x, currentPos.y, currentPos.z + 0.1f);
	Vec3 dir = m_pEntity->GetUpDir();

	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * 2, ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {
			//Debug
			if (pd) {
				pd->Begin("RaycastPlayerStand", true);
				//pd->AddText3D(hits[0].pt, 500, ColorF(0, 1, 0), 2, "Hit : %s", hitEntity->GetName());
				pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 0, 0), 2);
			}

			return false;
		}
	}
	return true;
}

void PlayerComponent::RecoilUpdate()
{
	m_targetRotation = Quat::CreateSlerp(m_targetRotation, m_cameraDefaultRotaion, m_returnSpeed * gEnv->pTimer->GetFrameTime());
	m_currentRotation = Quat::CreateSlerp(m_currentRotation, m_targetRotation, m_snapiness * gEnv->pTimer->GetFrameTime());
	m_cameraBase->SetRotation(m_currentRotation);
}

void PlayerComponent::UpdateCrosshair()
{
	if (!m_crosshairComp) {
		return;
	}
	if (!m_shootAccuracyComp) {
		return;
	}


	//spread crosshair if m_shootError is bigger than zero
	if (m_shootAccuracyComp->GetShootError() > 0) {
		m_crosshairComp->Spread();
	}
	//return back crosshair to normal if m_shootError is zero
	else {
		m_crosshairComp->BackToNormal();
	}

	//show / hide crosshair if play is aiming
	if (bIsAiming) {
		m_crosshairComp->Hide();
	}
	else {
		m_crosshairComp->Show();
	}
}

Vec2 PlayerComponent::GetRotationDelta()
{
	return m_rotationDelta;
}

void PlayerComponent::AddRecoil(Vec3 Amount)
{
	m_targetRotation += Quat::CreateRotationXYZ(Amount);
}

bool PlayerComponent::IsCrouching()
{
	return !bCanStand | bIsCrouching;
}

Cry::DefaultComponents::CCharacterControllerComponent* PlayerComponent::GetCharacterController()
{
	return m_characterControllerComp;
}

void PlayerComponent::UpdateFOV()
{
	if (bIsAiming) {
		//TODO 5 => az weawpon begire
		m_currentFov = CLAMP(m_currentFov - m_fovChangeSpeed * gEnv->pTimer->GetFrameTime(), m_deafultFov - 5, m_deafultFov);
	}
	else {
		m_currentFov = CLAMP(m_currentFov + m_fovChangeSpeed * gEnv->pTimer->GetFrameTime(), 0, m_deafultFov);
	}

	//perfom update 
	m_cameraComp->SetFieldOfView(CryTransform::CAngle::FromDegrees(m_currentFov));
}

void PlayerComponent::UpdateCrouch(Quat Rotation)
{
	if (bIsCrouching) {
		m_cameraRoot->SetLocalTM(Matrix34::Create(Vec3(1), Rotation, Vec3::CreateLerp(m_cameraRoot->GetLocalTM().GetTranslation(), Vec3(0, 0, 1.f), 0.5f * 4 * gEnv->pTimer->GetFrameTime())));
		m_capsuleComp->m_height = m_capsuleNormalHeight / 12;
		m_capsuleComp->SetTransformMatrix(Matrix34::Create(m_cameraComp->GetTransformMatrix().GetScale(), IDENTITY, Vec3(0, 0, 0.55f)));
	}
	else if (bCanStand && !bIsCrouching)
	{
		m_cameraRoot->SetLocalTM(Matrix34::Create(Vec3(1), Rotation, Vec3::CreateLerp(m_cameraRoot->GetLocalTM().GetTranslation(), Vec3(0, 0, 1.8f), 0.5f * 4 * gEnv->pTimer->GetFrameTime())));
		m_capsuleComp->m_height = m_capsuleNormalHeight;
		m_capsuleComp->SetTransformMatrix(Matrix34::Create(m_cameraComp->GetTransformMatrix().GetScale(), IDENTITY, Vec3(0, 0, 1.2f)));
	}
}


void PlayerComponent::HideHealthbar()
{
	m_healthbarUIElement->SetVisible(false);
}

void PlayerComponent::ShowHealthbar()
{
	m_healthbarUIElement->SetVisible(true);
}

void PlayerComponent::UpdateHealthbar()
{
	if (!m_healthbarUIElement) {
		return;
	}
	/*
	if (m_currentHealth == m_lastHealthUpdateAmount) {
		return;
	}
	*/

	//m_lastHealthUpdateAmount = m_currentHealth;
	SUIArguments args;
	args.AddArgument(m_health->GetHealth());
	m_healthbarUIElement->CallFunction("SetHealth", args);
}

void PlayerComponent::InteractableCheckRayast()
{
	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 4> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();

	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(m_cameraComp->GetCamera().GetPosition(), m_cameraComp->GetCamera().GetViewdir() * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 4, pSkippedEntities, 4)) {
		if (hits[0].pCollider) {

			//Debug
			if (pd) {
				pd->Begin("RaycastWeapon", true);
				//pd->AddText3D(hits[0].pt, 500, ColorF(0, 1, 0), 2, "Hit : %s", hitEntity->GetName());
				pd->AddSphere(hits[0].pt, 0.05f, ColorF(0, 0, 1), 2);
			}

			//return hitEntity if exist
			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				if (hitEntity->GetComponent<InteractableComponent>()) {
					m_currentInteractbleEntity = hitEntity;
				}
				else {
					m_currentInteractbleEntity = nullptr;
				}
			}
			else {
				m_currentInteractbleEntity = nullptr;
			}
		}
		else {
			m_currentInteractbleEntity = nullptr;
		}
	}
	else {
		//CryLog("Weapon hit nothing");
		m_currentInteractbleEntity = nullptr;
	}
}

void PlayerComponent::ReactToHit(IEntity* attacker)
{
	if (m_gettingHitSoundTimePassed >= m_timeBetweenPlayingGettingHitSound) {
		int32 randomInt = GetRandomInt(1, 4);
		const char* audioName;
		if (randomInt == 1) {
			audioName = "player_get_hit_sound_1";
		}
		else 	if (randomInt == 2) {
			audioName = "player_get_hit_sound_2";
		}
		else 	if (randomInt == 3) {
			audioName = "player_get_hit_sound_3";
		}
		else 	if (randomInt == 4) {
			audioName = "player_get_hit_sound_4";
		}
		m_audioComp->ExecuteTrigger(CryAudio::StringToId(audioName));
		m_gettingHitSoundTimePassed = 0;
	}

	//shake camera
	f32 x = -0.19f;
	f32 y = GetRandomValue(-0.02f, 0.02f);
	f32 z = GetRandomValue(-0.02f, 0.02f);
	AddRecoil(Vec3(x, y, z));
}

/*******************************************************************************************************************************/

f32 PlayerComponent::GetRandomValue(f32 min, f32 max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min));
}

int32 PlayerComponent::GetRandomInt(int32 min, int32 max)
{
	int32 range = max - min + 1;
	return rand() % range + min;
}