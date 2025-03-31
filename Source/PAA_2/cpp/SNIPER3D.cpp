#include "../headers/SNIPER3D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ASNIPER3D::ASNIPER3D()																									//classi essenziali da passare come parent class alle bluepritn class per far girare il render quando muovo le pedine
{
	//abilita il Tick per l'actor
	PrimaryActorTick.bCanEverTick = true;

	//imposta il Mobility del DefaultSceneRoot su Movable
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	//crea il componente StaticMesh
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetMobility(EComponentMobility::Movable);                                                      //imposta il Mobility su Movable
	StaticMeshComponent->SetupAttachment(RootComponent);                                                             //collega al RootComponent

	//crea il componente di collisione
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetMobility(EComponentMobility::Movable);																//imposta il Mobility su Movable
	BoxComponent->SetupAttachment(RootComponent);																    //collega al RootComponent
	BoxComponent->InitBoxExtent(FVector(50.0f, 50.0f, 50.0f));											    //imposta le dimensioni della collisione
	BoxComponent->SetCollisionProfileName(TEXT("BlockAll"));															//imposta il profilo di collisione

	StaticMeshComponent->SetRenderCustomDepth(true);
	StaticMeshComponent->CustomDepthStencilValue = 1;																	//assegna un valore per la priorità di profondità
	
}



//funzione per ruotare l'actor
void ASNIPER3D::RotateActor(float DeltaRotation)
{
	if (StaticMeshComponent)
	{
		FRotator NewRotation = StaticMeshComponent->GetComponentRotation();
		NewRotation.Yaw += DeltaRotation;																				//ruota lungo l'asse Z
		StaticMeshComponent->SetWorldRotation(NewRotation);
	}
} 

