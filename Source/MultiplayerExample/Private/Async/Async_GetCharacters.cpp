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

#include "Async/Async_GetCharacters.h"

#include "Core/HttpApi.h"
#include "Core/MGameInstance.h"
#include "Interfaces/IHttpResponse.h"
#include "UserInterface/HUDs/MLoginHUD.h"

UAsync_GetCharacters* UAsync_GetCharacters::WaitGetCharacters(ALoginController* InCaller)
{
	auto* Node = NewObject<UAsync_GetCharacters>();
	Node->Caller = InCaller;
	return Node;
}

void UAsync_GetCharacters::Activate()
{
#if !UE_SERVER
	if (Caller->IsLocalController())
	{
		if (UHttpAPI* API = Caller->GetGameInstance()->GetSubsystem<UHttpAPI>())
		{
			URequest* Request = API->CreateNewRequest(TEXT("getAllCharacters"));
			API->SetHeaders(Request);
			API->SetAuthHeader(Request, Caller->GetGameInstance<UMGameInstance>()->GetToken().IdToken);
			API->GET(Request);

			if (Caller->GetGameInstance<UMGameInstance>()->IsDebugMode())
			{
				UHttpAPI::DebugRequest(Request);
			}

			UHttpAPI::BindLambdaResponse(Request, [&](FHttpRequestPtr, FHttpResponsePtr Response, bool)
			{
				if (Caller->GetGameInstance<UMGameInstance>()->IsDebugMode())
				{
					UHttpAPI::DebugResponse(Response);
				}
				
				if (UHttpAPI::ValidateResponse(Response))
				{
					TArray<FCharacterData> CharacterList;
					TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
					const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
					if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
					{
						const TArray<TSharedPtr<FJsonValue>> ObjArray = JsonObject->GetArrayField(TEXT("data"));
						FJsonObjectConverter::JsonArrayToUStruct(ObjArray, &CharacterList, 0, 0);
					}

					if (CharacterList.Num())
					{
						Caller->GetGameInstance<UMGameInstance>()->UpdateCharacterList(CharacterList);
						Caller->GetHUD<ALoginHUD>()->PushCharacterListToWidget(CharacterList);
						this->OnComplete(EResponseType::Success);
					}
					else
					{
						this->OnComplete(EResponseType::Empty);
					}
				}
				else
				{
					this->OnComplete(EResponseType::Failed);
				}
			});
		}
	}
#endif
}

void UAsync_GetCharacters::OnComplete(const EResponseType& ResponseType) const
{
	OnGetCharactersComplete.Broadcast(ResponseType);
}
