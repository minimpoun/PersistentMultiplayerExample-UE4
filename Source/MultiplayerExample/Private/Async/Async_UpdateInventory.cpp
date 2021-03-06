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

#include "Async/Async_UpdateInventory.h"

#include "Core/HttpApi.h"
#include "Core/MGameInstance.h"
#include "Interfaces/IHttpResponse.h"
#include "Player/MPlayerController.h"
#include "Player/MPlayerState.h"

UAsync_UpdateInventory* UAsync_UpdateInventory::WaitUpdateInventory(AController* Caller, const FUpdateInventoryRequest& NewItem)
{
	auto* Node = NewObject<UAsync_UpdateInventory>();
	Node->Controller = Caller;
	Node->UpdateInventoryRequest = NewItem;
	return Node;
}

void UAsync_UpdateInventory::Activate()
{
#if UE_SERVER || UE_EDITOR
	UMGameInstance* GI = Controller->GetGameInstance<UMGameInstance>();
	if (UHttpAPI* API = GI->GetSubsystem<UHttpAPI>())
	{
		URequest* Request = API->CreateNewRequest(TEXT("updateInventory"));
		API->SetHeaders(Request);
		API->SetAuthHeader(Request, GI->GetToken().IdToken);
		API->POST<FUpdateInventoryRequest>(Request, &UpdateInventoryRequest);

		if (GI->IsDebugMode())
		{
			UHttpAPI::DebugRequest(Request);
		}

		UHttpAPI::BindLambdaResponse(Request, [&](FHttpRequestPtr, FHttpResponsePtr Response, bool)
		{
			if (GI->IsDebugMode())
			{
				UHttpAPI::DebugResponse(Response);
			}

			if (UHttpAPI::ValidateResponse(Response))
			{
				TArray<FInventoryJson> Inventory;
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
				const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
				if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
				{
					const TArray<TSharedPtr<FJsonValue>> ObjArray = JsonObject->GetArrayField(TEXT("data"));
					FJsonObjectConverter::JsonArrayToUStruct(ObjArray, &Inventory, 0, 0);
				}

				OnComplete(Inventory);
			}

			if (AMPlayerController* PC = Cast<AMPlayerController>(Controller))
				OnComplete(PC->GetPlayerState<AMPlayerState>()->GetInventory());
		});
	}
#endif
}

void UAsync_UpdateInventory::OnComplete(const TArray<FInventoryJson> NewInventory)
{
	OnUpdateInventoryComplete.Broadcast(NewInventory);
}
