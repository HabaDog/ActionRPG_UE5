// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "RPGCharacterBase.h"
#include "RPGArcherCharacter.generated.h"

class UStateTreeComponent;

class ARPGArrowProjectile;

/**
 * Archer enemy character that uses StateTree for AI behavior
 */
UCLASS()
class ACTIONRPG_API ARPGArcherCharacter : public ARPGCharacterBase
{
	GENERATED_BODY()

public:
	ARPGArcherCharacter();

protected:
	virtual void BeginPlay() override;

	/** StateTree component for AI behavior (will be added via Blueprint) */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	// class UStateTreeComponent* StateTreeComponent;

	/** Arrow projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	TSubclassOf<ARPGArrowProjectile> ArrowProjectileClass;

	/** Attack range for the archer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	float AttackRange;

	/** Detection range for finding targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	float DetectionRange;

	/** Minimum distance to maintain from target */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	float MinimumDistance;

	/** Movement speed when patrolling */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float PatrolSpeed;

	/** Movement speed when chasing target */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float ChaseSpeed;

	/** Current target actor */
	UPROPERTY(BlueprintReadWrite, Category = AI)
	AActor* CurrentTarget;

	/** Patrol center location */
	UPROPERTY(BlueprintReadWrite, Category = AI)
	FVector PatrolCenter;

	/** Patrol radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI)
	float PatrolRadius;

public:
	/** Get current target */
	UFUNCTION(BlueprintCallable, Category = AI)
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	/** Set current target */
	UFUNCTION(BlueprintCallable, Category = AI)
	void SetCurrentTarget(AActor* NewTarget) { CurrentTarget = NewTarget; }

	/** Find nearest enemy target */
	UFUNCTION(BlueprintCallable, Category = AI)
	AActor* FindNearestTarget();

	/** Check if target is in attack range */
	UFUNCTION(BlueprintCallable, Category = AI)
	bool IsTargetInAttackRange() const;

	/** Check if target is too close */
	UFUNCTION(BlueprintCallable, Category = AI)
	bool IsTargetTooClose() const;

	/** Fire arrow at target */
	UFUNCTION(BlueprintCallable, Category = Combat)
	void FireArrow();

	/** Get random patrol location */
	UFUNCTION(BlueprintCallable, Category = AI)
	FVector GetRandomPatrolLocation();
};
