#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "RPGStateTreeCondition_CheckDistance.generated.h"

USTRUCT()
struct FRPGStateTreeCondition_CheckDistanceInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	// PlayerTarget is optional - if not provided, will automatically find nearest player
	UPROPERTY(EditAnywhere, Category = "Input", meta = (Optional))
	TObjectPtr<AActor> PlayerTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxDistance = 1000.0f;
};

USTRUCT(meta = (DisplayName = "Check Distance"))
struct FRPGStateTreeCondition_CheckDistance : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeCondition_CheckDistanceInstanceData;

	FRPGStateTreeCondition_CheckDistance() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

