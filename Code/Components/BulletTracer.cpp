#include "StdAfx.h"
#include "BulletTracer.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterBulletTracerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(BulletTracerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterBulletTracerComponent);
}

void BulletTracerComponent::Initialize()
{
	m_meshComp = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CStaticMeshComponent>();
	m_meshComp->SetTransformMatrix(Matrix34::Create(Vec3(1.f), Quat::CreateRotationZ(DEG2RAD(90)), ZERO));
	m_meshComp->SetFilePath("Objects/effects/bullettracer/bullet_tracer_1.cgf");
	m_meshComp->LoadFromDisk();
	m_meshComp->ResetObject();

	SEntityPhysicalizeParams physParams;
	physParams.type = PE_RIGID;
	physParams.mass = 20000.f;
	m_pEntity->Physicalize(physParams);
}

Cry::Entity::EventFlags BulletTracerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void BulletTracerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {

	}break;
	case Cry::Entity::EEvent::Update: {
		float deltatime = event.fParam[0];

		Move(deltatime);
	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void BulletTracerComponent::Move(float DeltaTime)
{
	if (auto* pPhysics = GetEntity()->GetPhysics())
	{
		pe_action_impulse impulseAction;
		const float initialVelocity = 1000.f;
		impulseAction.impulse = GetEntity()->GetWorldRotation().GetColumn1() * initialVelocity;
		//pPhysics->Action(&impulseAction);

		Vec3 moveToPos = m_pEntity->GetPos();
		m_pEntity->SetPos(m_pEntity->GetPos() + m_pEntity->GetForwardDir() * DeltaTime * 50.1f);
	}
}