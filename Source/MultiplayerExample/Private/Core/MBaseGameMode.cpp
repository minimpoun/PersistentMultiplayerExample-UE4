/**
* MIT License
*
* Copyright (c) 2021 Chris
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
**/

#include "Core/MBaseGameMode.h"

#include "Async/Async_UpdateInventory.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MPlayerCharacter.h"
#include "Player/MPlayerController.h"
#include "Player/MPlayerState.h"

FExampleGameModeEvents::FReadyToSpawnPlayer FExampleGameModeEvents::ReadyToSpawnPlayerEvent;

AMultiplayerExampleGameMode::AMultiplayerExampleGameMode()
{
	bUseSeamlessTravel = false;
	FExampleGameModeEvents::ReadyToSpawnPlayerEvent.AddUObject(this, &ThisClass::PerformInitialSpawn);
}

void AMultiplayerExampleGameMode::PostLogin(APlayerController* NewPlayer)
{
	// Super::PostLogin(NewPlayer);

	UWorld* World = GetWorld();

	auto* PC = Cast<AMPlayerController>(NewPlayer);
	if (PC)
	{
		PC->Client_OnConnectionComplete();
	}
	else
	{
		return;
	}

	if (MustSpectate(NewPlayer))
	{
		NumSpectators++;
	}
	else if (World->IsInSeamlessTravel() || NewPlayer->HasClientLoadedCurrentWorld())
	{
		NumPlayers++;
	}
	else
	{
		NumTravellingPlayers++;
	}

	const FString Address = NewPlayer->GetPlayerNetworkAddress();
	const int32 Pos = Address.Find(TEXT(":"), ESearchCase::CaseSensitive);
	NewPlayer->PlayerState->SavedNetworkAddress = (Pos > 0) ? Address.Left(Pos) : Address;

	FindInactivePlayer(NewPlayer);
	GenericPlayerInitialization(NewPlayer);
	NewPlayer->ClientCapBandwidth(NewPlayer->Player->CurrentNetSpeed);

	if (MustSpectate(NewPlayer))
	{
		NewPlayer->ClientGotoState(NAME_Spectating);
	}
	else
	{
		const FUniqueNetIdRepl& ControllerStateUniqueId = NewPlayer->PlayerState->GetUniqueId();
		if (ControllerStateUniqueId.IsValid())
		{
			GetGameInstance()->AddUserToReplay(ControllerStateUniqueId.ToString());
		}
	}

	if (GameSession)
	{
		GameSession->PostLogin(NewPlayer);
	}

	K2_PostLogin(NewPlayer);
	FGameModeEvents::GameModePostLoginEvent.Broadcast(this, NewPlayer);
}

void AMultiplayerExampleGameMode::PerformInitialSpawn(AController* Controller, FCharacterData Character)
{
	if (Controller->GetPawn())
	{
		return;
	}
	AMPlayerController* PC = CastChecked<AMPlayerController>(Controller);

	if (Controller && !Controller->IsPendingKill())
	{
		if (!Character.IsValid())
		{
			PC->Client_DisconnectAndGotoLoginScreen();
		}

		AActor* StartSpot = FindPlayerStart(Controller);
		if (!StartSpot)
		{
			if (Controller->StartSpot.IsValid())
			{
				StartSpot = Controller->StartSpot.Get();
				UE_LOG(LogGameMode, Warning, TEXT("PerformFirstPlayerSpawn: Player start not found, using last start spot"));
			}
		}

		if (!StartSpot)
		{
			UE_LOG(LogGameMode, Error, TEXT("PerformFirstPlayerSpawn: Failed to find a player spot, aborting"));
			PC->Client_DisconnectAndGotoLoginScreen();
		}

		const FRotator SpawnRotation = StartSpot->GetActorRotation();
		UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart %s"), (Controller && Controller->PlayerState) ? *Controller->PlayerState->GetPlayerName() : TEXT("Unknown"));

		APawn* Pawn = nullptr;
		if (GetDefaultPawnClassForController(Controller))
		{
			Pawn = SpawnDefaultPawnFor(Controller, StartSpot);
		}

		if (Pawn)
		{
			Controller->SetPawn(Pawn);
		}
		Controller->Possess(Pawn);

		if (!Controller->GetPawn())
		{
			Controller->FailedToSpawnPawn();
			PC->Client_DisconnectAndGotoLoginScreen();
			return;
		}

		Controller->ClientSetRotation(Controller->GetPawn()->GetActorRotation(), true);

		FRotator NewControllerRot = SpawnRotation;
		NewControllerRot.Roll = 0.f;
		Controller->SetControlRotation(NewControllerRot);

		if (AMPlayerState* PS = Controller->GetPlayerState<AMPlayerState>())
		{
			PS->SetCharacterData(Character);
			PS->OnRep_CharacterData();
			ChangeName(Controller, Character.Name, true);
		}
		else
		{
			UE_LOG(LogGameMode, Error, TEXT("PerformFirstPlayerSpawn: Failed to cast to PlayerCharacter"));
			Controller->FailedToSpawnPawn();
			PC->Client_DisconnectAndGotoLoginScreen();
			return;
		}

		SetPlayerDefaults(Pawn);
		K2_OnRestartPlayer(Controller);
	}
}

void AMultiplayerExampleGameMode::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	GEngine->BlockTillLevelStreamingCompleted(GetWorld());
	GetWorldSettings()->NotifyBeginPlay();
	GetWorldSettings()->NotifyMatchStarted();

	const FString BugLocString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugLoc"));
	const FString BugRotString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugRot"));
	if (!BugLocString.IsEmpty() || !BugRotString.IsEmpty())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->CheatManager != nullptr)
			{
				PlayerController->CheatManager->BugItGoString(BugLocString, BugRotString);
			}
		}
	}

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}
}
