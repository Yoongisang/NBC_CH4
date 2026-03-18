// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCChatInput.h"
#include "Components/EditableTextBox.h"
#include "Player/NBCPlayerController.h"

void UNBCChatInput::NativeConstruct()
{
	Super::NativeConstruct();
	// TextBox가 이미바인딩이 돼있다면 리턴 
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
		return;
	// 아니라면 바인드
	EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	
}

void UNBCChatInput::NativeDestruct()
{
	Super::NativeDestruct();
	// TextBox가 바인딩이 안돼있다면 리턴
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
		return;
	// 아니라면 바인드 해제
	EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	
}

void UNBCChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 엔터키가 아니면 리턴
	if (CommitMethod != ETextCommit::OnEnter)
		return;
	
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (IsValid(OwningPlayerController) == false)
		return;
	ANBCPlayerController* OwningNBCPlayerController = Cast<ANBCPlayerController>(OwningPlayerController);
	if (IsValid(OwningNBCPlayerController) == false)
		return;
	// 플레이어 컨트롤러의 ChatMessageString에 TextBox에 입력된 텍스트 전달
	OwningNBCPlayerController->SetChatMessageString(Text.ToString());
	// 입력창 초기화
	EditableTextBox_ChatInput->SetText(FText());
}
