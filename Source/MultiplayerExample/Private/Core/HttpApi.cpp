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

#include "Core/HttpApi.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interfaces/IHttpResponse.h"

void UHttpAPI::Initialize(FSubsystemCollectionBase& Collection)
{
	FConfigFile GameConfig;
	if (!FConfigCacheIni::LoadLocalIniFile(GameConfig, TEXT("DefaultGame"), false))
	{
		UE_LOG(LogTemp, Fatal, TEXT("Failed to load DefaultGame ini, HTTP API route not initialized."));
		return;
	}

	FString APIRoute = "bad route";
	if (!GameConfig.GetString(TEXT("HttpApiDefaults"), TEXT("APIRoute"), APIRoute))
	{
		UE_LOG(LogTemp, Fatal, TEXT("Failed to find the API route in the DefaultGame ini"));
		return;
	}

	FString LoginRoute = "bad route";
	if (!GameConfig.GetString(TEXT("HttpApiDefaults"), TEXT("LoginRoute"), LoginRoute))
	{
		UE_LOG(LogTemp, Fatal, TEXT("Failed to find the login route in the DefaultGame ini"));
		return;
	}

	Route = FRoute(APIRoute, LoginRoute);

	GetWorld()->GetTimerManager().SetTimer(RequestUpdate_TimerHandle, this, &ThisClass::UpdateActiveRequests, UpdateFreq, true);
}

void UHttpAPI::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(RequestUpdate_TimerHandle);
}

URequest* UHttpAPI::CreateNewRequest(const FString& Subroute, bool bExplicitURL)
{
	FString URL = "";

	if (bExplicitURL)
	{
		URL = Subroute;
	}
	else if (IsValidSubroute(Subroute))
	{
		URL = Route.GetAPIRoute() + Subroute;
	}

	URequest* NewRequest = NewObject<URequest>();
	Subroute + "_" + FString::FromInt(NewRequest->GetUniqueID());
	const FName RequestName = FName(*Subroute);
	NewRequest->SetRequestName(RequestName);
	NewRequest->SetURL(URL);
	ActiveRequests.Emplace(NewRequest);
	return NewRequest;
}

URequest* UHttpAPI::CreateLoginRequest()
{
	if (RequestExists(FName(TEXT("login"))))
	{
		return FindRequest(FName(TEXT("login")))[0];
	}

	URequest* NewRequest = NewObject<URequest>();
	NewRequest->SetRequestName(TEXT("login"));
	NewRequest->SetURL(Route.GetLoginRoute());
	ActiveRequests.Emplace(NewRequest);
	return NewRequest;
}

void UHttpAPI::SetHeaders(URequest* InRequest) const
{
	if (InRequest)
	{
		SetHeaders(InRequest, EContentType::json);
	}
}

void UHttpAPI::SetHeaders(URequest* InRequest, const EContentType ContentType)
{
	if (InRequest)
	{
		InRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		InRequest->SetHeader(TEXT("Content-Type"), GetContentType(ContentType));
		InRequest->SetHeader(TEXT("Accept"), GetContentType(ContentType));
	}
}

void UHttpAPI::SetAuthHeader(URequest* InRequest, const FAuthToken& Hash)
{
	if (InRequest && Hash.IsValid())
	{
		InRequest->SetHeader(TEXT("Authorization"), Hash.GetToken());
	}
}

void UHttpAPI::ClearRequest(const FName& RequestName)
{
	if (!RequestName.IsNone())
	{
		TArray<URequest*> RequestsPendingDelete = FindRequest(RequestName);
		for (URequest* Element : RequestsPendingDelete)
		{
			FString CheckName = Element->GetRequestName().ToString();
			const FString Name = CheckName.LeftChop(CheckName.Find("_"));
			if (Element && FName(*Name) == RequestName)
			{
				ClearRequest(Element);
			}
		}
	}
}

