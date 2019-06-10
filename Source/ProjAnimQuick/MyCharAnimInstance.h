#pragma once

#include "Animation/AnimInstance.h"
#include "MyCharAnimInstance.generated.h"

class UAnimSequenceBase;

UENUM()
enum class EWeaponAnimState : uint8
{
	None UMETA(DisplayName="None"),
	Fire UMETA(DisplayName="Fire"),
	Reload UMETA(DisplayName="Reload"),
	Equip UMETA(DisplayName="Equip"),
	Num UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSingleAnimSequenceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequenceBase* Sequence = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BlendingIn = 0.25F;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BlendingOut = 0.25F;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LoopCount = 1.F;

	/**
	* If true, than this animation must always be finished before running another.
	* (cannot be interrupted).
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAlwaysFinish = true;
	
	/**
	* If true, then the next animation will be started when blending is started.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bStartNextOnBlendingStarted = true;
};

USTRUCT(BlueprintType)
struct FWeaponAnimInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSingleAnimSequenceInfo Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSingleAnimSequenceInfo Reload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSingleAnimSequenceInfo Equip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSingleAnimSequenceInfo Unequip;	
};

UCLASS()
class UMyCharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;
	
protected:
	UFUNCTION(BlueprintPure)
	bool IsUsingUpperBodySlot() const { return bUseUpperBodySlot; }

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	EWeaponAnimState WeaponAnim;

	/**
	* Weapon that is used right now (-1 if not used now).
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	int32 WeaponIndex;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bFalling = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bCrouching = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bJumping = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float Speed = 0.0F;	

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	float DeltaRotation = 0.0F; // Between -180 to 180

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	TArray<FWeaponAnimInfo> WeaponAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	FName UpperBodySlotName;

	UFUNCTION(BlueprintCallable)	
	void StartWeaponAnimationForState(const FWeaponAnimInfo& InAnimInfo, EWeaponAnimState InNewState);

	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartFireSequence(const FWeaponAnimInfo& InAnimInfo);

	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartEquipSequence(const FWeaponAnimInfo& InAnimInfo);
	
	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartReloadSequence(const FWeaponAnimInfo& InAnimInfo);

	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartUpperBodyAnim(const FSingleAnimSequenceInfo& InAnimInfo);

	UFUNCTION(BlueprintCallable)
	UAnimMontage* StartUpperBodySequence
	(
		UAnimSequenceBase* InAsset, 
		float InBlendIn = 0.25F, float InBlendOut = 0.25F, 
		float InPlayRate = 1.0F,
		float InLoopCount = 1,
		float InBlendOutTriggerTime = -1.F,
		float InTimeToStartMontageAt = 0.F
	);	

	/**
	* Returns current montage played on the upper body slot.
	* Returns nullptr if no upper body montage is played at this moment.
	*/
	UFUNCTION(BlueprintPure)
	UAnimMontage* GetUpperBodyMontage() const { return CurrUpperBodyMontage; }

	UFUNCTION(BlueprintPure)
	static const FSingleAnimSequenceInfo& GetAnimInfoForWeaponState
	(
		const FWeaponAnimInfo& InAnimInfo,
		EWeaponAnimState InAnimState
	);

private:
	UFUNCTION()
	void StartWeaponAnimations_IfNeededNow();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bUseUpperBodySlot;

	/**
	* If true, then blending out upper body started,
	* but does NOT necessarily that the next upper body montage is started out.
	*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bBlendingOutUpperBody;

	/**
	* If true, the current upper body montage is started when blending out the previous one.
	*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	bool bMontageStartedOnBlendOut;

	UFUNCTION()
	void MontageBlendingStarted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void MontageStarted(UAnimMontage* Montage);

	UPROPERTY()
	UAnimMontage* CurrUpperBodyMontage = nullptr;
};