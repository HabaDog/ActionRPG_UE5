#include "AI/RPGStateTreeTask_FireArrow.h"
#include "AIController.h"
#include "RPGArcherCharacter.h"
#include "RPGArcherEnemy.h"
#include "RPGCharacterBase.h"
#include "Abilities/RPGArrowProjectile.h"
#include "StateTreeExecutionContext.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

FRPGStateTreeTask_FireArrow::FRPGStateTreeTask_FireArrow()
{
}

EStateTreeRunStatus FRPGStateTreeTask_FireArrow::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 查找最近的目标
	AActor* Target = FindNearestTarget(Actor, InstanceData.DetectionRange);
	InstanceData.CachedTarget = Target;

	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 面向目标
	FVector DirectionToTarget = (Target->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
	FRotator TargetRotation = DirectionToTarget.Rotation();
	Actor->SetActorRotation(TargetRotation);

	// 立即射击一次
	InstanceData.TimeSinceLastAttack = InstanceData.AttackCooldown;
	FireArrow(InstanceData, Target);

	UE_LOG(LogTemp, Warning, TEXT("[FireArrow] EnterState: Start attacking target %s"), *Target->GetName());
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRPGStateTreeTask_FireArrow::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	AActor* Target = InstanceData.CachedTarget;

	if (!Actor || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 更新冷却时间
	InstanceData.TimeSinceLastAttack += DeltaTime;

	// 持续面向目标
	FVector DirectionToTarget = (Target->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
	FRotator TargetRotation = DirectionToTarget.Rotation();
	Actor->SetActorRotation(FMath::RInterpTo(Actor->GetActorRotation(), TargetRotation, DeltaTime, 5.0f));

	// 冷却时间到了就射击
	if (InstanceData.TimeSinceLastAttack >= InstanceData.AttackCooldown)
	{
		FireArrow(InstanceData, Target);
		InstanceData.TimeSinceLastAttack = 0.0f;
	}

	return EStateTreeRunStatus::Running;
}

void FRPGStateTreeTask_FireArrow::FireArrow(const FInstanceDataType& InstanceData, AActor* Target) const
{
	AActor* Actor = InstanceData.TargetActor;

	if (!Actor || !Target)
	{
		UE_LOG(LogTemp, Error, TEXT("[FireArrow] FireArrow: Actor or Target is NULL!"));
		return;
	}

	if (!InstanceData.ArrowProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[FireArrow] FireArrow: ArrowProjectileClass is not set!"));
		return;
	}

	// 生成箭矢
	FVector SpawnLocation = Actor->GetActorLocation() + Actor->GetActorForwardVector() * 100.0f + FVector(0, 0, 50.0f);
	FRotator SpawnRotation = (Target->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(Actor);
	SpawnParams.Owner = Actor;

	if (UWorld* World = Actor->GetWorld())
	{
		ARPGArrowProjectile* Arrow = World->SpawnActor<ARPGArrowProjectile>(InstanceData.ArrowProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Arrow)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FireArrow] Spawned arrow projectile at %s"), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[FireArrow] Failed to spawn arrow projectile!"));
		}
	}
}

AActor* FRPGStateTreeTask_FireArrow::FindNearestTarget(AActor* SearchOrigin, float Range) const
{
	if (!SearchOrigin)
	{
		return nullptr;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(SearchOrigin->GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);

	AActor* NearestTarget = nullptr;
	float MinDistanceSq = FMath::Square(Range);

	for (AActor* Actor : FoundActors)
	{
		ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(Actor);
		
		// 排除自己
		if (!Character || Character == SearchOrigin)
		{
			continue;
		}

		// 排除所有弓箭兵敌人（只攻击玩家控制的角色）
		if (Character->IsA<ARPGArcherCharacter>() || Character->IsA<ARPGArcherEnemy>())
		{
			continue;
		}

		// 只攻击玩家控制的角色
		APawn* Pawn = Cast<APawn>(Character);
		if (!Pawn || !Cast<APlayerController>(Pawn->GetController()))
		{
			continue;
		}

		// 检查距离
		float DistanceSq = FVector::DistSquared(SearchOrigin->GetActorLocation(), Character->GetActorLocation());
		if (DistanceSq < MinDistanceSq)
		{
			MinDistanceSq = DistanceSq;
			NearestTarget = Character;
		}
	}

	return NearestTarget;
}