void UHttpAPI::ClearRequest(URequest* Request)
{
	if (Request)
	{
		if (!Request->IsComplete())
		{
			Request->CancelRequest();
		}

		UE_LOG(LogTemp, Display, TEXT("Cleared Request: %s"), *Request->GetRequestName().ToString());
		ActiveRequests.Remove(Request);
		Request->ConditionalBeginDestroy();
	}
}

void UHttpAPI::ClearAllRequests()
{
	for (URequest* Element : ActiveRequests)
	{
		ClearRequest(Element);
	}
}

bool UHttpAPI::ValidateResponse(FHttpResponsePtr Response)
{
	bool bReturn = false;

	if (!Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("The response was invalid"));
	}

	if (Response->GetResponseCode() == EHttpResponseCodes::Denied)
	{
		UE_LOG(LogTemp, Warning, TEXT("Response was rejected with error code 401"));
	}

	if (Response->GetContentAsString().IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("The response was empty"));
	}

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Log, TEXT("response valid"));
		bReturn = true;
	}

	UE_LOG(LogTemp, Log, TEXT("Response finished with %d"), Response->GetResponseCode());
	return bReturn;
}

void UHttpAPI::BindLambdaResponse(URequest* InRequest, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> LambdaFunctor)
{
	if (InRequest)
	{
		InRequest->OnProcessRequestComplete().BindLambda(LambdaFunctor);
		InRequest->bResponseHandled = true;
	}
}

void UHttpAPI::GET(URequest* InRequest)
{
	if (InRequest)
	{
		InRequest->SetVerb(URequest::GET);
		InRequest->ProcessRequest();
	}
}

void UHttpAPI::DebugRequest(const URequest* InRequest)
{
	if (InRequest)
	{
#if WITH_EDITOR
		if (GEngine)
		{
			TArray<FString> HeaderStrings = InRequest->GetHeaders();
			for (auto Header : HeaderStrings)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Header: " + Header);
			}

			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Status: " + FString(EHttpRequestStatus::ToString(InRequest->GetStatus())));
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Request Name: " + InRequest->GetRequestName().ToString());
		}
#endif // WITH_EDITOR

		TArray<FString> HeaderStrings = InRequest->GetHeaders();
		for (auto Header : HeaderStrings)
		{
			UE_LOG(LogTemp, Display, TEXT("Header: %s"), *Header);
		}

		UE_LOG(LogTemp, Display, TEXT("Status: %s"), *FString(EHttpRequestStatus::ToString(InRequest->GetStatus())));
		UE_LOG(LogTemp, Display, TEXT("Request Name: %s"), *InRequest->GetRequestName().ToString());
	}
}

void UHttpAPI::DebugAllRequests()
{
	for (URequest* Element : ActiveRequests)
	{
		DebugRequest(Element);
	}
}

void UHttpAPI::DebugResponse(const FHttpResponsePtr Response)
{
	if (Response.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, Response->GetContentAsString());
		}

		UE_LOG(LogTemp, Display, TEXT("%s"), *Response->GetContentAsString());
	}
}

TArray<URequest*> UHttpAPI::FindRequest(const FName& RequestName)
{
	TArray<URequest*> OutRequests;
	for (URequest* const Element : ActiveRequests)
	{
		FString CheckName = Element->GetRequestName().ToString();
		
		if (CheckName.Contains(TEXT("_")))
		{
			CheckName = CheckName.LeftChop(CheckName.Find(TEXT("_")));
		}

		if (FName(*CheckName) == RequestName)
		{
			OutRequests.Add(Element);
		}
	}

	return OutRequests;
}

bool UHttpAPI::RequestExists(const FName& RequestName)
{
	for (const URequest* Element : ActiveRequests)
	{
		FString CheckName = Element->GetRequestName().ToString();
		CheckName = CheckName.LeftChop(CheckName.Find("_"));
		if (FName(*CheckName) == RequestName)
		{
			return true;
		}
	}

	return false;
}

bool UHttpAPI::IsValidSubroute(const FString& In)
{
	return !In.IsEmpty() && !In.StartsWith("/");
}

