// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "UnrealTournament.h"
#include "GameFramework/LocalMessage.h"
#include "Engine/Console.h"
#include "GameFramework/PlayerState.h"
#include "UTKillerMessage.h"
#include "UTVictimMessage.h"
#include "UTDeathMessage.h"

UUTDeathMessage::UUTDeathMessage(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MessageArea = FName(TEXT("ConsoleMessage"));
	bIsSpecial = false;

	Lifetime = 6.0f;
}

void UUTDeathMessage::ClientReceive(const FClientReceiveData& ClientData) const
{
	APlayerState* PlayerState_1 = Cast<APlayerState>(ClientData.RelatedPlayerState_1);
	APlayerState* PlayerState_2 = Cast<APlayerState>(ClientData.RelatedPlayerState_2);

	// Look to see if we should redirect this generic death message to either a killer message or a victim message.

	if (ClientData.RelatedPlayerState_1 == ClientData.LocalPC->PlayerState) // We need to add code to allow for spectators to see the death messages of those they are a spectator
	{
		// Interdict and send the child message instead.
		AUTHUD* UTHUD = Cast<AUTHUD>(ClientData.LocalPC->MyHUD);
		if ( UTHUD != NULL )
		{
			UTHUD->ReceiveLocalMessage(
			UUTKillerMessage::StaticClass(),
			ClientData.RelatedPlayerState_1,
			ClientData.RelatedPlayerState_2,
			ClientData.MessageIndex,
			GetDefault<UUTKillerMessage>()->ResolveMessage(ClientData.MessageIndex, ClientData.RelatedPlayerState_1== ClientData.LocalPC->PlayerState, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject),
			ClientData.OptionalObject );
		}
	}
	else if (ClientData.RelatedPlayerState_2 == ClientData.LocalPC->PlayerState)
	{
		AUTHUD* UTHUD = Cast<AUTHUD>(ClientData.LocalPC->MyHUD);
		if ( UTHUD != NULL )
		{
			UTHUD->ReceiveLocalMessage(
			UUTVictimMessage::StaticClass(),
			ClientData.RelatedPlayerState_1,
			ClientData.RelatedPlayerState_2,
			ClientData.MessageIndex,
			GetDefault<UUTVictimMessage>()->ResolveMessage(ClientData.MessageIndex, true, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject),
			ClientData.OptionalObject );
		}
	}

	// Also recieve the console message side of this.
	Super::ClientReceive(ClientData);
}

FText UUTDeathMessage::GetText(int32 Switch,bool bTargetsPlayerState1,class APlayerState* RelatedPlayerState_1,class APlayerState* RelatedPlayerState_2,class UObject* OptionalObject) const
{
	UClass* DamageTypeClass = Cast<UClass>(OptionalObject);
	if (DamageTypeClass != NULL)
	{
		UUTDamageType* DamageType = DamageTypeClass->GetDefaultObject<UUTDamageType>();			
		if (Switch == 1)	// Suicide
		{
			// We don't have a switch yet in the PRI for male vs Female..... so just use the mail for now.
			return DamageType->MaleSuicideMessage;
		}
		return DamageType->ConsoleDeathMessage;
	}

	return FText::GetEmpty();	
}
