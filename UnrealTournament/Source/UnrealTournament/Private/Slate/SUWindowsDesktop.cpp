// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "../UnrealTournament.h"
#include "../Public/UTLocalPlayer.h"
#include "Slate.h"
#include "Slate/SlateGameResources.h"
#include "SUWindowsDesktop.h"
#include "SUWindowsStyle.h"
#include "SUWSystemSettingsDialog.h"
#include "SUWPlayerSettingsDialog.h"
#include "SUWInputBox.h"

void SUWindowsDesktop::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	MenuBar = NULL;

	ChildSlot
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.MaxHeight(26)
			.HAlign(HAlign_Fill)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					[
						SNew(SImage)
						.Image(SUWindowsStyle::Get().GetBrush("UWindows.Standard.MenuBar.Background"))
					]
				]
				+SOverlay::Slot()
				[
					BuildMenuBar()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				SNew(SCanvas)
			]
		];
}

/****************************** [ Build Sub Menus ] *****************************************/

TSharedRef<SWidget> SUWindowsDesktop::BuildMenuBar()
{
	SAssignNew(MenuBar, SHorizontalBox);
	if (MenuBar.IsValid())
	{
		BuildFileSubMenu();
		BuildGameSubMenu();
		BuildOptionsSubMenu();
	}

	return MenuBar.ToSharedRef();
}

/**
 *	TODO: Look at converting these into some form of a generic menu bar system.  It's pretty ugly
 *  right now and can probably be converted in to something simple. But I need to figure out
 *  how to get underlining working so I can setup hotkeys first.
 **/

void SUWindowsDesktop::BuildFileSubMenu()
{
	TSharedPtr<SComboButton> DropDownButton = NULL;

	SAssignNew(DropDownButton, SComboButton)
	.Method(SMenuAnchor::UseCurrentWindow)
	.HasDownArrow(false)
	.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuButton")
	.ButtonContent()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT("SUWindowsDesktop","MenuBar_File","File").ToString())
	];
	
	if ( DropDownButton.IsValid() ) 
	{
		TSharedPtr<SVerticalBox> Menu = NULL;
	
		SAssignNew(Menu, SVerticalBox);
		if (Menu.IsValid())
		{
			if (GWorld->GetWorld()->GetNetMode() == NM_Client)
			{
				(*Menu).AddSlot()
				.AutoHeight()
				[
					SNew(SButton)
					.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
					.ContentPadding(FMargin(10.0f, 5.0f))
					.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_File_Disconnect", "Disconnect").ToString())
					.OnClicked(this, &SUWindowsDesktop::OnMenuConsoleCommand, FString(TEXT("Disconnect")))
				];
			}
			if (PlayerOwner->PlayerController != NULL)
			{
				const FURL& LastNetURL = GEngine->GetWorldContextFromWorldChecked(PlayerOwner->PlayerController->GetWorld()).LastRemoteURL;
				if (LastNetURL.Valid && LastNetURL.Host.Len() > 0)
				{
					(*Menu).AddSlot()
					.AutoHeight()
					[
						SNew(SButton)
						.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
						.ContentPadding(FMargin(10.0f, 5.0f))
						.Text(FText::Format(NSLOCTEXT("SUWindowsDesktop", "MenuBar_File_Reconnect", "Reconnect to {Addr}"), FText::FromString(FString::Printf(TEXT("%s:%i"), *LastNetURL.Host, LastNetURL.Port))).ToString())
						.OnClicked(this, &SUWindowsDesktop::OnMenuConsoleCommand, FString(TEXT("Reconnect")))
					];
				}
			}

			(*Menu).AddSlot()
			.AutoHeight()
			[
				SNew(SButton)
				.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
				.ContentPadding(FMargin(10.0f, 5.0f))
				.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_File_ConnectIP", "Connect To IP").ToString())
				.OnClicked(this, &SUWindowsDesktop::OnConnectIP)
			];

			(*Menu).AddSlot()
			.AutoHeight()
			[
				SNew(SButton)
				.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
				.ContentPadding(FMargin(10.0f, 5.0f))
				.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_File_ExitGame", "Exit Game").ToString())
				.OnClicked(this, &SUWindowsDesktop::OnMenuConsoleCommand, FString(TEXT("quit")))
			];

		}
		DropDownButton->SetMenuContent(Menu.ToSharedRef());
	}

	MenuBar->AddSlot()
	.AutoWidth()
	.Padding(FMargin(10.0f,0.0f,0.0f,0.0f))
	.HAlign(HAlign_Fill)
	[
		DropDownButton.ToSharedRef()
	];
}

