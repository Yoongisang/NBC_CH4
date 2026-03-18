// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBCPlayerController.generated.h"

class UNBCChatInput;
class UUserWidget;
/**
 * 
 */
UCLASS()
class NBC_CH4_API ANBCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ANBCPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;

	FTimerHandle NotificationTimerHandle;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNBCChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UNBCChatInput> ChatInputWidgetInstance;

	FString ChatMessageString;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;
};
