// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyAirplane.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;

struct FInputActionValue;

UCLASS()
class ASSIGNMENT_7TH_API AMyAirplane : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyAirplane();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UBoxComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	void MouseMove(const FInputActionValue& value);
	void Roll(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void Move(const FInputActionValue& value);
	void StartBoost(const FInputActionValue& value);
	void StopBoost(const FInputActionValue& value);
	void ToggleFreeCamera();

	bool CheckGround(); //���� Ȯ��
	bool bIsGround;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float BackwardSpeed;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float YawSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse")
	float MouseSpeed;

	FRotator CurrentRotation;  // ������ ȸ������ �ϳ��� ����

	float CurrentSpeed;        // ���� �ӵ�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float BoostMultiplier;     // Shift ������ �� �ִ� �ӵ� ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float AccelRate;           // �ӵ� ���� �ӵ�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float DecelRate;           // �ӵ� ���� �ӵ�

	bool bIsBoosting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MoveUp")
	float UpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	float Height;			   // ������ ����� �ּ� ����

	bool bIsFreeCamera;
	FRotator DefaultCameraRotation;

	float FallSpeed;

	UPROPERTY(EditAnywhere, Category = "Speed") //�߷� ���ӵ�
	float Gravity = -200.f;        

	UPROPERTY(EditAnywhere, Category = "Speed") //���� �ִ� �ӵ�
	float MaxFallSpeed = -600.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
