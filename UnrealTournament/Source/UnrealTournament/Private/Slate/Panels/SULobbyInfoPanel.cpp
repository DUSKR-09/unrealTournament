
// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "../Public/UnrealTournament.h"
#include "../Public/UTLocalPlayer.h"
#include "SlateBasics.h"
#include "Slate/SlateGameResources.h"
#include "../SUWPanel.h"
#include "../SUWindowsStyle.h"
#include "SUMidGameInfoPanel.h"
#include "SULobbyMatchSetupPanel.h"
#include "SULobbyInfoPanel.h"
#include "UTLobbyPC.h"



#if !UE_SERVER

void SULobbyInfoPanel::ConstructPanel(FVector2D ViewportSize)
{
	SUInGameHomePanel::ConstructPanel(ViewportSize);

	Tag = FName(TEXT("LobbyInfoPanel"));

	ChatDestination = ChatDestinations::Global;
	LastMatchState = ELobbyMatchState::Setup;

	AUTLobbyPlayerState* PlayerState = Cast<AUTLobbyPlayerState>(GetOwnerPlayerState());
	PlayerState->CurrentMatchChangedDelegate.BindSP(this, &SULobbyInfoPanel::OwnerCurrentMatchChanged);
	bShowingMatchDock = PlayerState->CurrentMatch == NULL;

	if (MenuArea.IsValid())
	{
		MenuArea->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(8)
				[
					SNew(SImage)
					.Image(SUWindowsStyle::Get().GetBrush("UT.TopMenu.Bar"))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				// Buttom Menu Bar
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					[
						SNew(SImage)
						.Image(SUWindowsStyle::Get().GetBrush("UT.TopMenu.TileBar"))
					]
				]
				+SOverlay::Slot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(56)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							[
								BuildMatchMenu()
							]
						]
					]
				]
			]
		];
	}



	if (ChatArea.IsValid())
	{

		ChatArea->AddSlot()
		[
			SAssignNew(MatchArea, SVerticalBox)
		];

		BuildMatchPanel();

		ChatArea->AddSlot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()		
			.FillHeight(1.0)
			[
				BuildChatArea()
			]
		];
	}


}

AUTPlayerState* SULobbyInfoPanel::GetOwnerPlayerState()
{
	AUTBasePlayerController* PC = Cast<AUTBasePlayerController>(PlayerOwner->PlayerController);
	if (PC) 
	{
		return Cast<AUTPlayerState>(PC->PlayerState);
	}
	return NULL;
}

void SULobbyInfoPanel::BuildMatchPanel()
{
	// Clear out any existing panels
	MatchArea->ClearChildren();

	AUTLobbyPlayerState* PlayerState = Cast<AUTLobbyPlayerState>(GetOwnerPlayerState());
	if (MatchArea.IsValid() && PlayerState)
	{
		if (bShowingMatchDock)
		{
			MatchArea->AddSlot()
			.AutoHeight()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()		
						.HAlign(HAlign_Fill)
						[
							SNew(SOverlay)
							+SOverlay::Slot()
							[
								SNew(SImage)
								.Image(SUWindowsStyle::Get().GetBrush("UWindows.Standard.DarkBackground"))
							]
							+SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()		
								.AutoHeight()
								[
									SNew(SScrollBox)
									.Orientation(Orient_Horizontal)
									+SScrollBox::Slot()
									[
										SAssignNew(MatchPanelDock, SHorizontalBox)
										+SHorizontalBox::Slot()
										.Padding(5.0f,0.0f,5.0f,0.0f)
										[
											BuildDefaultMatchMessage()
										]
									]
								]
							]
						]			
				];
		}

		else if (PlayerState->CurrentMatch)

		// We need to show the match setup panel.  Hosts will have an interactive panel, non-hosts will just get an info dump

		{
			MatchArea->AddSlot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()		
					.AutoHeight()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Fill)
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						[
							SNew(SImage)
							.Image(SUWindowsStyle::Get().GetBrush("UT.TopMenu.Bar"))
						]
						+SOverlay::Slot()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()		
							.AutoHeight()
							[
								SNew(SULobbyMatchSetupPanel)
								.PlayerOwner(PlayerOwner)
								.MatchInfo(PlayerState->CurrentMatch)
								.bIsHost(PlayerState->CurrentMatch->OwnerId == PlayerState->UniqueId)
							]
						]
					]			
				];
		}
	}
}

