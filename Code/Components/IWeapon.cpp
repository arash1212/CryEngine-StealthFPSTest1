#include "StdAfx.h"
#include "Player.h"
#include "ActorState.h"
#include "ShootAccuracy.h"
#include "IWeapon.h"
#include "BulletTracer.h"
#include "IAIActor.h"
#include "Soldier1.h"
#include "Health.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

/*
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
*/

IEntity* IWeaponComponent::Raycast(Vec3 from, Vec3 dir, Vec3 error)
{
	//apply error
	Vec3 finalDir = Vec3(dir.x, dir.y, dir.z);
	if (!bIsAiming) {
		finalDir.x += error.x;
		finalDir.y += error.y;
		finalDir.z += error.z;
	}

	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_ownerEntity->GetPhysics();
	pSkippedEntities[1] = m_pEntity->GetPhysics();

	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(from, finalDir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
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

				//todo : iActor beshe
				if (!hitEntity->GetComponent<Soldier1Component>()) {
					//apply impluse on entity at hit point
					pe_action_impulse impulse;
					impulse.impulse = m_pEntity->GetForwardDir() * 50;
					hits[0].pCollider->Action(&impulse);
				}


				//if hitEntity is an actor
				if (hitEntity->GetComponent<Soldier1Component>()) {
					hitEntity->GetComponent<Soldier1Component>()->ReactToHit(m_pEntity);
					CryLog("Weapon hit actor");
				}
				//if hitEntity is player
				else if (hitEntity->GetComponent<PlayerComponent>()) {
					CryLog("Weapon hit Player");
					hitEntity->GetComponent<PlayerComponent>()->ReactToHit(m_pEntity);
				}

				//if hitEntity has healthComponent
				if (hitEntity->GetComponent<HealthComponent>()) {
					hitEntity->GetComponent<HealthComponent>()->ApplyDamage(GetDamage());
				}

				return hitEntity;
			}
		}
	}
	else {
		//CryLog("Weapon hit nothing");
	}

	return nullptr;
}

bool IWeaponComponent::Fire(IEntity* target)
{
	ShootAccuracyComponent* shootAccuracyComp = m_ownerEntity->GetComponent<ShootAccuracyComponent>();
	if (!shootAccuracyComp) {
		CryLog("IWeaponComponent : Owner have no shootAccuracy assigned !");
		return false;
	}
	if (!m_muzzleAttachment) {
		CryLog("IWeaponComponent : Weapon muzzle attachment not found !");
		return false;
	}

	//apply shootError to owner
	//CryLog("error : %f", shootAccuracyComp->GetShootError());
	shootAccuracyComp->AddShootError(GetShootError());

	if (m_shotTimePassed >= m_timeBetweenShots) {
		//if owner is player
		if (m_cameraComp) {
			PlayerComponent* playerComp = m_ownerEntity->GetComponent<PlayerComponent>();
			playerComp->AddRecoil(GetCameraRecoilAmount());

			//apply kickback
			AddKickBack(Vec3(0, -0.07f, -0.05f));

			Vec3 shooterror = Vec3(GetShootError(shootAccuracyComp->GetShootError()), GetShootError(shootAccuracyComp->GetShootError()), GetShootError(shootAccuracyComp->GetShootError()));
			Raycast(m_cameraComp->GetCamera().GetPosition(), m_cameraComp->GetCamera().GetViewdir(), shooterror);

			//spawn bullet tracer
			Vec3 p = m_muzzleAttachment->GetAttWorldAbsolute().t - m_pEntity->GetWorldPos();
			SpawnBulletTracer(shooterror, m_pEntity->GetWorldPos() + p.normalized() * 1.3f, Quat::CreateRotationVDir(m_cameraComp->GetCamera().GetViewdir()));
		}
		//else
		else {
			//ye kar dige bokon ?
			Vec3 shooterror = Vec3(GetShootError(shootAccuracyComp->GetShootError()), GetShootError(shootAccuracyComp->GetShootError()), GetShootError(shootAccuracyComp->GetShootError()));

			Vec3 p = m_muzzleAttachment->GetAttWorldAbsolute().t - m_pEntity->GetWorldPos();
			//dir
			Vec3 targetPos = Vec3(target->GetWorldPos().x + 0.4f, target->GetWorldPos().y, target->GetWorldPos().z + 1.5f);
			if (target->GetComponent<PlayerComponent>()) {
				if (target->GetComponent<PlayerComponent>()->IsCrouching()) {
					targetPos = Vec3(target->GetWorldPos().x + 0.4f, target->GetWorldPos().y, target->GetWorldPos().z + 0.8f);
				}
			}

			Vec3 dir = targetPos - m_muzzleAttachment->GetAttWorldAbsolute().t;

			Raycast(m_pEntity->GetWorldPos() + p.normalized() * 1.3f, dir, shooterror);
			SpawnBulletTracer(shooterror, m_pEntity->GetWorldPos() + p.normalized() * 1.3f, Quat::CreateRotationVDir(dir.normalized()));
		}

		m_audioComp->ExecuteTrigger(GetRandomShootSound());


		//muzzleflashes
		m_MuzzleFlashDeActivationTimePassed = 0;

		m_shotTimePassed = 0;
		m_activeFragmentId = m_fireFragmentId;
		m_animationComp->GetActionController()->Flush();
		m_animationComp->QueueCustomFragment(*m_fireAction);

		//apply recoil to weapon model after shot
		m_targetRotation *= Quat::CreateRotationXYZ(GetMeshRecoilAmount());

		return true;
	}

	return false;
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

void IWeaponComponent::SetCameraBaseEntity(IEntity* cameraBaseEntity)
{
	m_cameraBaseEntity = cameraBaseEntity;
}

void IWeaponComponent::SetIsAiming(bool isAiming)
{
	bIsAiming = isAiming;
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
	f32 x = 0.19f;
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

void IWeaponComponent::ResetShootSoundNumber()
{
	m_currentShootSoundNumber = 0;
}

f32 IWeaponComponent::GetRandomValue(f32 min, f32 max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min));
}

