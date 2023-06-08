// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PPPSkateboard.h"
#include "SkateCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SkateCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None		UMETA(Hidden),
	CMOVE_Skate		UMETA(DisplayName = "Skate"),
	CMOVE_MAX		UMETA(Hidden),
};

/**
 * 
 */
UCLASS()
class PPPSKATEBOARD_API USkateCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	//Parameters
	UPROPERTY(EditDefaultsOnly, Category = "Skating") float SkateGravityForce = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating") float AerialGravityForce = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating") float SkateFriction = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating") float SkateFloorAlignmentTime = .5f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating") float SidewaysWheelSlide = .1f;

	// Transient
	UPROPERTY(Transient) ASkateCharacter* SkateCharacterOwner;

public:
	USkateCharacterMovementComponent();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode);

protected:
	virtual void InitializeComponent() override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

private:
	void PhysSkate(float deltaTime, int32 Iterations);
	bool GetSkateSurface(FHitResult& Hit) const;

public:

	UPROPERTY(BlueprintReadOnly, Category = "Skating") bool isAerial{false};
	
};
