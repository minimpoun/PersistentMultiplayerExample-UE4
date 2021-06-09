#include "Player/MLoginController.h"

#include "Core/MGameInstance.h"
#include "UserInterface/HUDs/MLoginHUD.h"

void ALoginController::OnLoginComplete()
{
	UMGameInstance* GI = GetGameInstance<UMGameInstance>();
	ALoginHUD* LoginHUD = GetHUD<ALoginHUD>();

	if (GI && LoginHUD)
	{
		LoginHUD->OnLogin();
		GI->GotoStateFast(MGameInstanceState::STATE_CharacterSelect);
	}
}

void ALoginController::BeginPlay()
{
	Super::BeginPlay();

	UMGameInstance* GI = GetGameInstance<UMGameInstance>();
	ALoginHUD* LoginHUD = GetHUD<ALoginHUD>();

	if (GI && LoginHUD && IsLocalController())
	{
		if (GI->HasValidToken())
		{
			LoginHUD->CreateMenuWidget<UCharacterSelectWidget>();
		}
		else
		{
			LoginHUD->CreateMenuWidget<ULoginWidget>();
		}
	}
}
