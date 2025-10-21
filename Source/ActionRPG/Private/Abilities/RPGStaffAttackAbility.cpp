// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGStaffAttackAbility.h"
#include "Abilities/RPGArcaneMissile.h"
#include "Items/RPGStaffItem.h"
#include "RPGCharacterBase.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

URPGStaffAttackAbility::URPGStaffAttackAbility()
{
	// Set default values
	TargetingRange = 1500.0f;
	SpawnOffset = FVector(100.0f, 0.0f, 0.0f);
	
	// Set ability tags (will be configured in Blueprint)
	// AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Magic.Staff")));
	
	// Set activation requirements (will be configured in Blueprint)
	// ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Magic")));
	
	// Set cooldown and cost tags (will be configured in Blueprint)
	// CooldownTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Cooldown.Skill")));
}

void URPGStaffAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Fire the arcane missile
	FireArcaneMissile();

	// End ability immediately (instant cast)
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void URPGStaffAttackAbility::FireArcaneMissile()
{
	if (!ProjectileClass)
	{
		return;
	}

	ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	// Calculate spawn location
	FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * SpawnOffset.X + 
							Character->GetActorRightVector() * SpawnOffset.Y + 
							Character->GetActorUpVector() * SpawnOffset.Z;

	// Calculate spawn rotation (forward direction)
	FRotator SpawnRotation = Character->GetActorRotation();

	// Find target
	AActor* Target = FindTarget();

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the projectile
	if (ARPGArcaneMissile* Missile = GetWorld()->SpawnActor<ARPGArcaneMissile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		// Create effect container for magic damage
		FRPGGameplayEffectContainer EffectContainer;
		if (EffectContainerMap.Contains(FGameplayTag::RequestGameplayTag(FName("Event.Montage.Shared.MagicHit"))))
		{
			EffectContainer = EffectContainerMap[FGameplayTag::RequestGameplayTag(FName("Event.Montage.Shared.MagicHit"))];
		}

		// Initialize the missile
		Missile->InitializeProjectile(Target, EffectContainer);
	}
}

AActor* URPGStaffAttackAbility::FindTarget()
{
	ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestScore = 0.0f;

	// Get all actors of class RPGCharacterBase
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor != Character)
		{
			if (ARPGCharacterBase* PotentialTarget = Cast<ARPGCharacterBase>(Actor))
			{
				// Simple team check - if character is player, target NPCs without PlayerController
				bool bIsValidTarget = false;
				if (Character->GetController() && Character->GetController()->IsPlayerController())
				{
					// Player targeting NPCs
					bIsValidTarget = !(PotentialTarget->GetController() && PotentialTarget->GetController()->IsPlayerController());
				}
				else
				{
					// NPC targeting player
					bIsValidTarget = (PotentialTarget->GetController() && PotentialTarget->GetController()->IsPlayerController());
				}

				if (bIsValidTarget)
				{
					float Distance = FVector::Dist(Character->GetActorLocation(), Actor->GetActorLocation());
					if (Distance <= TargetingRange)
					{
						// Calculate score based on distance (closer = higher score) and angle to forward vector
						FVector DirectionToTarget = (Actor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
						float DotProduct = FVector::DotProduct(Character->GetActorForwardVector(), DirectionToTarget);
						
						// Score combines distance preference and forward-facing preference
						float DistanceScore = (TargetingRange - Distance) / TargetingRange;
						float AngleScore = (DotProduct + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
						float TotalScore = DistanceScore * 0.7f + AngleScore * 0.3f;

						if (TotalScore > BestScore)
						{
							BestScore = TotalScore;
							BestTarget = Actor;
						}
					}
				}
			}
		}
	}

	return BestTarget;
}