
// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "../Public/UnrealTournament.h"
#include "SUTChallengePanel.h"
#include "../Public/UTLocalPlayer.h"
#include "SlateBasics.h"
#include "../SUWScaleBox.h"
#include "../Widgets/SUTButton.h"
#include "UTChallengeManager.h"
#include "UTAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Analytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"
#include "../SUWindowsMainMenu.h"

#if !UE_SERVER

void SUTChallengePanel::ConstructPanel(FVector2D ViewportSize)
{
	SelectedChallenge = NAME_None;

	this->ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SUWScaleBox)
					.bMaintainAspectRatio(false)
					[
						SNew(SImage)
						.Image(SUTStyle::Get().GetBrush("UT.HomePanel.Background"))
					]
				]
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(64.0, 15.0, 64.0, 15.0)
			.FillHeight(1.0)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(1792).HeightOverride(860)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f,0.0f,8.0f,0.0f)
						[
							SNew(SBox).WidthOverride(900)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.Padding(0.0,0.0,0.0,16.0)
								.AutoHeight()
								[
									SNew(SBorder)
									.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot()
										.HAlign(HAlign_Center)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("SUTChallengePanel","AvailableChallenges","AVAILABLE CHALLENGES"))
											.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium.Bold")
										]
									]
								]

								+SVerticalBox::Slot()
								.Padding(0.0,0.0,0.0,16.0)
								.FillHeight(1.0)
								[
									SNew(SBorder)
									.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
									[
										SNew(SScrollBox)
										.Style(SUWindowsStyle::Get(),"UT.ScrollBox.Borderless")
										+SScrollBox::Slot()
										[
											SAssignNew(ChallengeBox,SVerticalBox)
										]
									]
								]

								+SVerticalBox::Slot()
								.Padding(0.0,0.0,0.0,16.0)
								.AutoHeight()
								[
									SNew(SBorder)
									.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
									[
										SNew(SButton)
										.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
										.OnClicked(this, &SUTChallengePanel::CustomClicked)
										[
											SNew(SHorizontalBox)
											+SHorizontalBox::Slot()
											.HAlign(HAlign_Center)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("SUTChallengePanel","CustomChallenge","Create your own game."))
												.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium")
											]
										]									
									]
								]

							]
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(8.0f,0.0f,0.0f,0.0f)
						[
							SNew(SBox).WidthOverride(876)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.Padding(0.0,0.0,0.0,16.0)
								.AutoHeight()
								[
									SNew(SBox).HeightOverride(64)
									[
										SNew(SBorder)
										.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.SuperDark"))
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot()
											.HAlign(HAlign_Right)
											[
												SNew(SHorizontalBox)
												+SHorizontalBox::Slot()
												[
													SNew(STextBlock)
													.Text(this, &SUTChallengePanel::GetYourScoreText)
													.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Large")
												]
											]
										]
									]
								]
								+SVerticalBox::Slot()
								.Padding(0.0,0.0,0.0,16.0)
								.FillHeight(1.0)
								[
									SNew(SBorder)
									.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Medium"))
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot()
										.FillHeight(1.0)
										[
											SNew(SScrollBox)
											+ SScrollBox::Slot()
											.Padding(FMargin(0.0f, 5.0f, 0.0f, 5.0f))
											[
												SAssignNew(ChallengeDescription, SRichTextBlock)
												.Text(NSLOCTEXT("SUTChallengePanel", "Description", "This is the description"))	// Change me to a delegate call
												.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Medium")
												.Justification(ETextJustify::Center)
												.DecoratorStyleSet(&SUTStyle::Get())
												.AutoWrapText(true)
											]
										]
										+SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SBox).HeightOverride(64)
											[
												SNew(SVerticalBox)
												+SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(STextBlock)
													.Text(this,&SUTChallengePanel::GetCurrentScoreText)
													.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium.Bold")
												]
												+SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(STextBlock)
													.Text(this,&SUTChallengePanel::GetCurrentChallengeData)
													.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Small.Bold")
												]
											
											]
										]
									]
								]
								+SVerticalBox::Slot()
								.AutoHeight()
								[

									SNew(SBox).HeightOverride(148)
									[
										SNew(SBorder)
										.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.SuperDark"))
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot()
											.HAlign(HAlign_Center)
											.Padding(0.0,5.0)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("SUTChallengePanel","DifficultyText","Select Difficulty"))
												.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Small.Bold")
											]
											+SVerticalBox::Slot()
											.HAlign(HAlign_Fill)
											.Padding(0.0,5.0)
											.AutoHeight()
											[
												SNew(SBox).HeightOverride(96)
												[
													SNew(SHorizontalBox)
													+SHorizontalBox::Slot()
													.Padding(0.0,0.0,3.0,0.0)
													[
														SNew(SBorder)
														.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
														[
															SNew(SButton)
															.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
															.OnClicked(this, &SUTChallengePanel::StartClicked, 0)
															[
																SNew(SVerticalBox)
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(STextBlock)
																	.Text(NSLOCTEXT("SUTChallengePanel","Level_Easy","Easy"))
																	.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium")

																]
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(SHorizontalBox)
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star.Outline"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star.Outline"))
																		]
																	]
																]
															]
														]
													]
													+SHorizontalBox::Slot()
													.Padding(0.0,0.0,3.0,0.0)
													[
														SNew(SBorder)
														.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
														[
															SNew(SButton)
															.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
															.OnClicked(this, &SUTChallengePanel::StartClicked, 1)
															[
																SNew(SVerticalBox)
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(STextBlock)
																	.Text(NSLOCTEXT("SUTChallengePanel","Level_Medium","Medium"))
																	.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium")

																]
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(SHorizontalBox)
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star.Outline"))
																		]
																	]
																]
															]
														]										
													]
													+SHorizontalBox::Slot()
													[
														SNew(SBorder)
														.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
														[
															SNew(SButton)
															.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
															.OnClicked(this, &SUTChallengePanel::StartClicked, 2)
															[
																SNew(SVerticalBox)
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(STextBlock)
																	.Text(NSLOCTEXT("SUTChallengePanel","Level_Hard","Hard"))
																	.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium")

																]
																+SVerticalBox::Slot()
																.AutoHeight()
																.HAlign(HAlign_Center)
																[
																	SNew(SHorizontalBox)
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																	+SHorizontalBox::Slot()
																	.AutoWidth()
																	[
																		SNew(SBox).WidthOverride(32).HeightOverride(32)
																		[
																			SNew(SImage)
																			.Image(SUTStyle::Get().GetBrush("UT.Star"))
																		]
																	]
																]
															]
														]										
													]
												]
											]
										]
									]
								]
							]
						]
					]
				]
			]
		]
	];

	GenerateChallengeList();

}

