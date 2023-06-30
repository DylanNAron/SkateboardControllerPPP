// Fill out your copyright notice in the Description page of Project Settings.


#include "AnalogStickTrickSystem.h"

// Sets default values for this component's properties
UAnalogStickTrickSystem::UAnalogStickTrickSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAnalogStickTrickSystem::BeginPlay()
{
	Super::BeginPlay();

	// Get the owning actor
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		// Get the player controller
		PlayerController = OwnerActor->GetWorld()->GetFirstPlayerController();
	}
	
}

bool UAnalogStickTrickSystem::IsCloseToEdge(float x, float y)
{
	return false;
}

int UAnalogStickTrickSystem::GetSection(float x, float y)
{
	return 0;
}


// Called every frame
void UAnalogStickTrickSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlayerController)
	{
		PlayerController->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, _stickX, _stickY);
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("Right Stick:: X: %f, Y: %f"), _stickX, _stickY));
	}


}

