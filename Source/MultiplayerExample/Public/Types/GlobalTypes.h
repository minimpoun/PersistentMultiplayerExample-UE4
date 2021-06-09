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
#include "GlobalTypes.generated.h"

USTRUCT(BlueprintType)
struct FInventoryJson
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemCount;

	UPROPERTY(BlueprintReadOnly)
	FString ItemId;
};

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	int32 Level;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInventoryJson> Inventory;

	UPROPERTY(BlueprintReadOnly)
	FString ID;

	FString ToString() const
	{
		FString Out, InventoryString;

		for (auto Item: Inventory)
		{
			InventoryString += "ID: " + Item.ItemId + " : ";
			InventoryString += "Count: " + FString::FromInt(Item.ItemCount) + "\n";
		}
		
		Out += "Name: " + Name + " : ";
		Out += "Level: " + FString::FromInt(Level) + " : ";
		Out += "Inventory: " + InventoryString;
		return Out;
	}
};