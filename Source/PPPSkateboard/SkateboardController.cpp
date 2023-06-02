// Fill out your copyright notice in the Description page of Project Settings.


#include "SkateboardController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ASkateboardController::ASkateboardController()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkateBoardMesh(TEXT("/Script/Engine.StaticMesh'/Game/SkateAssets/Skateboard/skateboard.skateboard'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WheelMesh(TEXT("/Script/Engine.StaticMesh'/Game/ThirdPerson/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder'"));


 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#pragma region SkateboardConstruction
	// Create mesh component for the Skateboard
	Skateboard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skateboard"));
	Skateboard->SetStaticMesh(SkateBoardMesh.Object);
	Skateboard->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Skateboard->SetSimulatePhysics(true);
	Skateboard->SetAngularDamping(0.1f);
	Skateboard->SetLinearDamping(0.1f);
	Skateboard->BodyInstance.MassScale = 3.5f;
	Skateboard->BodyInstance.MaxAngularVelocity = 800.0f;
	Skateboard->SetNotifyRigidBodyCollision(true);
	RootComponent = Skateboard;

	//SkateWheels attached to the board
	//FL Wheel
	FLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FLWheel"));
	FLWheel->SetSimulatePhysics(true);
	FLWheel->SetStaticMesh(WheelMesh.Object);
	FLWheel->SetupAttachment(Skateboard);
	FLWheelPhysConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("FLWheelPhysConstraint"));
	FLWheelPhysConstraint->SetupAttachment(FLWheel);
	FLWheelPhysConstraint->ComponentName1.ComponentName = Skateboard->GetFName();
	FLWheelPhysConstraint->ComponentName2.ComponentName = FLWheel->GetFName();
	FLWheelPhysConstraint->SetAngularSwing1Limit(ACM_Locked, 45.f);
	FLWheelPhysConstraint->SetAngularTwistLimit(ACM_Locked, 45.f);
	FLWheelPhysConstraint->SetDisableCollision(true);

	//FR Wheel
	FRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FRWheel"));
	FRWheel->SetSimulatePhysics(true);
	FRWheel->SetStaticMesh(WheelMesh.Object);
	FRWheel->SetupAttachment(Skateboard);
	FRWheelPhysConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("FRWheelPhysConstraint"));
	FRWheelPhysConstraint->SetupAttachment(FRWheel);
	FRWheelPhysConstraint->ComponentName1.ComponentName = Skateboard->GetFName();
	FRWheelPhysConstraint->ComponentName2.ComponentName = FRWheel->GetFName();
	FRWheelPhysConstraint->SetAngularSwing1Limit(ACM_Locked, 45.f);
	FRWheelPhysConstraint->SetAngularTwistLimit(ACM_Locked, 45.f);
	FRWheelPhysConstraint->SetDisableCollision(true);

	//BL Wheel
	BLWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLWheel"));
	BLWheel->SetSimulatePhysics(true);
	BLWheel->SetStaticMesh(WheelMesh.Object);
	BLWheel->SetupAttachment(Skateboard);
	BLWheelPhysConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("BLWheelPhysConstraint"));
	BLWheelPhysConstraint->SetupAttachment(BLWheel);
	BLWheelPhysConstraint->ComponentName1.ComponentName = Skateboard->GetFName();
	BLWheelPhysConstraint->ComponentName2.ComponentName = BLWheel->GetFName();
	BLWheelPhysConstraint->SetAngularSwing1Limit(ACM_Locked, 45.f);
	BLWheelPhysConstraint->SetAngularTwistLimit(ACM_Locked, 45.f);
	BLWheelPhysConstraint->SetDisableCollision(true);

	//BR Wheel
	BRWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BRWheel"));
	BRWheel->SetSimulatePhysics(true);
	BRWheel->SetStaticMesh(WheelMesh.Object);
	BRWheel->SetupAttachment(Skateboard);
	BRWheelPhysConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("BRWheelPhysConstraint"));
	BRWheelPhysConstraint->SetupAttachment(BRWheel);
	BRWheelPhysConstraint->ComponentName1.ComponentName = Skateboard->GetFName();
	BRWheelPhysConstraint->ComponentName2.ComponentName = BRWheel->GetFName();
	BRWheelPhysConstraint->SetAngularSwing1Limit(ACM_Locked, 45.f);
	BRWheelPhysConstraint->SetAngularTwistLimit(ACM_Locked, 45.f);
	BRWheelPhysConstraint->SetDisableCollision(true);

#pragma endregion SkateboardConstruction

	// Create a camera boom attached to the root
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the ball should not affect rotation of boom
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->CameraLagSpeed = 3.f;
	// Create a camera and attach to boom
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera


	// Set up forces
	RollTorque = 50000000.0f;
	JumpImpulse = 350000.0f;
	bCanJump = true; // Start being able to jump

}

// Called when the game starts or when spawned
void ASkateboardController::BeginPlay()
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
	
}

// Called to bind functionality to input
void ASkateboardController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASkateboardController::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateboardController::Move);

		//Looking
		//EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APPPSkateboardCharacter::Look);

	}

}


void ASkateboardController::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	bCanJump = true;
}

void ASkateboardController::Jump()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("JUMP!"));
	if (bCanJump)
	{
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Skateboard->AddImpulse(Impulse);
		bCanJump = false;
	}
}

void ASkateboardController::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Torque = FVector(-1 * MovementVector.X * RollTorque, MovementVector.Y * RollTorque, 0.f);
	Skateboard->AddTorqueInRadians(Torque);

}

// Called every frame
void ASkateboardController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


