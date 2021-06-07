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
#include "GameFramework/HUD.h"
#include "UserInterface/Widgets/CharacterSelectWidget.h"
#include "UserInterface/Widgets/LoginWidget.h"
#include "MLoginHUD.generated.h"

struct FCharacterData;

UCLASS()
class ALoginHUD : public AHUD
{
	GENERATED_BODY()

public:

	template<typename Widget>
	void CreateMenuWidget();
	void RegisterNewLoginRequestWithWidget() const;
	void OnLogin() const;
	void PushCharacterListToWidget(const TArray<FCharacterData>& UpdatedList) const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TSubclassOf<ULoginWidget> LoginScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Menu Widgets")
	TSubclassOf<UCharacterSelectWidget> CharacterSelectWidgetClass;

private:

	UPROPERTY()
	UCharacterSelectWidget* CharacterSelectWidgetPtr;

	UPROPERTY()
	ULoginWidget* LoginWidgetPtr;
};

template <typename Widget>
void ALoginHUD::CreateMenuWidget()
{
	checkNoEntry();
}

template<>
inline void ALoginHUD::CreateMenuWidget<ULoginWidget>()
{
	if (!LoginWidgetPtr && LoginScreenWidgetClass)
	{
		LoginWidgetPtr = CreateWidget<ULoginWidget>(GetOwningPlayerController(), LoginScreenWidgetClass, TEXT("LoginWidget"));
		check(LoginWidgetPtr);
		LoginWidgetPtr->AddToViewport();
		return;
	}

	LoginWidgetPtr->AddToViewport();
}

template<>
inline void ALoginHUD::CreateMenuWidget<UCharacterSelectWidget>()
{
	if (!CharacterSelectWidgetPtr && CharacterSelectWidgetClass)
	{
		CharacterSelectWidgetPtr = CreateWidget<UCharacterSelectWidget>(
			GetOwningPlayerController(), CharacterSelectWidgetClass,TEXT("CharacterSelectWidget"));
		check(CharacterSelectWidgetPtr);
		CharacterSelectWidgetPtr->AddToViewport();
		return;
	}

	CharacterSelectWidgetPtr->AddToViewport();
}