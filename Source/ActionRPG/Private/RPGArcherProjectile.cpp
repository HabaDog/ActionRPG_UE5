// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGArcherProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"

ARPGArcherProjectile::ARPGArcherProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &ARPGArcherProjectile::OnProjectileHit);
	RootComponent = CollisionComponent;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f; // Slight gravity for arrow arc

	// Default values
	Damage = 15.0f;
	ProjectileLifespan = 10.0f;
	
	// Physical damage (no magical tag)
	// DamageTags can be set in blueprints if needed
}

void ARPGArcherProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Set lifespan
	SetLifeSpan(ProjectileLifespan);
}

void ARPGArcherProjectile::InitializeProjectile(float BaseDamage, AActor* DamageInstigator)
{
	Damage = BaseDamage;
	InstigatorActor = DamageInstigator;
}

void ARPGArcherProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit ourselves or our instigator
	if (OtherActor && OtherActor != this && OtherActor != InstigatorActor)
	{
		// Try to apply damage via Gameplay Ability System
		if (IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OtherActor))
		{
			UAbilitySystemComponent* TargetASC = AbilitySystemInterface->GetAbilitySystemComponent();
			UAbilitySystemComponent* SourceASC = nullptr;
			
			if (InstigatorActor && InstigatorActor->Implements<UAbilitySystemInterface>())
			{
				SourceASC = Cast<IAbilitySystemInterface>(InstigatorActor)->GetAbilitySystemComponent();
			}

			if (TargetASC && DamageEffectClass)
			{
				// Create effect context
				FGameplayEffectContextHandle EffectContext = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
				EffectContext.AddInstigator(InstigatorActor, this);
				EffectContext.AddHitResult(Hit);

				// Create spec and apply damage
				FGameplayEffectSpecHandle SpecHandle = SourceASC ? SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext) : 
					FGameplayEffectSpecHandle(new FGameplayEffectSpec(DamageEffectClass.GetDefaultObject(), EffectContext, 1.0f));
				
				if (SpecHandle.IsValid())
				{
					// Set the damage magnitude
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
					
					// Add damage type tags (physical damage by default)
					SpecHandle.Data->DynamicGrantedTags.AppendTags(DamageTags);

					// Apply the effect
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}

		// Destroy projectile after hit
		Destroy();
	}
}

