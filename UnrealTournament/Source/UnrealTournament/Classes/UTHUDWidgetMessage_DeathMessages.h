// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "UTHUDWidgetMessage_DeathMessages.generated.h"

UCLASS()
class UUTHUDWidgetMessage_DeathMessages : public UUTHUDWidgetMessage 
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void DrawMessages(float DeltaTime);
	virtual void LayoutMessage(int32 QueueIndex, TSubclassOf<class UUTLocalMessage> MessageClass, uint32 MessageIndex, FText LocalMessageText, int32 MessageCount, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject);

private:

};
