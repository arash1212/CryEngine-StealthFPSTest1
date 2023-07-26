#include "StdAfx.h"
#include "Crosshair.h"
#include "Player.h"
#include "GamePlugin.h"

#include <FlashUI/FlashUIElement.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterCrosshairComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CrosshairComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCrosshairComponent);
}

void CrosshairComponent::Initialize()
{
	m_crosshairUIElement = gEnv->pFlashUI->GetUIElement(CROSSHAIR_UI_ELEMENT_NAME);

	Show();
	BackToNormal();
}

Cry::Entity::EventFlags CrosshairComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void CrosshairComponent::ProcessEvent(const SEntityEvent& event)
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

void CrosshairComponent::Hide()
{
	m_crosshairUIElement->SetVisible(false);
}

void CrosshairComponent::Show()
{
	m_crosshairUIElement->SetVisible(true);
}

void CrosshairComponent::Spread()
{
	m_crosshairUIElement->CallFunction("spread");
}

void CrosshairComponent::BackToNormal()
{
	m_crosshairUIElement->CallFunction("backToNormal");
}