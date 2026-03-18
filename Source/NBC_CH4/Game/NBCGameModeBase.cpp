// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCGameModeBase.h"
#include "NBCGameStateBase.h"
#include "Player/NBCPlayerController.h"
#include "EngineUtils.h"
#include "Player/NBCPlayerState.h"

void ANBCGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ANBCPlayerController* NBCPlayerController = Cast<ANBCPlayerController>(NewPlayer);
	if (IsValid(NBCPlayerController) == false)
		return;

	AllPlayerControllers.Add(NBCPlayerController);

	ANBCPlayerState* NBCPS = NBCPlayerController->GetPlayerState<ANBCPlayerState>();
	if (IsValid(NBCPS) == false)
		return;

	NBCPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());

	ANBCGameStateBase* NBCGameStateBase = GetGameState<ANBCGameStateBase>();
	if (IsValid(NBCGameStateBase) == false)
		return;

	NBCGameStateBase->MulticastRPCBroadcastLoginMessage(NBCPS->PlayerNameString);
	
}

FString ANBCGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ANBCGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
			break;

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

FString ANBCGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ANBCGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
}

void ANBCGameModeBase::PrintChatMessageString(ANBCPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);

		ANBCPlayerState* NBCPS = InChattingPlayerController->GetPlayerState<ANBCPlayerState>();
		if (IsValid(NBCPS) == false)
			return;

		for (TActorIterator<ANBCPlayerController> It(GetWorld()); It; ++It)
		{
			ANBCPlayerController* NBCPlayerController = *It;
			if (IsValid(NBCPlayerController) == false)
				continue;

			FString CombinedMessageString = NBCPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString + TEXT(" -> ") + JudgeResultString;
			NBCPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			
		}
	}
	else
	{
		for (TActorIterator<ANBCPlayerController> It(GetWorld()); It; ++It)
		{
			ANBCPlayerController* NBCPlayerController = *It;
			if (IsValid(NBCPlayerController) == false)
				continue;

			NBCPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			
		}
	}
}

void ANBCGameModeBase::IncreaseGuessCount(ANBCPlayerController* InChattingPlayerController)
{
	ANBCPlayerState* NBCPS = InChattingPlayerController->GetPlayerState<ANBCPlayerState>();
	if (IsValid(NBCPS) == false)
		return;

	NBCPS->CurrentGuessCount++;
}

