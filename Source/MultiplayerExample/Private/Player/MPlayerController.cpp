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

#include "Player/MPlayerController.h"

#include "Async/Async_GetCharacter.h"
#include "Core/MBaseGameMode.h"
#include "Core/MGameInstance.h"
#include "Player/MPlayerState.h"
#include "UserInterface/HUDs/MGameHUD.h"

void AMPlayerController::Client_OnConnectionComplete_Implementation()
{
	auto* const GI = GetGameInstance<UMGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("Couldn't get the correct GameInstance, client can't get selected character for replication."));
		return;
	}

	const FString CharacterID = GI->RequestedCharacter;
	if (CharacterID.IsEmpty())
	{
		UE_LOG(LogTemp, Error,
		       TEXT("No RequestedCharacter in GameInstance, can't send selected character to server for validation."));
		return;
	}

	Server_GetCharacter(CharacterID, GI->GetToken().IdToken);
}

void AMPlayerController::Client_DisconnectAndGotoLoginScreen_Implementation()
{
	if (auto* GI = GetGameInstance<UMGameInstance>())
	{
		GI->GotoStateFast(MGameInstanceState::STATE_Login);
	}
}

void AMPlayerController::Server_GetCharacter_Implementation(const FString& CharacterID, const FString& BearerToken)
{
	auto* GetCharacter = UAsync_GetCharacter::WaitGetCharacter(GetGameInstance<UMGameInstance>(),
	                                                           FGetCharacterRequest(CharacterID), BearerToken);
	GetCharacter->OnGetCharacterComplete.AddDynamic(this, &ThisClass::OnGetCharacterCallback);
	GetCharacter->Activate();
}

void AMPlayerController::OnGetCharacterCallback(const FCharacterData& CharacterData)
{
	if (!CharacterData.IsValid())
	{
		Client_DisconnectAndGotoLoginScreen();
		return;
	}

	check(GetWorld());

	if (GetWorld()->GetAuthGameMode<AMultiplayerExampleGameMode>())
	{
		FExampleGameModeEvents::ReadyToSpawnPlayerEvent.Broadcast(this, CharacterData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s wasn't called on the server"), __func__);
	}
}

bool AMPlayerController::Server_GetCharacter_Validate(const FString& CharacterID, const FString& BearerToken)
{
	if (CharacterID.IsEmpty() || BearerToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterID or Bearer Token were invalid on server. Can't get character"));
		Client_DisconnectAndGotoLoginScreen();
		return false;
	}

	return true;
}

void AMPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (AMPlayerState* PS = GetPlayerState<AMPlayerState>())
	{
		PS->OnInventoryChanged.AddUObject(this, &ThisClass::PushInventoryToUserInterface);
	}
}

void AMPlayerController::PushInventoryToUserInterface(const TArray<FInventoryJson>& Inventory)
{
	if (AGameHUD* GameHUD = GetHUD<AGameHUD>())
	{
		GameHUD->ReceiveInventory(Inventory);
	}
}