void SUWindowsDesktop::BuildGameSubMenu()
{
	TSharedPtr<SComboButton> DropDownButton = NULL;

	SAssignNew(DropDownButton, SComboButton)
	.Method(SMenuAnchor::UseCurrentWindow)
	.HasDownArrow(false)
	.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuButton")
	.ButtonContent()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT("SUWindowsDesktop","MenuBar_Game","Game").ToString())
	];
	
	if ( DropDownButton.IsValid() ) 
	{
		TSharedPtr<SVerticalBox> Menu = NULL;
	
		SAssignNew(Menu, SVerticalBox);
		if (Menu.IsValid() && PlayerOwner != NULL && PlayerOwner->PlayerController != NULL)
		{

			AUTGameState* GS = GWorld->GetWorld()->GetGameState<AUTGameState>();
			if (GS != NULL && GS->bTeamGame)
			{
				AUTPlayerState* PS = Cast<AUTPlayerState>(PlayerOwner->PlayerController->PlayerState);
				if (PS != NULL && PS->Team != NULL)
				{
					if (PS->Team->GetTeamNum() == 0)
					{
						(*Menu).AddSlot()
						.AutoHeight()
						[
							SNew(SButton)
							.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
							.ContentPadding(FMargin(10.0f, 5.0f))
							.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Game_ChangeTeamBlue", "Switch to Blue").ToString())
							.OnClicked(this, &SUWindowsDesktop::OnChangeTeam, 1)
						];
					}
					else
					{
						(*Menu).AddSlot()
						.AutoHeight()
						[
							SNew(SButton)
							.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
							.ContentPadding(FMargin(10.0f, 5.0f))
							.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Game_ChangeTeamRed", "Switch to Red").ToString())
							.OnClicked(this, &SUWindowsDesktop::OnChangeTeam, 0)
						];
					}
				}
			}

			(*Menu).AddSlot()
			.AutoHeight()
			[
				SNew(SButton)
				.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
				.ContentPadding(FMargin(10.0f, 5.0f))
				.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Game_Suicide", "Suicide").ToString())
				.OnClicked(this, &SUWindowsDesktop::OnMenuConsoleCommand, FString(TEXT("suicide")))
			];
		}
		DropDownButton->SetMenuContent(Menu.ToSharedRef());
	}

	MenuBar->AddSlot()
	.AutoWidth()
	.Padding(FMargin(10.0f,0.0f,0.0f,0.0f))
	.HAlign(HAlign_Fill)
	[
		DropDownButton.ToSharedRef()
	];
}

void SUWindowsDesktop::BuildOptionsSubMenu()
{
	TSharedPtr<SComboButton> DropDownButton = NULL;

	SAssignNew(DropDownButton, SComboButton)
		.Method(SMenuAnchor::UseCurrentWindow)
		.HasDownArrow(false)
		.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuButton")
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Options", "Options").ToString())
		];

	DropDownButton->SetMenuContent
	(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
			.ContentPadding(FMargin(10.0f, 5.0f))
			.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Options_PlayerSettings", "Player Settings").ToString())
			.OnClicked(this, &SUWindowsDesktop::OpenPlayerSettings)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(SUWindowsStyle::Get(), "UWindows.Standard.MenuList")
			.ContentPadding(FMargin(10.0f, 5.0f))
			.Text(NSLOCTEXT("SUWindowsDesktop", "MenuBar_Options_SystemSettings", "System Settings").ToString())
			.OnClicked(this, &SUWindowsDesktop::OpenSystemSettings)
		]
	);

	MenuBar->AddSlot()
		.AutoWidth()
		.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
		.HAlign(HAlign_Fill)
		[
			DropDownButton.ToSharedRef()
		];
}

