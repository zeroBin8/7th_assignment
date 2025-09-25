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

	if (!bIsGround) //���߿� ���� ��
	{
		FallSpeed = FMath::Lerp(FallSpeed, MaxFallSpeed, DeltaTime);

		FVector DeltaFallLocation = FVector(0.f, 0.f, DeltaTime * FallSpeed);
		AddActorWorldOffset(DeltaFallLocation, true);
	}
	else //���� ���� ��
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

	//�ǽð� �̵� �ӵ� ���
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow,
		FString::Printf(TEXT("CurrentSpeed %.2f"), CurrentSpeed));
	
	//�ǽð� �ϰ� �ӵ� ���
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
		if (!FMath::IsNearlyZero(MouseInput.X) || !FMath::IsNearlyZero(MouseInput.Y)) //���� ���� ī�޶�
		{
			FRotator CameraRotation = SpringArmComp->GetRelativeRotation();

			CameraRotation.Yaw += MouseInput.X * MouseSpeed * DeltaTime;
			CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch + MouseInput.Y * MouseSpeed * DeltaTime, -80.f, 80.f);

			SpringArmComp->SetRelativeRotation(CameraRotation);
		}
	}
	else
	{
		if (!FMath::IsNearlyZero(MouseInput.X) || !FMath::IsNearlyZero(MouseInput.Y)) //����� ����
		{
			
			// ���Ʒ� (Pitch) - ���콺 Y
			CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + MouseInput.Y * MouseSpeed * DeltaTime, -70.f, 70.f);

			// �¿� (Yaw) - ���콺 X
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

	if (!FMath::IsNearlyZero(MoveInput.X)) //�����δ� ������ �ڷδ� ������
	{
		float Speed = MoveInput.X > 0 ? CurrentSpeed : BackwardSpeed;
		FVector DeltaLocation = GetActorForwardVector() * MoveInput.X * Speed * DeltaTime;

		FHitResult Hit;
		AddActorWorldOffset(DeltaLocation, true, &Hit);

		if (Hit.bBlockingHit)
		{
			// �浹 �鿡 �����ϰ� �̵�
			FVector Slide = FVector::VectorPlaneProject(DeltaLocation, Hit.Normal);
			AddActorWorldOffset(Slide, true);
		}

	}

	if (!FMath::IsNearlyZero(MoveInput.Y)) //ADŰ�� �¿� �̵��� �ƴ϶� Yawȸ��
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
		// ���� ���� ���� �� ī�޶� ���� ��ġ/������ �ǵ���
		SpringArmComp->SetRelativeRotation(DefaultCameraRotation);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                                   // Ű (-1 = �� �޽���)
			1.5f,                                 // ǥ�� �ð� (��)
			FColor::Green,                        // �۾� ��
			bIsFreeCamera
			? TEXT("Free Camera Enabled")
			: TEXT("Control Mode Enabled")     // ��� ���ڿ�
		);
	}
}

bool AMyAirplane::CheckGround()
{
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, Height); // �Ʒ��� Heigth ��ŭ
    FHitResult Hit;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    return bHit;
}

