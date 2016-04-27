// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTFlagRunGame.h"

#include "UTCTFRoundGameTommy.generated.h"


UCLASS()
class UNREALTOURNAMENT_API AUTCTFRoundGameTommy : public AUTFlagRunGame
{
	GENERATED_UCLASS_BODY()

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	virtual void InitRound() override;

	virtual void RestartPlayer(AController* aPlayer) override;

	/** Score round ending due some other reason than capture. */
	virtual void ScoreAlternateWin(int32 WinningTeamIndex, uint8 Reason=0) override;
	
	UPROPERTY()
	TSubclassOf<class AUTArmor> ChestArmorClass;
	TAssetSubclassOf<class AUTArmor> ChestArmorObject;
	
	UPROPERTY()
	TSubclassOf<class AUTInventory> DefenseActivatedPowerupClass;
	TAssetSubclassOf<class AUTInventory> DefenseActivatedPowerupObject;

	virtual void GiveDefaultInventory(APawn* PlayerPawn) override;

	virtual void ToggleSpecialFor(AUTCharacter* C) override;

protected:

	virtual bool IsPlayerOnLifeLimitedTeam(AUTPlayerState* PlayerState) const override;
};