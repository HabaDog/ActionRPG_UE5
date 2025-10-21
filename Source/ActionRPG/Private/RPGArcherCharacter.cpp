// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGArcherCharacter.h"
#include "Abilities/RPGArrowProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

ARPGArcherCharacter::ARPGArcherCharacter()
{
	// StateTree component will be added via Blueprint
	// StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));

	// Set default values
	AttackRange = 800.0f;
	DetectionRange = 1200.0f;
	MinimumDistance = 400.0f;
	PatrolSpeed = 200.0f;
	ChaseSpeed = 400.0f;
	PatrolRadius = 500.0f;
	CurrentTarget = nullptr;

	// Set team ID for enemy (will be set in Blueprint or via SetGenericTeamId)
	// TeamId = FGenericTeamId(1);
}

void ARPGArcherCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 强制启用重力和行走模式，确保角色落到地面
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->GravityScale = 1.0f;
		MovementComp->SetMovementMode(MOVE_Walking);
		MovementComp->MaxWalkSpeed = PatrolSpeed;
		
		UE_LOG(LogTemp, Warning, TEXT("[ArcherCharacter] BeginPlay: Initial position = %s"), *GetActorLocation().ToString());
	}

	// Set patrol center to starting location (在角色落地后会更新)
	PatrolCenter = GetActorLocation();
}

AActor* ARPGArcherCharacter::FindNearestTarget()
{
	AActor* NearestTarget = nullptr;
	float NearestDistance = DetectionRange;

	// Get all actors of class RPGCharacterBase
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor != this)
		{
			if (ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(Actor))
			{
				// Check if it's a player (different team)
				bool bIsValidTarget = false;
				if (Character->GetController() && Character->GetController()->IsPlayerController())
				{
					bIsValidTarget = true;
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

bool ARPGArcherCharacter::IsTargetInAttackRange() const
{
	if (!CurrentTarget)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= AttackRange;
}

bool ARPGArcherCharacter::IsTargetTooClose() const
{
	if (!CurrentTarget)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance < MinimumDistance;
}

void ARPGArcherCharacter::FireArrow()
{
	if (!ArrowProjectileClass || !CurrentTarget)
	{
		return;
	}

	// Calculate spawn location (in front of character)
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0, 0, 50);

	// Calculate direction to target
	FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = DirectionToTarget.Rotation();

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the arrow
	if (ARPGArrowProjectile* Arrow = GetWorld()->SpawnActor<ARPGArrowProjectile>(ArrowProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		// Initialize the arrow with target
		Arrow->InitializeArrow(CurrentTarget);
	}
}

FVector ARPGArcherCharacter::GetRandomPatrolLocation()
{
	// Generate random point within patrol radius
	FVector RandomDirection = FVector(
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f),
		0.0f
	).GetSafeNormal();

	float RandomDistance = FMath::RandRange(0.0f, PatrolRadius);
	FVector RandomLocation = PatrolCenter + RandomDirection * RandomDistance;

	// Keep the Z coordinate at ground level
	RandomLocation.Z = PatrolCenter.Z;

	return RandomLocation;
}
