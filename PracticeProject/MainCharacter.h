// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(Display = "Normal"),
	ESS_BelowMinimum UMETA(Display = "BelowMinimum"),
	ESS_Exhausted UMETA(Display = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(Display = "ExhaustedRecovering"),

	ESS_MAX UMETA(Display = "DefaultMax")
};


UCLASS()
class PRACTICEPROJECT_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

	TArray<FVector> PickupLocations;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	/** Camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	/** Base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;
	/**
	
	
	Player Stats	
	

	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerStats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
	float Health;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerStats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
	int32 Coins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Stamina properties */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	/** Interp properties */

	float InterpSpeed;

	bool bInterpToEnemy;

	/** Weapon variable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem;

	/** Related to weapon equiping */
	bool bUseDown;

	/** Related to attacking */
	bool bAttackDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for forwards/backwards input */
	void MoveForward(float value);

	/** Called for side to side input */
	void MoveRight(float value);

	/** Called via input to turn at a given rate
		@param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float rate);

	/** Called via input to look up/down at a given rate
		@param Rate This is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float rate);

	/** Weapon Equiping */

	void UseDown();

	void UseUp();

	/** Health and Stamina changing functions */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController *EventInstigator, AActor * DamageCauser) override;

	void DecrementHealth(float Amount);
	
	void IncrementHealth(float Amount);

	void IncrementCoins(int32 Amount);

	/** Called when the character dies*/
	void Die();

	/** Set Movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	/** Pressed down to enable sprinting */
	void ShiftKeyDown();

	/** Released to stop sprinting */
	void ShiftKeyUp();

	void AttackDown();

	void AttackUp();

	void Attack();

	void SetInterpToEnemy(bool Interp);

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	/** Shows Pickup locations */
	UFUNCTION(BlueprintCallable)
	void ShowPickupLocations();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();


	FRotator GetLookAtRotationYaw(FVector Target);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}
	
	FORCEINLINE class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status)
	{
		StaminaStatus = Status;
	}

	FORCEINLINE AWeapon* GetEquippedWeapon()
	{
		return EquippedWeapon;
	}

	void SetEquippedWeapon(AWeapon* WeapontToSet);

	FORCEINLINE void SetActiveOverlappingItem(AItem* ItemtToSet)
	{
		ActiveOverlappingItem = ItemtToSet;
	}

	FORCEINLINE void SetCombatTarget(AEnemy* Target)
	{
		CombatTarget = Target;
	}
};
