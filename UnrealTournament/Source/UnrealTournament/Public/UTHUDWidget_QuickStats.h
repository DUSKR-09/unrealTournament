// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTHUDWidget_QuickStats.generated.h"

class AUTWeapon;

USTRUCT()
struct FQStatLayoutInfo
{
	GENERATED_USTRUCT_BODY()

	// The tag for this layout.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FName Tag;

	// Where should the health widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bHorizontalBorder;

	// Where should the health widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D HealthOffset;

	// Where should the armor widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D ArmorOffset;

	// Where should the Ammo widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D AmmoOffset;

	// Where should the flag widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D FlagOffset;

	// Where should the flag widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D PowerupOffset;

	// Where should the flag widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D BootsOffset;

	// Where should the boost widget go.  In Pixels based on 1080p
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D BoostProvidedPowerupOffset;

	// If true, this layout will pivot based on the rotation of the widget on the hud
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bFollowRotation;

	//TODO: Add powerups and flag
};

namespace StatAnimTypes
{
	const FName Opacity = FName(TEXT("Opacity"));
	const FName Scale = FName(TEXT("Scale"));
	const FName PositionX = FName(TEXT("PosX"));
	const FName PositionY = FName(TEXT("PosY"));
	const FName None = FName(TEXT("None"));
}


struct FStatAnimInfo
{
	FName AnimType;
	float AnimDuration;
	float AnimTime;
	bool bBounce;
	float StartValue;
	float EndValue;

	FStatAnimInfo()
		: AnimType(StatAnimTypes::None)
		, AnimDuration(0.0f)
		, AnimTime(0.0f)
		, bBounce(false)
		, StartValue(0.0f)
		, EndValue(0.0f)
	{
	}

	FStatAnimInfo(FName inAnimType, float inAnimDuration, float inStartValue, float inEndValue, bool inbBounce)
		: AnimType(inAnimType)
		, AnimDuration(inAnimDuration)
		, AnimTime(0.0f)
		, StartValue(inStartValue)
		, EndValue(inEndValue)
		, bBounce(inbBounce)
	{
	}

};

struct FStatInfo
{
	// Is this stat visible
	bool bVisible;

	// What is the current value for this stat.
	int32 Value;
	int32 LastValue;
	FLinearColor BackgroundColor;
	FLinearColor IconColor;
	FLinearColor TextColor;

	FVector2D DrawOffset;

	float Opacity;

	float Scale;
	bool bInfinite;
	bool bAltIcon;
	float HighlightStrength;
	bool bNoText;
	bool bUseLabel;
	bool bUseLabelBackgroundImage;
	FText Label;

	TArray<FStatAnimInfo> AnimStack;

	FStatInfo()
	{
		bVisible = true;

		Value = 0;
		LastValue = 0;
		Scale = 1.0f;
		bInfinite = false;
		bAltIcon = false;
		bNoText = false;
		bUseLabel = false;
		bUseLabelBackgroundImage = false;
		HighlightStrength = 0.0f;
		
		IconColor = FLinearColor::White;
		TextColor = FLinearColor::White;
		DrawOffset = FVector2D(0.0f, 0.0f);
		Opacity = 1.0f;
	}

	void Animate(FName AnimType, float AnimDuration, float Start, float End, bool bBounce = false)
	{
		// If there is an animation of this type already on the stack, remove it
		int32 i = 0;
		while (i < AnimStack.Num())
		{
			if (AnimStack[i].AnimType == AnimType)
			{
				AnimStack.RemoveAt(i,1);
			}
			else
			{
				i++;
			}
		}

		AnimStack.Add(FStatAnimInfo(AnimType, AnimDuration, Start, End, bBounce));
	}

