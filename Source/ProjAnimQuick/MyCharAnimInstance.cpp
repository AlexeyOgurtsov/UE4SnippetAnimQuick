#include "MyCharAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(MyAnimLog, Log, All);
DEFINE_LOG_CATEGORY(MyAnimLog);

void UMyCharAnimInstance::NativeInitializeAnimation()
{
	bUseUpperBodySlot = false;
	bBlendingOutUpperBody = false;
	bMontageStartedOnBlendOut = false;

	WeaponAnim = EWeaponAnimState::None;
	WeaponIndex = 0;

	// WARNING!!! Very important
	Super::NativeInitializeAnimation();
	UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::NativeInitializeAnimation"));

	OnMontageBlendingOut.AddDynamic(this, &UMyCharAnimInstance::MontageBlendingStarted);
	OnMontageStarted.AddDynamic(this, &UMyCharAnimInstance::MontageStarted);
	OnMontageEnded.AddDynamic(this, &UMyCharAnimInstance::MontageEnded);
}

void UMyCharAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	// WARNING!!! Very important
	Super::NativeUpdateAnimation(DeltaTimeX);

	if(APawn* P = TryGetPawnOwner())
	{
		// Movement properties:
		if(ACharacter* C = Cast<ACharacter>(P))
		{
			bCrouching = C->bIsCrouched;
			bFalling = C->GetCharacterMovement()->IsFalling();
		}

		// Movement
		FVector const Velocity = P->GetVelocity();
		FVector const ControlRotation = P->GetControlRotation().Vector();

		Speed = Velocity.Size();

		// Dot: Proportional to Sin
		float Dot = Velocity.X * ControlRotation.X + Velocity.Y * ControlRotation.Y; 

		// Determinant: Proportional to Cos:
		float Determinant = Velocity.X * ControlRotation. Y + Velocity.Y * ControlRotation.X;
		DeltaRotation = FMath::Atan2(Dot, Determinant);
	}
	StartWeaponAnimations_IfNeededNow();
}

void UMyCharAnimInstance::StartWeaponAnimations_IfNeededNow()
{
	if(WeaponAnim != EWeaponAnimState::None)
	{
		if(WeaponAnims.IsValidIndex(WeaponIndex))
		{
			const FWeaponAnimInfo& AnimInfo = WeaponAnims[WeaponIndex];

			const FSingleAnimSequenceInfo& ActionAnimInfo = GetAnimInfoForWeaponState(AnimInfo, WeaponAnim);

			// If the animation can be interrupted we always start animation for state,
			// otherwise we start only if the previous upper-body animation is already finished playing
			bool bShouldStartAnimation = false;
			if( ! ActionAnimInfo.bAlwaysFinish )
			{
				bShouldStartAnimation = true;
			}
			else
			{
				if(GetUpperBodyMontage() == nullptr)
				{
					bShouldStartAnimation = true;
				}
				else
				{
					if(ActionAnimInfo.bStartNextOnBlendingStarted && bBlendingOutUpperBody && ( ! bMontageStartedOnBlendOut ))
					{
						bShouldStartAnimation = true;
						bMontageStartedOnBlendOut = true;
					}
				}
			}

			if( bShouldStartAnimation )
			{
				StartWeaponAnimationForState(AnimInfo, WeaponAnim);
			}
		
		}
	}
}

const FSingleAnimSequenceInfo& UMyCharAnimInstance::GetAnimInfoForWeaponState
(
	const FWeaponAnimInfo& InAnimInfo,
	EWeaponAnimState InAnimState
) 
{
	switch(InAnimState)
	{
	case EWeaponAnimState::None:
		checkNoEntry();
		break;

	case EWeaponAnimState::Fire:
		return InAnimInfo.Fire;

	case EWeaponAnimState::Reload:
		return InAnimInfo.Reload;

	case EWeaponAnimState::Equip:
		return InAnimInfo.Equip;

	default:
		checkNoEntry();
	}
	return InAnimInfo.Fire;
}

void UMyCharAnimInstance::StartWeaponAnimationForState(const FWeaponAnimInfo& InAnimInfo, EWeaponAnimState InNewState)
{
	switch(InNewState)
	{
	case EWeaponAnimState::None:
		checkNoEntry();
		break;

	case EWeaponAnimState::Fire:
		UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::StartWeaponAnimationForState: Fire"));
		StartFireSequence(InAnimInfo);
		break;

	case EWeaponAnimState::Reload:
		UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::StartWeaponAnimationForState: Reload"));
		StartReloadSequence(InAnimInfo);
		break;

	case EWeaponAnimState::Equip:
		UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::StartWeaponAnimationForState: Equip"));
		StartEquipSequence(InAnimInfo);
		break;

	default:
		checkNoEntry();
	}
}

void UMyCharAnimInstance::MontageBlendingStarted(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::MontageBlendingStarted"));
	if(Montage == CurrUpperBodyMontage)
	{
		UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::MontageBlendingStarted: Upper body montage blending started"));
		bBlendingOutUpperBody = true;
		// WARNING!
		// Here we should NOT set blending out upper body montage flag.
	}
}

void UMyCharAnimInstance::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::MontageEnded"));
	if(Montage == CurrUpperBodyMontage)
	{
		UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::OnMontageEnded: Upper body montage ended, and no other upper body montage is started"));
		CurrUpperBodyMontage = nullptr;
		bUseUpperBodySlot = false;
		bBlendingOutUpperBody = false;
		bMontageStartedOnBlendOut = false;
	}
}

void UMyCharAnimInstance::MontageStarted(UAnimMontage* Montage)
{
	UE_LOG(MyAnimLog, Log, TEXT("UMyCharAnimInstance::MontageStarted"));
	// Nothing is to be done here yet
}

UAnimMontage* UMyCharAnimInstance::StartFireSequence(const FWeaponAnimInfo& InAnimInfo)
{
	return StartUpperBodyAnim(InAnimInfo.Fire);
}

UAnimMontage* UMyCharAnimInstance::StartEquipSequence(const FWeaponAnimInfo& InAnimInfo)
{
	return StartUpperBodyAnim(InAnimInfo.Equip);
}
	
UAnimMontage* UMyCharAnimInstance::StartReloadSequence(const FWeaponAnimInfo& InAnimInfo)
{
	return StartUpperBodyAnim(InAnimInfo.Reload);
}

UAnimMontage* UMyCharAnimInstance::StartUpperBodyAnim(const FSingleAnimSequenceInfo& InAnimInfo)
{
	return StartUpperBodySequence
	(
		InAnimInfo.Sequence,
		/*PlayRate=*/1.0F,
		InAnimInfo.BlendingIn, InAnimInfo.BlendingOut,
		InAnimInfo.LoopCount
	);
}

UAnimMontage* UMyCharAnimInstance::StartUpperBodySequence
(
	UAnimSequenceBase* InAsset, 
	float InBlendIn, float InBlendOut, 
	float InPlayRate,
	float InLoopCount,
	float InBlendOutTriggerTime,
	float InTimeToStartMontageAt
)
{
	UAnimMontage* const Montage = PlaySlotAnimationAsDynamicMontage
	(
		InAsset, UpperBodySlotName,
		InBlendIn, InBlendOut,
		InPlayRate, InLoopCount,
		InBlendOutTriggerTime,
		InTimeToStartMontageAt
	);
	bUseUpperBodySlot = true;
	CurrUpperBodyMontage = Montage;
	return Montage;
}