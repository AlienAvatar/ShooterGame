// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ShootCharacter.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite,Category=CharacterInfo)
	float Speed;

	UPROPERTY(BlueprintReadWrite,Category=CharacterInfo)
	float Accleration;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=CharacterInfo,meta=(AllowPrivateAccess="true"))
	bool bIsInAir;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=CharacterInfo,meta=(AllowPrivateAccess="true"))
	bool bIsAccelerating;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Movement,meta=(AllowPrivateAccess="true"))
	AShootCharacter* Character;

	// 用Yaw差值做平移
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Movement,meta=(AllowPrivateAccess="true"))
	float MovementOffsetYaw;

	//停止移动的最后一帧Yaw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
protected:
	void NativeInitializeAnimation() override;
	
public:
	void NativeUpdateAnimation(float DeltaSeconds) override;

	
};
