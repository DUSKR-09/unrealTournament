// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTCarriedObjectMessage.h"
#include "UTCTFGameMessage.h"

UUTCTFGameMessage::UUTCTFGameMessage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MessageArea = FName(TEXT("GameMessages"));
	ReturnMessage = NSLOCTEXT("CTFGameMessage","ReturnMessage","{Player1Name} returned the {OptionalTeam} Flag!");
	ReturnedMessage = NSLOCTEXT("CTFGameMessage","ReturnedMessage","The {OptionalTeam} Flag was returned!");
	CaptureMessage = NSLOCTEXT("CTFGameMessage","CaptureMessage","{Player1Name} captured the flag and scored for {OptionalTeam}!");
	DroppedMessage = NSLOCTEXT("CTFGameMessage","DroppedMessage","{Player1Name} dropped the {OptionalTeam} Flag!");
	HasMessage = NSLOCTEXT("CTFGameMessage","HasMessage","{Player1Name} took the {OptionalTeam} Flag!");
	KilledMessage = NSLOCTEXT("CTFGameMessage","KilledMessage","{Player1Name} killed the {OptionalTeam} flag carrier!");
	HasAdvantageMessage = NSLOCTEXT("CTFGameMessage", "HasAdvantage", "{OptionalTeam} Team has Advantage");
	LosingAdvantageMessage = NSLOCTEXT("CTFGameMessage", "LostAdvantage", "{OptionalTeam} Team is losing advantage");

	bIsStatusAnnouncement = true;
	bIsPartiallyUnique = true;

}

FText UUTCTFGameMessage::GetText(int32 Switch, bool bTargetsPlayerState1, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{
	switch(Switch)
	{
		case 0 : return ReturnMessage; break;
		case 1 : return ReturnedMessage; break;
		case 2 : return CaptureMessage; break;
		case 3 : return DroppedMessage; break;
		case 4 : return HasMessage; break;
		case 5 : return KilledMessage; break;
		case 6 : return HasAdvantageMessage; break;
		case 7 : return LosingAdvantageMessage; break;
		case 8 : return CaptureMessage; break;
		case 9 : return CaptureMessage; break;
	}

	return FText::GetEmpty();
}

FName UUTCTFGameMessage::GetAnnouncementName_Implementation(int32 Switch, const UObject* OptionalObject) const
{
	uint8 TeamNum = 0;

	const AUTTeamInfo* TeamInfo = Cast<AUTTeamInfo>(OptionalObject);
	if (TeamInfo != NULL)
	{
		TeamNum = TeamInfo->GetTeamNum();
	}

	switch (Switch)
	{
	case 0: return TeamNum == 0 ? TEXT("RedFlagReturned") : TEXT("BlueFlagReturned"); break;
	case 1: return TeamNum == 0 ? TEXT("RedFlagReturned") : TEXT("BlueFlagReturned"); break;
	case 2: return TeamNum == 0 ? TEXT("RedTeamScores") : TEXT("BlueTeamScores"); break;
	case 3: return TeamNum == 0 ? TEXT("RedFlagDropped") : TEXT("BlueFlagDropped"); break;
	case 4: return TeamNum == 0 ? TEXT("RedFlagTaken") : TEXT("BlueFlagTaken"); break;
	case 8: return TeamNum == 0 ? TEXT("RedIncreasesLead") : TEXT("BlueIncreasesLead"); break;
	case 9: return TeamNum == 0 ? TEXT("RedDominating") : TEXT("BlueDominating"); break;
	case 10: return TeamNum == 0 ? TEXT("RedDominating") : TEXT("BlueDominating"); break;
	}

	return NAME_None;
}
