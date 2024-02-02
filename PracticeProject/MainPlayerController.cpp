// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset) {
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}

	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if(WEnemyHealthBar) {
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar) {
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
		
	}
	if (WPauseMenu) {
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu) {
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (EnemyHealthBar) {
		FVector2D PositionInVeiwport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInVeiwport);
		PositionInVeiwport.Y -= 50.f;

		FVector2D SizeInVeiwport(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInVeiwport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInVeiwport);
	}
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar) {
		bEnemyHealthbarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar) {
		bEnemyHealthbarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu) {
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUI;
		
		SetInputMode(InputModeGameAndUI);
		bShowMouseCursor = true;
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu) {
		bPauseMenuVisible = false;

		GameModeOnly();

		bShowMouseCursor = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible) {
		RemovePauseMenu();
	}else{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly;

	SetInputMode(InputModeGameOnly);

}
