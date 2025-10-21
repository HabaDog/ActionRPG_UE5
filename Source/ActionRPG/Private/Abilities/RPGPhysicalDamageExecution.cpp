// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGPhysicalDamageExecution.h"
#include "Abilities/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"

struct RPGPhysicalDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	RPGPhysicalDamageStatics()
	{
		// Capture the Target's DefensePower attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, DefensePower, Target, false);

		// Capture the Source's AttackPower. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, AttackPower, Source, true);

		// Also capture the source's raw Damage, which is normally passed in directly via the execution
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Damage, Source, true);
	}
};

static const RPGPhysicalDamageStatics& PhysicalDamageStatics()
{
	static RPGPhysicalDamageStatics PhysDmgStatics;
	return PhysDmgStatics;
}

URPGPhysicalDamageExecution::URPGPhysicalDamageExecution()
{
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().DamageDef);
}

void URPGPhysicalDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// --------------------------------------
	//	Physical Damage Done = Damage * AttackPower / DefensePower
	//	If DefensePower is 0, it is treated as 1.0
	// --------------------------------------

	float DefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalDamageStatics().DefensePowerDef, EvaluationParameters, DefensePower);
	if (DefensePower == 0.0f)
	{
		DefensePower = 1.0f;
	}

	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalDamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);

	float Damage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalDamageStatics().DamageDef, EvaluationParameters, Damage);

	float PhysicalDamageDone = Damage * AttackPower / DefensePower;
	if (PhysicalDamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(URPGAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, PhysicalDamageDone));
	}
}
