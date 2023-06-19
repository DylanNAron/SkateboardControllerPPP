// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SkateCharacter.generated.h"

UCLASS()
class PPPSKATEBOARD_API ASkateCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
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

	//Params
	UPROPERTY(EditDefaultsOnly, Category = "Spin/Flip") float HorizontalSpinSpeed = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Spin/Flip") float MomentumDecay = .07f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin/Flip", meta = (AllowPrivateAccess = "true")) float HorizontalSpinMomentum = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin/Flip", meta = (AllowPrivateAccess = "true")) float VerticalFlipMomentum = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Spin/Flip") float VerticalFlipSpeed = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Spin/Flip") float DistanceToCheckLanding = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Spin/Flip") float TimeToAdjustAerialInAir = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Crash") float SpinAngleForCrash = 17.f;
	UPROPERTY(EditDefaultsOnly, Category = "Crash") float MinSpeedForAngleCrash = 700.f;
	UPROPERTY(EditDefaultsOnly, Category = "Crash") float CrashResetTime = 4.f;

public:
	// Sets default values for this character's properties
	ASkateCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) class USkateCharacterMovementComponent* SkateCharacterMovementComponent;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveComplete(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/// <summary>
	/// Apply horizontal rotation in air based on input with a momentum
	/// </summary>
	/// <param name="input"></param>
	void Spin(const FVector2D& input);
	/// <summary>
	///  Apply vertical rotation in air based on input with a momentum, must be grabbing to flip
	/// </summary>
	/// <param name="input"></param>
	void Flip(const FVector2D& input);
	/// <summary>
	/// Called on tick ; when isAerial, apply slight adjustments to skater rotation for perfect landings (within reason)
	/// </summary>
	void AdjustAerialRotation();
	/// <summary>
	/// Called on tick ; check if we skater should bail/crash
	/// </summary>
	void CheckCrash();

	void CrashTimer();


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Skate")
		void Grab();

	UFUNCTION(BlueprintCallable, Category = "Skate")
		void Crash();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FCollisionQueryParams GetIgnoreCharacterParams() const;

private:

	FVector2D _movementVector;
	bool _isGrabbing{ false };

	float _aerialAdjustTime{ 0 };
	FVector _previousVelocity{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin/Flip", meta = (AllowPrivateAccess = "true")) 
	bool _wasAerial {false};
	bool _isCrash{ false };

	FTimerHandle _crashTimer;

	FTransform _meshOriginalTransform;

};
