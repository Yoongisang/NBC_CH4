// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCPlayerController.h"
#include "UI/NBCChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NBCGameModeBase.h"
#include "NBCPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBCPlayerController::ANBCPlayerController()
{
	bReplicates = true;
}

void ANBCPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 복제할 프로퍼티 등록
	DOREPLIFETIME(ThisClass, NotificationText);
}

void ANBCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == false)
		return;
	// 채팅 UI
	ChatInputWidgetInstance = CreateWidget<UNBCChatInput>(this, ChatInputWidgetClass);
	if (IsValid(ChatInputWidgetInstance) == false)
		return;

	// 알림 UI
	if (IsValid(NotificationTextWidgetClass) == false)
		return;

	NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
	if (IsValid(NotificationTextWidgetInstance) == false)
		return;

	ChatInputWidgetInstance->AddToViewport();
	NotificationTextWidgetInstance->AddToViewport();
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

