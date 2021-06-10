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

#include "Core/MGameInstance.h"

namespace MGameInstanceState
{
	const FName STATE_Login = FName(TEXT("STATE_LoginScreen"));
	const FName STATE_CharacterSelect = FName(TEXT("STATE_CharacterSelect"));
	const FName STATE_Playing = FName(TEXT("STATE_Playing"));
	const FName STATE_Quit = FName(TEXT("STATE_Quit"));
	const FName STATE_None = FName(TEXT("STATE_None"));
}

UMGameInstance::UMGameInstance()
{
	bDebugEnabled = false;
	CurrentState = MGameInstanceState::STATE_None;
}

bool UMGameInstance::IsDebugMode() const
{
#if UE_EDITOR || UE_BUILD_DEBUG
	return bDebugEnabled;
#else
	return false;
#endif
}

void UMGameInstance::SetDebugModeEnabled(const bool bNewDebug)
{
	bDebugEnabled = bNewDebug;
}

void UMGameInstance::UpdateCharacterList(const FCharacterData& NewCharacter)
{
	CharacterList.Add(NewCharacter);
}

void UMGameInstance::LogoutAndReturnToMenu()
{
	
}

bool UMGameInstance::IsInMenus() const
{
	return GetWorld()->GetMapName() == GetMenuMap(EMenuMap::Login) || GetWorld()->GetMapName() == GetMenuMap(
		EMenuMap::CharacterSelect);
}

FName UMGameInstance::GetCurrentState() const
{
	return CurrentState;
}

void UMGameInstance::GotoStateFast(FName State)
{
	ChangeState(State);
}

void UMGameInstance::GotoStateSafe(FGameplayTag State)
{
	const FName StateName = State.GetTagName();
	if (MGameInstanceState::IsValidState(StateName))
	{
		ChangeState(StateName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s tried to enter a invalid state: %s"), __func__, *StateName.ToString());
	}
}

FName UMGameInstance::GetInitialState() const
{
	return InitialState;
}

void UMGameInstance::GotoInitialState()
{
	ChangeState(InitialState);
}

void UMGameInstance::Init()
{
	Super::Init();

	InitialState = MGameInstanceState::STATE_Login;

	CharacterList.Empty();
	LoginToken = FLoginResponse();
}

void UMGameInstance::ChangeState(FName State)
{
	PendingState = State;
	EndCurrentState();
	BeginNewState();
}

void UMGameInstance::EndCurrentState()
{
	if (CurrentState == MGameInstanceState::STATE_Login)
	{
		EndLoginScreenState();
	}
	else if (CurrentState == MGameInstanceState::STATE_CharacterSelect)
	{
		EndCharacterSelectState();
	}
}

void UMGameInstance::BeginNewState()
{
	if (PendingState == MGameInstanceState::STATE_Login)
	{
		BeginLoginState();
	}
	else if (PendingState == MGameInstanceState::STATE_CharacterSelect)
	{
		BeginCharacterSelectState();
	}

	CurrentState = PendingState;
}

void UMGameInstance::BeginLoginState()
{
#if !UE_SERVER
	if (!IsInMenus())
	{
		LoginToken = FLoginResponse();
		CharacterList.Empty();
		
		UWorld* const World = GetWorld();
		check(World);

		UNetDriver* NetDriver = World->GetNetDriver();
		if (NetDriver)
		{
			const FString TravelURL = GetMenuMap(EMenuMap::Login);
			GEngine->HandleDisconnect(World, NetDriver);
			NotifyPreClientTravel(TravelURL, ETravelType::TRAVEL_Absolute, false);
			GEngine->SetClientTravel(World, *TravelURL, ETravelType::TRAVEL_Absolute);
		}
		else
		{
			UE_LOG(LogNet, Fatal, TEXT("Failed to get the games NetDriver--Couldn't client travel"));
		}
	}
#endif
}

void UMGameInstance::BeginCharacterSelectState()
{
#if !UE_SERVER
	const FString TravelURL = GetMenuMap(EMenuMap::CharacterSelect);
	if (GetWorld()->GetMapName() != TravelURL)
	{
		if (LoginToken.IsValid())
		{
			NotifyPreClientTravel(TravelURL, ETravelType::TRAVEL_Absolute, false);
			GEngine->SetClientTravel(GetWorld(), *TravelURL, ETravelType::TRAVEL_Absolute);
		}
		else
		{
			GotoStateFast(MGameInstanceState::STATE_Login);
		}
	}
#endif
}

void UMGameInstance::EndLoginScreenState()
{
#if !UE_SERVER

#endif
}

void UMGameInstance::EndCharacterSelectState()
{
#if !UE_SERVER

#endif
}