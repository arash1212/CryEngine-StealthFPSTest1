#include "StdAfx.h"
#include "Player.h"
#include "IWeapon.h"
#include "Crosshair.h"
#include "SpawnPoint.h"
#include "ShootAccuracy.h"
#include "GamePlugin.h"

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
	m_capsuleComp->SetTransformMatrix(Matrix34::Create(Vec3(0.99f, 1.1f, 1.1f), IDENTITY, Vec3(0, 0, 1.2f)));

	m_inputComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();

	m_listenerComp = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();

	InitCamera();
	InitInputs();

	//primary weapon init
	m_primaryWeapon = m_cameraBase->GetOrCreateComponent<IWeaponComponent>();
	m_primaryWeapon->SetCharacterController(m_characterControllerComp);
	m_primaryWeapon->SetCameraComponent(m_cameraComp);
	m_primaryWeapon->SetOwnerEntity(m_pEntity);
	m_primaryWeapon->SetCameraBaseEntity(m_cameraRoot);

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
	m_cameraRoot->SetLocalTM(Matrix34::Create(Vec3(1), IDENTITY, Vec3(0, 0, 1.8f)));

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
				m_currentlySelectedWeapon->Fire();
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
}

void PlayerComponent::HeadBob(float deltatime)
{
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

void PlayerComponent::RecoilUpdate()
{
	m_targetRotation = Quat::CreateSlerp(m_targetRotation, m_cameraDefaultRotaion, 0.5f *  m_returnSpeed * gEnv->pTimer->GetFrameTime());
	m_currentRotation = Quat::CreateSlerp(m_currentRotation, m_targetRotation, 0.4f * m_snapiness * gEnv->pTimer->GetFrameTime());
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