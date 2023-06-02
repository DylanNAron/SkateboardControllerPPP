// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SkateboardController.generated.h"

UCLASS()
class PPPSKATEBOARD_API ASkateboardController : public APawn
{
	GENERATED_BODY()

	/** StaticMesh used for the Skateboard */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Skateboard;

	/** SkateWheels */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* FLWheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UPhysicsConstraintComponent* FLWheelPhysConstraint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* FRWheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UPhysicsConstraintComponent* FRWheelPhysConstraint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* BLWheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UPhysicsConstraintComponent* BLWheelPhysConstraint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* BRWheel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skateboard, meta = (AllowPrivateAccess = "true"))
		class UPhysicsConstraintComponent* BRWheelPhysConstraint;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* LookAction;

public:
	// Sets default values for this pawn's properties
	ASkateboardController();

	/** Vertical impulse to apply when pressing jump */
	UPROPERTY(EditAnywhere, Category = Ball)
		float JumpImpulse;

	/** Torque to apply when trying to roll ball */
	UPROPERTY(EditAnywhere, Category = Ball)
		float RollTorque;

	/** Indicates whether we can currently jump, use to prevent double jumping */
	bool bCanJump;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void Jump();

	void Move(const FInputActionValue& Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
