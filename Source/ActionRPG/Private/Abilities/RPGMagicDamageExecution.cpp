// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGMagicDamageExecution.h"
#include "Abilities/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"

struct RPGMagicDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicDefensePower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicAttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	RPGMagicDamageStatics()
	{
		// Capture the Target's MagicDefensePower attribute. Do not snapshot it, because we want to use the value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, MagicDefensePower, Target, false);

		// Capture the Source's MagicAttackPower. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, MagicAttackPower, Source, true);

		// Also capture the source's raw Damage, which is normally passed in directly via the execution
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Damage, Source, true);
	}
};

static const RPGMagicDamageStatics& MagicDamageStatics()
{
	static RPGMagicDamageStatics MagicDmgStatics;
	return MagicDmgStatics;
}

URPGMagicDamageExecution::URPGMagicDamageExecution()
{
	RelevantAttributesToCapture.Add(MagicDamageStatics().MagicDefensePowerDef);
	RelevantAttributesToCapture.Add(MagicDamageStatics().MagicAttackPowerDef);
	RelevantAttributesToCapture.Add(MagicDamageStatics().DamageDef);
}

void URPGMagicDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
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
	//	Magic Damage Done = Damage * MagicAttackPower / MagicDefensePower
	//	If MagicDefensePower is 0, it is treated as 1.0
	// --------------------------------------

	float MagicDefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicDamageStatics().MagicDefensePowerDef, EvaluationParameters, MagicDefensePower);
	if (MagicDefensePower == 0.0f)
	{
		MagicDefensePower = 1.0f;
	}

	float MagicAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicDamageStatics().MagicAttackPowerDef, EvaluationParameters, MagicAttackPower);

	float Damage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicDamageStatics().DamageDef, EvaluationParameters, Damage);

	float MagicDamageDone = Damage * MagicAttackPower / MagicDefensePower;
	if (MagicDamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(URPGAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, MagicDamageDone));
	}
}
