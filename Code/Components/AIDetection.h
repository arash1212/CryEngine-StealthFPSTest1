#pragma once

enum class EDetectionState {
	IDLE,
	CAUTIOUS,
	HIGHT_ALERT,
	COMBAT
};

static constexpr f32 DEFAULT_MAX_DETECTION_AMOUNT = 1.f;

class AIDetectionComponent final : public IEntityComponent
{
public:
	AIDetectionComponent() = default;
	virtual ~AIDetectionComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<AIDetectionComponent>& desc)
	{
		desc.SetGUID("{7E235301-7B22-4B96-981B-8A4A5A0C7828}"_cry_guid);
	}

private:
	IEntity* m_currentTarget = nullptr;

	f32 m_detectionHeight = 1.8f;

	f32 m_maxDetectionAmount = DEFAULT_MAX_DETECTION_AMOUNT;
	f32 m_detectionAmount = 0.f;
	bool bIsTargetFound = false;
	bool bCanReturnToIdleState = false;

	int32 m_maxDetectionDegree = 70;
	int32 m_minDetectionDegree = 0;
	EDetectionState m_detectionState = EDetectionState::IDLE;

private:
	void UpdateDetectionAmount(f32 DeltaTime);
	void UpdateDetectionState();

public:
	bool IsInView(IEntity* target);
	bool IsVisible(IEntity* target);
	bool IsVisibleFrom(Vec3 from, IEntity* target);
	bool IsTargetCanBeSeen(IEntity* target);

	//Getter & Setter

	void SetCurrentTarget(IEntity* currentTarget);
	bool IsTargetFound();
	EDetectionState GetDetectionState();
	f32 GetDetectionAmount();
	void SetDetectionToMax();
	void SetDetectionToCatious();
	f32 GetMaxDetectionAmount();
	void SetCanReturnToIdleState(bool canReturntoIdle);
	void SetMaxDetectionDegree(int32 degree);
	void SetMinDetectionDegree(int32 degree);
	void SetDetectionHeight(f32 height);
};