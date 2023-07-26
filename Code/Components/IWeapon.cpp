#include "StdAfx.h"
#include "Player.h"
#include "ActorState.h"
#include "ShootAccuracy.h"
#include "IWeapon.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterIWeaponComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(IWeaponComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterIWeaponComponent);
}

void IWeaponComponent::Initialize()
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
	m_shootSound = CryAudio::StringToId("glock_fire_sound_1");
}

Cry::Entity::EventFlags IWeaponComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void IWeaponComponent::ProcessEvent(const SEntityEvent& event)
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

		//apply sway if owner is player
		if (m_ownerEntity && m_ownerEntity->GetComponent<PlayerComponent>()) {
			Vec2 rotationDelta = m_ownerEntity->GetComponent<PlayerComponent>()->GetRotationDelta();
			Sway(rotationDelta.y, rotationDelta.x);
		}
		
		//timers 
		if (m_shotTimePassed < m_timeBetweenShots) {
			m_shotTimePassed += 0.5f * deltatime;
		}

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

IEntity* IWeaponComponent::Raycast(Vec3 from, Vec3 dir, f32 error)
{
	//apply error
	dir.x += GetShootError(error);
	dir.y += GetShootError(error);
	dir.z += GetShootError(error);

	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_ownerEntity->GetPhysics();
	pSkippedEntities[1] = m_pEntity->GetPhysics();

	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(from, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {

			//Debug
			if (pd) {
				pd->Begin("Raycast", true);
				//pd->AddText3D(hits[0].pt, 500, ColorF(0, 1, 0), 2, "Hit : %s", hitEntity->GetName());
				pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 0, 0), 2);
			}

			//return hitEntity if exist
			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				CryLog("Weapon hit %s", hitEntity->GetName());
				return hitEntity;
			}
		}
	}
	else {
		CryLog("Weapon hit nothing");
	}

	return nullptr;
}

void IWeaponComponent::Fire()
{
	ShootAccuracyComponent* shootAccuracyComp = m_ownerEntity->GetComponent<ShootAccuracyComponent>();
	if (!shootAccuracyComp) {
		CryLog("IWeaponComponent : Owner have no shootAccuracy assigned !");
		return;
	}
	//apply shootError to owner
	//CryLog("error : %f", shootAccuracyComp->GetShootError());
	shootAccuracyComp->AddShootError(GetShootError());

	if (m_shotTimePassed >= m_timeBetweenShots) {
		//if owner is player
		if (m_cameraComp) {
			PlayerComponent* playerComp = m_ownerEntity->GetComponent<PlayerComponent>();
			Raycast(m_cameraComp->GetCamera().GetPosition(), m_cameraComp->GetCamera().GetViewdir(), shootAccuracyComp->GetShootError());
			playerComp->AddRecoil(GetCameraRecoilAmount());

			//apply kickback
			AddKickBack(Vec3(0, -0.12f, -0.05f));
		}
		//else
		else {
			//ye kar dige bokon ?
		}

		m_audioComp->ExecuteTrigger(m_shootSound);

		m_shotTimePassed = 0;
		m_activeFragmentId = m_fireFragmentId;
		m_animationComp->GetActionController()->Flush();
		m_animationComp->QueueCustomFragment(*m_fireAction);

		//apply recoil to weapon model after shot
		m_targetRotation *= Quat::CreateRotationXYZ(GetMeshRecoilAmount());
	}
}

void IWeaponComponent::SetCameraComponent(Cry::DefaultComponents::CCameraComponent* cameraComp)
{
	this->m_cameraComp = cameraComp;
}

void IWeaponComponent::SetMuzzlePos(Vec3 muzzlePos)
{
	this->m_muzzlePos = muzzlePos;
}

void IWeaponComponent::SetOwnerEntity(IEntity* ownerEntity)
{
	this->m_ownerEntity = ownerEntity;
}

void IWeaponComponent::SetCharacterController(Cry::DefaultComponents::CCharacterControllerComponent* characterControllerComp)
{
	this->m_characteControllerrComp = characterControllerComp;
}

float IWeaponComponent::GetDamage()
{
	return m_damage;
}

f32 IWeaponComponent::GetShootError()
{
	return m_shootError;
}

Vec3 IWeaponComponent::GetCameraRecoilAmount()
{
	f32 x = GetRandomValue(0.013f, 0.17f);
	f32 y = GetRandomValue(-0.02f, 0.02f);
	f32 z = GetRandomValue(-0.02f, 0.02f);
	//CryLog("x : %f, y : %f , z : %f", x, y, z);
	return Vec3(x, y, z);
}

