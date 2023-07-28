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
	FVector SlopeNormal;
	if (GetSkateSurfaceNormalAvg(SlopeNormal))
	{
		isAerial = false;

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
	if (GetSkateSurfaceNormalAvg(SlopeNormal))
	{
		FRotator GroundAlignment = FRotationMatrix::MakeFromZX(SlopeNormal, UpdatedComponent->GetForwardVector()).Rotator();
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

	//DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, .1f, 0, 5);

	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, SkateCharacterOwner->GetIgnoreCharacterParams());
}

bool USkateCharacterMovementComponent::GetSkateSurfaceNormalAvg(FVector& normalAverage) const
{
	UCapsuleComponent* CharacterCapsule = CharacterOwner->GetCapsuleComponent(); // Replace with your character's capsule component reference
	FVector ForwardVector = CharacterOwner->GetActorForwardVector();
	FVector DownVector = CharacterOwner->GetActorUpVector();

	// Calculate the start and end locations for the first line trace (front of the capsule)
	FVector StartLocationFront = CharacterCapsule->GetComponentLocation() + (ForwardVector * CharacterCapsule->GetScaledCapsuleRadius());
	FVector EndLocationFront = StartLocationFront - (DownVector * CharacterCapsule->GetScaledCapsuleHalfHeight() * 1.5f);
	FHitResult FirstHitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("Trace")), true, nullptr);
	
//	DrawDebugLine(GetWorld(), StartLocationFront, EndLocationFront, FColor::Purple, false, .1f, 0, 5);
	bool isFrontGrounded = GetWorld()->LineTraceSingleByChannel(FirstHitResult, StartLocationFront, EndLocationFront, ECC_Visibility, TraceParams);

	// Calculate the start and end locations for the second line trace (back of the capsule)
	FVector StartLocationBack = CharacterCapsule->GetComponentLocation() - (ForwardVector * CharacterCapsule->GetScaledCapsuleRadius());
	FVector EndLocationBack = StartLocationBack - (DownVector * CharacterCapsule->GetScaledCapsuleHalfHeight() * 1.5f);
	FHitResult SecondHitResult;
//	DrawDebugLine(GetWorld(), StartLocationBack, EndLocationBack, FColor::Purple, false, .1f, 0, 5);
	bool isBackGrounded = GetWorld()->LineTraceSingleByChannel(SecondHitResult, StartLocationBack, EndLocationBack, ECC_Visibility, TraceParams);

	// Calculate the average normal vector
	FVector AverageNormal = (FirstHitResult.Normal + SecondHitResult.Normal).GetSafeNormal();

	if (!isBackGrounded && !isFrontGrounded)
	{
		return false;
	}

	normalAverage = AverageNormal;
	return true;
}

