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

#include "Async/Async_CreateCharacter.h"

#include "Core/HttpApi.h"
#include "Core/MGameInstance.h"

UAsync_CreateCharacter* UAsync_CreateCharacter::WaitCreateCharacter(UMGameInstance* GameInstance, FCreateCharacterRequest CharacterName)
{
	auto* Node = NewObject<UAsync_CreateCharacter>();
	Node->NewCharacter = CharacterName;
	Node->GI = GameInstance;
	return Node;
}

void UAsync_CreateCharacter::Activate()
{
#if !UE_SERVER
	if (UHttpAPI* API = GI->GetSubsystem<UHttpAPI>())
	{
		URequest* Request = API->CreateNewRequest("createCharacter");
		API->SetHeaders(Request);
		API->SetAuthHeader(Request, GI->GetToken().IdToken);
		API->POST<FCreateCharacterRequest>(Request, &NewCharacter);

		if (GI->IsDebugMode())
		{
			API->DebugRequest(Request);
		}

		UHttpAPI::BindLambdaResponse(Request, [&](FHttpRequestPtr, FHttpResponsePtr Response, bool)
		{
			if (UHttpAPI::ValidateResponse(Response))
			{
				FCharacterData NewChar;
				NewChar.Name = NewCharacter.Name;
				NewChar.Level = 1;
				NewChar.Inventory.Empty();
				GI->UpdateCharacterList(NewChar);
				OnComplete(true, GI->GetCharacterList());
			}
			else
			{
				OnComplete(false, GI->GetCharacterList());
			}
		});
	}
#endif
}

void UAsync_CreateCharacter::OnComplete(const bool& bSuccessful, const TArray<FCharacterData>& CharacterList)
{
	OnCreateCharacterComplete.Broadcast(bSuccessful, CharacterList);
}
