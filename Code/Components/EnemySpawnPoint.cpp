#include "StdAfx.h"
#include "EnemySpawnPoint.h"
#include "IAIActor.h"
#include "Soldier1.h"
#include "GamePlugin.h"
#include "ActorInfo.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterEnemySpawnPointComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(EnemySpawnPointComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterEnemySpawnPointComponent);
}

void EnemySpawnPointComponent::Initialize()
{
}

Cry::Entity::EventFlags EnemySpawnPointComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void EnemySpawnPointComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case Cry::Entity::EEvent::Initialize: {

	}break;
	case Cry::Entity::EEvent::GameplayStarted: {
		Spawn();

	}break;
	case Cry::Entity::EEvent::Update: {

	}break;
	case Cry::Entity::EEvent::Reset: {

	}break;
	default:
		break;
	}
}

void EnemySpawnPointComponent::Spawn()
{
	SEntitySpawnParams spawnParams;
	spawnParams.vPosition = m_pEntity->GetWorldPos();
	spawnParams.qRotation = m_pEntity->GetWorldRotation();
	IEntity* spawnedEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	Soldier1Component* soldier1 = spawnedEntity->GetOrCreateComponent<Soldier1Component>();
	soldier1->SetPatrolPathName(spawnInfo.m_patrolPathName);
	soldier1->SetSpawnPos(m_pEntity->GetWorldPos());

	//todo : ye fekri be halesh bokon
	if (spawnInfo.m_factionNumber == "1") {
		spawnedEntity->GetComponent<ActorInfoComponent>()->SetFaction(EFaction::FACTION1);
	}
	else if (spawnInfo.m_factionNumber == "2") {
		spawnedEntity->GetComponent<ActorInfoComponent>()->SetFaction(EFaction::FACTION2);
	}
	else if (spawnInfo.m_factionNumber == "3") {
		spawnedEntity->GetComponent<ActorInfoComponent>()->SetFaction(EFaction::FACTION3);
	}
}

inline bool EnemySpawnPointComponent::SSpawnPointDefinition::Serialize(Serialization::IArchive& archive)
{
	archive.openBlock("SpawnPointDefinition", "SpawnPointDefinition");
	archive(m_patrolPathName, "PatrolPathName", "PatrolPathName");
	archive(m_factionNumber, "FactionNum", "FactionNum");
	archive.closeBlock();
	return true;
}
