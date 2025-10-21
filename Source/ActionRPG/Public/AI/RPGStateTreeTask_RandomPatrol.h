#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RPGStateTreeTask_RandomPatrol.generated.h"

class ARPGArcherCharacter;

USTRUCT()
struct FRPGStateTreeTask_RandomPatrolInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float PatrolRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float PatrolSpeed = 200.0f;

	UPROPERTY()
	FVector CurrentPatrolLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bIsMoving = false;
};

USTRUCT(meta = (DisplayName = "Random Patrol"))
struct FRPGStateTreeTask_RandomPatrol : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeTask_RandomPatrolInstanceData;

	FRPGStateTreeTask_RandomPatrol();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	FVector GetRandomPatrolLocation(const FVector& Origin, float Radius, UObject* WorldContext) const;
};

