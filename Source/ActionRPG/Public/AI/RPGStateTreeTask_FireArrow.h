#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RPGStateTreeTask_FireArrow.generated.h"

class ARPGArrowProjectile;

USTRUCT()
struct FRPGStateTreeTask_FireArrowInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TSubclassOf<ARPGArrowProjectile> ArrowProjectileClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AttackCooldown = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DetectionRange = 1200.0f;

	UPROPERTY()
	TObjectPtr<AActor> CachedTarget = nullptr;

	UPROPERTY()
	float TimeSinceLastAttack = 0.0f;
};

USTRUCT(meta = (DisplayName = "Fire Arrow"))
struct FRPGStateTreeTask_FireArrow : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRPGStateTreeTask_FireArrowInstanceData;

	FRPGStateTreeTask_FireArrow();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	void FireArrow(const FInstanceDataType& InstanceData, AActor* Target) const;
	AActor* FindNearestTarget(AActor* SearchOrigin, float Range) const;
};

