// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnalogStickTrickSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PPPSKATEBOARD_API UAnalogStickTrickSystem : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	class APlayerController* PlayerController;

public:	
	// Sets default values for this component's properties
	UAnalogStickTrickSystem();

	UPROPERTY(EditDefaultsOnly, Category = "Tolerance") float EdgeThreshold = 0.1f;



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	/// <summary>
	/// Detect when the analog stick has hit the edge of the circle, using Edge threshold value to allow for some tolerance.
	/// </summary>
	/// <param name="x"> x position</param>
	/// <param name="y"> y position</param>
	/// <returns>boolean whether stick is at edge of circle</returns>
	bool IsCloseToEdge(float x, float y);

	int GetSection(float x, float y);

	double _stickX{};
	double _stickY{};

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
