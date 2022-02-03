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

	bool FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation);
	void UpdateDestinationMarker();
	void UpdateBlinkers();
	void UpdateSpline(const TArray<FVector> &Path);
	void DrawTeleportPath(const TArray<FVector>& Path);
	FVector2D GetBlinkerCentre();

	UPROPERTY()
	class UCameraComponent* Camera;
	UPROPERTY()
	class UMotionControllerComponent* LeftController;
	UPROPERTY()
	class UMotionControllerComponent* RightController;

	UPROPERTY()
	class USceneComponent* VRRoot;
	
	class APlayerCameraManager* CameraManager;

	UPROPERTY(VisibleAnywhere)
	class USplineComponent* TeleportPath;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY()
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY()
	TArray<class USplineMeshComponent*> TeleportPathMeshPool;


	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void BeginTeleport();
	void Teleport();

	FVector PrevLocation;
	FVector ActualLocation;

private:
	
	UPROPERTY(EditAnywhere)
	float TeleportProjectileRadius = 10.f;

	UPROPERTY(EditAnywhere)
	float TeleportProjectileSpeed = 800.f;

	UPROPERTY(EditAnywhere)
	float TeleportSimulationTime = 2.f;
		
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

	UPROPERTY(EditDefaultsOnly)
	class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* TeleportArchMaterial;

};
