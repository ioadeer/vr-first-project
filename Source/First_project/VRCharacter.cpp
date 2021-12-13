// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	// APlayerController* PC = Cast<APlayerController>(GetController());
	// CameraManager = PC->PlayerCameraManager;
	CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(this,0));
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null Pointer to Camera Manager at setup!"));
	}
	
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	PrevLocation = Camera->GetRelativeLocation();
	DestinationMarker->SetVisibility(false);
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0.0f;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

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

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"),EInputEvent::IE_Released,this, &AVRCharacter::BeginTeleport);
}

bool AVRCharacter::FindTeleportDestination(FVector& OutLocation)
{
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * MaxTeleportDistance;

	// FVector LineTracedEnd = PlayerViewPointPosition + PlayerViewPointRotation.Vector() * MaxTeleportDistance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		Params
	)) {
		UWorld* MyWorld = GetWorld();
		FNavLocation NavLocation;
		if (MyWorld)
		{
			if (UNavigationSystemV1::GetCurrent(MyWorld)->ProjectPointToNavigation(
				HitResult.Location,
				NavLocation,
				TeleportProjectionExtent
			))
			{
				/*DestinationMarker->SetVisibility(true);
				DestinationMarker->SetWorldLocation(NavLocation.Location);*/
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
	FVector Location;
	if (FindTeleportDestination(Location))
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Location);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
	}
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
	NewLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SetActorLocation(NewLocation);
	CameraManager->StartCameraFade(1, 0, FadeInTime, FColor(0));

}


