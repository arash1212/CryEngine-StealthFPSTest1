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

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

bool AIDetectionComponent::IsInView(IEntity* target)
{
	Vec3 dir = target->GetWorldPos() - m_pEntity->GetWorldPos();
	float dot = m_pEntity->GetForwardDir().normalized().dot(dir);
	return dot > 0;
}

bool AIDetectionComponent::IsVisible(IEntity* target)
{
	int flags = rwi_colltype_any | rwi_stop_at_pierceable;
	std::array<ray_hit, 2> hits;
	static IPhysicalEntity* pSkippedEntities[10];
	pSkippedEntities[0] = m_pEntity->GetPhysics();

	Vec3 currentPos = m_pEntity->GetPos();
	Vec3 origin = Vec3(currentPos.x, currentPos.y, currentPos.z + 2);
	Vec3 dir = target->GetWorldPos() - m_pEntity->GetWorldPos();
	IPersistantDebug* pd = gEnv->pGameFramework->GetIPersistantDebug();
	if (gEnv->pPhysicalWorld->RayWorldIntersection(origin, dir * gEnv->p3DEngine->GetMaxViewDistance(), ent_all, flags, hits.data(), 2, pSkippedEntities, 2)) {
		if (hits[0].pCollider) {

			//Debug
			if (pd) {
				pd->Begin("Raycast", true);
				pd->AddSphere(hits[0].pt, 0.2f, ColorF(1, 0, 0), 2);
				pd->AddSphere(origin, 0.3f, ColorF(0, 1, 0), 2);
			}

			//return true if hitEntity is target
			IEntity* hitEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hits[0].pCollider);
			if (hitEntity) {
				CryLog("Guard found %s", hitEntity->GetName());

				if (hitEntity = target) {
					return true;
				}
			}
		}
	}
	return false;
}
