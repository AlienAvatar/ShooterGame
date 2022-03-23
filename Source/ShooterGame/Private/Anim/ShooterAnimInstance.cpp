// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/ShooterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AShootCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(Character)
	{
		//获取速度
		FVector Velocity = Character->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//是否在空中
		bIsInAir = Character->GetCharacterMovement()->IsFalling();

		//是否有加速度
		if(Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f)
		{
			bIsAccelerating = true;
			UE_LOG(LogTemp,Warning,TEXT("True"));
		}else
		{
			bIsAccelerating = false;
			UE_LOG(LogTemp,Warning,TEXT("False"));
		}

		FRotator AimRotation = Character->GetBaseAimRotation();
		//MovementRotation和Velocity.Rotation()是相等的
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation).Yaw;
		FString OffsetMessage =
				FString::Printf(
					TEXT("Movement Offset Yaw: %f"),
					MovementOffsetYaw);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		}

		if (Speed > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
	}else
	{
		Character = Cast<AShootCharacter>(TryGetPawnOwner());
	}
	
}
