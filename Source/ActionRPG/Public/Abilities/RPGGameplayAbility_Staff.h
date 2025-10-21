// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/RPGGameplayAbility.h"
#include "RPGHomingProjectile.h"
#include "RPGGameplayAbility_Staff.generated.h"

class ARPGHomingProjectile;

/**
 * Gameplay ability for staff weapons
 * Fires homing projectiles at enemies
 */
UCLASS()
class ACTIONRPG_API URPGGameplayAbility_Staff : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGameplayAbility_Staff();

	/** Actually activate ability, this is called when all checks pass */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Spawns a homing projectile towards the target */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SpawnHomingProjectile();

	/** Find the best target for the projectile */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	AActor* FindBestTarget();

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	TSubclassOf<ARPGHomingProjectile> ProjectileClass;

	/** Range to search for targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	float TargetSearchRange;

	/** Base damage for the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	float BaseDamage;

	/** Offset from character for spawning projectile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	FVector SpawnOffset;
};

