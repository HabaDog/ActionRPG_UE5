#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RPGStateTreeTask_ChasePlayer.generated.h"

USTRUCT()
struct FRPGStateTreeTask_ChasePlayerInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ChaseSpeed = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DetectionRange = 1200.0f;

	UPROPERTY()
	TObjectPtr<AActor> CachedTarget = nullptr;
};

USTRUCT(meta = (DisplayName = "Chase Player"))
struct FRPGStateTreeTask_ChasePlayer : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeTask_ChasePlayerInstanceData;

	FRPGStateTreeTask_ChasePlayer();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	AActor* FindNearestTarget(AActor* SearchOrigin, float Range) const;
};

