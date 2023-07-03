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

	//BASIC OLLIE -- THESE SHOULD BE RETRIEVED FROM CUSTOM DATA STRUCTURE
	_possibleCombos.Add( TArray{1,5} );
	
}

bool UAnalogStickTrickSystem::IsCloseToEdge(const float x, const float y)
{
	double distance = std::sqrt(x * x + y * y);
	return std::abs(distance - 1) <= EdgeThreshold;
}

int UAnalogStickTrickSystem::GetSection(const float x, const float y)
{
	float angle = std::atan2(y, x);
	if (angle < 0) {
		angle += 2 * PI;
	}
	float sectionSize = 2 * PI / numSections;
	float adjustedAngle = angle - offsetAngle;
	if (adjustedAngle < 0) {
		adjustedAngle += 2 * PI;
	}
	int section = static_cast<int>(adjustedAngle / sectionSize) + 1;
	return section;
}


void UAnalogStickTrickSystem::CheckValidTrick()
{
	for (const auto& combo : _possibleCombos)
	{
		if (currentCombo == combo)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("U FLICKED AN OLLIE!!!!!!!!")));
			isComboStart = false;
		}
	}
}

// Called every frame
void UAnalogStickTrickSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlayerController)
	{
		PlayerController->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, _stickX, _stickY);
	}

	if (IsCloseToEdge(_stickX, _stickY))
	{
		isComboStart = true;
		int SectionHIT = GetSection(_stickX, _stickY);
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("%d SectionHit!"), SectionHIT));
		if (currentCombo.IsEmpty() || currentCombo.Last() != SectionHIT)
		{
			currentCombo.Add(SectionHIT);
		}
	}

	if (isComboStart)
	{
		CheckValidTrick();
	}


	//DEBUG PRINTING
	FString ArrayContents;
	for (int32 Element : currentCombo)
	{
		ArrayContents += FString::Printf(TEXT("%d "), Element);
	}
	FString DebugMessage = FString::Printf(TEXT("CurrentCombo: %s"), *ArrayContents);
	GEngine->AddOnScreenDebugMessage(-1, .001f, FColor::Magenta, DebugMessage);



}

