// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTHud.h"
#include "UTLocalPlayer.h"
#include "UTPlayerState.h"
#include "UTPlayerController.h"
#include "UTCharacterMovement.h"
#include "ActiveSound.h"
#include "AudioDevice.h"
#include "UTPickup.h"
#include "UTPickupInventory.h"
#include "UTPickupWeapon.h"
#include "UTAnnouncer.h"
#include "UTHUDWidgetMessage.h"
#include "UTPlayerInput.h"
#include "UTPlayerCameraManager.h"

AUTPlayerController::AUTPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bAutoWeaponSwitch = true;

	MaxDodgeClickTime = 0.25f;
	LastTapLeftTime = -10.f;
	LastTapRightTime = -10.f;
	LastTapForwardTime = -10.f;
	LastTapBackTime = -10.f;

	PlayerCameraManagerClass = AUTPlayerCameraManager::StaticClass();

}

void AUTPlayerController::SetGravity(float NewGravity)
{
	if (GetPawn() && Cast<UUTCharacterMovement>(GetPawn()->GetMovementComponent()))
	{
		Cast<UUTCharacterMovement>(GetPawn()->GetMovementComponent())->SetGravityScale(NewGravity);
	}
}

void AUTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AUTPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AUTPlayerController::MoveRight);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AUTPlayerController::Jump);

	InputComponent->BindAction("TapLeft", IE_Pressed, this, &AUTPlayerController::OnTapLeft);
	InputComponent->BindAction("TapRight", IE_Pressed, this, &AUTPlayerController::OnTapRight);
	InputComponent->BindAction("TapForward", IE_Pressed, this, &AUTPlayerController::OnTapForward);
	InputComponent->BindAction("TapBack", IE_Pressed, this, &AUTPlayerController::OnTapBack);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APlayerController::AddYawInput);
	InputComponent->BindAxis("TurnRate", this, &AUTPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APlayerController::AddPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AUTPlayerController::LookUpAtRate);

	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AUTPlayerController::PrevWeapon);
	InputComponent->BindAction("NextWeapon", IE_Released, this, &AUTPlayerController::NextWeapon);

	InputComponent->BindAction("StartFire", IE_Pressed, this, &AUTPlayerController::OnFire);
	InputComponent->BindAction("StopFire", IE_Released, this, &AUTPlayerController::OnStopFire);
	InputComponent->BindAction("StartAltFire", IE_Pressed, this, &AUTPlayerController::OnAltFire);
	InputComponent->BindAction("StopAltFire", IE_Released, this, &AUTPlayerController::OnStopAltFire);
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AUTPlayerController::TouchStarted);

	InputComponent->BindAction("ShowScores", IE_Pressed, this, &AUTPlayerController::OnShowScores);
	InputComponent->BindAction("ShowScores", IE_Released, this, &AUTPlayerController::OnHideScores);

	InputComponent->BindAction("ShowMenu", IE_Released, this, &AUTPlayerController::ShowMenu);
}

void AUTPlayerController::InitInputSystem()
{
	if (PlayerInput == NULL)
	{
		PlayerInput = ConstructObject<UUTPlayerInput>(UUTPlayerInput::StaticClass(), this);
	}

	Super::InitInputSystem();

	if (RewardAnnouncerPath.ClassName.Len() > 0)
	{
		TSubclassOf<UUTAnnouncer> RewardAnnouncerClass = LoadClass<UUTAnnouncer>(NULL, *RewardAnnouncerPath.ClassName, NULL, 0, NULL);
		if (RewardAnnouncerClass != NULL && RewardAnnouncerClass.GetDefaultObject()->IsRewardAnnouncer())
		{
			RewardAnnouncer = NewObject<UUTAnnouncer>(this, RewardAnnouncerClass);
		}
	}
	if (StatusAnnouncerPath.ClassName.Len() > 0)
	{
		TSubclassOf<UUTAnnouncer> StatusAnnouncerClass = LoadClass<UUTAnnouncer>(NULL, *StatusAnnouncerPath.ClassName, NULL, 0, NULL);
		if (StatusAnnouncerClass != NULL && StatusAnnouncerClass.GetDefaultObject()->IsStatusAnnouncer())
		{
			StatusAnnouncer = NewObject<UUTAnnouncer>(this, StatusAnnouncerClass);
		}
	}
}

