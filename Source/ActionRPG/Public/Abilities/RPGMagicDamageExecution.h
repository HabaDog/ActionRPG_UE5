// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RPGMagicDamageExecution.generated.h"

/**
 * A magic damage execution, which allows doing magic damage by combining a raw Damage number with MagicAttackPower and MagicDefensePower
 * This is separate from physical damage to allow for different resistances and calculations
 */
UCLASS()
class ACTIONRPG_API URPGMagicDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGMagicDamageExecution();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
