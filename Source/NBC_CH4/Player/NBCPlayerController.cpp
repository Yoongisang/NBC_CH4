// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCPlayerController.h"
#include "UI/NBCChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NBCGameModeBase.h"
#include "NBCPlayerState.h"

void ANBCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == false)
		return;

	ChatInputWidgetInstance = CreateWidget<UNBCChatInput>(this, ChatInputWidgetClass);
	if (IsValid(ChatInputWidgetInstance) == false)
		return;

	ChatInputWidgetInstance->AddToViewport();
	
}
void ANBCPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == false)
		return;

	ServerRPCPrintChatMessageString(InChatMessageString);
}

void ANBCPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	UKismetSystemLibrary::PrintString(this, InChatMessageString, true, true, FLinearColor::Red, 5.0f);
}

void ANBCPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ANBCPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == false)
		return;

	ANBCGameModeBase* NBCGM = Cast<ANBCGameModeBase>(GM);
	if (IsValid(NBCGM) == false)
		return;

	NBCGM->PrintChatMessageString(this, InChatMessageString);
}