/* Cache a copy of the PlayerState cast'd to AUTPlayerState for easy reference.  Do it both here and when the replicated copy of APlayerState arrives in OnRep_PlayerState */
void AUTPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	UTPlayerState = Cast<AUTPlayerState>(PlayerState);
}

void AUTPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	UTPlayerState = Cast<AUTPlayerState>(PlayerState);
}

void AUTPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	UTCharacter = Cast<AUTCharacter>(InPawn);
}

bool AUTPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	// unfortunately have to go roundabout because this is the only InputKey() that's virtual
	UUTPlayerInput* Input = Cast<UUTPlayerInput>(PlayerInput);
	if (Input != NULL)
	{
		Input->ExecuteCustomBind(Key, EventType);
		// ...unsure if we should eat the input on success
	}
	return Super::InputKey(Key, EventType, AmountDepressed, bGamepad);
}

void AUTPlayerController::SwitchToBestWeapon()
{
	if (UTCharacter != NULL && IsLocalPlayerController())
	{
		AUTWeapon* BestWeapon = NULL;
		for (AUTInventory* Inv = UTCharacter->GetInventory(); Inv != NULL; Inv = Inv->GetNext())
		{
			AUTWeapon* Weap = Cast<AUTWeapon>(Inv);
			if (Weap != NULL && Weap->HasAnyAmmo())
			{
				if (BestWeapon == NULL || Weap->Group > BestWeapon->Group)
				{
					BestWeapon = Weap;
				}
			}
		}
		UTCharacter->SwitchWeapon(BestWeapon);
	}
}
void AUTPlayerController::PrevWeapon()
{
	SwitchWeaponInSequence(true);
}
void AUTPlayerController::NextWeapon()
{
	SwitchWeaponInSequence(false);
}
void AUTPlayerController::SwitchWeaponInSequence(bool bPrev)
{
	if (UTCharacter != NULL && IsLocalPlayerController())
	{
		if (UTCharacter->GetWeapon() == NULL)
		{
			SwitchToBestWeapon();
		}
		else
		{
			AUTWeapon* Best = NULL;
			AUTWeapon* WraparoundChoice = NULL;
			int32 CurrentGroup = (UTCharacter->GetPendingWeapon() != NULL) ? UTCharacter->GetPendingWeapon()->Group : UTCharacter->GetWeapon()->Group;
			int32 CurrentSlot = (UTCharacter->GetPendingWeapon() != NULL) ? UTCharacter->GetPendingWeapon()->GroupSlot : UTCharacter->GetWeapon()->GroupSlot;
			for (AUTInventory* Inv = UTCharacter->GetInventory(); Inv != NULL; Inv = Inv->GetNext())
			{
				AUTWeapon* Weap = Cast<AUTWeapon>(Inv);
				if (Weap != NULL && Weap != UTCharacter->GetWeapon() && Weap->HasAnyAmmo())
				{
					if (bPrev)
					{
						if ( (Weap->Group < CurrentGroup || (Weap->Group == CurrentGroup && Weap->GroupSlot < CurrentSlot)) &&
							(Best == NULL || Weap->Group > Best->Group || Weap->GroupSlot > Best->GroupSlot) )
						{
							Best = Weap;
						}
						if (WraparoundChoice == NULL || Weap->Group > WraparoundChoice->Group || (Weap->Group == WraparoundChoice->Group && Weap->GroupSlot > WraparoundChoice->GroupSlot))
						{
							WraparoundChoice = Weap;
						}
					}
					else
					{
						if ( (Weap->Group > CurrentGroup || (Weap->Group == CurrentGroup && Weap->GroupSlot > CurrentSlot)) &&
							(Best == NULL || Weap->Group < Best->Group || Weap->GroupSlot < Best->GroupSlot) )
						{
							Best = Weap;
						}
						if (WraparoundChoice == NULL || Weap->Group < WraparoundChoice->Group || (Weap->Group == WraparoundChoice->Group && Weap->GroupSlot < WraparoundChoice->GroupSlot))
						{
							WraparoundChoice = Weap;
						}
					}
				}
			}
			if (Best == NULL)
			{
				Best = WraparoundChoice;
			}
			UTCharacter->SwitchWeapon(Best);
		}
	}
}
void AUTPlayerController::CheckAutoWeaponSwitch(AUTWeapon* TestWeapon)
{
	if (UTCharacter != NULL && IsLocalPlayerController())
	{
		AUTWeapon* CurWeapon = UTCharacter->GetPendingWeapon();
		if (CurWeapon == NULL)
		{
			CurWeapon = UTCharacter->GetWeapon();
		}
		if (CurWeapon == NULL || (bAutoWeaponSwitch && !CurWeapon->IsFiring() && TestWeapon->Group > CurWeapon->Group))
		{
			UTCharacter->SwitchWeapon(TestWeapon);
		}
	}
}
void AUTPlayerController::SwitchWeapon(int32 Group)
{
	if (UTCharacter != NULL && IsLocalPlayerController())
	{
		// if current weapon isn't in the specified group, pick lowest GroupSlot in that group
		// if it is, then pick next highest slot, or wrap around to lowest if no higher slot
		AUTWeapon* CurrWeapon = (UTCharacter->GetPendingWeapon() != NULL) ? UTCharacter->GetPendingWeapon() : UTCharacter->GetWeapon();
		AUTWeapon* LowestSlotWeapon = NULL;
		AUTWeapon* NextSlotWeapon = NULL;
		for (AUTInventory* Inv = UTCharacter->GetInventory(); Inv != NULL; Inv = Inv->GetNext())
		{
			AUTWeapon* Weap = Cast<AUTWeapon>(Inv);
			if (Weap != NULL && Weap != UTCharacter->GetWeapon() && Weap->HasAnyAmmo())
			{
				if (Weap->Group == Group)
				{
					if (LowestSlotWeapon == NULL || LowestSlotWeapon->GroupSlot > Weap->GroupSlot)
					{
						LowestSlotWeapon = Weap;
					}
					if (CurrWeapon != NULL && CurrWeapon->Group == Group && Weap->GroupSlot > CurrWeapon->GroupSlot && (NextSlotWeapon == NULL || NextSlotWeapon->GroupSlot > Weap->GroupSlot))
					{
						NextSlotWeapon = Weap;
					}
				}
			}
		}
		if (NextSlotWeapon != NULL)
		{
			UTCharacter->SwitchWeapon(NextSlotWeapon);
		}
		else if (LowestSlotWeapon != NULL)
		{
			UTCharacter->SwitchWeapon(LowestSlotWeapon);
		}
	}
}

