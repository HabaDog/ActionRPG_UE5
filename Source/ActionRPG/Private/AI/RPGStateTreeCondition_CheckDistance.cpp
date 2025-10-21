#include "AI/RPGStateTreeCondition_CheckDistance.h"
#include "StateTreeExecutionContext.h"
#include "RPGCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

bool FRPGStateTreeCondition_CheckDistance::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.TargetActor;
	if (!Actor)
	{
		return false;
	}

	// 始终自动查找最近的玩家（忽略 PlayerTarget 绑定）
	AActor* Target = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Actor->GetWorld(), ARPGCharacterBase::StaticClass(), FoundActors);
	
	float MinDistSq = MAX_FLT;
	for (AActor* FoundActor : FoundActors)
	{
		// 排除自己
		if (FoundActor == Actor)
		{
			continue;
		}

		// 只查找玩家控制的角色
		APawn* Pawn = Cast<APawn>(FoundActor);
		if (Pawn && Cast<APlayerController>(Pawn->GetController()))
		{
			float DistSq = FVector::DistSquared(Actor->GetActorLocation(), FoundActor->GetActorLocation());
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				Target = FoundActor;
			}
		}
	}

	if (!Target)
	{
		return false;
	}

	// 计算距离
	float Distance = FVector::Dist(Actor->GetActorLocation(), Target->GetActorLocation());

	// 检查是否在范围内
	bool bInRange = Distance >= InstanceData.MinDistance && Distance <= InstanceData.MaxDistance;
	
	UE_LOG(LogTemp, Warning, TEXT("[CheckDistance] Actor: %s, Target: %s, Distance: %f, MinDist: %f, MaxDist: %f, Result: %d"), 
		*Actor->GetName(), *Target->GetName(), Distance, InstanceData.MinDistance, InstanceData.MaxDistance, bInRange);
	
	return bInRange;
}

