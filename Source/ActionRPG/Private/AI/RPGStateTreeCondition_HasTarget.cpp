#include "AI/RPGStateTreeCondition_HasTarget.h"
#include "RPGCharacterBase.h"
#include "RPGArcherCharacter.h"
#include "RPGArcherEnemy.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

bool FRPGStateTreeCondition_HasTarget::TestCondition(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("[HasTarget] TestCondition: TargetActor is NULL!"));
		return false;
	}

	// 查找最近的目标
	AActor* Target = FindNearestTarget(Actor, InstanceData.DetectionRange);
	InstanceData.FoundTarget = Target;

	// 获取当前状态信息（如果可能）
	static int32 CallCount = 0;
	CallCount++;

	if (Target)
	{
		float Distance = FVector::Dist(Actor->GetActorLocation(), Target->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("[HasTarget #%d] Found target: %s at distance: %f (Range: %f) - Returning TRUE"), 
			CallCount, *Target->GetName(), Distance, InstanceData.DetectionRange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HasTarget #%d] No target found in range %f for %s - Returning FALSE"), 
			CallCount, InstanceData.DetectionRange, *Actor->GetName());
	}

	return Target != nullptr;
}

AActor* FRPGStateTreeCondition_HasTarget::FindNearestTarget(AActor* SearchOrigin, float Range) const
{
	if (!SearchOrigin)
	{
		return nullptr;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(SearchOrigin->GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);

	AActor* NearestTarget = nullptr;
	float MinDistanceSq = FMath::Square(Range);

	UE_LOG(LogTemp, Verbose, TEXT("[HasTarget] FindNearestTarget: Found %d characters in world"), FoundActors.Num());

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
			UE_LOG(LogTemp, Verbose, TEXT("[HasTarget] Skipping archer: %s"), *Character->GetName());
			continue;
		}

		// 只攻击玩家控制的角色
		APawn* Pawn = Cast<APawn>(Character);
		if (!Pawn || !Cast<APlayerController>(Pawn->GetController()))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[HasTarget] Skipping non-player character: %s"), *Character->GetName());
			continue;
		}

		// 检查距离
		float DistanceSq = FVector::DistSquared(SearchOrigin->GetActorLocation(), Character->GetActorLocation());
		if (DistanceSq < MinDistanceSq)
		{
			MinDistanceSq = DistanceSq;
			NearestTarget = Character;
			UE_LOG(LogTemp, Verbose, TEXT("[HasTarget] Found player target: %s at distance: %f"), 
				*Character->GetName(), 
				FMath::Sqrt(DistanceSq));
		}
	}

	return NearestTarget;
}

