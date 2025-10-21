// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGArrowProjectile.h"
#include "RPGCharacterBase.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ARPGArrowProjectile::ARPGArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->SetSphereRadius(3.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootComponent = CollisionComponent;

	// Create projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f; // Slight gravity for realistic arc

	// Create mesh component
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(RootComponent);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set default values
	ProjectileLifespan = 8.0f;
	BaseDamage = 25.0f;
	TargetActor = nullptr;

	// Bind hit event
	CollisionComponent->OnComponentHit.AddDynamic(this, &ARPGArrowProjectile::OnHit);
}

void ARPGArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Set lifespan
	SetLifeSpan(ProjectileLifespan);
}

void ARPGArrowProjectile::InitializeArrow(AActor* Target, float Damage)
{
	TargetActor = Target;
	BaseDamage = Damage;
}

void ARPGArrowProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit the instigator
	if (OtherActor && OtherActor != GetInstigator())
	{
		// Apply damage if we hit a character
		if (ARPGCharacterBase* HitCharacter = Cast<ARPGCharacterBase>(OtherActor))
		{
			if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(HitCharacter->GetAbilitySystemComponent()))
			{
				// Create a simple damage effect
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				EffectContext.AddHitResult(Hit);

				// Apply direct damage (we'll create a proper GameplayEffect later)
				// For now, directly modify the damage attribute
				FGameplayEffectSpecHandle DamageSpec = ASC->MakeOutgoingSpec(nullptr, 1.0f, EffectContext);
				if (DamageSpec.IsValid())
				{
					// Set damage value directly
					DamageSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), BaseDamage);
					
					// Apply the damage
					ASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
				}
			}
		}

		// Destroy the projectile
		Destroy();
	}
}
