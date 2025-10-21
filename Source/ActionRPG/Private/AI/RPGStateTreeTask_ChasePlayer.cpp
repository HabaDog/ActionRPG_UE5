#include "AI/RPGStateTreeTask_ChasePlayer.h"
#include "AIController.h"
#include "RPGArcherCharacter.h"
#include "RPGArcherEnemy.h"
#include "RPGCharacterBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

FRPGStateTreeTask_ChasePlayer::FRPGStateTreeTask_ChasePlayer()
{
}

EStateTreeRunStatus FRPGStateTreeTask_ChasePlayer::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	UE_LOG(LogTemp, Warning, TEXT("[ChasePlayer] EnterState called!"));

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("[ChasePlayer] EnterState: TargetActor is NULL!"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ChasePlayer] TargetActor: %s"), *Actor->GetName());

	// 查找最近的目标
	AActor* Target = FindNearestTarget(Actor, InstanceData.DetectionRange);
	InstanceData.CachedTarget = Target;

	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("[ChasePlayer] EnterState: FindNearestTarget returned NULL! Detection Range: %f"), InstanceData.DetectionRange);
		return EStateTreeRunStatus::Failed;
	}

	// 设置追击速度
	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = InstanceData.ChaseSpeed;
			UE_LOG(LogTemp, Warning, TEXT("[ChasePlayer] Set chase speed to %f"), InstanceData.ChaseSpeed);
		}
	}

	// 开始追击
	if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
	{
		AIController->MoveToActor(Target, InstanceData.AcceptanceRadius);
		UE_LOG(LogTemp, Warning, TEXT("[ChasePlayer] Start chasing target: %s"), *Target->GetName());
		return EStateTreeRunStatus::Running;
	}

	UE_LOG(LogTemp, Error, TEXT("[ChasePlayer] EnterState: No AI Controller found!"));
	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FRPGStateTreeTask_ChasePlayer::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	AActor* Target = InstanceData.CachedTarget;

	if (!Actor || !Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 持续追击目标
	if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
	{
		AIController->MoveToActor(Target, InstanceData.AcceptanceRadius);
	}

	return EStateTreeRunStatus::Running;
}

void FRPGStateTreeTask_ChasePlayer::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (Actor)
	{
		if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}

	InstanceData.CachedTarget = nullptr;
}

AActor* FRPGStateTreeTask_ChasePlayer::FindNearestTarget(AActor* SearchOrigin, float Range) const
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

