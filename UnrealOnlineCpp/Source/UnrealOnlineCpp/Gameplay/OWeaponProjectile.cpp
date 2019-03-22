// Copyright (c) 2019 Jasper Drescher.

#include "OWeaponProjectile.h"

// Sets default values
AOWeaponProjectile::AOWeaponProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
