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

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "Types/ApiTypes.h"
#include "Types/GlobalTypes.h"

#include "MGameInstance.generated.h"

namespace MGameInstanceState
{
	/*
	 *	This is a very basic example of how you could hand state changes for your game instance.
	 *	This shouldn't be looked at as "the way" to do it, just a starting point.
	 *	All of the state demonstrated are client side only
	 *	However the state machine itself can handle both server side and client side states
	 **/
	
	/*
	*	Sends the player to the login menu if they are not already there.
	**/
	extern const FName STATE_Login;
	
	/*
	*	Sends the player to the Character select screen and requests the character list
	**/
	extern const FName STATE_CharacterSelect;
	
	/*
	*	Connects the player to the server and sets them up to begin play
	**/
	extern const FName STATE_Playing;

	/*
	 *	Disconnects the player from the server and updates their inventory with the backend before exiting the game
	 **/
	extern const FName STATE_Quit;

	extern const FName STATE_None;

	inline bool IsValidState(const FName& InState)
	{
		return InState == STATE_Login			||
			   InState == STATE_CharacterSelect ||
			   InState == STATE_Playing			|| 
			   InState == STATE_Quit;
	}
}

UENUM(BlueprintType)
enum class EMenuMap : uint8
{
	Login,
	CharacterSelect,
};

UCLASS()
class UMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UMGameInstance();

	UFUNCTION(BlueprintPure, Category = "Debug")
	bool IsDebugMode() const;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugModeEnabled(const bool bNewDebug);
	
	UFUNCTION(BlueprintPure, Category = "Auth")
	FORCEINLINE bool HasValidToken() const { return LoginToken.IsValid(); }

	FORCEINLINE void SetNewToken(const FLoginResponse NewToken) { LoginToken = NewToken; }
	FORCEINLINE const FLoginResponse& GetToken() const { return LoginToken; }

	void UpdateCharacterList(const FCharacterData& NewCharacter);
	FORCEINLINE void UpdateCharacterList(const TArray<FCharacterData>& NewList) { CharacterList = NewList; }
	
	UFUNCTION(BlueprintPure, Category = "Character")
	FORCEINLINE TArray<FCharacterData> GetCharacterList() const { return CharacterList; }

	UFUNCTION(BlueprintCallable)
	void LogoutAndReturnToMenu();

	UFUNCTION(BlueprintPure)
	FString GetMenuMap(const EMenuMap Map) const { return *MenuMaps.Find(Map); }

	UFUNCTION(BlueprintPure)
	bool IsInMenus() const;

	/************************************************************************/
	/* State Machine                                                        */
	/************************************************************************/

	UFUNCTION(BlueprintPure, Category = "State Machine")
	FName GetCurrentState() const;

	void GotoStateFast(FName State);

	UFUNCTION(BlueprintCallable, Category = "State Machine", meta = (DisplayName="GotoState"))
	void GotoStateSafe(FGameplayTag State);

	UFUNCTION(BlueprintPure, Category = "State Machine")
	FName GetInitialState() const;

	void GotoInitialState();

public:

	/*
	 *	The character ID the player wants to play. The server validates this ID when a login request happens
	 **/
	UPROPERTY(BlueprintReadOnly)
	FString RequestedCharacter;

protected:

	UPROPERTY(EditDefaultsOnly)
	TMap<EMenuMap, FString> MenuMaps;

	virtual void Init() override;

	/************************************************************************/
	/* State Machine                                                        */
	/************************************************************************/

	void ChangeState(FName State);

	/*
	*	Will end the current state and fire delegates so game code has a chance to handle any state-based clean-up BEFORE it ends
	**/
	void EndCurrentState();

	void BeginNewState();
	
	void BeginLoginState();
	void BeginCharacterSelectState();
	
	void EndLoginScreenState();
	void EndCharacterSelectState();

private:

	UPROPERTY(Transient)
	TArray<FCharacterData> CharacterList;

	UPROPERTY()
	FLoginResponse LoginToken;

	UPROPERTY(EditDefaultsOnly)
	uint8 bDebugEnabled:1;

	FName InitialState;

	UPROPERTY(Transient)
	FName CurrentState;
	
	UPROPERTY(Transient)
	FName PendingState;
	
};
