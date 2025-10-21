// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGArcaneMissile.h"
#include "RPGCharacterBase.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ARPGArcaneMissile::ARPGArcaneMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->SetSphereRadius(5.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootComponent = CollisionComponent;

	// Create projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Create mesh component
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set default values
	HomingRange = 1000.0f;
	HomingAcceleration = 2000.0f;
	ProjectileLifespan = 5.0f;
	HomingTarget = nullptr;

	// Bind hit event
	CollisionComponent->OnComponentHit.AddDynamic(this, &ARPGArcaneMissile::OnHit);
}

void ARPGArcaneMissile::BeginPlay()
{
	Super::BeginPlay();
	
	// Set lifespan
	SetLifeSpan(ProjectileLifespan);

	// Find initial target if none set
	if (!HomingTarget)
	{
		HomingTarget = FindNearestTarget();
	}
}

void ARPGArcaneMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update homing behavior
	UpdateHoming(DeltaTime);
}

void ARPGArcaneMissile::InitializeProjectile(AActor* Target, const FRPGGameplayEffectContainer& EffectContainer)
{
	HomingTarget = Target;
	HitEffectContainer = EffectContainer;
}

void ARPGArcaneMissile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit the instigator
	if (OtherActor && OtherActor != GetInstigator())
	{
		// Apply gameplay effects if we hit a character
		if (ARPGCharacterBase* HitCharacter = Cast<ARPGCharacterBase>(OtherActor))
		{
			if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(HitCharacter->GetAbilitySystemComponent()))
			{
				// Apply effects to the hit target
				for (const TSubclassOf<UGameplayEffect>& EffectClass : HitEffectContainer.TargetGameplayEffectClasses)
				{
					if (EffectClass)
					{
						FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
						EffectContext.AddSourceObject(this);
						EffectContext.AddHitResult(Hit);

						FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
						if (EffectSpec.IsValid())
						{
							ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
						}
					}
				}
			}
		}

		// Destroy the projectile
		Destroy();
	}
}

AActor* ARPGArcaneMissile::FindNearestTarget()
{
	AActor* NearestTarget = nullptr;
	float NearestDistance = HomingRange;

	// Get all actors of class RPGCharacterBase
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor != GetInstigator())
		{
			// Check if it's an enemy (different team)
			if (ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(Actor))
			{
				// Simple team check - if instigator is player, target enemies, and vice versa
				bool bIsValidTarget = false;
				if (ARPGCharacterBase* InstigatorCharacter = Cast<ARPGCharacterBase>(GetInstigator()))
				{
					// For now, simple check: if instigator has PlayerController, target NPCs without PlayerController
					bIsValidTarget = (InstigatorCharacter->GetController() && InstigatorCharacter->GetController()->IsPlayerController()) != 
									 (Character->GetController() && Character->GetController()->IsPlayerController());
				}

				if (bIsValidTarget)
				{
					float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
					if (Distance < NearestDistance)
					{
						NearestDistance = Distance;
						NearestTarget = Actor;
					}
				}
			}
		}
	}

	return NearestTarget;
}

void ARPGArcaneMissile::UpdateHoming(float DeltaTime)
{
	if (HomingTarget && ProjectileMovement)
	{
		// Check if target is still valid and in range
		float DistanceToTarget = FVector::Dist(GetActorLocation(), HomingTarget->GetActorLocation());
		if (DistanceToTarget > HomingRange * 2.0f || !IsValid(HomingTarget))
		{
			// Target too far or invalid, find new target
			HomingTarget = FindNearestTarget();
		}

		if (HomingTarget)
		{
			// Calculate direction to target
			FVector DirectionToTarget = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			
			// Apply homing acceleration
			FVector CurrentVelocity = ProjectileMovement->Velocity;
			FVector DesiredVelocity = DirectionToTarget * ProjectileMovement->MaxSpeed;
			FVector AccelerationVector = (DesiredVelocity - CurrentVelocity) * HomingAcceleration * DeltaTime;
			
			// Limit acceleration magnitude
			if (AccelerationVector.Size() > HomingAcceleration * DeltaTime)
			{
				AccelerationVector = AccelerationVector.GetSafeNormal() * HomingAcceleration * DeltaTime;
			}
			
			// Apply the acceleration
			ProjectileMovement->Velocity += AccelerationVector;
			
			// Ensure we don't exceed max speed
			if (ProjectileMovement->Velocity.Size() > ProjectileMovement->MaxSpeed)
			{
				ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * ProjectileMovement->MaxSpeed;
			}
		}
	}
}
