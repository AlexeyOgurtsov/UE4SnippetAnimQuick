#include "MyChar.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(MyCharLog, Log, All);
DEFINE_LOG_CATEGORY(MyCharLog);

AMyChar::AMyChar()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetMesh()->SetAnimationMode(EAnimationMode::Type::AnimationBlueprint);
}

void AMyChar::BeginPlay()
{
	// Very important (otherwise default movement component will not work)
	Super::BeginPlay();
	UE_LOG(MyCharLog, Log, TEXT("AMyChar::BeginPlay"));

	// @TODO
}