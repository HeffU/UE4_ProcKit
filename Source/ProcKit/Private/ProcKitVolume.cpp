// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcKitVolume.h"
#include "IProcKitPlugin.h"
#include "Engine/World.h"
#include "Engine/Selection.h"
#include "LevelEditorActions.h"

void AProcKitVolume::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	//UE_LOG(ProcKitLog, Warning, TEXT("OnConstruction NYI"));
	for (auto& transform : Transforms)
	{
		//UE_LOG(ProcKitLog, Warning, TEXT("transform"));
	}
}

void AProcKitVolume::BeginPlay()
{
	// can probably get away without checking these as the delegate wont fire unless it's PIE?
	//if (GEngine->GetNetMode(GetWorld()) == NM_Standalone
	//if (GetWorld()->WorldType == EWorldType::PIE)

	//TODO: do we ever remove this delegate?
	EndPIEdelegate = FEditorDelegates::PrePIEEnded.AddUObject(this, &AProcKitVolume::OnPIEStopping);

	FVector location;
	FRotator rotation = FRotator(0,0,0);
	AActor* actor;
	FActorSpawnParameters SpawnInfo;

	switch (SpawnPolicy)
	{
	case EProcSpawnTimingPolicy::PSTP_PhysicsBakedToStatic:
		if (bSpawnOnNextBeginPlay)
		{
			bSpawnOnNextBeginPlay = false;
			bShouldKeepSimulatedResults = true;

			NumMovingActors = 0;

			for (AActor* actor : SpawnedActors)
			{
				actor->Destroy();
			}
			SpawnedActors.Empty();

			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

			for (int n = 0; n < 3; n++)
			{
				location = FMath::RandPointInBox(this->GetBounds().GetBox());
				// bounds are global ofc, no need to append actor location, 
				// but possibly should remove it if we want movable stuff?
				//location += this->GetActorLocation();
				//Transforms.Add(FTransform(location));

				actor = GetWorld()->SpawnActor(SpawnClass, &location, &rotation, SpawnInfo);
				if (IsValid(actor))
				{
					SpawnedActors.Add(actor);
					UPrimitiveComponent* root = static_cast<UPrimitiveComponent*>(actor->GetRootComponent());
					if (root)
					{
						//TODO: currently the bGenerateWakeEvents is set on the template asset in this dev project
						// some sources state that setting it from code wont work, look into how that is handled
						// could be its only working when it generates an propertyeditevent
						// in any case, ensure these events are on here rather than leaving that burden on the user
						// that also lets them be disabled for packaged games without user intervention etc
						root->OnComponentWake.AddDynamic(this, &AProcKitVolume::OnSpawnedActorPhysicsAwake);
						root->OnComponentSleep.AddDynamic(this, &AProcKitVolume::OnSpawnedActorPhysicsSleep);
					}
					else
					{
						UE_LOG(ProcKitLog, Warning, TEXT("Warning, could not find physics-enabled root component to listen to."));
					}
				}
			}
		}

		break;
	default:
		break;
	}
}

void AProcKitVolume::SpawnBakedActors(TSubclassOf<AActor> ActorClass, UWorld* World)
{
	AActor* actor;
	FActorSpawnParameters SpawnInfo;

	switch (SpawnPolicy)
	{
	case EProcSpawnTimingPolicy::PSTP_PhysicsBakedToStatic:
		// todo: check if old actors should be deleted etc
		for (AActor* actor : SpawnedActors)
		{
			actor->Destroy();
		}
		SpawnedActors.Empty();

		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

		for (FTransform& transform : Transforms)
		{
			actor = World->SpawnActor(ActorClass, &transform, SpawnInfo);
			if (IsValid(actor))
			{
				SpawnedActors.Add(actor);
			}
		}
		break;
	default:
		break;
	}


}

#if WITH_EDITOR
void AProcKitVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProcKitVolume, bMarkActorSpawnDirty))
	{
		//UE_LOG(ProcKitLog, Warning, TEXT("property changed"));
		SpawnBakedActors(SpawnClass, GetWorld());
		// TODO: check if we are ingame or something?
		
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AProcKitVolume, bSpawnOnNextBeginPlay))
	{
		UE_LOG(ProcKitLog, Warning, TEXT("spawnOnNextBeginPlay toggled"));
	}
}

void AProcKitVolume::OnPIEStopping(bool bIsSimulating)
{
	//UE_LOG(ProcKitLog, Warning, TEXT("PIE closing"));
	FEditorDelegates::PrePIEEnded.Remove(EndPIEdelegate);

	BakeSpawnedStates();
}

