#pragma once

#include <DefaultComponents/Geometry/StaticMeshComponent.h>

class BulletTracerComponent final : public IEntityComponent
{
public:

	BulletTracerComponent() = default;
	virtual ~BulletTracerComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<BulletTracerComponent>& desc)
	{
		desc.SetGUID("{081A3F3E-400D-4722-9CC7-0128EBC13A6B}"_cry_guid);
	}

private :

	Cry::DefaultComponents::CStaticMeshComponent* m_meshComp;

private :

	void Move(float deltaTime);
};