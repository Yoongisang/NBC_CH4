// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBCGameModeBase.generated.h"

class ANBCPlayerController;

/**
 * 
 */
UCLASS()
class NBC_CH4_API ANBCGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	void PrintChatMessageString(ANBCPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void IncreaseGuessCount(ANBCPlayerController* InChattingPlayerController);

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ANBCPlayerController>> AllPlayerControllers;
};
