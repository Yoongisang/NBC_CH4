// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBCPlayerController.generated.h"

class UNBCChatInput;
/**
 * 
 */
UCLASS()
class NBC_CH4_API ANBCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessgeString);

	void PrintChatMessageString(const FString& InChatMessageString);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNBCChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UNBCChatInput> ChatInputWidgetInstance;

	FString ChatMessageString;
};
