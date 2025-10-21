// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGArcherEnemy.h"
#include "RPGArcherProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

ARPGArcherEnemy::ARPGArcherEnemy()
{
	// Set default values for archer combat
	IdealCombatDistance = 800.0f;
	MinimumCombatDistance = 400.0f;
	AttackRange = 1200.0f;
	DetectionRange = 1500.0f;
	RangedDamage = 20.0f;
	AttackCooldown = 2.0f;
	
	CurrentTarget = nullptr;
	LastAttackTime = -999.0f;
}

void ARPGArcherEnemy::FireProjectileAtTarget(AActor* Target)
{
	if (!Target || !ProjectileClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Check cooldown
	float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	// Calculate spawn location (from character's hand or weapon socket)
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	// Get socket location if available (e.g., "hand_r" socket)
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh && SkelMesh->DoesSocketExist(FName("hand_r")))
	{
		SpawnLocation = SkelMesh->GetSocketLocation(FName("hand_r"));
	}
	else
	{
		// Default offset if no socket
		SpawnLocation += GetActorForwardVector() * 50.0f + GetActorUpVector() * 100.0f;
	}

	// Aim at target
	FVector TargetLocation = Target->GetActorLocation();
	
	// Aim slightly above target for better hit chance
	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		TargetLocation.Z += 50.0f; // Aim at chest height
	}

	FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
	SpawnRotation = Direction.Rotation();

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARPGArcherProjectile* Projectile = World->SpawnActor<ARPGArcherProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	if (Projectile)
	{
		// Initialize projectile with damage
		Projectile->InitializeProjectile(RangedDamage, this);
		
		// Update last attack time
		LastAttackTime = CurrentTime;
	}
}

bool ARPGArcherEnemy::CanAttackTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// Check if target is alive
	if (ARPGCharacterBase* TargetCharacter = Cast<ARPGCharacterBase>(Target))
	{
		if (TargetCharacter->GetHealth() <= 0.0f)
		{
			return false;
		}
	}

	// Check if in attack range
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());
	if (Distance > AttackRange)
	{
		return false;
	}

	// Check cooldown
	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastAttackTime < AttackCooldown)
		{
			return false;
		}
	}

	return true;
}

