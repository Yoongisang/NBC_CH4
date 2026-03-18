// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCPlayerController.h"
#include "UI/NBCChatInput.h"
#include "Kismet/KismetSystemLibrary.h"

void ANBCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == false)
		return;

	ChatInputWidgetInstance = CreateWidget<UNBCChatInput>(this, ChatInputWidgetClass);
	if (IsValid(ChatInputWidgetInstance) == false)
		return;

	ChatInputWidgetInstance->AddToViewport();
	
}
void ANBCPlayerController::SetChatMessageString(const FString& InChatMessgeString)
{
	ChatMessageString = InChatMessgeString;
	// 출력 함수로 전달
	PrintChatMessageString(ChatMessageString);
}

void ANBCPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	// 메세지 출력
	UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
}

