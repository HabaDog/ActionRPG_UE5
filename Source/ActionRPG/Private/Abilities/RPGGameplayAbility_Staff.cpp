// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGGameplayAbility_Staff.h"
#include "RPGHomingProjectile.h"
#include "RPGCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"

URPGGameplayAbility_Staff::URPGGameplayAbility_Staff()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	TargetSearchRange = 2000.0f;
	BaseDamage = 25.0f;
	SpawnOffset = FVector(50.0f, 0.0f, 50.0f);
}

void URPGGameplayAbility_Staff::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Spawn the projectile
	SpawnHomingProjectile();

	// End ability immediately as projectile handles the rest
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void URPGGameplayAbility_Staff::SpawnHomingProjectile()
{
	if (!ProjectileClass)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find target
	AActor* Target = FindBestTarget();

	// Calculate spawn location
	FVector SpawnLocation = Character->GetActorLocation();
	FRotator SpawnRotation = Character->GetActorRotation();

	// Add offset (forward and up from character)
	FVector ForwardOffset = Character->GetActorForwardVector() * SpawnOffset.X;
	FVector RightOffset = Character->GetActorRightVector() * SpawnOffset.Y;
	FVector UpOffset = Character->GetActorUpVector() * SpawnOffset.Z;
	SpawnLocation += ForwardOffset + RightOffset + UpOffset;

	// If we have a target, aim towards it
	if (Target)
	{
		FVector DirectionToTarget = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
		SpawnRotation = DirectionToTarget.Rotation();
	}

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Character;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARPGHomingProjectile* Projectile = World->SpawnActor<ARPGHomingProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	if (Projectile)
	{
		// Initialize projectile with damage and instigator
		Projectile->InitializeProjectile(BaseDamage, Character);

		// Set homing target if we found one
		if (Target)
		{
			Projectile->SetHomingTarget(Target);
		}
	}
}

AActor* URPGGameplayAbility_Staff::FindBestTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get all actors in range
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	FVector SearchLocation = Character->GetActorLocation();
	bool bFoundTargets = World->OverlapMultiByChannel(
		OverlapResults,
		SearchLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(TargetSearchRange),
		QueryParams
	);

	if (!bFoundTargets)
	{
		return nullptr;
	}

	// Find closest valid enemy target
	AActor* BestTarget = nullptr;
	float ClosestDistance = TargetSearchRange;

	ARPGCharacterBase* PlayerCharacter = Cast<ARPGCharacterBase>(Character);

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* PotentialTarget = Result.GetActor();
		if (!PotentialTarget)
		{
			continue;
		}

		// Check if it's an enemy character
		ARPGCharacterBase* TargetCharacter = Cast<ARPGCharacterBase>(PotentialTarget);
		if (!TargetCharacter)
		{
			continue;
		}

		// Skip if it's on the same team (simple team check - can be enhanced)
		if (PlayerCharacter && TargetCharacter->GetTeamId() == PlayerCharacter->GetTeamId())
		{
			continue;
		}

		// Skip if dead
		if (TargetCharacter->GetHealth() <= 0.0f)
		{
			continue;
		}

		// Check distance
		float Distance = FVector::Distance(SearchLocation, PotentialTarget->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			BestTarget = PotentialTarget;
		}
	}

	return BestTarget;
}

