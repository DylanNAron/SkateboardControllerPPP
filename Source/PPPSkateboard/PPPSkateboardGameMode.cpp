// Copyright Epic Games, Inc. All Rights Reserved.

#include "PPPSkateboardGameMode.h"
#include "PPPSkateboardCharacter.h"
#include "UObject/ConstructorHelpers.h"

APPPSkateboardGameMode::APPPSkateboardGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
