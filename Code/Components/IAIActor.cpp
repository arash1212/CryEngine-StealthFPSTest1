#include "StdAfx.h"
#include "IAIActor.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CryCore/StaticInstanceList.h>


f32 IAIActorComponent::GetRandomValue(f32 min, f32 max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max - min));
}

int32 IAIActorComponent::GetRandomInt(int32 min, int32 max)
{
	int32 range = max - min + 1;
	return rand() % range + min;
}