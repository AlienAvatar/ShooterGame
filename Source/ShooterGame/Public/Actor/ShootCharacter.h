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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
	
	/** Default camera field of view value */
	float CameraDefaultFOV;

	/** Field of view value for when zoomed in */
	float CameraZoomedFOV;

	/** 当前帧缩放*/
	float CameraCurrentFOV;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	//瞄准时鼠标X轴移动速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	//瞄准时鼠标Y轴移动速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	//Hip时鼠标X轴移动速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	//Hip时鼠标Y轴移动速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//准星缩放 速度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	//准星缩放 空中时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	//准星缩放 瞄准时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	//准星缩放 开火时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	//准星系数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	//是否跳跃
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	bool bIsJumping;

	FTimerHandle FireTimerHandle;

	bool bShouldFire;

	bool bFireWeaponPressed;

	float FireRate;

	bool bFiringBullet;

	FTimerHandle CrosshairShootTimer;

	float ShootTimeDuration;

	/** True if we should trace every frame for items */
	bool bShouldTraceForItems;

	/** Number of overlapped AItems */
	int8 OverlappedItemCount;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void LookUpRate(float Value);

	void TurnRate(float Value);

	void LookUp(float Value);

	void Turn(float Value);
	
	void FireWeaponPressed();

	void FireWeaponRealeased();
	
	void PlayWeaponEffect(FVector BeamLocation,FTransform SocketTransform);

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	void AimingPressed();
	
	void AimingRealeaed();

	void CameraInterpZoom(float DeltaTime);

	void CalculateCrosshairSpread(float DeltaTime);

	void JumpingPressed();

	void JumpingReleased();

	void FireWeapon();

	void StartFire();
	
	void ResetFire();

	void StartCrosshairBulletFire();

	void FinishCrosshairBulletFire();

	bool TraceUnderCrosshairs(FHitResult& OutHitResult,FVector& OutHitLocation);

	/** Trace for items if OverlappedItemCount > 0 */
	void TraceForItems();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE bool GetIsAiming() const{ return bIsAiming; }

	/** Adds/subtracts to/from OverlappedItemCount and updates bShouldTraceForItems */
	void IncrementOverlappedItemCount(int8 Amount);
};
