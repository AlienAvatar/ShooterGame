// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Sound/SoundCue.h"
#include "ShootCharacter.generated.h"

UCLASS()
class SHOOTERGAME_API AShootCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShootCharacter();

protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Comp",meta=(AllowPrivateAccess = true))
	USpringArmComponent* SpringArmComp;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Comp",meta=(AllowPrivateAccess = true))
	UCameraComponent* CameraComp;

	/*UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Comp",meta=(AllowPrivateAccess = true))
	TSubclassOf<>* CameraComp;*/

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Sound",meta=(AllowPrivateAccess = true))
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Sound",meta=(AllowPrivateAccess = true))
	UAnimMontage* FireMontage;

	/** 转动速率 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** 向上看速率*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//枪口效果
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Effect",meta=(AllowPrivateAccess = true))
	UParticleSystem* MuzzleParticle;

	//子弹爆炸效果
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Effect",meta=(AllowPrivateAccess = true))
	UParticleSystem* HitWorldParticle;

	//烟雾轨迹
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Effect",meta=(AllowPrivateAccess = true))
	UParticleSystem* BeamParticles;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void LookUpRate(float Value);

	void TurnRate(float Value);
	
	void FireWeaponPressed();

	void PlayWeaponEffect(FVector BeamLocation,FTransform SocketTransform);

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
