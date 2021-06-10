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

#include "Player/MPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMPlayerState::Server_AddInventoryItem_Implementation()
{
	FInventoryJson NewItem;
	NewItem.ItemCount = 1;
	NewItem.ItemId = "pumpkin";

	Inventory.Add(NewItem);
}

bool AMPlayerState::Server_AddInventoryItem_Validate()
{
	return true;
}

void AMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AMPlayerState, Inventory, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AMPlayerState, CharacterData, COND_None, REPNOTIFY_Always);
}

void AMPlayerState::SetCharacterData(FCharacterData InData)
{
	if (GetLocalRole() < ROLE_Authority) return;
	UE_LOG(LogTemp, Warning, TEXT("data: %s"), *InData.ToString());
	CharacterData = InData;
	Inventory = CharacterData.Inventory;
	OnRep_CharacterData();
}

void AMPlayerState::OnRep_CharacterData()
{
	UE_LOG(LogTemp, Warning, TEXT("name: %s"), *CharacterData.Name);
	SetPlayerName(CharacterData.Name);
}

void AMPlayerState::OnRep_InventoryChanged()
{
	OnInventoryChanged.Broadcast(Inventory);
}
