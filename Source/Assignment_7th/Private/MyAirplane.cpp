// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAirplane.h"
#include "MyPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"

// Sets default values
AMyAirplane::AMyAirplane()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	CollisionComp->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	MeshComp->SetRelativeScale3D(FVector(0.25f));
	MeshComp->SetupAttachment(CollisionComp);;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 3000.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	ForwardSpeed = 1000.f;
	BackwardSpeed = 400.f;
	YawSpeed = 30.f;
	MouseSpeed = 90.f;

	CurrentSpeed = ForwardSpeed;
	BoostMultiplier = 1.5f;
	AccelRate = 400.f;
	DecelRate = 200.f;

	UpSpeed = 500.f;

	GroundSpeed = 1000.f;
	Height = 300.f;

	bIsFreeCamera = false;
}

// Called when the game starts or when spawned
void AMyAirplane::BeginPlay()
{
	Super::BeginPlay();

	DefaultCameraRotation = SpringArmComp->GetRelativeRotation();
	CurrentRotation = GetActorRotation();
}


// Called every frame
void AMyAirplane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsFreeCamera)
	{
		CurrentRotation.Normalize();
		SetActorRotation(CurrentRotation);
	}


	bIsGround = CheckGround();

	if (!bIsGround) //공중에 있을 때
	{
		FallSpeed = FMath::Lerp(FallSpeed, MaxFallSpeed, DeltaTime);

		FVector DeltaFallLocation = FVector(0.f, 0.f, DeltaTime * FallSpeed);
		AddActorWorldOffset(DeltaFallLocation, true);
	}
	else //땅에 있을 때
	{
		FallSpeed = 0.f;
	}

	float TargetSpeed = bIsGround ? GroundSpeed : ForwardSpeed;

	if (bIsBoosting)
	{
		TargetSpeed *= BoostMultiplier;
	}

	if (CurrentSpeed < TargetSpeed)
	{
		CurrentSpeed += AccelRate * DeltaTime;
		if (CurrentSpeed > TargetSpeed)
		{
			CurrentSpeed = TargetSpeed;
		}
	}
	else if (CurrentSpeed > TargetSpeed)
	{
		CurrentSpeed -= DecelRate * DeltaTime;
		if (CurrentSpeed < TargetSpeed)
		{
			CurrentSpeed = TargetSpeed;
		}
	}

	//실시간 이동 속도 출력
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow,
		FString::Printf(TEXT("CurrentSpeed %.2f"), CurrentSpeed));
	
	//실시간 하강 속도 출력
	GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Yellow,
		FString::Printf(TEXT("FallSpeed %.2f"), FallSpeed));
}

// Called to bind functionality to input
void AMyAirplane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &AMyAirplane::Move);
			}

			if (PlayerController->RollAction)
			{
				EnhancedInput->BindAction(PlayerController->RollAction, ETriggerEvent::Triggered, this, &AMyAirplane::Roll);
			}

			if (PlayerController->MoveUpAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveUpAction, ETriggerEvent::Triggered, this, &AMyAirplane::MoveUp);
			}

			if (PlayerController->MouseAction)
			{
				EnhancedInput->BindAction(PlayerController->MouseAction, ETriggerEvent::Triggered, this, &AMyAirplane::MouseMove);
			}

			if (PlayerController->BoostAction)
			{
				EnhancedInput->BindAction(PlayerController->BoostAction, ETriggerEvent::Triggered, this, &AMyAirplane::StartBoost);
				EnhancedInput->BindAction(PlayerController->BoostAction, ETriggerEvent::Completed, this, &AMyAirplane::StopBoost);
			}

			if (PlayerController->FreeCamera)
			{
				EnhancedInput->BindAction(PlayerController->FreeCamera, ETriggerEvent::Started, this, &AMyAirplane::ToggleFreeCamera);
			}
		}
	}
}

void AMyAirplane::MouseMove(const FInputActionValue& value)
{
	FVector2D MouseInput = value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (!Controller) return;

	if (bIsFreeCamera) 
	{
		if (!FMath::IsNearlyZero(MouseInput.X) || !FMath::IsNearlyZero(MouseInput.Y)) //자유 시점 카메라
		{
			FRotator CameraRotation = SpringArmComp->GetRelativeRotation();

			CameraRotation.Yaw += MouseInput.X * MouseSpeed * DeltaTime;
			CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch + MouseInput.Y * MouseSpeed * DeltaTime, -80.f, 80.f);

			SpringArmComp->SetRelativeRotation(CameraRotation);
		}
	}
	else
	{
		if (!FMath::IsNearlyZero(MouseInput.X) || !FMath::IsNearlyZero(MouseInput.Y)) //비행기 조종
		{
			
			// 위아래 (Pitch) - 마우스 Y
			CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + MouseInput.Y * MouseSpeed * DeltaTime, -70.f, 70.f);

			// 좌우 (Yaw) - 마우스 X
			CurrentRotation.Yaw += MouseInput.X * MouseSpeed * DeltaTime;
			CurrentRotation.Roll += MouseInput.X * MouseSpeed * DeltaTime;
		}
	}
}

void AMyAirplane::Roll(const FInputActionValue& value)
{
	if (!Controller) return;

	float RollInput = value.Get<float>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	CurrentRotation.Roll += RollInput * 180 * DeltaTime;
}

void AMyAirplane::MoveUp(const FInputActionValue& value)
{
	if (!Controller) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float MoveUpInput = value.Get<float>();

	FVector DeltaLocation = FVector(0.f, 0.f, MoveUpInput) * UpSpeed * DeltaTime;
	AddActorWorldOffset(DeltaLocation, true);
}

void AMyAirplane::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	const FVector2D MoveInput = value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X)) //앞으로는 빠르게 뒤로는 느리게
	{
		float Speed = MoveInput.X > 0 ? CurrentSpeed : BackwardSpeed;
		FVector DeltaLocation = GetActorForwardVector() * MoveInput.X * Speed * DeltaTime;

		FHitResult Hit;
		AddActorWorldOffset(DeltaLocation, true, &Hit);

		if (Hit.bBlockingHit)
		{
			// 충돌 면에 평행하게 이동
			FVector Slide = FVector::VectorPlaneProject(DeltaLocation, Hit.Normal);
			AddActorWorldOffset(Slide, true);
		}

	}

	if (!FMath::IsNearlyZero(MoveInput.Y)) //AD키는 좌우 이동이 아니라 Yaw회전
	{
		float YawRotation = MoveInput.Y * YawSpeed * DeltaTime;

		CurrentRotation.Yaw += YawRotation;

		SetActorRotation(CurrentRotation);
	}
}

void AMyAirplane::StartBoost(const FInputActionValue& value)
{
	bIsBoosting = true;
}

void AMyAirplane::StopBoost(const FInputActionValue& value)
{
	bIsBoosting = false;
}

void AMyAirplane::ToggleFreeCamera()
{
	bIsFreeCamera = !bIsFreeCamera;

	if (!bIsFreeCamera)
	{
		// 자유 시점 종료 시 카메라를 원래 위치/각도로 되돌림
		SpringArmComp->SetRelativeRotation(DefaultCameraRotation);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                                   // 키 (-1 = 새 메시지)
			1.5f,                                 // 표시 시간 (초)
			FColor::Green,                        // 글씨 색
			bIsFreeCamera
			? TEXT("Free Camera Enabled")
			: TEXT("Control Mode Enabled")     // 출력 문자열
		);
	}
}

bool AMyAirplane::CheckGround()
{
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, Height); // 아래로 Heigth 만큼
    FHitResult Hit;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    return bHit;
}

