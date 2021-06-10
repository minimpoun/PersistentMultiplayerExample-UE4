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

#include "UserInterface/Widgets/CharacterSelectWidget.h"
#include "Core/MGameInstance.h"

void UCharacterSelectWidget::PlaySelectedCharacter(const FString& CharacterID, const bool bForceLocal)
{
	if (UMGameInstance* GI = GetGameInstance<UMGameInstance>())
	{
		GI->RequestedCharacter = CharacterID;

		if (bForceLocal)
		{
			GetOwningPlayer()->ClientTravel(TEXT("127.0.0.1"), ETravelType::TRAVEL_Absolute, false);
			return;
		}
		
		FConfigFile GameConfig;
		if (FConfigCacheIni::LoadLocalIniFile(GameConfig, TEXT("DefaultGame"), false))
		{
			FString TravelURL = "127.0.0.1";
			if (!GameConfig.GetString(TEXT("ServerAddress"), TEXT("Address"), TravelURL))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to get the server address"));
				return;
			}

			GetOwningPlayer()->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute, false);
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to travel to the server"));
}
