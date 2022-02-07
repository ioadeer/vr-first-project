// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "MotionControllerComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "HandController.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

	// APlayerController* PC = Cast<APlayerController>(GetController());
	// CameraManager = PC->PlayerCameraManager;
	CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(this,0));
	/*if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null Pointer to Camera Manager at setup!"));
	}*/

	
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	PrevLocation = Camera->GetRelativeLocation();
	DestinationMarker->SetVisibility(false);

	if (!BlinkerMaterialBase)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null Pointer to Blinker Material Base at begin play!"));
	}
	else
	{
		//MaterialInstance = UMaterialInstanceDynamic::Create();
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
	}

	LeftController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	if (LeftController != nullptr)
	{
		LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->SetHand(EControllerHand::Left);
		LeftController->SetOwner(this);
	}
	RightController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	if (RightController != nullptr)
	{
		RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		RightController->SetHand(EControllerHand::Right);
		RightController->SetOwner(this);
	}

}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0.0f;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateBlinkers();
	UpdateDestinationMarker();
	//DrawDebugLine(
	//	GetWorld(),
	//	PlayerViewPointPosition,
	//	LineTracedEnd,
	//	FColor(0, 255, 0),
	//	false,
	//	0.f,
	//	0,
	//	5.f
	//);
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Move_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move_X"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"),EInputEvent::IE_Released,this, &AVRCharacter::BeginTeleport);
}

bool AVRCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	// Set teleport location with camera
	/*FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * MaxTeleportDistance;*/

	// Set teleport location with right controller
	FVector Start = RightController->GetActorLocation();
	FVector Look = RightController->GetActorForwardVector();
	Look = Look.RotateAngleAxis(30, RightController->GetActorRightVector());

	FPredictProjectilePathParams Params(
		TeleportProjectileRadius,
		Start,
		Look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this
	);
	// Draw debug line for Projectile Path
	// Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.bTraceComplex = true;
	FPredictProjectilePathResult Result;
	if (UGameplayStatics::PredictProjectilePath(
		//MyWorld,
		this,
		Params,
		Result
		)
	) {
		// UWorld* MyWorld = GetWorld();
		FNavLocation NavLocation;
		UWorld* MyWorld = GetWorld();
		if (MyWorld)
		{
			if (UNavigationSystemV1::GetCurrent(MyWorld)->ProjectPointToNavigation(
				//HitResult.Location,
				Result.HitResult.Location,
				NavLocation,
				TeleportProjectionExtent
			))
			{
			for (FPredictProjectilePathPointData PointData : Result.PathData)
			{
				OutPath.Add(PointData.Location);
			}
				OutLocation = NavLocation.Location;
				return true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NULL POINTER TO WORLD"));
			return false;
		}
		return false;
	}
	else {
		return false;
	}
	// return false;
}

void AVRCharacter::UpdateDestinationMarker()
{
	TArray<FVector> Path;
	FVector Location;
	if (FindTeleportDestination(Path, Location))
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Location);
		DrawTeleportPath(Path);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
		TArray<FVector> EmptyPath;
		DrawTeleportPath(EmptyPath);
	}
}

void AVRCharacter::UpdateBlinkers()
{
	if (RadiusVsVelocity == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("NULL POINTER TO Curve"));
		return;
	}
	//if (RadiusVsVelocity == nullptr) return;
	
	float Speed = GetVelocity().Size();
	// UE_LOG(LogTemp, Warning, TEXT("Speed %f"), Speed);
	float Radius = RadiusVsVelocity->GetFloatValue(Speed);
	// UE_LOG(LogTemp, Warning, TEXT("Radius %f"), Radius);
	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);

	FVector2D Centre = GetBlinkerCentre();
	//UE_LOG(LogTemp, Warning, TEXT("Centre X : %f"), Centre.X);
	//UE_LOG(LogTemp, Warning, TEXT("Centre Y : %f"), Centre.Y);
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Centre"), FLinearColor(Centre.X, Centre.Y, 0.f));
}

void AVRCharacter::UpdateSpline(const TArray<FVector> &Path)
{
	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < Path.Num(); ++i) {
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false);
	}
	TeleportPath->UpdateSpline();
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}
	int32 SegmentNum = Path.Num() - 1;
	for (int32 i = 0; i < SegmentNum; i++)
	{
		if (TeleportPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);
			SplineMesh->SetMaterial(0, TeleportArchMaterial);
			SplineMesh->RegisterComponent();

			TeleportPathMeshPool.Add(SplineMesh);
		}
		if (TeleportPathMeshPool.IsValidIndex(i))
		{
			USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
			SplineMesh->SetVisibility(true);
			FVector StartPos, StartTangent, EndPos, EndTangent;
			TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
			TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i+1, EndPos, EndTangent);
			SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Teleport Path Mesh Pool index out of bounds."));
		}
	}
}

FVector2D AVRCharacter::GetBlinkerCentre()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();
	if (MovementDirection.IsNearlyZero())
	{
		// early return
		return FVector2D(0.5f, 0.5f);
	}
	FVector WorldStationaryLocation;
	if (FVector::DotProduct(Camera->GetForwardVector(), MovementDirection) > 0)
	{
		WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 1000;
	}
	else
	{
		WorldStationaryLocation = Camera->GetComponentLocation() - MovementDirection * 1000;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NullPtr Player Controller"));
		return FVector2D(0.5f, 0.5f);
	}
	FVector2D ScreenStationaryLocation;
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;
	return ScreenStationaryLocation;
}

void AVRCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(Camera->GetForwardVector() * AxisValue);
}

void AVRCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(Camera->GetRightVector() * AxisValue);
}

void AVRCharacter::BeginTeleport()
{
	

	if (CameraManager)
	{
		CameraManager->StartCameraFade(0, 1, FadeOutTime, FColor(0));
		FTimerHandle PlayerTeleportHandle;
		FTimerDelegate PlayerTeleportDelegate = FTimerDelegate::CreateUObject(
			this,
			&AVRCharacter::Teleport
		);
		UWorld* MyWorld = GetWorld();
		if (!MyWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("NULL POINTER TO WORLD"));
			return;
		}
		GetWorld()->GetTimerManager().SetTimer(PlayerTeleportHandle, PlayerTeleportDelegate, FadeOutTime, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Null Pointer to Camera Manager"));
	}
	
}

void AVRCharacter::Teleport()
{
	FVector NewLocation = DestinationMarker->GetComponentLocation();
	// NewLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	NewLocation += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
	SetActorLocation(NewLocation);
	CameraManager->StartCameraFade(1, 0, FadeInTime, FColor(0));

}


