#pragma once

#include <DefaultComponents/Effects/DecalComponent.h>

class CustomDecalComponent final : public IEntityComponent
{
public:
	CustomDecalComponent() = default;
	virtual ~CustomDecalComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CustomDecalComponent>& desc)
	{
		desc.SetGUID("{26B8249F-6229-48A2-ACB4-BCED2B161807}"_cry_guid);
	}

private:
	Cry::DefaultComponents::CDecalComponent* m_decalComp;
	string m_decalFilePath = "";

public:
	void SetDecalFilePath(string decalFilePath);
	static void SpawnAt(Vec3 pos);
};