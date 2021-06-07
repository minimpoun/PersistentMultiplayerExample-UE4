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
#include "Async_Login.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoginComplete, bool, bSuccess, FLoginResponse, LoginResponse, FString, Error);

class UMGameInstance;

UCLASS()
class MULTIPLAYEREXAMPLE_API UAsync_Login : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnLoginComplete OnLoginComplete;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = true))
	static UAsync_Login* WaitGoogleLogin(UMGameInstance* InGI, const FUserCredentials& InUserCredentials);

	virtual void Activate() override;

protected:

	UFUNCTION()
	void ExecuteLogin() const;

private:
	
	UPROPERTY()
	UMGameInstance* GameInstance;

	UPROPERTY()
	FUserCredentials UserCredentials;

	UPROPERTY()
	bool bSuccessful;

	UPROPERTY()
	FString Error;

	UPROPERTY()
	FLoginResponse LoginResponse;
};
