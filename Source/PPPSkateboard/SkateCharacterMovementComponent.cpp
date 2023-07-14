// Fill out your copyright notice in the Description page of Project Settings.


#include "SkateCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

USkateCharacterMovementComponent::USkateCharacterMovementComponent()
{
}


bool USkateCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode)
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void USkateCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SkateCharacterOwner = Cast<ASkateCharacter>(GetOwner());
}

void USkateCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Skate:
		PhysSkate(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

void USkateCharacterMovementComponent::PhysSkate(float deltaTime, int32 Iterations)
{

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	RestorePreAdditiveRootMotionVelocity();

	//Adjust Velocity based on slope or if aerial
	FHitResult SurfaceHit;
	if (GetSkateSurface(SurfaceHit))
	{
		isAerial = false;

	/*	
	* OLD VERSION
	*	FVector SlopeForce = SurfaceHit.Normal;
	*	SlopeForce.Z *= -1;
	*	Velocity += SkateGravityForce * SlopeForce * deltaTime;
	*/

		FVector SlopeNormal = SurfaceHit.Normal;
		//SlopeNormal.Z *= -1;
		// Calculate the parallel and perpendicular components of gravity
		FVector GravityDirection = FVector::DownVector; // Assuming gravity acts straight down
		FVector ParallelGravity = FVector::DotProduct(GravityDirection, SlopeNormal) * SlopeNormal;
		FVector PerpendicularGravity = GravityDirection - ParallelGravity;
		// Calculate the parallel acceleration using the magnitude of the parallel gravity force
		float ParallelAcceleration = ParallelGravity.Size() / Mass;
		// Calculate the sliding velocity based on the parallel acceleration and time
		float SlidingTime = 10.0f; 
		float SlidingVelocity = ParallelAcceleration * SlidingTime;

		Velocity += SkateGravityForce * SlopeNormal * SlidingVelocity * deltaTime;
		//gravity
		Velocity += SkateGravityForce * FVector::DownVector * deltaTime;

		//turning
		if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .1)
		{
			Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
		}
		else
		{
			Acceleration = FVector::ZeroVector;
		}
	}
	else
	{
		isAerial = true;

		//Turning not possible mid air
		Acceleration = FVector::ZeroVector;

		//gravity
		Velocity += AerialGravityForce * FVector::DownVector * deltaTime;
	}


	if (!isAerial)
	{

		//Turn to local coordinates because we are rotating the skater
		double localForwardVelocity = FVector::DotProduct(UpdatedComponent->GetForwardVector(), Velocity);
		double localUpVelocity = FVector::DotProduct(UpdatedComponent->GetUpVector(), Velocity);
		double localRightVelocity = FVector::DotProduct(UpdatedComponent->GetRightVector(), Velocity);

		//constrain to only moving forward/backward (with a little bit of sliding based on SidewaysWheelSlide value)
		localRightVelocity *= SidewaysWheelSlide;

		//convert to global for velocity
		Velocity = localForwardVelocity * UpdatedComponent->GetForwardVector() + localUpVelocity * UpdatedComponent->GetUpVector() + localRightVelocity * UpdatedComponent->GetRightVector();

	}

	//Calculating Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, SkateFriction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(deltaTime);

	//Perform Actual Movement!!
	Iterations++;
	bJustTeleported = false;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	
	//Rotate player to slope if grounded
	FQuat NewRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	if (GetSkateSurface(SurfaceHit))
	{
		//Rotating here messes up constraining velocity
		FRotator GroundAlignment = FRotationMatrix::MakeFromZX(SurfaceHit.Normal, UpdatedComponent->GetForwardVector()).Rotator();
		NewRotation = FMath::RInterpTo(UpdatedComponent->GetComponentRotation(), GroundAlignment, deltaTime, SkateFloorAlignmentTime).Quaternion();
	}


	//THIS ACTUALLY DOES THE MOVE
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	FHitResult NewSurfaceHit;
	if (!GetSkateSurface(NewSurfaceHit))
	{
		//Aerial
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

}

bool USkateCharacterMovementComponent::GetSkateSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.5f * (-1 * CharacterOwner->GetActorUpVector());//FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");

	DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, .1f, 0, 5);


	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, SkateCharacterOwner->GetIgnoreCharacterParams());
}