void AUTPlayerController::OnFire()
{
	if (UTCharacter != NULL)
	{
		if (StateName == NAME_Playing)
		{
			UTCharacter->StartFire(0);
		}
	}
	else
	{
		ServerRestartPlayer();
	}

}
void AUTPlayerController::OnStopFire()
{
	if (UTCharacter != NULL)
	{
		UTCharacter->StopFire(0);
	}
}
void AUTPlayerController::OnAltFire()
{
	if (UTCharacter != NULL)
	{
		if (StateName == NAME_Playing)
		{
			UTCharacter->StartFire(1);
		}
	}
}
void AUTPlayerController::OnStopAltFire()
{
	if (UTCharacter != NULL)
	{
		UTCharacter->StopFire(1);
	}
}
void AUTPlayerController::MoveForward(float Value)
{
	if (Value != 0.0f && GetPawn() != NULL)
	{
		// find out which way is forward
		const FRotator Rotation = GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// add movement in that direction
		GetPawn()->AddMovementInput(Direction, Value);
	}
}
void AUTPlayerController::MoveRight(float Value)
{
	if (Value != 0.0f && GetPawn() != NULL)
	{
		// find out which way is right
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		GetPawn()->AddMovementInput(Direction, Value);
	}
}
void AUTPlayerController::TurnAtRate(float Rate)
{
	if (GetPawn() != NULL)
	{
		// calculate delta for this frame from the rate information
		AddYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}
void AUTPlayerController::LookUpAtRate(float Rate)
{
	if (GetPawn() != NULL)
	{
		// calculate delta for this frame from the rate information
		AddPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}
void AUTPlayerController::Jump()
{
	if (GetCharacter() != NULL)
	{
		GetCharacter()->bPressedJump = true;
	}
}

void AUTPlayerController::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// only fire for first finger down
	if (FingerIndex == 0)
	{
		OnFire();
	}
}

void AUTPlayerController::HearSound(USoundBase* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, bool bStopWhenOwnerDestroyed)
{
	bool bIsOccluded = false;
	if (SoundPlayer == this || (GetViewTarget() != NULL && InSoundCue->IsAudible(SoundLocation, GetViewTarget()->GetActorLocation(), (SoundPlayer != NULL) ? SoundPlayer : this, bIsOccluded, true)))
	{
		// we don't want to replicate the location if it's the same as Actor location (so the sound gets played attached to the Actor), but we must if the source Actor isn't relevant
		UNetConnection* Conn = Cast<UNetConnection>(Player);
		FVector RepLoc = (SoundPlayer != NULL && SoundPlayer->GetActorLocation() == SoundLocation && (Conn == NULL || Conn->ActorChannels.Contains(SoundPlayer))) ? FVector::ZeroVector : SoundLocation;
		ClientHearSound(InSoundCue, SoundPlayer, RepLoc, bStopWhenOwnerDestroyed, bIsOccluded);
	}
}

void AUTPlayerController::ClientHearSound_Implementation(USoundBase* TheSound, AActor* SoundPlayer, FVector SoundLocation, bool bStopWhenOwnerDestroyed, bool bIsOccluded)
{
	if (SoundPlayer == this || SoundPlayer == GetViewTarget())
	{
		// no attenuation/spatialization, full volume
		FActiveSound NewActiveSound;
		NewActiveSound.World = GetWorld();
		NewActiveSound.Sound = TheSound;

		NewActiveSound.VolumeMultiplier = 1.0f;
		NewActiveSound.PitchMultiplier = 1.0f;

		NewActiveSound.RequestedStartTime = 0.0f;

		NewActiveSound.bLocationDefined = false;
		NewActiveSound.bIsUISound = false;
		NewActiveSound.bHasAttenuationSettings = false;
		NewActiveSound.bAllowSpatialization = false;

		// TODO - Audio Threading. This call would be a task call to dispatch to the audio thread
		GEngine->GetAudioDevice()->AddNewActiveSound(NewActiveSound);
	}
	else if (!SoundLocation.IsZero())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TheSound, SoundLocation, bIsOccluded ? 0.5f : 1.0f);
	}
	else if (SoundPlayer != NULL)
	{
		UGameplayStatics::PlaySoundAttached(TheSound, SoundPlayer->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, bStopWhenOwnerDestroyed, bIsOccluded ? 0.5f : 1.0f);
	}
}

