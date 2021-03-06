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

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Types/GlobalTypes.h"

#include "MPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FInventoryJson>&);

UCLASS()
class AMPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintAuthorityOnly, BlueprintImplementableEvent)
	void SyncPlayerState();

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void Server_AddInventoryItem();

	FOnInventoryChanged OnInventoryChanged;

	TArray<FInventoryJson> GetInventory() const { return Inventory; }

	void SetCharacterData(FCharacterData InData);
	const FCharacterData& GetCharacterData() const { return CharacterData; }

	UFUNCTION(BlueprintPure, meta = (DisplayName="GetCharacterData"))
	FCharacterData K2_GetCharacterData() const { return CharacterData; }

	UFUNCTION()
	void OnRep_CharacterData();

protected:
	
	UPROPERTY(ReplicatedUsing=OnRep_InventoryChanged)
	TArray<FInventoryJson> Inventory;

	UFUNCTION()
	void OnRep_InventoryChanged();

	UFUNCTION()
	void OnUpdatedInventory(const TArray<FInventoryJson>& UpdatedInventory);

private:

	UPROPERTY(ReplicatedUsing=OnRep_CharacterData)
	FCharacterData CharacterData;
	
};