int32 IWeaponComponent::GetRandomInt(int32 min, int32 max)
{
	int32 range = max - min + 1;
	return rand() % range + min;
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
	m_targetRotation = Quat::CreateSlerp(m_targetRotation, m_meshefaultRotaion, m_returnSpeed * gEnv->pTimer->GetFrameTime());
	m_currentRotation = Quat::CreateSlerp(m_currentRotation, m_targetRotation, m_snapiness * gEnv->pTimer->GetFrameTime());
	m_animationComp->SetTransformMatrix(Matrix34::Create(m_animationComp->GetTransform().get()->GetScale(), m_currentRotation, m_animationComp->GetTransform().get()->GetTranslation()));
}

f32 IWeaponComponent::GetShootError(f32 error)
{
	return GetRandomValue(-error, error);
}

void IWeaponComponent::SpawnBulletTracer(Vec3 error, Vec3 pos, Quat dir)
{
	Vec3 spawnPos = pos;
	Quat rotation = dir;
	rotation.v.x += error.x;
	rotation.v.y += error.y;
	rotation.v.z += error.z;

	SEntitySpawnParams bulletTracerSpawnParams;
	bulletTracerSpawnParams.vPosition = spawnPos;
	bulletTracerSpawnParams.qRotation = rotation;
	IEntity* spawnedBulletTracerEntity = gEnv->pEntitySystem->SpawnEntity(bulletTracerSpawnParams);

	spawnedBulletTracerEntity->GetOrCreateComponent<BulletTracerComponent>();
}

void IWeaponComponent::UpdateMuzzleFlashes()
{
	if (m_MuzzleFlashDeActivationTimePassed >= m_timeBetweenMuzzleFlashDeActivation) {
		m_muzzleFlash1Attachment->HideAttachment(1);
		m_muzzleFlash2Attachment->HideAttachment(1);
		m_muzzleFlash3Attachment->HideAttachment(1);
		bCanChangeMuzzleFlash = true;
	}
	else if (bCanChangeMuzzleFlash) {
		int32 randomInt = GetRandomInt(1, 3);
		bCanChangeMuzzleFlash = false;
		if (randomInt == 1) {
			m_muzzleFlash1Attachment->HideAttachment(0);
		}
		else if (randomInt == 2) {
			m_muzzleFlash2Attachment->HideAttachment(0);
		}
		else if (randomInt == 3) {
			m_muzzleFlash3Attachment->HideAttachment(0);
		}
	}
}

void IWeaponComponent::Aim()
{
	if (bIsAiming) {
		m_animationComp->SetTransformMatrix(Matrix34::Create(m_defaultSize, m_aimRotation,
			Vec3::CreateLerp(m_animationComp->GetTransform().get()->GetTranslation(), m_aimPosition, 0.1f * 3.f * gEnv->pTimer->GetFrameTime())));
	}

}

CryAudio::ControlId IWeaponComponent::GetRandomShootSound()
{
	/*
	if (m_currentShootSoundNumber >= m_shootSounds.Size() - 1) {
		m_currentShootSoundNumber = 0;
	}
	else {
		m_currentShootSoundNumber++;
	}
	*/
	int32 randomNum = 0;
	if (m_shootSounds.Size() > 1) {
		randomNum = GetRandomInt(0, m_shootSounds.Size() - 1);
	}
	return m_shootSounds.At(randomNum);
}
