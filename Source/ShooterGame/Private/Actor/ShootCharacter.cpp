// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ShootCharacter.h"

#include "DrawDebugHelpers.h"
#include "Actor/ItemBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShootCharacter::AShootCharacter():
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//相机视距的值
	CameraDefaultFOV(0.0f),
	CameraZoomedFOV(60.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	AimingTurnRate(0.2f),
	AimingLookUpRate(0.2f),
	HipTurnRate(1.0f),
	HipLookUpRate(1.0f),
	// Automatic fire variables
	FireRate(0.2f),
	bShouldFire(true),
	bFireWeaponPressed(false),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	bShouldTraceForItems(false),
	OverlappedItemCount(0)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;//摇臂在角色多少距离跟随
	SpringArmComp->bUsePawnControlRotation = true;//Controller根据摇臂旋转
	SpringArmComp->SocketOffset = FVector(0.f, 50.f, 50.f);//设置摇臂偏移
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->bUsePawnControlRotation = false;//Controller不跟随摄像机旋转
	CameraComp->SetupAttachment(SpringArmComp);
	
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	//在下落时角色可用的横向移动控制量  
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShootCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (CameraComp)
	{
		CameraDefaultFOV = CameraComp->FieldOfView;
	}
}

// Called every frame
void AShootCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	CameraInterpZoom(DeltaTime);

	//计算准星扩展系数
	CalculateCrosshairSpread(DeltaTime);
	
	FHitResult ItemTraceResult;
	FVector HitLocation;
	TraceUnderCrosshairs(ItemTraceResult,HitLocation);
	if (ItemTraceResult.bBlockingHit)
	{
		AItemBase* HitItem = Cast<AItemBase>(ItemTraceResult.Actor);
		if (HitItem && HitItem->GetPickupWidget())
		{
			// Show Item's Pickup Widget
			HitItem->GetPickupWidget()->SetVisibility(true);
		}
	}
}

void AShootCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShootCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShootCharacter::LookUpRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShootCharacter::TurnRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShootCharacter::LookUp(float Value)
{
	float LookUpRateScale;
	if(bIsAiming)
	{
		LookUpRateScale = AimingLookUpRate;
	}else
	{
		LookUpRateScale = HipLookUpRate;
	}
	
	AddControllerPitchInput(Value * LookUpRateScale);
}

void AShootCharacter::Turn(float Value)
{
	float TurnRateScale;
	if(bIsAiming)
	{
		TurnRateScale = AimingTurnRate;
	}else
	{
		TurnRateScale =  HipTurnRate;
	}
	AddControllerYawInput(Value * TurnRateScale);
}

void AShootCharacter::FireWeaponPressed()
{
	bFireWeaponPressed = true;
	StartFire();
}

void AShootCharacter::FireWeaponRealeased()
{
	bFireWeaponPressed = false;
}

void AShootCharacter::PlayWeaponEffect(FVector BeamLocation, FTransform SocketTransform)
{
	if(MuzzleParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),MuzzleParticle,SocketTransform);
	}

	if(HitWorldParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),HitWorldParticle,BeamLocation);
	}

	if(BeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if(Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamLocation);
		}
	}
}

bool AShootCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	
	if (bCrosshairHit) // was there a trace hit?
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}else
	{
		//no crosshair trace it
	}

	//发射第二条用来进行射线检测
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ OutBeamLocation };
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);
	if(WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}

	return false;
}

void AShootCharacter::AimingPressed()
{
	bIsAiming = true;
}

void AShootCharacter::AimingRealeaed()
{
	bIsAiming = false;
}

void AShootCharacter::CameraInterpZoom(float DeltaTime)
{
	if(bIsAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraZoomedFOV,
			DeltaTime,
			ZoomInterpSpeed);

		
	}else
	{
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraDefaultFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}
	CameraComp->SetFieldOfView(CameraCurrentFOV);
}

void AShootCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	FVector2D Speed(0,MaxSpeed);
	FVector2D Range(0,1);
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(Speed,Range,Velocity.Size());

	if(bIsAiming)
	{
		CrosshairAimFactor = 0.5f;
	}else
	{
		CrosshairAimFactor = 0.0f;
	}

	if(GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = 0.5f;
	}else
	{
		CrosshairInAirFactor = 0.0f;
	}
	CrosshairSpreadMultiplier = 0.5 + CrosshairVelocityFactor - CrosshairAimFactor + CrosshairInAirFactor;
	
}

void AShootCharacter::JumpingPressed()
{
	bIsJumping = true;
}

void AShootCharacter::JumpingReleased()
{
	bIsJumping = false;
}

void AShootCharacter::FireWeapon()
{
	//播放开火声音
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(),FireSound);
	}
	//获取枪口位置
	const FTransform BarrelSocketTransform = GetMesh()->GetSocketTransform(FName("BarrelSocket"));
	FVector BeamLocation;
	bool bBeamEnd = GetBeamEndLocation(BarrelSocketTransform.GetLocation(),BeamLocation);

	if(bBeamEnd)
	{
		//播放粒子效果
		PlayWeaponEffect(BeamLocation,BarrelSocketTransform);
	}
	
	//播放开火动画
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
	}
}

void AShootCharacter::StartFire()
{
	if(bShouldFire)
	{
		bShouldFire = false;
		FireWeapon();
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&AShootCharacter::ResetFire,
			FireRate);
	}
}

void AShootCharacter::ResetFire()
{
	bShouldFire = true;
	if(bFireWeaponPressed)
	{
		StartFire();
	}
}

void AShootCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
	CrosshairShootTimer, 
	this, 
	&AShootCharacter::FinishCrosshairBulletFire, 
	ShootTimeDuration);
}

void AShootCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

bool AShootCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult,FVector& OutHitLocation)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// Trace from Crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

void AShootCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			AItemBase* HitItem = Cast<AItemBase>(ItemTraceResult.Actor);
			if (HitItem && HitItem->GetPickupWidget())
			{
				// Show Item's Pickup Widget
				HitItem->GetPickupWidget()->SetVisibility(true);
			}
		}
	}
}


// Called to bind functionality to input
void AShootCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire",EInputEvent::IE_Pressed,this,&AShootCharacter::FireWeaponPressed);
	PlayerInputComponent->BindAction("Fire",EInputEvent::IE_Released,this,&AShootCharacter::FireWeaponRealeased);
	
	PlayerInputComponent->BindAxis("MoveForward",this,&AShootCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&AShootCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate",this,&AShootCharacter::TurnRate);
	PlayerInputComponent->BindAxis("LookUpRate",this,&AShootCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn",this,&AShootCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&AShootCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &AShootCharacter::AimingPressed);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &AShootCharacter::AimingRealeaed);
}

void AShootCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