void AProcKitVolume::BakeSpawnedStates()
{
	if (bShouldKeepSimulatedResults)
	{
		bShouldKeepSimulatedResults = false;

		// TODO selecting this before and after the check is dirty, refactor this
		GEditor->SelectActor(this, true, true, true, false);
		if (FLevelEditorActionCallbacks::CanExecuteKeepSimulationChanges())
		{
			UE_LOG(ProcKitLog, Warning, TEXT("Saving states of dynamic procgen"));
			for (AActor* actor : SpawnedActors)
			{
				Transforms.Add(actor->GetTransform());
				// because we add transforms to this actor and it is selected due to us editing it, we do not need this
				// however, when in the future this is automated, all actors scheduled for saving properties will have to be selected
				// also possibly deselect all before this?
				//GEditor->SelectActor(actor, true, true, true, false);

				UPrimitiveComponent* root = static_cast<UPrimitiveComponent*>(actor->GetRootComponent());
				if (root)
				{
					//TODO: is this the ok way to do it that wont risk messing with users customizations?
					root->OnComponentWake.RemoveAll(this);
					root->OnComponentSleep.RemoveAll(this);
				}
			}

			// Save currently selected things
			//TArray<UObject*> SelectedObjects;
			//GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);

			// Todo: figure out how to do this without crashing, see below
			//TArray<UObject*> SelectedComponents;
			//GEditor->GetSelectedComponents()->GetSelectedObjects(SelectedComponents);

			TArray<UObject*> SelectedActors;
			GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);

			// clear selections
			GEditor->SelectNone(true, true, false);
			GEditor->GetSelectedActors()->DeselectAll();
			GEditor->GetSelectedObjects()->DeselectAll();
			GEditor->GetSelectedComponents()->DeselectAll();

			// Select this actor that we're keeping simulated properties on
			GEditor->SelectActor(this, true, true, true, false);

			// Save the properties to the editor versions of this actor
			FLevelEditorActionCallbacks::OnKeepSimulationChanges();

			// restore previous selection
			GEditor->SelectNone(true, true, false);
			GEditor->GetSelectedActors()->DeselectAll();
			GEditor->GetSelectedObjects()->DeselectAll();
			GEditor->GetSelectedComponents()->DeselectAll();

			//TODO what are these, and how can we select objects again?
			/*
			GEditor->GetSelectedObjects()->BeginBatchSelectOperation();
			for (UObject* obj : SelectedObjects)
			{
			}
			GEditor->GetSelectedObjects()->EndBatchSelectOperation();
			*/

			//TODO seems to crash if component was selected, look into this
			// possibly dont notify of these selection changes will help?
			// could well break something else though
			/*
			GEditor->GetSelectedComponents()->BeginBatchSelectOperation();
			for (UObject* comp : SelectedComponents)
			{
				UActorComponent* component = static_cast<UActorComponent*>(comp);
				if (component)
				{
					GEditor->SelectComponent(component, true, true);
				}
			}
			GEditor->GetSelectedComponents()->EndBatchSelectOperation();
			*/

			GEditor->GetSelectedActors()->BeginBatchSelectOperation();
			for (UObject* act : SelectedActors)
			{
				AActor* actor = static_cast<AActor*>(act);
				if (actor)
				{
					GEditor->SelectActor(actor, true, true);
				}
			}
			GEditor->GetSelectedActors()->EndBatchSelectOperation();

			// TODO hack, see above todo about refactor
			GEditor->SelectActor(this, false, true, true, false);

			AProcKitVolume *editorActor = static_cast<AProcKitVolume*>(EditorUtilities::GetEditorWorldCounterpartActor(this));
			if (editorActor)
			{
				editorActor->SpawnBakedActors(SpawnClass, GEditor->EditorWorld);
			}
			else
			{
				UE_LOG(ProcKitLog, Warning, TEXT("Could not find editor version of procVolume"));
			}

			// TODO: possibly add delay here / call this on timer after everything is confirmed to be done?
			// TODO: this process is already happening when this is called via the delegate for PIEEnding, is that ok?
			GEditor->RequestEndPlayMap();
		}
		else
		{
			UE_LOG(ProcKitLog, Warning, TEXT("Could not execute KeepSimulationChanges; Were no objects selected?"));
		}
	}
}


void AProcKitVolume::OnSpawnedActorPhysicsAwake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	NumMovingActors++;
	//UE_LOG(ProcKitLog, Warning, TEXT("Spawned Actors awake count: %i"), NumMovingActors);
}

void AProcKitVolume::OnSpawnedActorPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	NumMovingActors--;
	//UE_LOG(ProcKitLog, Warning, TEXT("Spawned Actors awake count: %i"), NumMovingActors);
	switch (PhysicsEventPolicy)
	{
	case EProcPhysicsSleepPolicy::PPSP_NoChange:
		break;
	case EProcPhysicsSleepPolicy::PPSP_StaticOnSleep:
		SleepingComponent->OnComponentWake.RemoveAll(this);
		SleepingComponent->OnComponentSleep.RemoveAll(this);
		SleepingComponent->SetSimulatePhysics(false);
		break;
	case EProcPhysicsSleepPolicy::PPSP_StaticOnAllSleep:
		if (NumMovingActors == 0)
		{
			for (AActor* actor : SpawnedActors)
			{
				UPrimitiveComponent* root = static_cast<UPrimitiveComponent*>(actor->GetRootComponent());
				if (root)
				{
					//TODO: is this the ok way to do it that wont risk messing with users customizations?
					root->OnComponentWake.RemoveAll(this);
					root->OnComponentSleep.RemoveAll(this);
					root->SetSimulatePhysics(false);
				}
			}
		}
		break;
	default:
		break;
	}

	if (bQuitWhenPhysicsIsFinished && NumMovingActors == 0)
	{
		//TODO: this is a hack, refactor so that we have a central place to call
		UE_LOG(ProcKitLog, Warning, TEXT("physics all sleeping, quitting sim"));
		FEditorDelegates::PrePIEEnded.Remove(EndPIEdelegate);

		BakeSpawnedStates();
	}
}

#endif