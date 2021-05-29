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

#include "Engine/GameInstance.h"
#include "Types/ApiTypes.h"
#include "MGameInstance.generated.h"

UCLASS()
class UMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UMGameInstance();

	UFUNCTION(BlueprintPure)
	bool IsDebugMode() const;

	UFUNCTION(BlueprintCallable)
	void SetDebugModeEnabled(const bool& bNewDebug);
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasValidToken() const { return LoginToken.IsValid(); }

	FORCEINLINE void SetNewToken(const FLoginResponse NewToken) { LoginToken = NewToken; }

private:

	UPROPERTY(Transient)
	FLoginResponse LoginToken;

	UPROPERTY(Transient)
	uint8 bDebugEnabled:1;
};