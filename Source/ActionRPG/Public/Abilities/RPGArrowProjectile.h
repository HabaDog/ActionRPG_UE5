// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Abilities/RPGAbilityTypes.h"
#include "RPGArrowProjectile.generated.h"

class ARPGCharacterBase;

UCLASS()
class ACTIONRPG_API ARPGArrowProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ARPGArrowProjectile();

protected:
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	UProjectileMovementComponent* ProjectileMovement;

	/** Mesh component for the arrow */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
	UStaticMeshComponent* ArrowMesh;

	/** Effect container to apply on hit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffects)
	FRPGGameplayEffectContainer HitEffectContainer;

	/** Target to aim at (optional) */
	UPROPERTY(BlueprintReadWrite, Category = Targeting)
	AActor* TargetActor;

	/** Lifespan of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile)
	float ProjectileLifespan;

	/** Damage amount */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	float BaseDamage;

public:	
	/** Initialize the arrow with target and damage */
	UFUNCTION(BlueprintCallable, Category = Projectile)
	void InitializeArrow(AActor* Target, float Damage = 25.0f);

	/** Called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