void SUTChallengePanel::GenerateChallengeList()
{
	ChallengeManager = UUTChallengeManager::StaticClass()->GetDefaultObject<UUTChallengeManager>();
	if (ChallengeManager.IsValid())
	{
		int32 ButtonIndex = 0;
		for (auto It = ChallengeManager->Challenges.CreateConstIterator(); It; ++It)
		{
			if (SelectedChallenge == NAME_None) 
			{
				SelectedChallenge = It.Key();
			}

			TSharedPtr<SUTButton> Button;

			const FUTChallengeInfo Challenge = It.Value();
			FName ChallengeTag = It.Key();

			ChallengeBox->AddSlot()
			.Padding(10.0,6.0,10.0,6.0)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SBox).WidthOverride(860).HeightOverride(96)
					[
						SAssignNew(Button, SUTButton)
						.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
						.IsToggleButton(true)
						.OnClicked(this, &SUTChallengePanel::ChallengeClicked, ChallengeTag)
						[
							SNew(SBorder)
							.BorderImage(SUTStyle::Get().GetBrush(Challenge.SlateUIImageName))
							[
								SNew(SOverlay)
								+SOverlay::Slot()
								[
									CreateCheck(ChallengeTag)
								]
								+SOverlay::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(70.0,0.0,0.0,0.0)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(FText::FromString(Challenge.Title))
										.TextStyle(SUTStyle::Get(),"UT.Font.NormalText.Medium.Bold")
										.ShadowOffset(FVector2D(2.0f,2.0f))
										.ShadowColorAndOpacity(FLinearColor(0.0f,0.0f,0.0f,0.5))
									]
								]
								+SOverlay::Slot()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.HAlign(HAlign_Right)
									[
										CreateStars(ChallengeTag)
									]
								]
							]
						]
					]
				]
			];

			ButtonMap.Add(ChallengeTag, Button);
		}
	}

	ChallengeClicked(SelectedChallenge);
}

TSharedRef<SWidget> SUTChallengePanel::CreateCheck(FName ChallengeTag)
{
	int32 NoStars = PlayerOwner->GetChallengeStars(ChallengeTag);
	if (NoStars > 0)
	{
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(0.0,0.0,2.0,0.0)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox).WidthOverride(64).HeightOverride(64)
				[
					SNew(SImage)
					.Image(SUTStyle::Get().GetBrush("UT.Icon.Checkmark"))
				]
			]
		];
	}

	return SNew(SCanvas);
}

