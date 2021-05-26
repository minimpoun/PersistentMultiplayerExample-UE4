// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerExampleGameMode.h"
#include "MultiplayerExampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiplayerExampleGameMode::AMultiplayerExampleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