Vec3 IWeaponComponent::GetMeshRecoilAmount()
{
	recoilInv *= -1;
	return Vec3(GetRandomValue(-0.07f, -0.05f), recoilInv * 0.02f, recoilInv * 0.02f);
}

f32 IWeaponComponent::GetRandomValue(f32 min, f32 max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min));
}

void IWeaponComponent::UpdateAnimation()
{
	if (!m_characteControllerrComp) {
		return;
	}
	if (!m_ownerEntity) {
		return;
	}

	FragmentID currentFragmentId;
	if (m_ownerEntity->GetComponent<ActorStateComponent>()->GetState() == EActorState::WALKING) {
		currentFragmentId = m_walkFragmentId;
	}
	else if (m_ownerEntity->GetComponent<ActorStateComponent>()->GetState() == EActorState::RUNNING) {
		currentFragmentId = m_runFragmentId;
	}
	else {
		currentFragmentId = m_idleFragmentId;
	}

	if (m_activeFragmentId != currentFragmentId) {
		m_activeFragmentId = currentFragmentId;
		m_animationComp->QueueFragmentWithId(m_activeFragmentId);
	}
}

void IWeaponComponent::Sway(f32 mouseX, f32 mouseY)
{
	mouseX *= 0.04f;
	mouseY *= -0.035f;

	Quat rotationX = Quat::CreateRotationY(mouseX);
	Quat rotationY = Quat::CreateRotationX(mouseY);

	Quat targetRotation = rotationX * rotationY;

	m_animationComp->SetTransformMatrix(Matrix34::Create(m_animationComp->GetTransformMatrix().GetScale(),
		Quat::CreateSlerp(
			m_animationComp->GetTransform().get()->GetRotation().ToQuat(),
			m_defaultAnimationCompRotation + targetRotation, 0.3f * m_swaySpeed * gEnv->pTimer->GetFrameTime()),
		m_animationComp->GetTransformMatrix().GetTranslation()
	));
}

void IWeaponComponent::KickBack()
{
	Vec3 endPos = Vec3(m_defaultAnimationCompPosition.x + m_kickBackAmount.x, m_defaultAnimationCompPosition.y + m_kickBackAmount.y, m_defaultAnimationCompPosition.z + m_kickBackAmount.z);

	Matrix34 matrix;
	matrix.SetScale(m_animationComp->GetTransformMatrix().GetScale());
	matrix.SetRotation33(m_animationComp->GetTransform().get()->GetRotation().ToMatrix33());
	matrix.SetTranslation(Vec3::CreateLerp(m_animationComp->GetTransformMatrix().GetTranslation(), endPos, 0.6f * m_kickBackSpeed * gEnv->pTimer->GetFrameTime()));
	m_animationComp->SetTransformMatrix(matrix);

	f32 frameTime = gEnv->pTimer->GetFrameTime();
	if (m_kickBackAmount.y > 0) {
		m_kickBackAmount.y = CLAMP(m_kickBackAmount.y - frameTime, 1, 0);
	}
	else if (m_kickBackAmount.y < 0) {
		m_kickBackAmount.y = CLAMP(m_kickBackAmount.y + frameTime, -1, 0);
	}

	if (m_kickBackAmount.z > 0) {
		m_kickBackAmount.z = CLAMP(m_kickBackAmount.z - frameTime, 1, 0);
	}
	else if (m_kickBackAmount.z < 0) {
		m_kickBackAmount.z = CLAMP(m_kickBackAmount.z + frameTime, -1, 0);
	}

	if (m_kickBackAmount.x > 0) {
		m_kickBackAmount.x = CLAMP(m_kickBackAmount.x - frameTime, 1, 0);
	}
	else if (m_kickBackAmount.x < 0) {
		m_kickBackAmount.x = CLAMP(m_kickBackAmount.x + frameTime, -1, 0);
	}
}

void IWeaponComponent::AddKickBack(Vec3 amount)
{
	m_kickBackAmount = amount;
}

void IWeaponComponent::Recoil()
{
	m_targetRotation = Quat::CreateSlerp(m_targetRotation, m_meshefaultRotaion, 0.5f * m_returnSpeed * gEnv->pTimer->GetFrameTime());
	m_currentRotation = Quat::CreateSlerp(m_currentRotation, m_targetRotation, 0.4f * m_snapiness * gEnv->pTimer->GetFrameTime());
	m_animationComp->SetTransformMatrix(Matrix34::Create(m_animationComp->GetTransform().get()->GetScale(), m_currentRotation, m_animationComp->GetTransform().get()->GetTranslation()));
}

f32 IWeaponComponent::GetShootError(f32 error)
{
	return GetRandomValue(-error, error);
}
