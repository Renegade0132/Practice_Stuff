// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gameframework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "PracticeSaveGame.h"
#include "ItemStorage.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom (Pulls towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 450.f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Set size for capsule
	GetCapsuleComponent()->SetCapsuleSize(42.f, 97.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	
	// Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't rotate when the controller rotates
	// Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 840.f, 0.0f);// ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.4f;

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bUseDown = false;
	bAttacking = false;
	bAttackDown = false;
	bESCDown = false;
	//Initialize Enums

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;


	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

}



// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	MainPlayerController = Cast<AMainPlayerController>(GetController());

	LoadGameNoSwitch();
	if (MainPlayerController) {
		MainPlayerController->GameModeOnly();
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }

	switch (StaminaStatus) {
	case EStaminaStatus::ESS_Normal:
		
		if (bShiftKeyDown && (bMovingForward ||bMovingRight)) {
			if (Stamina - DeltaStamina <= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else {
				Stamina -= DeltaStamina;
			}
			if(bMovingForward || bMovingRight){
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else {
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else /** Shift key up */ {
			if (Stamina + DeltaStamina >= MaxStamina) {
				Stamina = MaxStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}

		break;

	case EStaminaStatus::ESS_BelowMinimum:
		
		if (bShiftKeyDown) {
			if (Stamina - DeltaStamina <= 0.f) {
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else {
				Stamina -= DeltaStamina;
				if (bMovingForward || bMovingForward) {
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else {
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}
		else /** Shift key up */ {

			if (Stamina + DeltaStamina >= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:
		
		if (bShiftKeyDown) {
			Stamina = 0.f;
		}
		else /** Shift key up */ {
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
		
		if (Stamina + DeltaStamina >= MinSprintStamina) {
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else {
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);

		break;

	default:
		;
	}

	if (bInterpToEnemy && CombatTarget) {

		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}
	if (CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController) {
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::ShiftKeyUp);

	FInputActionBinding& PauseButton = PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMainCharacter::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMainCharacter::ESCUp);

	PauseButton.bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainCharacter::AttackDown);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AMainCharacter::AttackUp);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AMainCharacter::UseDown);
	PlayerInputComponent->BindAction("Use", IE_Released, this, &AMainCharacter::UseUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);
}



/** Functions for moving */
bool AMainCharacter::CanMove(float value) {
	if(MainPlayerController){
		return (Controller != nullptr) &&
		(value != 0.0f) && (!bAttacking) &&
		(MovementStatus != EMovementStatus::EMS_Dead);
	}
	return false;
}

void AMainCharacter::MoveForward(float value) {

	bMovingForward = false;
	if (CanMove(value)) {
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);

		bMovingForward = true;
	}
}

void AMainCharacter::MoveRight(float value) {

	bMovingRight = false;
	if (CanMove(value)) {
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);

		bMovingRight = true;
	}
}


/** Functions for looking */
void AMainCharacter::TurnAtRate(float rate)
{
	AddControllerYawInput(rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float rate)
{
	AddControllerPitchInput(rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::UseDown()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }
	bUseDown = true;
	if (ActiveOverlappingItem) {
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon) {
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
}

void AMainCharacter::UseUp()
{
	bUseDown = false;
}

void AMainCharacter::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController) {
		MainPlayerController->TogglePauseMenu();
	}
}

void AMainCharacter::ESCUp()
{
	bESCDown = true;
}

void AMainCharacter::DecrementHealth(float Amount)
{
	
	if (Health - Amount <= 0) {
		
		Health -= Amount;
		Die();
	}
	else {
		Health -= Amount;
	}
}

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0) {

		Health -= DamageAmount;
		Die();
		if (DamageCauser) {
			
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy) {
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else {
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AMainCharacter::IncrementHealth(float Amount)
{

	if (Health + Amount >= MaxHealth) {

		Health = MaxHealth;
	}
	else {
		Health += Amount;
	}
}

void AMainCharacter::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMainCharacter::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMainCharacter::Jump() {

	if (MovementStatus != EMovementStatus::EMS_Dead && !bAttacking) {
		Super::Jump();
	}
}

void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMainCharacter::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting) {

		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMainCharacter::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMainCharacter::AttackDown()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }
	bAttackDown = true;
	if (EquippedWeapon) {
		Attack();
	}
}

void AMainCharacter::AttackUp()
{
	bAttackDown = false;
}

void AMainCharacter::Attack()
{
	if(!bAttacking && MovementStatus != EMovementStatus::EMS_Dead){
		
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) {
			int32 Section = FMath::RandRange(0, 1);
			switch (Section) {
			case 0:

				AnimInstance->Montage_Play(CombatMontage, 1.7f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);

				break;
			case 1:

				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);

				break;
			default:
				;
			}
			
		}
	}
}

void AMainCharacter::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

void AMainCharacter::AttackEnd()
{
	bAttacking = false;
	bInterpToEnemy = false;
	SetInterpToEnemy(false);
	if (bAttackDown) {
		Attack();
	}
}


void AMainCharacter::ShowPickupLocations()
{
	for (int i = 0; i < PickupLocations.Num(); i++) {
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 12, FLinearColor::Green, 10.f, .5f);
	}
	for (FVector Location : PickupLocations) {
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 12, FLinearColor::Green, 10.f, .5f);
	}
}

void AMainCharacter::PlaySwingSound()
{
	if(EquippedWeapon->SwingSound){
	UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

FRotator AMainCharacter::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);

	return LookAtRotationYaw;
}


void AMainCharacter::SetEquippedWeapon(AWeapon* WeapontToSet)
{
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeapontToSet;
}

void AMainCharacter::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0) { 
		if (MainPlayerController) MainPlayerController->RemoveEnemyHealthBar();
		return; 
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy) {
		FVector MainCharacterLocation = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - MainCharacterLocation).Size();

		for (auto Actor : OverlappingActors) {
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy) {
				float DistanceToActor = (Enemy->GetActorLocation() - MainCharacterLocation).Size();
				if (DistanceToActor < MinDistance) {
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController) {
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}

	
}
	
void AMainCharacter::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();

	if (World) {
		FString CurrentLevel = World->GetMapName();
		FName CurrentLevelName(*CurrentLevel);

		if (CurrentLevelName != LevelName) {
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMainCharacter::SaveGame()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }
	UPracticeSaveGame* SaveGameInstance = Cast<UPracticeSaveGame>(UGameplayStatics::CreateSaveGameObject(UPracticeSaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	if (EquippedWeapon) {

		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->WeaponName;
	}


	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMainCharacter::LoadGame(bool SetPosition)
{
	UPracticeSaveGame* LoadGameInstance = Cast<UPracticeSaveGame>(UGameplayStatics::CreateSaveGameObject(UPracticeSaveGame::StaticClass()));

	LoadGameInstance = Cast<UPracticeSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina= LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		
		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;
			
			if(Weapons->WeaponMap.Contains(WeaponName)){
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
	

	if (SetPosition) {
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
	
	if (LoadGameInstance->CharacterStats.LevelName != TEXT("")) {
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		
		SwitchLevel(LevelName);
	}
}

void AMainCharacter::LoadGameNoSwitch()
{
	UPracticeSaveGame* LoadGameInstance = Cast<UPracticeSaveGame>(UGameplayStatics::CreateSaveGameObject(UPracticeSaveGame::StaticClass()));

	LoadGameInstance = Cast<UPracticeSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);

		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}
