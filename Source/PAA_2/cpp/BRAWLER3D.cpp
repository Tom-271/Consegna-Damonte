#include "../headers/Brawler3D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ABrawler3D::ABrawler3D()																								//classi essenziali per caricare i render 3d che faccio girare quando premo infobrawler e infosniper buttons
{
	//abilita il Tick per l'actor
	PrimaryActorTick.bCanEverTick = true;

	//imposta il Mobility del DefaultSceneRoot su Movable
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	//crea il componente StaticMesh
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetMobility(EComponentMobility::Movable);														//imposta il Mobility su Movable
	StaticMeshComponent->SetupAttachment(RootComponent);																//collega al RootComponent

	//crea il componente di collisione
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetMobility(EComponentMobility::Movable);																//imposta il Mobility su Movable
	BoxComponent->SetupAttachment(RootComponent);																	//collega al RootComponent
	BoxComponent->InitBoxExtent(FVector(50.0f, 50.0f, 50.0f));												//imposta le dimensioni della collisione
	BoxComponent->SetCollisionProfileName(TEXT("BlockAll")); // Imposta il profilo di collisione
}


// Funzione per ruotare l'actor
void ABrawler3D::RotateActor(float DeltaRotation)
{
	if (StaticMeshComponent)
	{
		FRotator NewRotation = StaticMeshComponent->GetComponentRotation();
		NewRotation.Yaw += DeltaRotation; // Ruota lungo l'asse Z
		StaticMeshComponent->SetWorldRotation(NewRotation);
	}
}
