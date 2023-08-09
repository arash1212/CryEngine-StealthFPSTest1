#pragma once

enum class EFaction {
	FACTION1,
	FACTION2,
	FACTION3
};

class ActorInfoComponent final : public IEntityComponent
{
public:
	ActorInfoComponent() = default;
	virtual ~ActorInfoComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<ActorInfoComponent>& desc)
	{
		desc.SetGUID("{E86195F7-CAED-4765-A55A-1033B0FB464F}"_cry_guid);
	}

private:
	EFaction m_faction = EFaction::FACTION1;
	Schematyc::CArray<IEntity*> m_hostileEntities;
	Schematyc::CArray<IEntity*> m_allieEntities;

	bool bIsGameStarted = false;

private:
	void FillEntities();

public:
	EFaction GetFaction();
	void SetFaction(EFaction faction);



	IEntity* GetHostileEntity();
};