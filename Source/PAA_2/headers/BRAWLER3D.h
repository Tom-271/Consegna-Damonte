#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h" // Aggiungi questo per BoxComponent
#include "GameFramework/Actor.h"
#include "Brawler3D.generated.h"

UCLASS()
class PAA_2_API ABrawler3D : public AActor
{
	GENERATED_BODY()

public:
	//costruttore
	ABrawler3D();
	
	//funzione per ruotare l'attore
	void RotateActor(float DeltaRotation);

private:
	//componente StaticMesh per il Brawler
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	//componente Box per la collisione
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComponent;
};
