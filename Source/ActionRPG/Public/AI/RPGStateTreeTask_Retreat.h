#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RPGStateTreeTask_Retreat.generated.h"

USTRUCT()
struct FRPGStateTreeTask_RetreatInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float RetreatDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float RetreatSpeed = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DetectionRange = 1200.0f;

	UPROPERTY()
	TObjectPtr<AActor> CachedTarget = nullptr;

	UPROPERTY()
	FVector RetreatLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bHasRetreatLocation = false;
};

USTRUCT(meta = (DisplayName = "Retreat"))
struct FRPGStateTreeTask_Retreat : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeTask_RetreatInstanceData;

	FRPGStateTreeTask_Retreat();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AActor* FindNearestTarget(AActor* SearchOrigin, float Range) const;
};

