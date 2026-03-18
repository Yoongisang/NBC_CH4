// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/NBCPlayerController.h"

void ANBCGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == true)
		return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
		return;
	ANBCPlayerController* NBCPC = Cast<ANBCPlayerController>(PC);
	if (IsValid(NBCPC) == false)
		return;

	FString NotificationString = InNameString + TEXT(" has joined the game.");
	NBCPC->PrintChatMessageString(NotificationString);
}
