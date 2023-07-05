// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "AnalogStickTrickSystem.generated.h"


USTRUCT(BlueprintType)
struct FTrickComboStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TrickName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int> InputCombo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* PlayerMontage;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float JumpHeight {700};
};


DECLARE_MULTICAST_DELEGATE_OneParam(FAnalogTrickSystemDelegate, FTrickComboStruct Trick);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PPPSKATEBOARD_API UAnalogStickTrickSystem : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	APlayerController* PlayerController;

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
	bool IsCloseToEdge(const float x, const float y);

	/// <summary>
	/// Get Section of the circle that we have hit, numSections defaulted to 8 with an offset so that position 1 will be at bottom of circle
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns>Section of Circle we have hit with analog stick</returns>
	int GetSection(const float x, const float y);
	static const int numSections{ 8 };
	static constexpr float offsetAngle{ (90 - (360/numSections)/2 ) * PI/180 }; //Have section 1 be at bottom of circle with a little offset

	void CheckValidTrick();

	float _stickX{};
	float _stickY{};
	TArray<int> currentCombo{};
	bool isComboStart{ false };


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FAnalogTrickSystemDelegate OnTrickFlicked;

		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTrickComboStruct> _possibleTrickCombos{};

};

