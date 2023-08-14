#include "StdAfx.h"
#include "Player.h"
#include "AIDetection.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterAIDetectionComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(AIDetectionComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterAIDetectionComponent);
}


void AIDetectionComponent::Initialize()
{

}

Cry::Entity::EventFlags AIDetectionComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void AIDetectionComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		f32 deltaTime = event.fParam[0];

		UpdateDetectionAmount(deltaTime);
		UpdateDetectionState();

	}break;
	case Cry::Entity::EEvent::Reset: {
		m_currentTarget = nullptr;
		m_detectionAmount = 0.f;
		bIsTargetFound = false;
		bCanReturnToIdleState = false;

	}break;
	default:
		break;
	}
}

void AIDetectionComponent::UpdateDetectionAmount(f32 DeltaTime)
{
	if (m_currentTarget) {
		if (IsTargetCanBeSeen(m_currentTarget)) {
			//CryLog("target can be seen !");
			if (m_detectionAmount < m_maxDetectionAmount) {
				m_detectionAmount = CLAMP(m_detectionAmount + DeltaTime, 0, m_maxDetectionAmount);
			}
		}
		else if (m_detectionAmount > m_maxDetectionAmount / 2) {
			m_detectionAmount = CLAMP(m_detectionAmount - DeltaTime, m_maxDetectionAmount / 2, m_maxDetectionAmount);
		}
		//return to idle state ?
		else if (m_detectionAmount >= m_maxDetectionAmount / 2 && bCanReturnToIdleState) {
			m_detectionAmount = CLAMP(m_detectionAmount - DeltaTime, 0, m_maxDetectionAmount);
		}
	}
	else if (m_detectionAmount > 0) {
		m_detectionAmount = CLAMP(m_detectionAmount - DeltaTime, 0, m_maxDetectionAmount);
	}

	if (m_detectionAmount >= m_maxDetectionAmount || bIsTargetFound && m_detectionAmount >= m_maxDetectionAmount / 2) {
		bIsTargetFound = true;
	}
	else if (m_detectionAmount <= m_maxDetectionAmount / 2) {
		bIsTargetFound = false;
	}

	//CryLog("Detection amount %f !", m_detectionAmount);
}

void AIDetectionComponent::UpdateDetectionState()
{
	if (m_detectionAmount == 0) {
		m_detectionState = EDetectionState::IDLE;
	}else if (m_detectionAmount > m_maxDetectionAmount / 2 && m_detectionAmount < m_maxDetectionAmount && !bIsTargetFound) {
		m_detectionState = EDetectionState::CAUTIOUS;
	}
	else if (m_detectionAmount >= m_maxDetectionAmount || bIsTargetFound && m_detectionAmount >= m_maxDetectionAmount / 2) {
		m_detectionState = EDetectionState::COMBAT;
	}
}

bool AIDetectionComponent::IsInView(IEntity* target)
{
	Vec3 dir = target->GetWorldPos() - m_pEntity->GetWorldPos();
	float dot = m_pEntity->GetForwardDir().normalized().dot(dir.normalized());
	float degree = RAD2DEG(crymath::acos(dot));

	//CryLog("degree :%f ", degree);
	return degree >= m_minDetectionDegree && degree <= m_maxDetectionDegree;
}

bool AIDetectionComponent::IsVisible(IEntity* target)
{
	int flags = rwi_pierceability(9);
	std::array<ray_hit, 4> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();

	Vec3 currentPos = m_pEntity->GetPos();

	Vec3 origin = Vec3(currentPos.x, currentPos.y, currentPos.z + m_detectionHeight);
	Vec3 targetPos = target->GetWorldPos();
	if (target->GetComponent<PlayerComponent>()) {
		if (!target->GetComponent<PlayerComponent>()->IsCrouching()) {
			targetPos = Vec3(targetPos.x, targetPos.y, targetPos.z + m_targetNormalHeight);
		}
		else {
			targetPos = Vec3(targetPos.x, targetPos.y, targetPos.z - m_targetCrouchHeight);
		}
	}
	Vec3 dir = targetPos - m_pEntity->GetWorldPos();
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 4, pSkippedEntities, 4)) {
		for (int32 i = 0; i < hits.size(); i++) {
			if (hits[i].pCollider) {

				//Debug
				if (pd) {
					pd->Begin("RaycastDetectionComp", true);
					//pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 0, 0), 2);
					//pd->AddSphere(hits[0].pt, 0.3f, ColorF(0, 1, 0), 2);
					//pd->AddLine(origin, hits[i].pt, ColorF(1, 0, 0), 1);
				}


				//return true if hitEntity is target
				IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[i].pCollider);
				if (hitEntity) {
					if (hitEntity == target) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool AIDetectionComponent::IsVisibleFrom(Vec3 from, IEntity* target)
{
	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();

	Vec3 currentPos = from;
	Vec3 origin = Vec3(currentPos.x, currentPos.y, currentPos.z + 1.f);
	Vec3 dir = target->GetWorldPos() - m_pEntity->GetWorldPos();

	if (gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {

			//return true if hitEntity is target
			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				if (hitEntity == target) {
					return true;
				}
			}
		}
	}
	return false;
}

bool AIDetectionComponent::IsTargetCanBeSeen(IEntity* target)
{

	return IsInView(target) && IsVisible(target);
}

void AIDetectionComponent::SetCurrentTarget(IEntity* currentTarget)
{
	this->m_currentTarget = currentTarget;
}

bool AIDetectionComponent::IsTargetFound()
{
	return bIsTargetFound;
}

void AIDetectionComponent::SetFoundTarget(bool isFound)
{
	this->bIsTargetFound = isFound;
}

EDetectionState AIDetectionComponent::GetDetectionState()
{
	return m_detectionState;
}

f32 AIDetectionComponent::GetDetectionAmount()
{
	return m_detectionAmount;
}

void AIDetectionComponent::SetDetectionToMax()
{
	m_detectionAmount = m_maxDetectionAmount;
}

void AIDetectionComponent::SetDetectionToCatious()
{
	m_detectionAmount = (m_maxDetectionAmount / 2) + 0.1f;
}

f32 AIDetectionComponent::GetMaxDetectionAmount()
{
	return m_maxDetectionAmount;
}

void AIDetectionComponent::SetCanReturnToIdleState(bool canReturntoIdle)
{
	this->bCanReturnToIdleState = canReturntoIdle;
}

void AIDetectionComponent::SetMaxDetectionDegree(int32 degree)
{
	this->m_maxDetectionDegree = degree;
}

void AIDetectionComponent::SetMinDetectionDegree(int32 degree)
{
	this->m_minDetectionDegree = degree;
}

void AIDetectionComponent::SetDetectionHeight(f32 height)
{
	this->m_detectionHeight = height;
}

void AIDetectionComponent::SetTargetNormalHeight(f32 height)
{
	m_targetNormalHeight = height;
}

void AIDetectionComponent::SetTargetCrouchHeight(f32 croutchHeight)
{
	m_targetCrouchHeight = croutchHeight;
}