bool AUTPlayerController::CheckDodge(float LastTapTime, bool bForward, bool bBack, bool bLeft, bool bRight)
{
	UUTCharacterMovement* MyCharMovement = UTCharacter ? Cast<UUTCharacterMovement>(UTCharacter->CharacterMovement) : NULL;
	if (MyCharMovement && (GetWorld()->GetTimeSeconds() - LastTapTime < MaxDodgeClickTime))
	{
		MyCharMovement->bPressedDodgeForward = bForward;
		MyCharMovement->bPressedDodgeBack = bBack;
		MyCharMovement->bPressedDodgeLeft = bLeft;
		MyCharMovement->bPressedDodgeRight = bRight;
	}
	return false;
}

void AUTPlayerController::OnTapForward()
{
	LastTapForwardTime = (!UTCharacter || CheckDodge(LastTapForwardTime, true, false, false, false)) ? -10.f : GetWorld()->GetTimeSeconds();
}

void AUTPlayerController::OnTapBack()
{
	LastTapBackTime = (!UTCharacter || CheckDodge(LastTapBackTime, false, true, false, false)) ? -10.f : GetWorld()->GetTimeSeconds();
}

void AUTPlayerController::OnTapLeft()
{
	LastTapLeftTime = (!UTCharacter || CheckDodge(LastTapLeftTime, false, false, true, false)) ? -10.f : GetWorld()->GetTimeSeconds();
}