/****************************** [ Other ] *****************************************/


void SUWindowsDesktop::OnMenuOpened()
{
	GameViewportWidget = FSlateApplication::Get().GetKeyboardFocusedWidget();
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EKeyboardFocusCause::Keyboard);
}

void SUWindowsDesktop::OnMenuClosed()
{
	FSlateApplication::Get().SetKeyboardFocus(GameViewportWidget);
}

bool SUWindowsDesktop::SupportsKeyboardFocus() const
{
	return true;
}

FReply SUWindowsDesktop::OnKeyboardFocusReceived( const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent )
{
	return FReply::Handled().ReleaseMouseCapture();

}

FReply SUWindowsDesktop::OnKeyUp( const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent )
{
	if (InKeyboardEvent.GetKey() == EKeys::Escape)
	{
		CloseMenus();
		return FReply::Handled();
	}
	else
	{
		return FReply::Unhandled();
	}
}

void SUWindowsDesktop::CloseMenus()
{
	if (PlayerOwner != NULL)
	{
		PlayerOwner->HideMenu();
	}
}


FReply SUWindowsDesktop::OnKeyDown( const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent )
{
	return FReply::Handled();
}


FReply SUWindowsDesktop::OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	return FReply::Handled();
}

FReply SUWindowsDesktop::OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	CloseMenus();
	return FReply::Handled();
}

FReply SUWindowsDesktop::OnChangeTeam(int32 NewTeamIndex)
{
	return OnMenuConsoleCommand(FString::Printf(TEXT("changeteam %i"), NewTeamIndex));
}
FReply SUWindowsDesktop::OnMenuConsoleCommand(FString Command)
{
	if (PlayerOwner.IsValid())
	{
		PlayerOwner->ViewportClient->ConsoleCommand(Command);
	}

	CloseMenus();
	return FReply::Handled();
}
FReply SUWindowsDesktop::OpenPlayerSettings()
{
	PlayerOwner->OpenDialog(SNew(SUWPlayerSettingsDialog).PlayerOwner(PlayerOwner));
	return FReply::Handled();
}
FReply SUWindowsDesktop::OpenSystemSettings()
{
	PlayerOwner->OpenDialog(SNew(SUWSystemSettingsDialog).PlayerOwner(PlayerOwner));
	return FReply::Handled();
}

/** last input to connect IP dialog */
static FString LastIP;

FReply SUWindowsDesktop::OnConnectIP()
{
	PlayerOwner->OpenDialog(
							SNew(SUWInputBox)
							.DefaultInput(LastIP)
							.OnDialogResult(this, &SUWindowsDesktop::ConnectIPDialogResult)
							.PlayerOwner(PlayerOwner)
							.MessageTitle(NSLOCTEXT("SUWindowsDesktop", "ConnectToIP", "Connect to IP"))
							.MessageText(NSLOCTEXT("SUWindowsDesktop", "ConnecToIPDesc", "Enter address to connect to:"))
							);
	return FReply::Handled();
}
void SUWindowsDesktop::ConnectIPDialogResult(const FString& InputText, bool bCancelled)
{
	if (!bCancelled && InputText.Len() > 0 && PlayerOwner.IsValid())
	{
		FString AdjustedText = InputText.Replace(TEXT("://"), TEXT(""));
		LastIP = AdjustedText;
		PlayerOwner->ViewportClient->ConsoleCommand(*FString::Printf(TEXT("open %s"), *AdjustedText));
	}
}
