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

bool UAnalogStickTrickSystem::IsCloseToEdge(const float x, const float y)
{
	double distance = std::sqrt(x * x + y * y);
	return std::abs(distance - 1) <= EdgeThreshold;
}

ESection UAnalogStickTrickSystem::GetSection(const float x, const float y)
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
	int section = static_cast<int>(adjustedAngle / sectionSize);
	return static_cast<ESection>(section);
}


void UAnalogStickTrickSystem::CheckValidTrick()
{

	for (const auto& combo : _possibleTrickCombos)
	{
		if (currentCombo == combo.InputCombo)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, combo.TrickName);
			isComboStart = false;
			currentCombo.Empty();

			OnTrickFlicked.Broadcast(combo);
		}
	}
}

void UAnalogStickTrickSystem::CheckDeadZone()
{
	//Start Timer if at Zero , and Cancel if not 
	if (FMath::IsNearlyZero(_stickX) && FMath::IsNearlyZero(_stickY))
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(_DeadZoneTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(_DeadZoneTimerHandle, this, &UAnalogStickTrickSystem::OnDeadZoneTimerElapsed, _DeadZoneTimeTolerance, false);
		}
	}
	else
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(_DeadZoneTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(_DeadZoneTimerHandle);
		}
	}

}

void UAnalogStickTrickSystem::OnDeadZoneTimerElapsed()
{
	isComboStart = false;
	currentCombo.Empty();
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
		ESection SectionHIT = GetSection(_stickX, _stickY);
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Emerald, FString::Printf(TEXT("%d SectionHit!"), SectionHIT));
		if (currentCombo.IsEmpty() || currentCombo.Last() != SectionHIT)
		{
			currentCombo.Add(SectionHIT);
		}
	}

	if (isComboStart)
	{
		CheckValidTrick();
		CheckDeadZone();
	}








	//**********DEBUG PRINTING********************
	//UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMyEnum"), true);
	FString EnumString;
	for (int32 Index = 0; Index < currentCombo.Num(); ++Index)
	{
		ESection EnumValue = currentCombo[Index];
		FString EnumName = *UEnum::GetValueAsString<ESection>(EnumValue);
		if (Index > 0)
		{
			EnumString += ", ";
		}
		EnumString += EnumName;
	}
	FString DebugMessage = FString::Printf(TEXT("CurrentCombo: %s"), *EnumString);
	GEngine->AddOnScreenDebugMessage(-1, .001f, FColor::Magenta, DebugMessage);
	//**********DEBUG PRINTING********************


}

