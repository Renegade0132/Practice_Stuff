// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "MainCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	LeftAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftAttackCollision"));
	LeftAttackCollision->SetupAttachment(GetMesh(), FName("LeftAttackCollision"));
	
	RightAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightAttackCollision"));
	RightAttackCollision->SetupAttachment(GetMesh(), FName("RightAttackCollision"));

	bOverlappingCombatSphere = false;

	Health = 75.f;
	Health = 100;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;
	
	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;

	

	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	LeftAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::LeftAttackOnOverlapBegin);
	LeftAttackCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::LeftAttackOnOverlapEnd);

	LeftAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftAttackCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftAttackCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftAttackCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::RightAttackOnOverlapBegin);
	RightAttackCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::RightAttackOnOverlapEnd);

	RightAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightAttackCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightAttackCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightAttackCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {
			MoveToTarget(Main);
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) {
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {		
			bHasValidTarget = false;
			if (Main->CombatTarget == this) {
				Main->SetCombatTarget(nullptr);
			}
			Main->SetHasCombatTarget(false);
			Main->UpdateCombatTarget();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController) {
				AIController->StopMovement();
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {

		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {

			bHasValidTarget = true;

			Main->SetCombatTarget(this);
			Main->SetHasCombatTarget(true);

			Main->UpdateCombatTarget();

			CombatTarget = Main;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp) {

		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {
			bOverlappingCombatSphere = false;
			MoveToTarget(Main);
			CombatTarget = nullptr;

			if(Main->CombatTarget == this){
				Main->SetCombatTarget(nullptr);
				Main->bHasCombatTarget = false;
				Main->UpdateCombatTarget();
			}
			if (Main->MainPlayerController) {
				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (MainMesh) Main->MainPlayerController->RemoveEnemyHealthBar();
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

void AEnemy::LeftAttackOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (OtherActor && Alive()) {

		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {

			if (Main->HitParticles) {

				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("OnTheLeftTipSocket");
				if (TipSocket) {

					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
			if (Main->HitSound) {

				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass) {
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemy::LeftAttackOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::RightAttackOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && Alive()) {

		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main) {

			if (Main->HitParticles) {

				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("OnTheRightTipSocket");
				if (TipSocket) {

					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
			if (Main->HitSound) {

				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass) {
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemy::RightAttackOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::MoveToTarget(AMainCharacter* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController) {
		/** AI's move request to player set here */
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(20.0f);

		/** Create NavPath for AI*/
		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);
		
		/** Debug Spheres for AI nav path point to show
		TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();

		for (auto Point : PathPoints) {

			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 15.f, 1.f);
		}*/
	}
}

void AEnemy::ActivateLeftCollision()
{
	LeftAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftCollision()
{
	LeftAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightCollision()
{
	RightAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightCollision()
{
	RightAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEnemy::Attack()
{
	if(Alive() && bHasValidTarget){
		if (AIController) {

			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking) {

			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance) {
				
				
				int32 Section = FMath::RandRange(1, 3);

				
				switch (Section) {
					case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.2f);
					AnimInstance->Montage_JumpToSection(FName("LeftAttack"), CombatMontage);
						break;
					case 2:
						AnimInstance->Montage_Play(CombatMontage, 1.2f);
						AnimInstance->Montage_JumpToSection(FName("RightAttack"), CombatMontage);
						break;
					case 3:
						AnimInstance->Montage_Play(CombatMontage, 1.2f);
						AnimInstance->Montage_JumpToSection(FName("SpecialAttack"), CombatMontage);
						break;
					default:
						;
				}
			}
		}
	}
}

void AEnemy::Die(AActor* Causer)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	LeftAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMainCharacter* MainCharacter = Cast<AMainCharacter>(Causer);
	if (MainCharacter) {
		MainCharacter->UpdateCombatTarget();
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(AfterDeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere) {
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	Destroy();
}


float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f) {
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else{
		Health -= DamageAmount;
	}
	return DamageAmount;
}