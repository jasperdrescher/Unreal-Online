// Copyright (c) 2019 Jasper Drescher.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OWeaponProjectile.generated.h"

UCLASS()
class UNREALONLINECPP_API AOWeaponProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOWeaponProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