TSharedRef<SWidget> SULobbyInfoPanel::BuildMatchMenu()
{
	return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(0.0f,0.0f,5.0f,0.0f)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button")
			.ContentPadding(FMargin(25.0,0.0,25.0,5.0))
			.OnClicked(this, &SULobbyInfoPanel::MatchButtonClicked)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SULobbyInfoPanel::GetMatchButtonText)
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.TextStyle")
				]
			]
		];
}

FReply SULobbyInfoPanel::MatchButtonClicked()
{
	if (PlayerOwner.IsValid() && PlayerOwner->PlayerController)
	{
		AUTLobbyPlayerState* PlayerState = Cast<AUTLobbyPlayerState>(PlayerOwner->PlayerController->PlayerState);
		if (PlayerState)
		{
			PlayerState->MatchButtonPressed();
		}
	}

	return FReply::Handled();
}

FText SULobbyInfoPanel::GetMatchButtonText() const
{
	if (PlayerOwner.IsValid() && PlayerOwner->PlayerController && PlayerOwner->PlayerController->PlayerState)
	{
		AUTLobbyPlayerState* LPS = Cast<AUTLobbyPlayerState>(PlayerOwner->PlayerController->PlayerState);
		if (LPS)
		{
			if (LPS->CurrentMatch)
			{
				return NSLOCTEXT("Gerneric","LeaveMatch","LEAVE MATCH");
			}
			return NSLOCTEXT("Gerneric","NewMatch","NEW MATCH");
	
		}
	}
	return FText::GetEmpty();
}



bool SULobbyInfoPanel::AlreadyTrackingMatch(AUTLobbyMatchInfo* TestMatch)
{
	for (int32 i=0;i<MatchData.Num();i++)
	{
		if (MatchData[i].Get()->Info == TestMatch)
		{
			return true;
		}
	}

	return false;

}
void SULobbyInfoPanel::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	AUTLobbyPlayerState* PlayerState = Cast<AUTLobbyPlayerState>(GetOwnerPlayerState());

	if (bShowingMatchDock)
	{
		// Look to see if we have been waiting for a valid GameModeData to replicate.  If we have, open the right panel.
		if (PlayerState && PlayerState->CurrentMatch && PlayerState->CurrentMatch->MatchIsReadyToJoin(PlayerState))
		{
			ShowMatchSetupPanel();
			return;
		}

		// We are showing the dock, look to see if new matches to join have been added to the queue and if they
		// have, add them

		AUTLobbyGameState* LobbyGameState = GWorld->GetGameState<AUTLobbyGameState>();
		if (LobbyGameState)
		{
			bool bDockRequiresUpdate = false;

			// Expire out any old matches that are no longer useful.

			for (int32 i=0;i<MatchData.Num();i++)
			{
				if (MatchData[i]->Info.IsValid())
				{
					if (!LobbyGameState->IsMatchStillValid(MatchData[i]->Info.Get()) || !MatchData[i]->Info->ShouldShowInDock())
					{
						bDockRequiresUpdate = true;
					}
				}
			}

			// Look for any matches not in the dock
			for (int32 i=0;i<LobbyGameState->AvailableMatches.Num();i++)
			{
				AUTLobbyMatchInfo* Match = LobbyGameState->AvailableMatches[i];
				if (Match && Match->CurrentState != ELobbyMatchState::Dead && Match->CurrentState != ELobbyMatchState::Recycling)
				{
					if (!AlreadyTrackingMatch(LobbyGameState->AvailableMatches[i]))
					{
						bDockRequiresUpdate = true;
					}
				}
			}

			if (bDockRequiresUpdate)
			{
				MatchData.Empty();
				MatchPanelDock->ClearChildren();

				if ( LobbyGameState->AvailableMatches.Num() >0 )
				{
					for (int32 i=0;i<LobbyGameState->AvailableMatches.Num();i++)
					{
						if (LobbyGameState->AvailableMatches[i] && LobbyGameState->AvailableMatches[i]->ShouldShowInDock())
						{
							TSharedPtr<SUMatchPanel> MP;
							TWeakObjectPtr<AUTLobbyMatchInfo> MatchInfo = LobbyGameState->AvailableMatches[i];
							MatchPanelDock->AddSlot()
							[
								SAssignNew(MP,SUMatchPanel)
								.MatchInfo(MatchInfo)
								.PlayerOwner(PlayerOwner)
							];

							MatchData.Add( FMatchData::Make(LobbyGameState->AvailableMatches[i], MP));
						}
					}
				}
				else
				{
					MatchPanelDock->AddSlot()
					[
						BuildDefaultMatchMessage()
					];
				}
			}
		}
	}

	UpdateUserList();
	UpdateChatText();
}

