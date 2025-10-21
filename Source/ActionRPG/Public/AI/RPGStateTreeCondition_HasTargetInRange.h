#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "RPGStateTreeCondition_HasTargetInRange.generated.h"

USTRUCT()
struct FRPGStateTreeCondition_HasTargetInRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> FoundTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DetectionRange = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxDistance = 1000.0f;
};

USTRUCT(meta = (DisplayName = "Has Target In Range"))
struct FRPGStateTreeCondition_HasTargetInRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeCondition_HasTargetInRangeInstanceData;

	FRPGStateTreeCondition_HasTargetInRange() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	AActor* FindNearestTarget(AActor* SearchOrigin, float Range) const;
};

