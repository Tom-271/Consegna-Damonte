#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GridManagerCPP.generated.h"

class UMaterialInterface;

UCLASS()
class PAA_2_API AGridManagerCPP : public AActor                                                                         
{
    GENERATED_BODY()

public:
    AGridManagerCPP();
    virtual void BeginPlay() override;                                                                                  //beginplay una delle prime ad essere considerate quando si accede alla classe

    //dettagli per la griglia
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSize;                                                                                                     //per generazione della griglia serve la dimensione 25x25

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float TileSize;                                                                                                     //dimensione fisica della singola cella

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<AActor> TileBlueprintClass;                                                                             //collegamento a blueprint

    //materiali
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    UMaterialInterface* DefaultTileMaterial;                                                                            //questo viene utilizzato per evidenziare le celle raggiungibili da pedina
                                                                                                                        //è il colore di default
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    UMaterialInterface* HighlightTileMaterial;                                                                          //è il colore effettivo per evidenziare, gestiti fisicamente in blueprint 

    //operazioni sulla griglia
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GenerateGrid();                                                                                                //genera la griglia di base

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void HighlightCell(int32 X, int32 Y, bool bHighlight);                                                              //evidenzia la griglia usando i materiali
                                                                                                                        
private:
    TArray<TArray<AActor*>> GridTiles;                                                                                  //array per memorizzare le celle
};