TSharedRef<SWidget> SULobbyInfoPanel::BuildDefaultMatchMessage()
{
	AUTLobbyGameState* LobbyGameState = GWorld->GetGameState<AUTLobbyGameState>();
	if (!LobbyGameState || LobbyGameState->AvailableMatches.Num() <= 0)
	{
		return SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text( NSLOCTEXT("LobbyMatches","NoMatches","There are currently no matches, start one!"))
			.TextStyle(SUWindowsStyle::Get(), "UWindows.Lobby.MatchButton.Action.TextStyle")
		];
	}
	
	return SNew(SCanvas);

}
void SULobbyInfoPanel::OwnerCurrentMatchChanged(AUTLobbyPlayerState* LobbyPlayerState)
{
	BuildChatDestinationMenu();

	if (LobbyPlayerState->CurrentMatch && LobbyPlayerState->CurrentMatch->MatchIsReadyToJoin(LobbyPlayerState))
	{
		if (bShowingMatchDock)
		{
			ShowMatchSetupPanel();
		}
	}
	else
	{
		if (!bShowingMatchDock)
		{
			ShowMatchDock();		
		}
	}
}

void SULobbyInfoPanel::BuildChatDestinationMenu()
{
	if (ChatMenu.IsValid())
	{
		ChatMenu->ClearChildren();
		ChatMenu->AddSlot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(SUWindowsStyle::Get(), "UT.ContextMenu.Button")
			.ContentPadding(FMargin(10.0f, 5.0f))
			.Text(NSLOCTEXT("Chat", "ChatDestination_HUB", "EVERYONE").ToString())
			.TextStyle(SUWindowsStyle::Get(), "UT.ContextMenu.TextStyle")
			.OnClicked(this, &SUInGameHomePanel::ChangeChatDestination, ChatDestinationsButton, ChatDestinations::Global)
		];
	
		AUTLobbyPlayerState* PS = Cast<AUTLobbyPlayerState>(GetOwnerPlayerState());
		if (PS && PS->CurrentMatch)
		{
			ChatMenu->AddSlot()
			.AutoHeight()
			[
				SNew(SButton)
				.ButtonStyle(SUWindowsStyle::Get(), "UT.ContextMenu.Button")
				.ContentPadding(FMargin(10.0f, 5.0f))
				.Text(NSLOCTEXT("Chat", "ChatDestination_MATCH", "Players in Match").ToString())
				.TextStyle(SUWindowsStyle::Get(), "UT.ContextMenu.TextStyle")
				.OnClicked(this, &SUInGameHomePanel::ChangeChatDestination, ChatDestinationsButton, ChatDestinations::Match)
			];
		}

		if (PlayerOwner->IsLoggedIn())
		{
			ChatMenu->AddSlot()
			.AutoHeight()
			[
				SNew(SButton)
				.ButtonStyle(SUWindowsStyle::Get(), "UT.ContextMenu.Button")
				.ContentPadding(FMargin(10.0f, 5.0f))
				.Text(NSLOCTEXT("Chat", "ChatDestination_Friends", "Friends").ToString())
				.TextStyle(SUWindowsStyle::Get(), "UT.ContextMenu.TextStyle")
				.OnClicked(this, &SUInGameHomePanel::ChangeChatDestination, ChatDestinationsButton, ChatDestinations::Friends)
			];
		}
	}
}


void SULobbyInfoPanel::ShowMatchSetupPanel()
{
	bShowingMatchDock = false;
	BuildMatchPanel();
}

void SULobbyInfoPanel::ShowMatchDock()
{
	bShowingMatchDock = true;
	MatchData.Empty();
	BuildMatchPanel();
}

