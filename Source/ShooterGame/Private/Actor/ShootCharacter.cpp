// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ShootCharacter.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShootCharacter::AShootCharacter():
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
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
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->DeltaTimeSeconds);
}

void AShootCharacter::TurnRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->DeltaTimeSeconds);
}

void AShootCharacter::FireWeaponPressed()
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
	//获取当前视口尺寸
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation (ViewportSize.X / 2,ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);
	if(bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };
		
		OutBeamLocation = End;
		//光束终点现在跟踪命中位置
		GetWorld()->LineTraceSingleByChannel(
				ScreenTraceHit, 
				Start, 
				End, 
				ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit) // was there a trace hit?
		{
			OutBeamLocation = End;
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
		}
		return true;
	}
	return false;
}

// Called every frame
void AShootCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShootCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire",EInputEvent::IE_Pressed,this,&AShootCharacter::FireWeaponPressed);
	
	PlayerInputComponent->BindAxis("MoveForward",this,&AShootCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&AShootCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate",this,&AShootCharacter::TurnRate);
	PlayerInputComponent->BindAxis("LookUpRate",this,&AShootCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn",this,&AShootCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",this,&AShootCharacter::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}

