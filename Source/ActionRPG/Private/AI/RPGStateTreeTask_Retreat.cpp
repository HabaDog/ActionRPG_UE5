#include "AI/RPGStateTreeTask_Retreat.h"
#include "AIController.h"
#include "RPGArcherCharacter.h"
#include "RPGCharacterBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

FRPGStateTreeTask_Retreat::FRPGStateTreeTask_Retreat()
{
}

EStateTreeRunStatus FRPGStateTreeTask_Retreat::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	// 设置撤退速度
	if (ARPGArcherCharacter* Archer = Cast<ARPGArcherCharacter>(Actor))
	{
		if (UCharacterMovementComponent* MovementComp = Archer->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = InstanceData.RetreatSpeed;
		}
	}

	// 计算撤退方向（从玩家指向弓箭兵的方向）
	FVector DirectionAwayFromPlayer = (Actor->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	
	// 计算撤退目标位置
	InstanceData.RetreatLocation = Actor->GetActorLocation() + DirectionAwayFromPlayer * InstanceData.RetreatDistance;
	InstanceData.bHasRetreatLocation = true;

	// 移动到撤退位置
	if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
	{
		AIController->MoveToLocation(InstanceData.RetreatLocation, 100.0f);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FRPGStateTreeTask_Retreat::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	AActor* Target = InstanceData.CachedTarget;

	if (!Actor || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 检查是否到达撤退位置或距离目标足够远
	float DistanceToRetreatLocation = FVector::Dist(Actor->GetActorLocation(), InstanceData.RetreatLocation);
	float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), Target->GetActorLocation());

	// 如果已经撤退到足够远的距离，任务完成
	if (DistanceToPlayer >= InstanceData.RetreatDistance * 0.8f || DistanceToRetreatLocation < 150.0f)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FRPGStateTreeTask_Retreat::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bHasRetreatLocation = false;
	InstanceData.CachedTarget = nullptr;

	AActor* Actor = InstanceData.TargetActor;
	if (Actor)
	{
		if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}
}

AActor* FRPGStateTreeTask_Retreat::FindNearestTarget(AActor* SearchOrigin, float Range) const
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
		
		// 排除自己和其他弓箭兵（只攻击玩家）
		if (Character && Character != SearchOrigin && !Character->IsA<ARPGArcherCharacter>())
		{
			float DistanceSq = FVector::DistSquared(SearchOrigin->GetActorLocation(), Character->GetActorLocation());
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				NearestTarget = Character;
			}
		}
	}

	return NearestTarget;
}

