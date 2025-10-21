// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RPGArcherProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystemComponent;

/**
 * Simple arrow projectile for archer enemies
 * Flies in straight line and deals physical damage
 */
UCLASS()
class ACTIONRPG_API ARPGArcherProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ARPGArcherProjectile();

	/** Sets the damage and power for this projectile */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeProjectile(float BaseDamage, AActor* DamageInstigator);

protected:
	virtual void BeginPlay() override;

	/** Called when projectile hits something */
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	USphereComponent* CollisionComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	/** Mesh component for the arrow */
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	UStaticMeshComponent* MeshComponent;

	/** Base damage to apply on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Damage;

	/** Actor that instigated this damage */
	UPROPERTY()
	AActor* InstigatorActor;

	/** Gameplay effect class to apply damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** Gameplay tags to apply with damage (physical damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FGameplayTagContainer DamageTags;

	/** Lifespan of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileLifespan;
};

