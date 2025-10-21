// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Items/RPGWeaponItem.h"
#include "RPGStaffItem.generated.h"

/** Native base class for magic staffs, should be blueprinted */
UCLASS()
class ACTIONRPG_API URPGStaffItem : public URPGWeaponItem
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGStaffItem()
	{
		ItemType = URPGAssetManager::WeaponItemType;
		MagicAttackPowerBonus = 0.0f;
		ProjectileSpeed = 1000.0f;
	}

	/** Projectile class to spawn for arcane missiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Staff)
	TSubclassOf<AActor> ArcaneMissileClass;

	/** Magic attack power bonus for this staff */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Staff)
	float MagicAttackPowerBonus;

	/** Projectile speed for arcane missiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Staff)
	float ProjectileSpeed;
};
