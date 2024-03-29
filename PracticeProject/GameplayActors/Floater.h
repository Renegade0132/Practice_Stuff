// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floater.generated.h"

UCLASS()
class PRACTICEPROJECT_API AFloater : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloater();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActorMeshComponents")
	UStaticMeshComponent* StaticMesh;

	//Location used by SetActorLocation() when BeginPlay() is called
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Floater Variables")
	FVector InitialLocation;

	//Location of the Actor when dragged in from the editor
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Floater Variables")
	FVector PlacedLocation;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Floater Variables")
	FVector WorldOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	FVector InitialDirection;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Floater Variables")
	FVector InitialForce;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Floater Variables")
	FVector InitialTorque;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	bool bShouldFloat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Floater Variables")
	bool bInitializeFloaterLocations;

	//Amplitude
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	float A;

	//Period
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	float B;

	//Phase Shift
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	float C;

	//Vertical Shift
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floater Variables")
	float D;

private:

	float RunningTime;

	float BaseZLocation;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
