// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "ProcKitVolume.generated.h"

UENUM(BlueprintType)
enum class EProcSpawnTimingPolicy : uint8
{
	PSTP_PhysicsBakedToStatic			UMETA(DisplayName = "Spawn with physics, bake results"),
};

UENUM(BlueprintType)
enum class EProcPhysicsSleepPolicy : uint8
{
	PPSP_NoChange			UMETA(DisplayName = "No change"),
	PPSP_StaticOnSleep		UMETA(DisplayName = "Make static on sleep"),
	PPSP_StaticOnAllSleep	UMETA(DisplayName = "Make static when none are awake"),
};

/**
 * 
 */
UCLASS()
class PROCKIT_API AProcKitVolume : public AVolume
{
	GENERATED_BODY()

	virtual void OnConstruction(const FTransform &Transform) override;
	virtual void BeginPlay() override;

	void SpawnBakedActors(TSubclassOf<AActor> ActorClass, UWorld* World);
	void BakeSpawnedStates();

	UFUNCTION()
	void OnPIEStopping(bool bIsSimulating);

	FDelegateHandle EndPIEdelegate;

	UPROPERTY(EditAnywhere)
	bool bQuitWhenPhysicsIsFinished;

	UPROPERTY(EditAnywhere)
	TArray<FTransform> Transforms;

	UPROPERTY(EditAnywhere)
	bool bMarkActorSpawnDirty;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> SpawnClass;

	UPROPERTY(EditAnywhere)
	EProcSpawnTimingPolicy SpawnPolicy;

	UPROPERTY(EditAnywhere)
	EProcPhysicsSleepPolicy PhysicsEventPolicy;

	UPROPERTY(EditAnywhere)
	bool bSpawnOnNextBeginPlay;

	UPROPERTY(EditAnywhere)
	bool bLimitInstancesToVolumeBounds;

	UPROPERTY(EditAnywhere)
	bool bContinuousSpawnMonitoring = true;

	UPROPERTY(EditAnywhere)
	bool bAutoQuitWhenAtRest = true;

	bool bShouldKeepSimulatedResults = false;
	TArray<AActor*> SpawnedActors;

	int NumMovingActors;


#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	UFUNCTION()
	void OnSpawnedActorPhysicsAwake(UPrimitiveComponent* WakingComponent, FName BoneName);
	//FDelegateHandle OnSpawnedActorPhysicsAwakeDelegate;
	
	UFUNCTION()
	void OnSpawnedActorPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);
	//FDelegateHandle OnSpawnedActorPhysicsSleepDelegate;
#endif
};
