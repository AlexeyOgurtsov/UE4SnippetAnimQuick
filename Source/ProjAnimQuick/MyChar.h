#pragma once

#include "GameFramework/Character.h"
#include "MyChar.generated.h"

UCLASS()
class AMyChar : public ACharacter
{
	GENERATED_BODY()

public:
	AMyChar();
	virtual void BeginPlay() override;
};