TSharedRef<SWidget> SULobbyInfoPanel::BuildChatArea()
{
	SAssignNew( Splitter, SSplitter )
	.Orientation(Orient_Horizontal)

	// Left side of the spliter is the chat, right side is the user list.

	+SSplitter::Slot()
	.Value(0.85f)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()		
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(20,36,8,16)
		[
			SAssignNew(ChatScroller, SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ChatDisplay, SRichTextBlock)
					.TextStyle(SUWindowsStyle::Get(),"UWindows.Chat.Text.Global")
					.Justification(ETextJustify::Left)
					.DecoratorStyleSet( &SUWindowsStyle::Get() )
					.AutoWrapText( true )
				]
			]
		]
	]

	// Right side of the splitter is the player list

	+SSplitter::Slot()
	.Value(0.15f)
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			SNew(SImage)
			.Image(SUWindowsStyle::Get().GetBrush(PlayerOwner->TeamStyleRef("UWindows.MidGameMenu.UserList.Background")))
		]
		+SOverlay::Slot()
		.Padding(10.0f)
		[
			SNew(SBox)
			.WidthOverride(200)
			[
				SAssignNew( UserListView, SListView< TSharedPtr<FPlayerData> > )
				// List view items are this tall
				.ItemHeight(24)
				// Tell the list view where to get its source data
				.ListItemsSource( &UserList)
				// When the list view needs to generate a widget for some data item, use this method
				.OnGenerateRow( this, &SULobbyInfoPanel::OnGenerateWidgetForList)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];

	return Splitter.ToSharedRef();
}

TSharedRef<ITableRow> SULobbyInfoPanel::OnGenerateWidgetForList( TSharedPtr<FPlayerData> InItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	FSlateFontInfo Font = SUWindowsStyle::Get().GetFontStyle("UWindows.Standard.Chat");
	return SNew(STableRow<TSharedPtr<FSimpleListData>>, OwnerTable)
		.Style(SUWindowsStyle::Get(),PlayerOwner->TeamStyleRef("UWindows.MidGameMenu.UserList.Row"))
		.Padding(5)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FLinearColor::White)
			.Font(Font)
			.Text(InItem->PlayerState->PlayerName)
		];
}

void SULobbyInfoPanel::UpdateUserList()
{
	// Grab a list of users
	AUTGameState* GS = PlayerOwner->GetWorld()->GetGameState<AUTGameState>();
	if (GS)
	{
		for (int32 i=0; i<GS->PlayerArray.Num();i++)
		{
			AUTPlayerState* PS = Cast<AUTPlayerState>(GS->PlayerArray[i]);
			if (PS != NULL)
			{
				bool bFound = false;
				// Look to see if this player is in the list.
				for (int32 j=0; j < UserList.Num(); j++)
				{
					if (UserList[i]->PlayerState.Get() == PS)
					{
						bFound = true;
					}
				}

				if (!bFound)
				{
					UserList.Add(FPlayerData::Make(PS));
				}
			}
		}

		for (auto It = UserList.CreateIterator(); It; It++)
		{
			TSharedPtr<FPlayerData> Data = *It;
			if (GS->PlayerArray.Find(Data->PlayerState.Get()) == INDEX_NONE)
			{
				UserList.Remove(Data);
			}
		}

		UserListView->RequestListRefresh();
	}
}

void SULobbyInfoPanel::UpdateChatText()
{
	if (PlayerOwner->ChatArchive.Num() != LastChatCount)
	{
		LastChatCount = PlayerOwner->ChatArchive.Num();

		FString RichText = TEXT("");
		for (int32 i=0;i<PlayerOwner->ChatArchive.Num(); i++)
		{
			TSharedPtr<FStoredChatMessage> Msg = PlayerOwner->ChatArchive[i];
			FString Style;

			if (i>0) RichText += TEXT("\n");

			if (Msg->Type == ChatDestinations::Friends)
			{
				Style = TEXT("UWindows.Chat.Text.Friends");
			}
			else if (Msg->Type == ChatDestinations::System)
			{
				Style = TEXT("UWindows.Chat.Text.Admin");
			}
			else if (Msg->Type == ChatDestinations::Lobby)
			{
				Style = TEXT("UWindows.Chat.Text.Lobby");
			}
			else if (Msg->Type == ChatDestinations::Match)
			{
				Style = TEXT("UWindows.Chat.Text.Match");
			}
			else if (Msg->Type == ChatDestinations::Local)
			{
				Style = TEXT("UWindows.Chat.Text.Local");
			}
			else if (Msg->Type == ChatDestinations::Team)
			{
				if (Msg->Color.R > Msg->Color.B)
				{
					Style = TEXT("UWindows.Chat.Text.Team.Red"); ;
				}
				else
				{
					Style = TEXT("UWindows.Chat.Text.Team.Blue"); ;
				}
			}
			else
			{
				Style = TEXT("UWindows.Chat.Text.Global"); 
			}

			RichText += FString::Printf(TEXT("[%s]%s</>"), *Msg->Type.ToString(), *Msg->Message);
		}

		ChatDisplay->SetText(FText::FromString(RichText));
		ChatScroller->SetScrollOffset(290999);
	}
}

#endif