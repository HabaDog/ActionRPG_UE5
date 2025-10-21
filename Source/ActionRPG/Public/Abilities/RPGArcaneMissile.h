// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Abilities/RPGAbilityTypes.h"
#include "RPGArcaneMissile.generated.h"

class ARPGCharacterBase;

UCLASS()
class ACTIONRPG_API ARPGArcaneMissile : public AActor
{
	GENERATED_BODY()
	
public:	
	ARPGArcaneMissile();

protected:
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	UProjectileMovementComponent* ProjectileMovement;

	/** Mesh component for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
	UStaticMeshComponent* ProjectileMesh;

	/** Effect container to apply on hit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffects)
	FRPGGameplayEffectContainer HitEffectContainer;

	/** Target to home in on */
	UPROPERTY(BlueprintReadWrite, Category = Targeting)
	AActor* HomingTarget;

	/** Maximum homing range */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Targeting)
	float HomingRange;

	/** Homing acceleration magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Targeting)
	float HomingAcceleration;

	/** Lifespan of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile)
	float ProjectileLifespan;

public:	
	virtual void Tick(float DeltaTime) override;

	/** Initialize the projectile with target and effect container */
	UFUNCTION(BlueprintCallable, Category = Projectile)
	void InitializeProjectile(AActor* Target, const FRPGGameplayEffectContainer& EffectContainer);

	/** Called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	/** Find the nearest enemy target within range */
	AActor* FindNearestTarget();

	/** Update homing behavior */
	void UpdateHoming(float DeltaTime);
};