TSharedRef<SWidget> SUTChallengePanel::CreateStars(FName ChallengeTag)
{
	int32 NoStars = PlayerOwner->GetChallengeStars(ChallengeTag);
	TSharedPtr<SHorizontalBox> Box;
	SAssignNew(Box, SHorizontalBox);

	for (int32 i=0; i < 3; i++)
	{
		if (i < NoStars)
		{
			Box->AddSlot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(0.0,0.0,2.0,0.0)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox).WidthOverride(32).HeightOverride(32)
					[
						SNew(SImage)
						.Image(SUTStyle::Get().GetBrush("UT.Star"))
					]
				]
			];
		}
		else
		{
			Box->AddSlot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(0.0,0.0,2.0,0.0)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox).WidthOverride(32).HeightOverride(32)
					[
						SNew(SImage)
						.Image(SUTStyle::Get().GetBrush("UT.Star.Outline"))
					]
				]
			];
		}
	}

	return Box.ToSharedRef();
}

FText SUTChallengePanel::GetYourScoreText() const
{
	return FText::Format(NSLOCTEXT("SUTChallengePanel","StarTotalFormat","You have earned a total of {0} stars"), FText::AsNumber(PlayerOwner->GetTotalChallengeStars()));
}

FText SUTChallengePanel::GetCurrentScoreText() const
{
	return FText::Format(NSLOCTEXT("SUTChallengePanel","StarsForChallengeFormat","You have earned {0} stars for this challenge"), FText::AsNumber(PlayerOwner->GetChallengeStars(SelectedChallenge)));
}

FText SUTChallengePanel::GetCurrentChallengeData() const
{
	return FText::Format(NSLOCTEXT("SUTChallengePanel","DateForChallengeFormat","Last Completed: {0}"), FText::FromString(PlayerOwner->GetChallengeDate(SelectedChallenge)));
}


FReply SUTChallengePanel::ChallengeClicked(FName ChallengeTag)
{
	if (ChallengeManager.IsValid() && ChallengeManager->Challenges.Contains(ChallengeTag))
	{
		FString Description = ChallengeManager->Challenges[ChallengeTag].Description;
		ChallengeDescription->SetText(FText::FromString(Description));
		SelectedChallenge = ChallengeTag;

		for (auto It = ButtonMap.CreateConstIterator(); It; ++It)
		{
			TSharedPtr<SUTButton> Button = It.Value();
			FName Key = It.Key();
			if (Key == ChallengeTag)
			{
				Button->BePressed();
			}
			else
			{
				Button->UnPressed();
			}
		}
	}

	return FReply::Handled();
}

FReply SUTChallengePanel::StartClicked(int32 Difficulty)
{
	if (ChallengeManager.IsValid() && ChallengeManager->Challenges.Contains(SelectedChallenge))
	{
		const FUTChallengeInfo Challenge = ChallengeManager->Challenges[SelectedChallenge];

		// Kill any existing Dedicated servers
		if (PlayerOwner->DedicatedServerProcessHandle.IsValid())
		{
			FPlatformProcess::TerminateProc(PlayerOwner->DedicatedServerProcessHandle,true);
			PlayerOwner->DedicatedServerProcessHandle.Reset();
		}

		if (FUTAnalytics::IsAvailable())
		{
			TArray<FAnalyticsEventAttribute> ParamArray;
			ParamArray.Add(FAnalyticsEventAttribute(TEXT("Challenge"), SelectedChallenge.ToString()));
			ParamArray.Add(FAnalyticsEventAttribute(TEXT("Difficulty"), Difficulty));
			FUTAnalytics::GetProvider().RecordEvent( TEXT("StartChallenge"), ParamArray );
		}

		FString Options = FString::Printf(TEXT("%s?Game=%s?Challenge=%s?ChallengeDiff=%i"), *Challenge.Map, *Challenge.GameMode, *SelectedChallenge.ToString(), Difficulty);
		ConsoleCommand(TEXT("Open ") + Options);
	}
		
	return FReply::Handled();
}

FReply SUTChallengePanel::CustomClicked()
{
	TSharedPtr<SUWindowsMainMenu> MainMenu = StaticCastSharedPtr<SUWindowsMainMenu>(PlayerOwner->GetCurrentMenu());
	if (MainMenu.IsValid())
	{
		MainMenu->ShowCustomGamePanel();
	}

	return FReply::Handled();
}



#endif