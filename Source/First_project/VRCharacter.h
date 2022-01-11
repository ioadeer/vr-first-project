// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()

class FIRST_PROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	bool FindTeleportDestination(FVector &OutLocation);
	void UpdateDestinationMarker();
	void UpdateBlinkers();
	FVector2D GetBlinkerCentre();

	UPROPERTY()
	class UCameraComponent* Camera;
	UPROPERTY()
	class USceneComponent* VRRoot;
	
	class APlayerCameraManager* CameraManager;

	UPROPERTY()
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* BlinkerMaterialInstance;
	//class UObject* BlinkerMaterialInstance;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void BeginTeleport();
	void Teleport();

	FVector PrevLocation;
	FVector ActualLocation;

private:
	UPROPERTY(EditAnywhere)
	float MaxTeleportDistance = 1000.f;
		
	UPROPERTY(EditAnywhere)
	float FadeOutTime = 0.25f;

	UPROPERTY(EditAnywhere)
	float FadeInTime = 0.25f;
	
	UPROPERTY(EditAnywhere)
	FVector TeleportProjectionExtent = FVector(100.f, 100.f, 100.f);

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* RadiusVsVelocity;
};
