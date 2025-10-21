#include "AI/RPGStateTreeTask_RandomPatrol.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "RPGArcherCharacter.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

FRPGStateTreeTask_RandomPatrol::FRPGStateTreeTask_RandomPatrol()
{
}

EStateTreeRunStatus FRPGStateTreeTask_RandomPatrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("[RandomPatrol] EnterState FAILED: TargetActor is NULL!"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] EnterState: Actor = %s"), *Actor->GetName());

	// 设置移动速度
	if (ARPGArcherCharacter* Archer = Cast<ARPGArcherCharacter>(Actor))
	{
		if (UCharacterMovementComponent* MovementComp = Archer->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = InstanceData.PatrolSpeed;
			UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Set patrol speed to %f"), InstanceData.PatrolSpeed);
		}
	}

	// 获取随机巡逻点
	InstanceData.CurrentPatrolLocation = GetRandomPatrolLocation(Actor->GetActorLocation(), InstanceData.PatrolRadius, Actor);
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Patrol location: %s"), *InstanceData.CurrentPatrolLocation.ToString());
	
	// 移动到巡逻点
	AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[RandomPatrol] EnterState FAILED: AIController is NULL!"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Moving to patrol location..."));
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Current location: %s"), *Actor->GetActorLocation().ToString());
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Target location: %s"), *InstanceData.CurrentPatrolLocation.ToString());
	
	float ActualDistance = FVector::Dist(Actor->GetActorLocation(), InstanceData.CurrentPatrolLocation);
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Actual distance to target: %f"), ActualDistance);
	
	// 检查 AI Controller 的移动组件
	UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
	if (!PathFollowing)
	{
		UE_LOG(LogTemp, Error, TEXT("[RandomPatrol] AIController has NO PathFollowingComponent!"));
		return EStateTreeRunStatus::Failed;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] PathFollowingComponent exists"));
	
	FAIMoveRequest MoveRequest(InstanceData.CurrentPatrolLocation);
	MoveRequest.SetAcceptanceRadius(50.0f); // 减小接受半径
	MoveRequest.SetUsePathfinding(true);
	FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
	
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Move request result: %d (0=Failed, 1=Success, 2=AlreadyAtGoal)"), (int32)MoveResult.Code);
	
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Error, TEXT("[RandomPatrol] Failed to start movement! Possibly no valid path."));
		return EStateTreeRunStatus::Failed;
	}
	
	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] System thinks we're already at goal! Distance is %f but acceptance radius is 50.0"), ActualDistance);
		// 立即返回成功，让 Tick 函数重新选择目标
		return EStateTreeRunStatus::Running;
	}
	
	InstanceData.bIsMoving = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRPGStateTreeTask_RandomPatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 检查是否到达巡逻点
	float DistanceToTarget = FVector::Dist(Actor->GetActorLocation(), InstanceData.CurrentPatrolLocation);
	
	// 检查 AI 是否还在移动
	UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
	bool bIsMoving = PathFollowing && PathFollowing->GetStatus() != EPathFollowingStatus::Idle;
	
	// 如果到达目标或停止移动，选择新的巡逻点
	if (DistanceToTarget < 150.0f || !bIsMoving)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Tick: Reached patrol point or stopped (Distance: %f, IsMoving: %d), selecting new target..."), DistanceToTarget, bIsMoving);
		
		// 选择新的巡逻点
		InstanceData.CurrentPatrolLocation = GetRandomPatrolLocation(Actor->GetActorLocation(), InstanceData.PatrolRadius, Actor);
		
		UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Tick: New patrol location: %s"), *InstanceData.CurrentPatrolLocation.ToString());
		
		// 移动到新的巡逻点
		FAIMoveRequest MoveRequest(InstanceData.CurrentPatrolLocation);
		MoveRequest.SetAcceptanceRadius(50.0f);
		MoveRequest.SetUsePathfinding(true);
		FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
		
		UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Tick: Move request result: %d"), (int32)MoveResult.Code);
		
		if (MoveResult.Code == EPathFollowingRequestResult::Failed)
		{
			UE_LOG(LogTemp, Error, TEXT("[RandomPatrol] Tick: Failed to move to new location!"));
		}
	}

	return EStateTreeRunStatus::Running;
}

void FRPGStateTreeTask_RandomPatrol::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bIsMoving = false;

	AActor* Actor = InstanceData.TargetActor;
	if (Actor)
	{
		if (AAIController* AIController = Cast<AAIController>(Actor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}
}

FVector FRPGStateTreeTask_RandomPatrol::GetRandomPatrolLocation(const FVector& Origin, float Radius, UObject* WorldContext) const
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(WorldContext);
	if (NavSys)
	{
		FNavLocation RandomPt;
		int32 Attempts = 0;
		const int32 MaxAttempts = 10;
		const float MinDistance = 200.0f; // 最小距离
		
		// 多次尝试，确保找到一个足够远的点
		while (Attempts < MaxAttempts)
		{
			if (NavSys->GetRandomPointInNavigableRadius(Origin, Radius, RandomPt))
			{
				float Distance = FVector::Dist(Origin, RandomPt.Location);
				if (Distance >= MinDistance)
				{
					UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Found patrol point at distance: %f"), Distance);
					return RandomPt.Location;
				}
			}
			Attempts++;
		}
		
		// 如果找不到足够远的点，返回一个基于方向的点
		FVector RandomDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
		FVector TargetLocation = Origin + RandomDirection * Radius * 0.7f;
		
		// 尝试投影到导航网格
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(TargetLocation, ProjectedLocation, FVector(100.0f, 100.0f, 500.0f)))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Using projected patrol point"));
			return ProjectedLocation.Location;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[RandomPatrol] Failed to find valid patrol point, staying at origin"));
	return Origin;
}

