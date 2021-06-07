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

#include "Async/Async_Login.h"

#include "JsonObjectConverter.h"
#include "Core/HttpApi.h"
#include "Core/MGameInstance.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UAsync_Login* UAsync_Login::WaitGoogleLogin(UMGameInstance* InGI, const FUserCredentials& InUserCredentials)
{
	UAsync_Login* Node = NewObject<UAsync_Login>();
	Node->GameInstance = InGI;
	Node->UserCredentials = InUserCredentials;
	return Node;
}

void UAsync_Login::Activate()
{
	if (UserCredentials.IsValid() && !GameInstance->HasValidToken())
	{
		if (UHttpAPI* API = GameInstance->GetSubsystem<UHttpAPI>())
		{
			URequest* Request = API->CreateLoginRequest();
			if (Request->GetStatus() != EHttpRequestStatus::Processing)
			{
				API->SetHeaders(Request);
				API->POST<FUserCredentials>(Request, &UserCredentials);
				
				if (GameInstance->IsDebugMode())
				{
					API->DebugRequest(Request);
				}
				
				API->BindLambdaResponse(Request, [&](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
				{
					if (API->ValidateResponse(Response))
					{
						FString Json = Response->GetContentAsString();
						FLoginResponse NewCredentials;
						if (!FJsonObjectConverter::JsonObjectStringToUStruct<FLoginResponse>(Json, &NewCredentials, 0, 0))
						{
							this->Error = Json;
							bSuccess = false;
						}
						
						if (GameInstance->IsDebugMode())
						{
							UE_LOG(LogTemp, Display, TEXT("UAsync_Login: %s success %d"), *Json, bSuccess);
						}
						
						LoginResponse = NewCredentials;
						GameInstance->SetNewToken(NewCredentials);
					}

					this->bSuccessful = bSuccess;
					ExecuteLogin();
				});
			}
		}
	}
}

void UAsync_Login::ExecuteLogin() const
{
	OnLoginComplete.Broadcast(bSuccessful, LoginResponse, Error);
}
