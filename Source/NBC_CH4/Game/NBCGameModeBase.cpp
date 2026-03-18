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

	NBCPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

	// 5초 후 초기화
	GetWorldTimerManager().SetTimer(
		NBCPlayerController->NotificationTimerHandle,
		[NBCPlayerController]()
		{
			NBCPlayerController->NotificationText = FText::GetEmpty();
		},
		5.0f, false);

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

			if (UniqueDigits.Contains(C))
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
	UE_LOG(LogTemp, Warning, TEXT("%s"), *SecretNumberString);
}

void ANBCGameModeBase::PrintChatMessageString(ANBCPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	ANBCPlayerState* NBCPS = InChattingPlayerController->GetPlayerState<ANBCPlayerState>();
	if (IsValid(NBCPS) == false)
		return;


	if (InChatMessageString.Len() == 3 &&  IsGuessNumberString(InChatMessageString) == true)
	{
		// 기회를 모두 소진했을 경우
		if (NBCPS->CurrentGuessCount >= NBCPS->MaxGuessCount)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("기회를 모두 소진했습니다."));
			return;
		}

		// 정답만 제출했고 조건이 맞을때
		FString JudgeResultString = JudgeResult(SecretNumberString, InChatMessageString);
		IncreaseGuessCount(InChattingPlayerController);

		for (TActorIterator<ANBCPlayerController> It(GetWorld()); It; ++It)
		{
			ANBCPlayerController* NBCPlayerController = *It;
			if (IsValid(NBCPlayerController) == false)
				continue;

			FString CombinedMessageString = NBCPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString + TEXT(" -> ") + JudgeResultString;
			NBCPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			
			
		}
		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
	}
	else if (InChatMessageString.Len() == 3 && InChatMessageString.IsNumeric())
	{
		// 3자리이지만 규칙에 안맞을때 나오는 안내
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("다시 입력하세요."));
	}
	else
	{
		// 일반 채팅
		for (TActorIterator<ANBCPlayerController> It(GetWorld()); It; ++It)
		{
			ANBCPlayerController* NBCPlayerController = *It;
			if (IsValid(NBCPlayerController) == false)
				continue;

			FString CombinedMessageString = NBCPS->PlayerNameString + TEXT(": ") + InChatMessageString;
			NBCPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			
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

void ANBCGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *SecretNumberString);

	for (const auto& NBCPlayerController : AllPlayerControllers)
	{
		ANBCPlayerState* NBCPS = NBCPlayerController->GetPlayerState<ANBCPlayerState>();
		if (IsValid(NBCPS) == false)
			continue;

		NBCPS->CurrentGuessCount = 0;
	}
}

void ANBCGameModeBase::JudgeGame(ANBCPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ANBCPlayerState* NBCPS = InChattingPlayerController->GetPlayerState<ANBCPlayerState>();
		for (const auto& NBCPlayerController : AllPlayerControllers)
		{
			if (IsValid(NBCPS) == false)
				continue;
			FString CombinedMessageString = NBCPS->PlayerNameString + TEXT(" has won the game.");
			NBCPlayerController->NotificationText = FText::FromString(CombinedMessageString);

			// 5초 후 초기화
			GetWorldTimerManager().SetTimer(
				NBCPlayerController->NotificationTimerHandle,
				[NBCPlayerController]()
				{
					NBCPlayerController->NotificationText = FText::GetEmpty();
				},
				5.0f, false);
		}
		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& NBCPlayerController : AllPlayerControllers)
		{
			ANBCPlayerState* NBCPS = NBCPlayerController->GetPlayerState<ANBCPlayerState>();
			if (IsValid(NBCPS) == false)
				continue;

			if (NBCPS->CurrentGuessCount >= NBCPS->MaxGuessCount)
				continue;

			bIsDraw = false;
			break;
		}

		if (bIsDraw == true)
		{
			for (const auto& NBCPlayerController : AllPlayerControllers)
			{
				NBCPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				// 5초 후 초기화
				GetWorldTimerManager().SetTimer(
					NBCPlayerController->NotificationTimerHandle,
					[NBCPlayerController]()
					{
						NBCPlayerController->NotificationText = FText::GetEmpty();
					},
					5.0f, false);
			}
			ResetGame();
		}
	}
}

