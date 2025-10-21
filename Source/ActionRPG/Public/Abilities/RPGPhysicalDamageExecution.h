// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RPGPhysicalDamageExecution.generated.h"

/**
 * A physical damage execution, which allows doing physical damage by combining a raw Damage number with AttackPower and DefensePower
 * This is the same as the original RPGDamageExecution but explicitly tagged as physical damage
 */
UCLASS()
class ACTIONRPG_API URPGPhysicalDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGPhysicalDamageExecution();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
