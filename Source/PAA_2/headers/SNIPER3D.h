#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h" // Aggiungi questo per BoxComponent
#include "SNIPER3D.generated.h"


//dichiarazione della classe ASNIPER3D
UCLASS()
class PAA_2_API ASNIPER3D : public AActor
{
	GENERATED_BODY()

public:
	ASNIPER3D();

	//funzione per la rotazione dell'actor
	void RotateActor(float DeltaRotation);

	//componente per la mesh statica
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	//componente per la collisione (Box)
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComponent;
};
