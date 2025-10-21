// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RPGHomingProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystemComponent;

/**
 * Homing projectile that automatically tracks and follows enemies
 * Used for staff weapon's basic attack
 */
UCLASS()
class ACTIONRPG_API ARPGHomingProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ARPGHomingProjectile();

	/** Sets the projectile's target to home in on */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetHomingTarget(AActor* NewTarget);

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

	/** Particle system for visual effect */
	UPROPERTY(VisibleDefaultsOnly, Category = "Effects")
	UParticleSystemComponent* ParticleComponent;

	/** Mesh component for the projectile */
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

	/** Gameplay tags to apply with damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FGameplayTagContainer DamageTags;

	/** Lifespan of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileLifespan;
};