void AUTPlayerController::OnTapRight()
{
	LastTapRightTime = (!UTCharacter || CheckDodge(LastTapRightTime, false, false, false, true)) ? -10.f : GetWorld()->GetTimeSeconds();
}


void AUTPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& HiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, HiddenComponents);

	for (int32 i = RecentWeaponPickups.Num() - 1; i >= 0; i--)
	{
		if (RecentWeaponPickups[i] == NULL)
		{
			RecentWeaponPickups.RemoveAt(i, 1, false);
		}
		else if (!RecentWeaponPickups[i]->IsTaken(GetPawn()))
		{
			RecentWeaponPickups[i]->PlayRespawnEffects();
			RecentWeaponPickups.RemoveAt(i, 1, false);
		}
		else if (RecentWeaponPickups[i]->GetMesh() != NULL)
		{
			HiddenComponents.Add(RecentWeaponPickups[i]->GetMesh()->ComponentId);
		}
	}
}

void AUTPlayerController::SetName(const FString& S)
{
	if (!S.IsEmpty())
	{
		Super::SetName(S);

		UUTGameUserSettings* Settings;
		Settings = Cast<UUTGameUserSettings>(GEngine->GetGameUserSettings());
		if (Settings)
		{
			Settings->SetPlayerName(S);
			Settings->SaveSettings();
		}
	}
}

void AUTPlayerController::ToggleScoreboard(bool bShow)
{
	if (MyHUD != NULL && Cast<AUTHUD>(MyHUD) != NULL)
	{
		Cast<AUTHUD>(MyHUD)->ToggleScoreboard(bShow);
	}
}

void AUTPlayerController::ClientToggleScoreboard_Implementation(bool bShow)
{
	ToggleScoreboard(bShow);
}

void AUTPlayerController::ClientSetHUDAndScoreboard_Implementation(TSubclassOf<class AHUD> NewHUDClass, TSubclassOf<class UUTScoreboard> NewScoreboardClass)
{
	// First, create the HUD

	ClientSetHUD_Implementation(NewHUDClass);

	MyUTHUD = Cast<AUTHUD>(MyHUD);
	if (MyUTHUD != NULL && NewScoreboardClass != NULL)
	{
		MyUTHUD->CreateScoreboard(NewScoreboardClass);
	}
	
}

void AUTPlayerController::OnShowScores()
{
	ToggleScoreboard(true);
}

void AUTPlayerController::OnHideScores()
{
	ToggleScoreboard(false);
}

AUTCharacter* AUTPlayerController::GetUTCharacter()
{
	return UTCharacter;
}

void AUTPlayerController::ServerRestartPlayer_Implementation()
{
	if (!GetWorld()->GetAuthGameMode()->HasMatchStarted() && UTPlayerState != NULL)
	{
		UTPlayerState ->bReadyToPlay = true;
	}

	Super::ServerRestartPlayer_Implementation();

}

bool AUTPlayerController::CanRestartPlayer()
{
	return  Super::CanRestartPlayer() && UTPlayerState->RespawnTime <= 0.0f;
}


void AUTPlayerController::BehindView(bool bWantBehindView)
{
	SetCameraMode( (bWantBehindView ? FName(TEXT("FreeCam")) : FName(TEXT("Default"))));
}

bool AUTPlayerController::IsBehindView()
{
	if (PlayerCameraManager != NULL)
	{
		return PlayerCameraManager->CameraStyle == FName(TEXT("FreeCam"));
	}
	return false;
}

void AUTPlayerController::ClientSetCameraMode_Implementation( FName NewCamMode )
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->CameraStyle = NewCamMode;
	}

	if (UTCharacter != NULL && IsLocalPlayerController())
	{
		UTCharacter->SetMeshVisibility(NewCamMode == FName(TEXT("FreeCam")));
	}
}

void AUTPlayerController::SetCameraMode( FName NewCamMode )
{
	ClientSetCameraMode_Implementation(NewCamMode);
	
	if ( GetNetMode() == NM_DedicatedServer )
	{
		ClientSetCameraMode( NewCamMode );
	}
}

