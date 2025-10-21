#include "AI/RPGStateTreeCondition_HasTargetInRange.h"
#include "RPGCharacterBase.h"
#include "RPGArcherCharacter.h"
#include "RPGArcherEnemy.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

bool FRPGStateTreeCondition_HasTargetInRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		UE_LOG(LogTemp, Error, TEXT("[HasTargetInRange] TestCondition: TargetActor is NULL!"));
		return false;
	}

	// 查找最近的目标
	AActor* Target = FindNearestTarget(Actor, InstanceData.DetectionRange);
	InstanceData.FoundTarget = Target;

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HasTargetInRange] No target found in range %f for %s"), 
			InstanceData.DetectionRange, *Actor->GetName());
		return false;
	}

	// 计算距离
	float Distance = FVector::Dist(Actor->GetActorLocation(), Target->GetActorLocation());

	// 检查是否在距离范围内
	bool bInRange = Distance >= InstanceData.MinDistance && Distance <= InstanceData.MaxDistance;
	
	UE_LOG(LogTemp, Warning, TEXT("[HasTargetInRange] Actor: %s, Target: %s, Distance: %f, MinDist: %f, MaxDist: %f, Result: %d"), 
		*Actor->GetName(), *Target->GetName(), Distance, InstanceData.MinDistance, InstanceData.MaxDistance, bInRange);
	
	return bInRange;
}

AActor* FRPGStateTreeCondition_HasTargetInRange::FindNearestTarget(AActor* SearchOrigin, float Range) const
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

