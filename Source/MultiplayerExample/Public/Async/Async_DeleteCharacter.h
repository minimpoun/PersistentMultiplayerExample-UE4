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

#include "Kismet/BlueprintAsyncActionBase.h"
#include "Types/ApiTypes.h"
#include "Types/GlobalTypes.h"

#include "Async_DeleteCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeleteCharacterComplete, bool, bSuccessful,
                                             const TArray<FCharacterData>&, CharacterList);

class UMGameInstance;

UCLASS()
class UAsync_DeleteCharacter : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnDeleteCharacterComplete OnDeleteCharacterComplete;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Example | Async API", meta = (BlueprintInternalUseOnly = true))
	static UAsync_DeleteCharacter* WaitDeleteCharacter(UMGameInstance* GameInstance, const FDeleteCharacterRequest& CharacterId);

	virtual void Activate() override;

protected:

	UFUNCTION()
	void OnComplete(const bool& bSuccessful, const TArray<FCharacterData>& CharacterList);

private:

	UPROPERTY()
	UMGameInstance* GI;

	UPROPERTY()
	FDeleteCharacterRequest DeleteCharacterRequest;
};