	void UpdateAnimation(float DeltaTime)
	{
		int32 i = 0;
		while (i < AnimStack.Num())
		{
			if (AnimStack[i].AnimDuration > 0.0f)
			{
				AnimStack[i].AnimTime += DeltaTime;
				float Position = FMath::Clamp<float>(AnimStack[i].AnimTime / AnimStack[i].AnimDuration, 0.0f, 1.0f);
				float Value = 0.0f;
				if (AnimStack[i].bBounce)
				{
					Value = UUTHUDWidget::BounceEaseOut(AnimStack[i].StartValue, AnimStack[i].EndValue, Position, 6.0f);
				}
				else
				{
					Value = FMath::Lerp<float>(AnimStack[i].StartValue, AnimStack[i].EndValue, Position);
				}

				// Apply the animation
				if (AnimStack[i].AnimType == StatAnimTypes::Opacity) Opacity = Value;
				else if (AnimStack[i].AnimType == StatAnimTypes::Scale) Scale = Value;
				else if (AnimStack[i].AnimType == StatAnimTypes::PositionX) DrawOffset.X = Value;
				else if (AnimStack[i].AnimType == StatAnimTypes::PositionY) DrawOffset.Y = Value;
			}

			if (AnimStack[i].AnimDuration <= 0.0f || AnimStack[i].AnimTime >= AnimStack[i].AnimDuration)
			{
				AnimStack.RemoveAt(i,1);
			}
			else
			{
				i++;
			}
		}
	}

};

UCLASS()
class UNREALTOURNAMENT_API UUTHUDWidget_QuickStats : public UUTHUDWidget
{
	GENERATED_UCLASS_BODY()

	virtual void InitializeWidget(AUTHUD* Hud);
	virtual void Draw_Implementation(float DeltaTime) override;
	virtual void PreDraw(float DeltaTime, AUTHUD* InUTHUDOwner, UCanvas* InCanvas, FVector2D InCanvasCenter);
	virtual bool ShouldDraw_Implementation(bool bShowScores);



protected:

	// the image to use for the background when the widget is in the horizontal layout mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	TArray<FQStatLayoutInfo> Layouts;

	// the image to use for the background when the widget is in the horizontal layout mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FHUDRenderObject_Texture HorizontalBackground;

	// the image to use for the background when the widget is in the vertical layout mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FHUDRenderObject_Texture VerticalBackground;

	// the image to use for the highlight in horizontial layout mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FHUDRenderObject_Texture HorizontalHighlight;

	// the image to use for the highlight in vertical layout mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FHUDRenderObject_Texture VerticalHighlight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture HealthIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture ArmorIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture ArmorIconWithShieldBelt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture InfiniteIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Text TextTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture FlagIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture BootsIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture BoostIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icons")
	FHUDRenderObject_Texture LabelBackgroundIcon;

	// NOTE: This icon will be generated from the data in the actual powerup
	UPROPERTY()
	FHUDRenderObject_Texture PowerupIcon;


	virtual float GetDrawScaleOverride();

private:
	FName CurrentLayoutTag;
	int32 CurrentLayoutIndex;
	bool bHorizBorders;

	FStatInfo HealthInfo;
	FStatInfo ArmorInfo;
	FStatInfo AmmoInfo;

	FStatInfo FlagInfo;
	FStatInfo BootsInfo;
	FStatInfo PowerupInfo;
	FStatInfo BoostProvidedPowerupInfo;

	AUTWeapon* LastWeapon;

	// Checks the Stat for updates.
	bool CheckStatForUpdate(float DeltaTime, FStatInfo& Stat, bool bLookForChange = true);
	void GetHighlightStrength(FStatInfo& Stat, float Perc, float WarnPerc);
	FLinearColor InterpColor(FLinearColor DestinationColor, float Delta);
	FVector2D CalcRotOffset(FVector2D InitialPosition, float Angle);

	// The Draw Angle this frame
	float DrawAngle;

	float DrawScale;

	FLinearColor WeaponColor;
	float ForegroundOpacity;

	void DrawStat(FVector2D StatOffset, FStatInfo& StatInfo, FHUDRenderObject_Texture Icon);
};