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

#include "Types/GlobalTypes.h"
#include "ApiTypes.generated.h"

USTRUCT(BlueprintType)
struct FUpdateInventoryRequest
{
	GENERATED_BODY()


	UPROPERTY(BlueprintReadWrite)
	FString id;

	UPROPERTY(BlueprintReadWrite)
	FInventoryJson NewItem;
	
};

USTRUCT(BlueprintType)
struct FDeleteCharacterRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString id;
};

USTRUCT(BlueprintType)
struct FGetCharacterRequest
{
	GENERATED_BODY()

	FGetCharacterRequest() {}
	explicit FGetCharacterRequest(const FString& InID)
		: id(InID) {}
	
	UPROPERTY(BlueprintReadOnly)
	FString id;
};

USTRUCT(BlueprintType)
struct FLoginResponse
{
	GENERATED_BODY()

	FLoginResponse()
	{
		Kind = "";
		LocalID = "";
		Email = "";
		DisplayName = "";
		IdToken = "";
		Registered = false;
		RefreshToken = "";
		ExpiresIn = -1;
	}

	FORCEINLINE bool IsValid() const { return !Email.IsEmpty() && !IdToken.IsEmpty() && Registered && !RefreshToken.IsEmpty(); }

	UPROPERTY()
	FString Kind;

	UPROPERTY(BlueprintReadOnly)
	FString LocalID;

	UPROPERTY(BlueprintReadOnly)
	FString Email;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FString IdToken;

	UPROPERTY()
	bool Registered;
	
	UPROPERTY()
	FString RefreshToken;

	UPROPERTY()
	int32 ExpiresIn;
};

USTRUCT(BlueprintType)
struct FUserCredentials
{
	GENERATED_BODY()

public:

	FUserCredentials(){}

	FUserCredentials(FString NewID, FString NewPass)
		: Email(NewID)
		, Password(NewPass)
	{}

	FORCEINLINE bool IsValid() const { return !Email.IsEmpty() && !Password.IsEmpty(); }

	UPROPERTY(BlueprintReadWrite)
	FString Email;

	UPROPERTY(BlueprintReadWrite)
	FString Password;

	UPROPERTY()
	bool ReturnSecureToken = true;
};

USTRUCT(BlueprintType)
struct FCreateCharacterRequest
{
	GENERATED_BODY()

	FCreateCharacterRequest(){}
	explicit FCreateCharacterRequest(const FString In) : Name(In) {}

	UPROPERTY(BlueprintReadWrite)
	FString Name;
};