#pragma once

struct IUIElement;
class PlayerComponent;

static string CROSSHAIR_UI_ELEMENT_NAME = "crosshair";

class CrosshairComponent final : public IEntityComponent
{
public:
	CrosshairComponent() = default;
	virtual ~CrosshairComponent() = default;

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CrosshairComponent>& desc)
	{
		desc.SetGUID("{A2D737C1-F64B-4087-B89B-E965D547E80F}"_cry_guid);
	}

private:
	IUIElement* m_crosshairUIElement = nullptr;

public:
	void Hide();
	void Show();
	void Spread();
	void BackToNormal();
};