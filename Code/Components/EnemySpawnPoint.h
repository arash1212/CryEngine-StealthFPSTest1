#pragma once

#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class EnemySpawnPointComponent final : public IEntityComponent
{
public:
	EnemySpawnPointComponent() = default;
	virtual ~EnemySpawnPointComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	struct SSpawnPointDefinition
	{
		static void ReflectType(Schematyc::CTypeDesc<SSpawnPointDefinition>& desc)
		{
			desc.SetLabel("SpawnPointDefinition");
			desc.SetGUID("{AAED0CC6-730E-4EC8-8BBA-994FA5008DD5}"_cry_guid);
			desc.AddMember(&SSpawnPointDefinition::m_patrolPathName, 'pna', "pathpatronname", "Path Patrol Name", "Set Path Patro Namme", "");
		}

		inline bool Serialize(Serialization::IArchive& archive);

		inline bool operator==(const SSpawnPointDefinition& rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }
		inline bool operator!=(const SSpawnPointDefinition& rhs) const { return !(*this == rhs); }

		Schematyc::CSharedString m_patrolPathName = "";
	};
	typedef SSpawnPointDefinition SpawnPoint;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<EnemySpawnPointComponent>& desc)
	{
		desc.SetGUID("{CDA35FD4-4536-418A-BFF2-11DB58390F62}"_cry_guid);
		desc.AddMember(&EnemySpawnPointComponent::spawnInfo, 'swn', "spawnpointinfos", "Spawn Point Info", "Set Spawn Point Info", SpawnPoint());
	}


private:
	SpawnPoint spawnInfo;

private:
	void Spawn();
};