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
	// 로그인 알림 텍스트
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
	// 플레이어 이름 
	NBCPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());

	ANBCGameStateBase* NBCGameStateBase = GetGameState<ANBCGameStateBase>();
	if (IsValid(NBCGameStateBase) == false)
		return;
	// 로그인 알림 브로드 캐스트
	NBCGameStateBase->MulticastRPCBroadcastLoginMessage(NBCPS->PlayerNameString);
	// 새로 접속한 플레이어에게도 직접 출력
	NBCPlayerController->ClientRPCPrintChatMessageString(NBCPS->PlayerNameString + TEXT(" has joined the game."));
}

FString ANBCGameModeBase::GenerateSecretNumber()
{
	// 1 ~ 9 숫자 배열 
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}
	// 랜덤 시드 초기화(현재 시간 기준)
	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	// 랜덤으로 3자리 중복 없는 숫자 생성
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
		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			// 숫자가 아니거나 0 이면 실패
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}
			// 중복 체크
			if (UniqueDigits.Contains(C))
			{
				bIsUnique = false;
				break;
			}
			// 추가
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
			// 맞추면 스트라이크
			StrikeCount++;
		}
		else
		{
			// 아니면 볼
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}
	// 아무것도 못맞추면 아웃
	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ANBCGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	// 시작과 동시에 정답 생성
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
	for (const auto& NBCPlayerController : AllPlayerControllers)
	{
		// 모든 플레이어의 CurrentGuessCount를 0으로
		ANBCPlayerState* NBCPS = NBCPlayerController->GetPlayerState<ANBCPlayerState>();
		if (IsValid(NBCPS) == false)
			continue;

		NBCPS->CurrentGuessCount = 0;
	}
	// 리셋하면서 새로운 정답 번호 생성
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *SecretNumberString);
}

void ANBCGameModeBase::JudgeGame(ANBCPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		// 스트라이크 면 모든 플래이어에게 맞춘 플레이어 승리 알림
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
		// 리셋
		ResetGame();
	}
	else
	{
		// 정답자가 없다면 순회하면서 플레이어들의 CurrentGuessCount 체크
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
		// 모두 횟수를 소진했으면 무승부 알림 후 리셋
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