void AUTPlayerController::ClientGameEnded_Implementation(AActor* EndGameFocus, bool bIsWinner)
{
	ChangeState(FName(TEXT("GameOver")));
	FinalViewTarget = EndGameFocus;
	BehindView(true);
	Super::ClientGameEnded_Implementation(EndGameFocus, bIsWinner);
}

void AUTPlayerController::SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	if (FinalViewTarget != NULL)
	{
		NewViewTarget = FinalViewTarget;
	}

	Super::SetViewTarget(NewViewTarget, TransitionParams);
}

// LEAVE ME for quick debug commands when we need them.
void AUTPlayerController::DebugTest()
{
	UE_LOG(UT,Log,TEXT("DEBUG"));
	Cast<UUTLocalPlayer>(Player)->ShowMessage(NSLOCTEXT("TestNS","TestA","Caption"), NSLOCTEXT("TestNS","TestB","This is a test"),UTDIALOG_BUTTON_OK | UTDIALOG_BUTTON_CANCEL | UTDIALOG_BUTTON_RECONNECT, this, NAME_None);
}


void AUTPlayerController::PlayerTick( float DeltaTime )
{
	Super::PlayerTick(DeltaTime);
	if (StateName == FName(TEXT("GameOver")))
	{
		UpdateRotation(DeltaTime);
	}
}

void AUTPlayerController::NotifyTakeHit_Implementation(AController* InstigatedBy, int32 Damage, FVector Momentum, const FDamageEvent& DamageEvent)
{
	APlayerState* InstigatedByState = (InstigatedBy != NULL) ? InstigatedBy->PlayerState : NULL;
	FVector RelHitLocation(FVector::ZeroVector);
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		RelHitLocation = ((FPointDamageEvent*)&DamageEvent)->HitInfo.Location - GetPawn()->GetActorLocation();
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID) && ((FRadialDamageEvent*)&DamageEvent)->ComponentHits.Num() > 0)
	{
		RelHitLocation = ((FRadialDamageEvent*)&DamageEvent)->ComponentHits[0].Location - GetPawn()->GetActorLocation();
	}
	ClientNotifyTakeHit(InstigatedByState, Damage, Momentum, RelHitLocation, DamageEvent.DamageTypeClass);
}

void AUTPlayerController::ClientNotifyTakeHit_Implementation(APlayerState* InstigatedBy, int32 Damage, FVector Momentum, FVector RelHitLocation, TSubclassOf<UDamageType> DamageType)
{
	if (MyUTHUD != NULL)
	{
		MyUTHUD->PawnDamaged(((GetPawn() != NULL) ? GetPawn()->GetActorLocation() : FVector::ZeroVector) + RelHitLocation, Damage, DamageType);
	}
}
void AUTPlayerController::ShowMenu()
{
	UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(Player);
	if (LP != NULL)
	{
		LP->ShowMenu();
	}

}

void AUTPlayerController::ShowMessage(FText MessageTitle, FText MessageText, uint16 Buttons, UObject* Host, FName ResultFunction)
{
	UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(Player);
	if (LP != NULL)
	{
		LP->ShowMessage(MessageTitle, MessageText, Buttons, Host, ResultFunction);
	}	
}


void AUTPlayerController::K2_ReceiveLocalizedMessage(TSubclassOf<ULocalMessage> Message, int32 Switch, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	ClientReceiveLocalizedMessage(Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
}

uint8 AUTPlayerController::GetTeamNum() const
{
	AUTPlayerState* PS = Cast<AUTPlayerState>(PlayerState);
	return (PS != NULL && PS->Team != NULL) ? PS->Team->TeamIndex : 255;
}

void AUTPlayerController::ChangeTeam(uint8 NewTeamIndex)
{
	AUTPlayerState* PS = Cast<AUTPlayerState>(PlayerState);
	if (PS != NULL)
	{
		PS->ServerRequestChangeTeam(NewTeamIndex);
	}
}

void AUTPlayerController::Suicide()
{
	if (GetPawn() != NULL)
	{
		AUTCharacter* Char = Cast<AUTCharacter>(GetPawn());
		if (Char != NULL)
		{
			Char->PlayerSuicide();
		}
	}

}