// Fill out your copyright notice in the Description page of Project Settings.


#include "SkateCharacter.h"


#include "SkateCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ASkateCharacter::ASkateCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<USkateCharacterMovementComponent>(CharacterMovementComponentName))
{
	SkateCharacterMovementComponent = Cast<USkateCharacterMovementComponent>(GetCharacterMovement());

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Analog Trick System actor component
	TrickSystem = CreateDefaultSubobject<UAnalogStickTrickSystem>(TEXT("TrickSystem"));
}


// Called when the game starts or when spawned
void ASkateCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//GetPositionOfMeshForRespawningAfterBails
	_meshOriginalTransform = GetMesh()->GetRelativeTransform();

	//Setup collision delegate
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ASkateCharacter::OnCapsuleComponentHit);


	//Delegate for Flicking Flatground Tricks
	TrickSystem->OnTrickFlicked.AddUObject(this, &ASkateCharacter::HandleTrickSystemFlick);
}

// Called every frame
void ASkateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Spin(_movementVector);
	Flip(_movementVector);
	AdjustAerialRotation();
	CheckCrash();

}


//////////////////////////////////////////////////////////////////////////
// Input

void ASkateCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASkateCharacter::MoveComplete);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Look);

	}

}


FCollisionQueryParams ASkateCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void ASkateCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	_movementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		FVector ForwardDirection;
		FVector RightDirection;


		if (GetCharacterMovement()->MovementMode == MOVE_Custom)
		{
			//Skate Controlling based on camera position
			ForwardDirection = FollowCamera->GetForwardVector();
			RightDirection = FollowCamera->GetRightVector();

		}
		else
		{
			//Biped Controlling
			// find out which way is forward based on controller rotation
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}

		// add movement 
		AddMovementInput(ForwardDirection, _movementVector.Y);
		AddMovementInput(RightDirection, _movementVector.X);


	}
}

void ASkateCharacter::MoveComplete(const FInputActionValue& Value)
{
	_movementVector = FVector2D::Zero();
}

void ASkateCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateCharacter::Spin(const FVector2D& input)
{
	if (Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial)
	{
		HorizontalSpinMomentum += input.X * HorizontalSpinSpeed * GetWorld()->GetDeltaSeconds();
		HorizontalSpinMomentum *= (1.0 - MomentumDecay);
		GetCapsuleComponent()->AddWorldRotation(FRotator(0, HorizontalSpinMomentum, 0));
	}
	else
	{
		HorizontalSpinMomentum = 0;
	}
}

void ASkateCharacter::Flip(const FVector2D& input)
{
	if (Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial)
	{
		if (_isGrabbing)
		{
			VerticalFlipMomentum += input.Y * VerticalFlipSpeed * GetWorld()->GetDeltaSeconds();
		}
		VerticalFlipMomentum *= (1.0 - MomentumDecay);
		GetCapsuleComponent()->AddLocalRotation(FRotator(-VerticalFlipMomentum, 0, 0));
	}
	else
	{
		VerticalFlipMomentum = 0;
	}
}

void ASkateCharacter::AdjustAerialRotation()
{
	if (Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial)
	{
		//Raycast from relative direction vector
		
		FVector Velocity = GetCharacterMovement()->Velocity;
		FVector StartTrace = GetActorLocation();
		FVector EndTrace = GetActorLocation() + Velocity.GetSafeNormal() * DistanceToCheckLanding;
		FHitResult OutHit;

		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Purple, false, 3, 0, 5);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, StartTrace, EndTrace, ECC_WorldStatic, GetIgnoreCharacterParams()))
		{
			//rotate
			FRotator LandAlignment = FRotationMatrix::MakeFromZX(OutHit.ImpactNormal, GetActorForwardVector()).Rotator();

			_aerialAdjustTime += GetWorld()->GetDeltaSeconds();
			LandAlignment.Yaw = GetActorRotation().Yaw;
			LandAlignment.Roll = GetActorRotation().Roll;

			double debugPitchDelta = LandAlignment.Pitch - GetActorRotation().Pitch;
			//Adjust player pitch (flipping)
			//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("Pitch Delta:: %f"), debugPitchDelta));

			SetActorRotation(FMath::Lerp(GetActorRotation(), LandAlignment, _aerialAdjustTime / TimeToAdjustAerialInAir));


		}
		else
		{
			_aerialAdjustTime = 0;
		}
	}
	else
	{
		_aerialAdjustTime = 0;
	}

}

void ASkateCharacter::CheckCrash()
{

	//On land
	if(_wasAerial && !Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial && !_isCrash)
	{
		//Check if we land sideways (greater than given angle to fall off board)
		FVector CurrentVelocity = GetMovementComponent()->Velocity;
		FVector PrevVelocity = _previousVelocity;

		FVector prevVelocityLocal = ActorToWorld().InverseTransformVector(PrevVelocity);
		FVector currentVelocityLocal = ActorToWorld().InverseTransformVector(CurrentVelocity);
		prevVelocityLocal.Normalize();
		currentVelocityLocal.Normalize();

		// Calculate the dot product between the normalized vectors to get angle
		float dotProduct = FVector::DotProduct(prevVelocityLocal, currentVelocityLocal);
		float angleInRadians = FMath::Acos(dotProduct);
		float Angle = FMath::RadiansToDegrees(angleInRadians);

		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("Angler:: %f"), Angle));
		//RagDoll
		if (Angle > SpinAngleForCrash && CurrentVelocity.Size() > MinSpeedForAngleCrash)
		{
			Crash();
		}
	}

	_previousVelocity = GetMovementComponent()->Velocity;
	_wasAerial = Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial;
}

void ASkateCharacter::CrashTimer()
{
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	GetMesh()->SetRelativeTransform(_meshOriginalTransform);
	_isCrash = false;

	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController != nullptr)
	{
		PlayerController->EnableInput(PlayerController);
	}
}

void ASkateCharacter::HandleTrickSystemFlick(FTrickComboStruct Trick)
{
	GetCharacterMovement()->AddImpulse(GetActorUpVector() * Trick.JumpHeight, true);
	GetMesh()->GetAnimInstance()->Montage_Play(Trick.PlayerMontage);
}


void ASkateCharacter::Crash()
{
	_isCrash = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetWorldTimerManager().SetTimer(_crashTimer, this, &ASkateCharacter::CrashTimer, CrashResetTime, false);
	SetActorRotation(FRotator(0, CameraBoom->GetRelativeRotation().Yaw, 0));
	
	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController != nullptr)
	{
		PlayerController->DisableInput(PlayerController);
	}
}

void ASkateCharacter::Grab()
{
	_isGrabbing = !_isGrabbing;
}

void ASkateCharacter::OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//If capsule component is hit and we are still in an aerial then that means we landed upside down
	if (Cast<USkateCharacterMovementComponent>(GetCharacterMovement())->isAerial)
	{
		Crash();
	}
}