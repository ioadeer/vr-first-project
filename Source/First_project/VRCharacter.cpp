// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	PrevLocation = Camera->GetRelativeLocation();
	
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//PrevLocation = Camera->GetRelativeLocation();
	// PrevLocation = Camera->GetComponentLocation();
	// UE_LOG(LogTemp, Warning, TEXT("Camera component x and y location %f  %f"), PrevLocation.X, PrevLocation.Y);
	// FVector ActorLocation = GetActorLocation();
	// UE_LOG(LogTemp, Warning, TEXT("Actor x location %f  , %f"), ActorLocation.X, ActorLocation.Y);
	// float XDiff = FMath::FInterpTo(0, ActorLocation.X - PrevLocation.X, DeltaTime, 0.05f);
	// float YDiff = FMath::FInterpTo(0, ActorLocation.Y -PrevLocation.Y, DeltaTime, 0.05f);
	// FVector Offset(XDiff, YDiff, 0.f);
	// VRRoot->AddRelativeLocation(Offset);
	// FVector Final = VRRoot->GetComponentLocation();
	// UE_LOG(LogTemp, Warning, TEXT("VRoot x and y location %f, %f"), Final.X, Final.Y);
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0.0f;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);	
		
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AVRCharacter::MoveRight);

}

void AVRCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(Camera->GetForwardVector() * AxisValue);
}

void AVRCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(Camera->GetRightVector() * AxisValue);
}

