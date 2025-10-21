// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RPGCharacterBase.h"
#include "RPGArcherEnemy.generated.h"

/**
 * Archer enemy character that uses StateTree AI
 * Maintains distance from target and attacks with ranged projectiles
 */
UCLASS()
class ACTIONRPG_API ARPGArcherEnemy : public ARPGCharacterBase
{
	GENERATED_BODY()

public:
	ARPGArcherEnemy();

	/** Ideal distance to maintain from target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float IdealCombatDistance;

	/** Minimum distance - if target gets closer, retreat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float MinimumCombatDistance;

	/** Maximum attack range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float AttackRange;

	/** Detection/Aggro range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float DetectionRange;

	/** Projectile class to spawn when attacking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	TSubclassOf<class ARPGArcherProjectile> ProjectileClass;

	/** Base damage for ranged attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float RangedDamage;

	/** Attack cooldown time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float AttackCooldown;

	/** Fire a projectile at the target */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireProjectileAtTarget(AActor* Target);

	/** Check if can attack target */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanAttackTarget(AActor* Target) const;

	/** Get current target actor */
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	/** Set current target actor */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetCurrentTarget(AActor* NewTarget) { CurrentTarget = NewTarget; }

protected:
	/** Current target for AI */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	AActor* CurrentTarget;

	/** Last time we attacked */
	float LastAttackTime;
};

