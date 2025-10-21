// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Abilities/RPGGameplayAbility.h"
#include "RPGStaffAttackAbility.generated.h"

class ARPGArcaneMissile;
class URPGStaffItem;

/**
 * Staff attack ability that fires homing arcane missiles
 */
UCLASS()
class ACTIONRPG_API URPGStaffAttackAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGStaffAttackAbility();

	/** Actually activate ability, do not call this directly */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile)
	TSubclassOf<ARPGArcaneMissile> ProjectileClass;

	/** Range to search for targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Targeting)
	float TargetingRange;

	/** Spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile)
	FVector SpawnOffset;

	/** Fire the arcane missile */
	UFUNCTION(BlueprintCallable, Category = Ability)
	void FireArcaneMissile();

	/** Find the best target for the missile */
	UFUNCTION(BlueprintCallable, Category = Targeting)
	AActor* FindTarget();
};
