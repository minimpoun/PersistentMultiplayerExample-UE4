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
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpModule.h"
#include "Engine/EngineTypes.h"
#include "JsonObjectConverter.h"
#include "HttpApi.generated.h"

UENUM()
enum class EContentType : uint8
{
	urlencoded,
	json,
	text,
};


USTRUCT()
struct FRoute
{
	GENERATED_BODY()

	FRoute()
	{
		API = "bad route";
		Login = "bad route";
	}

	FRoute(const FString InAPI, const FString InLogin)
		: API(InAPI)
		, Login(InLogin)
	{}

	FString GetAPIRoute() const { return API; }
	FString GetLoginRoute() const { return Login; }

private:

	UPROPERTY()
	FString API;

	UPROPERTY()
	FString Login;
};

UCLASS()
class URequest : public UObject
{
	GENERATED_BODY()

public:

	enum EVerb
	{
		POST,
		GET,
		DELETE,
		HEAD,
		PUT,
	};

	void SetRequestName(const FName& Name);
	FORCEINLINE FName GetRequestName() const { return RequestName; }

	void SetVerb(EVerb Verb) const;
	void SetURL(const FString& URL) const;
	void SetContent(const FString& Content) const;
	void SetHeader(const FString& HeaderName, const FString& HeaderValue) const;
	void ProcessRequest() const;
	EHttpRequestStatus::Type GetStatus() const;
	TArray<FString> GetHeaders() const;
	FHttpResponsePtr GetResponse() const;
	float GetElapsedTime() const;
	void CancelRequest() const;

	bool IsComplete() const;

	bool IsValid() const;

	FHttpRequestCompleteDelegate& OnProcessRequestComplete() const;
	FHttpRequestProgressDelegate& OnRequestProgress() const;
	FHttpRequestHeaderReceivedDelegate& OnHeaderReceived() const;

private:

	friend class UHttpAPI;

	static FString GetVerbString(EVerb InVerb);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	UPROPERTY()
	FName RequestName;

	UPROPERTY()
	bool bResponseHandled;
};

USTRUCT()
struct FAuthToken
{
	GENERATED_BODY()

	FAuthToken() = default;
	FAuthToken(const FString& InToken) : Token(InToken) {}

	FORCEINLINE bool IsValid() const { return !Token.IsEmpty() && GetToken().Len() > 7; }
	FORCEINLINE FString GetToken() const { return "Bearer " + Token; }

protected:

	UPROPERTY()
	FString Token;
};

UCLASS()
class MULTIPLAYEREXAMPLE_API UHttpAPI : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY()
	float UpdateFreq = 15.f;

	URequest* CreateNewRequest(const FString& Subroute, bool bExplicitURL = false);
	URequest* CreateLoginRequest();

	void SetHeaders(URequest* InRequest) const;
	static void SetHeaders(URequest* InRequest, EContentType ContentType);

	static void SetAuthHeader(URequest* InRequest, const FAuthToken& Hash);

	/*
	 *	Will clear a request by name.
	 *	WARNING: This does NOT go off of the UID. This will clear ALL requests with this name
	 **/
	void ClearRequest(const FName& RequestName);

	/*
	 *	Will clear the specific request
	 **/
	void ClearRequest(URequest* Request);

	void ClearAllRequests();

	static bool ValidateResponse(FHttpResponsePtr Response);

	static void BindLambdaResponse(URequest* InRequest, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> LambdaFunctor);

	template<typename ContentType>
	void POST(URequest* InRequest, const ContentType& Payload);

	template<typename ContentType>
	void POST(URequest* InRequest, const ContentType* Payload);

	template<typename ContentType>
	void POST(URequest* InRequest, ContentType Payload);

	template<typename ContentType>
	void DELETE(URequest* InRequest, const ContentType& Payload);

	template<typename ContentType>
	void DELETE(URequest* InRequest, const ContentType* Payload);

	template<typename ContentType>
	void DELETE(URequest* InRequest, ContentType Payload);

	static void GET(URequest* InRequest);

	template<typename ContentType>
	static ContentType ToStruct(const FString& FromString);

	template<typename ContentType>
	static FString FromStruct(const ContentType& FromStruct);

	static void DebugRequest(const URequest* InRequest);

	UFUNCTION(BlueprintCallable)
	void DebugAllRequests();

	static void DebugResponse(const FHttpResponsePtr Response);

	/*
	 *	Returns an array of all requests that match the name
	 *	Can lead to false positives if the request is cleared before ActiveRequests is updated
	 **/
	TArray<URequest*> FindRequest(const FName& RequestName);

	/*
	 *	Will attempt to see if a request matching RequestName is currently active
	 *	Can lead to false positives if the request is cleared before ActiveRequests is updated
	 **/
	bool RequestExists(const FName& RequestName);

protected:
	
	static bool IsValidSubroute(const FString& In);
	void UpdateActiveRequests();

	static void POSTImpl(URequest* InRequest, const FString& Payload);
	static void DELETEImpl(URequest* InRequest, const FString& Payload);

	static void ProcessRequest(URequest* const InRequest);

	static FString GetContentType(EContentType C);

private:
	
	UPROPERTY()
	TArray<URequest*> ActiveRequests;

	UPROPERTY()
	FRoute Route;

	UPROPERTY()
	FTimerHandle RequestUpdate_TimerHandle;
};

template<typename ContentType>
void UHttpAPI::POST(URequest* InRequest, const ContentType& Payload)
{
	if (InRequest)
	{
		POSTImpl(InRequest, FromStruct<ContentType>(Payload));
	}
}

template<typename ContentType>
void UHttpAPI::POST(URequest* InRequest, const ContentType* Payload)
{
	if (InRequest)
	{
		POSTImpl(InRequest, FromStruct<ContentType>(*Payload));
	}
}

template<typename ContentType>
void UHttpAPI::POST(URequest* InRequest, ContentType Payload)
{
	if (InRequest)
	{
		POSTImpl(InRequest, FromStruct<ContentType>(Payload));
	}
}

template<>
inline void UHttpAPI::POST<FString>(URequest* InRequest, FString Payload)
{
	if (InRequest)
	{
		POSTImpl(InRequest, Payload);
	}
}

template<typename ContentType>
void UHttpAPI::DELETE(URequest* InRequest, const ContentType& Payload)
{
	if (InRequest)
	{
		DELETEImpl(InRequest, FromStruct<ContentType>(Payload));
	}
}

template<typename ContentType>
void UHttpAPI::DELETE(URequest* InRequest, const ContentType* Payload)
{
	if (InRequest)
	{
		DELETEImpl(InRequest, FromStruct<ContentType>(*Payload));
	}
}

template<typename ContentType>
void UHttpAPI::DELETE(URequest* InRequest, ContentType Payload)
{
	if (InRequest)
	{
		DELETEImpl(InRequest, FromStruct<ContentType>(Payload));
	}
}

template<typename ContentType>
ContentType UHttpAPI::ToStruct(const FString& FromString)
{
	if (!FromString.IsEmpty())
	{
		ContentType Out;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct(FromString, &Out, 0, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to convert %s to a USTRUCT. Make sure the struct contains the require properties."), *FromString);
			return Out;
		}

		return Out;
	}


	UE_LOG(LogTemp, Warning, TEXT("The FromString was empty"));
	return ContentType();
}

template<typename ContentType>
FString UHttpAPI::FromStruct(const ContentType& FromStruct)
{
	FString Out;
	if (!FJsonObjectConverter::UStructToJsonObjectString(FromStruct, Out))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to convert a struct into a string"));
		return TEXT("");
	}

	return Out;
}