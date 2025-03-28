#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BP_Obstacles.generated.h"

USTRUCT()
struct FCellInfo                                                                                                        //struttura
{
    GENERATED_BODY()

    bool bIsObstacle = false;
    TWeakObjectPtr<AActor> TileActor;                                                                                   //uso un puntatore debole anziché un raw pointer come in origine, perche? TWeakObjectPtr per evitare di accedere a memoria già liberata: se un Tile viene distrutto da qualche altra parte
                                                                                                                        //il puntatore debole non risulterà valido e potremo saltare il blocco di distruzione evitando l'access violation.
};                                                                                                                      //tutto questo perchè prima usavo un puntatore alla tile raw che faceva crashare in fase di distruzione. vedi dopo...

UCLASS()
class PAA_2_API ABP_Obstacles : public AActor
{
    GENERATED_BODY()

public:
    ABP_Obstacles();                                                                                                    //costruttore                                  
    
    virtual void BeginPlay() override;

    //info griglia
    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 GridSize;                                                                                                     //mumero di celle per lato del quadrato

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    float GridTile;                                                                                                     //dimensione fisica di ogni cella

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    TSubclassOf<AActor> TileBlueprintClass;                                                                             //blueprint per spawnare un tile

    UPROPERTY(EditAnywhere, Category = "Grid")
    float SpawnProbability;                                                                                             //probabilità di spawn ostacolo
    
    void GenerateGrid();                                                                                                //crea tutta la griglia degli ostacoli
    void DestroyGrid();                                                                                                 //rimuove tutti i tile, mi serve per essere sicuro di poter applicare la percentuale tramite lo slider
    FCellInfo GetCellInfo(int32 X, int32 Y) const;                                                                      //restituisce info cella
    bool FindRandomEmptyCell(int32& OutX, int32& OutY);                                                                 //ottiene cella libera casuale
    void SetSpawnProbability(float NewProbability);                                                                     //aggiorna probabilità spawn ostacoli

    
    bool GetCellCoordinatesFromWorldPosition(const FVector& WorldPosition, int32& OutX, int32& OutY) const;
    FVector GetCellWorldPosition(int32 X, int32 Y) const;                                                               //prende coordinate                                  

    //path
    TArray<TPair<int32, int32>> FindReachableCells(int32 StartX, int32 StartY, int32 Steps);        
    TArray<FVector> CalculatePath(FVector StartLocation, FVector DestinationLocation);                                  //due funzioni essenziali per lo spawn, le commento meglio nel cpp
    
    void CreateObstacleMap(TArray<TArray<bool>>& ObstacleMap);
    bool AreAllCellsReachable(const TArray<TArray<bool>>& ObstacleMap);
    void BFS(const TArray<TArray<bool>>& ObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY);       //idem

protected:
    TArray<TArray<FCellInfo>> GridCells;                                                                                //array per la memorizzazione delle celle

    // Flag per evitare chiamate concorrenti a GenerateGrid/DestroyGrid
    bool bIsGridUpdating = false;
private:
    bool bIsGeneratingGrid;
    
};
