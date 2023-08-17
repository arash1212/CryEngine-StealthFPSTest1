#include "StdAfx.h"
#include "CustomDecal.h"
#include "GamePlugin.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterCustomDecalComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CustomDecalComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCustomDecalComponent);
}


void CustomDecalComponent::Initialize()
{
	/*
	m_decalComp = m_pEntity->GetOrCreateComponent < Cry::DefaultComponents::CDecalComponent>();
	m_decalComp->SetMaterialFileName(m_decalFilePath);
	m_decalComp->EnableAutomaticSpawn(true);
	m_decalComp->SetDepth(4.f);
	m_decalComp->Spawn();
	*/
}

Cry::Entity::EventFlags CustomDecalComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void CustomDecalComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {


	}break;
	case Cry::Entity::EEvent::Update: {
		//f32 deltaTime = event.fParam[0];

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void CustomDecalComponent::SetDecalFilePath(string decalFilePath)
{
	this->m_decalFilePath = decalFilePath;
}

void CustomDecalComponent::SpawnAt(Vec3 pos)
{
}