void UHttpAPI::UpdateActiveRequests()
{
 	for (int32 i = 0; i < ActiveRequests.Num(); ++i)
 	{
 		if (ActiveRequests[i]->IsComplete())
 		{
 			ClearRequest(ActiveRequests[i]);
 			continue;
 		}
 
 		if (ActiveRequests[i]->GetStatus() == EHttpRequestStatus::Failed || ActiveRequests[i]->GetStatus() == EHttpRequestStatus::Failed_ConnectionError)
 		{
 			UE_LOG(LogTemp, Warning, TEXT("The request %s failed with status %s"), *ActiveRequests[i]->GetRequestName().ToString(), *FString(EHttpRequestStatus::ToString(ActiveRequests[i]->GetStatus())));
 			ClearRequest(ActiveRequests[i]);
 		}
 	}
}

void UHttpAPI::POSTImpl(URequest* InRequest, const FString& Payload)
{
	if (InRequest)
	{
		InRequest->SetVerb(URequest::POST);
		InRequest->SetContent(Payload);
		InRequest->ProcessRequest();
	}
}

void UHttpAPI::DELETEImpl(URequest* InRequest, const FString& Payload)
{
	if (InRequest)
	{
		InRequest->SetVerb(URequest::DELETE);
		InRequest->SetContent(Payload);
		InRequest->ProcessRequest();
	}
}

void UHttpAPI::ProcessRequest(URequest* const InRequest)
{
	if (InRequest)
	{
		InRequest->ProcessRequest();
	}
}

FString UHttpAPI::GetContentType(EContentType C)
{
	switch (C)
	{
	case EContentType::urlencoded:	return TEXT("application/x-www-form-urlencoded");
	case EContentType::json:		return TEXT("application/json");
	case EContentType::text:		return TEXT("text/plain");
	default: return TEXT("bad content type");
	}
}

void URequest::SetRequestName(const FName& Name)
{
	RequestName = Name;
}

void URequest::SetVerb(const EVerb Verb) const
{
	Request->SetVerb(GetVerbString(Verb));
}

void URequest::SetURL(const FString& URL) const
{
	Request->SetURL(URL);
}

void URequest::SetContent(const FString& Content) const
{
	Request->SetContentAsString(Content);
}

void URequest::SetHeader(const FString& HeaderName, const FString& HeaderValue) const
{
	Request->SetHeader(HeaderName, HeaderValue);
}

void URequest::ProcessRequest() const
{
	Request->ProcessRequest();
}

EHttpRequestStatus::Type URequest::GetStatus() const
{
	return Request->GetStatus();
}

TArray<FString> URequest::GetHeaders() const
{
	return Request->GetAllHeaders();
}

FHttpResponsePtr URequest::GetResponse() const
{
	return Request->GetResponse();
}

float URequest::GetElapsedTime() const
{
	return Request->GetElapsedTime();
}

void URequest::CancelRequest() const
{
	Request->CancelRequest();
}

bool URequest::IsComplete() const
{
	return !(GetStatus() == EHttpRequestStatus::NotStarted || GetStatus() == EHttpRequestStatus::Processing) && bResponseHandled;
}

bool URequest::IsValid() const
{
	return Request->GetStatus() != EHttpRequestStatus::Failed && Request->GetStatus() != EHttpRequestStatus::Failed_ConnectionError;
}

FHttpRequestCompleteDelegate& URequest::OnProcessRequestComplete() const
{
	return Request->OnProcessRequestComplete();
}

FHttpRequestProgressDelegate& URequest::OnRequestProgress() const
{
	return Request->OnRequestProgress();
}

FHttpRequestHeaderReceivedDelegate& URequest::OnHeaderReceived() const
{
	return Request->OnHeaderReceived();
}

FString URequest::GetVerbString(EVerb InVerb)
{
	switch (InVerb)
	{
		case URequest::POST:	return TEXT("POST");
		case URequest::GET:		return TEXT("GET");
		case URequest::DELETE:	return TEXT("DELETE");
		case URequest::HEAD:	return TEXT("HEAD");
		case URequest::PUT:		return TEXT("PUT");

		default: return TEXT("bad verb");
	}
}
