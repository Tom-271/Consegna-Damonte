#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIMode.generated.h"

class ABP_Obstacles;
class AMyGameMode;

//DELEGATI!!
//vittoria/Pareggio
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerVictory);                                                                   //notifica la vittoria del giocatore
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDraw);                                                                            //notifica un pareggio

//movimento completato
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAIBrawlerMovementCompleted);                                                      //notifica che il movimento del Brawler AI è completato
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAISniperMovementCompleted);                                                       //notifica che il movimento dello Sniper AI è completato

//attacco
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIBrawlerAttack, int32, Damage);                                         //notifica l'attacco del Brawler AI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAISniperAttack, int32, Damage);                                          //notifica l'attacco dello Sniper AI

//cambio salute
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIBrawlerHealthChanged, float, HealthPercentage);                        //notifica variazione salute Brawler AI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAISniperHealthChanged, float, HealthPercentage);                         //notifica la variazione di salte Sniper AI

//danno al giocatore
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBrawlerDamaged, int32, Damage);                                    //notifica danno al giocatore dal Brawler AI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSniperDamaged, int32, Damage);                                     //notifica danno al giocatore dallo Sniper AI

UCLASS()
class PAA_2_API AAIMode : public AActor
{
    GENERATED_BODY()

public:
    //costruttore
    AAIMode();

    //funzioni di Spawning
    void SpawnAIBrawler();                                                                                              //spawna un AI Brawler
    void SpawnAISniper();                                                                                               //spawna un AI Sniper
    bool bHasSpawnedAIBrawler;                                                                                          //booleano: Brawler AI è stato spawnato?
    bool bHasSpawnedAISniper;                                                                                           //booleano: Sniper AI è stato spawnato?

    DECLARE_MULTICAST_DELEGATE(FOnAITurnFinished);
    FOnAITurnFinished OnAITurnFinished;
    
    //funzioni di Movimento
    void MoveAICharacters();                                                                                            //muove tutti i personaggi AI
    void MoveAISniper();                                                                                                //muove lo Sniper AI
    void MoveRandomly(AActor* AICharacter, TFunction<void()> OnMovementCompleted);                                      //muove l'AI in modo casuale
    
    //funzione di Targeting per gli attacchi
    AActor* ChooseTarget(AActor* AICharacter);                                                                          //seleziona il bersaglio per l'AI

    //funzioni UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    ULateralPanelWidget* GetLateralPanelWidget() const;                                                                 //restituisce il riferimento al widget laterale

    //gestione Salute e Danni
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnAIBrawlerHealthChanged OnAIBrawlerHealthChanged;                                                                 //collegat delegato per variazione salute Brawler AI
    FOnAISniperHealthChanged OnAISniperHealthChanged;                                                                   //delegato per variazione salute Sniper AI
    float AIBrawlerHealth;                                                                                              //salute corrente del Brawler AI
    float AISniperHealth;                                                                                               //salute corrente dello Sniper AI

    void ApplyDamageToAIBrawler(int32 Damage);                                                                          //applica danno al Brawler AI
    void ApplyDamageToAISniper(int32 Damage);                                                                           //applica danno allo Sniper AI

    //funzioni di Attacco
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnAIBrawlerAttack OnAIBrawlerAttack;                                                                               //o per attacco del Brawler AI
    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnAISniperAttack OnAISniperAttack;                                                                                 //delegato per attacco dello Sniper AI

    UFUNCTION()
    void AttackIfPossible(AActor* AICharacter);                                                                         //effettua l'attacco se possibile
    
    //funzioni di Contrattacco ai
    void CounterAttack(AActor* Attacker, AActor* Defender);                                                             //esegue il contrattacco
    UFUNCTION()
    void HandleAICounterAttack(AActor* Attacker, AActor* Defender);                                                     //gestisce la logica del contrattacco

    //notifica Danni al Giocatore
    UPROPERTY(BlueprintAssignable, Category = "Damage")
    FOnPlayerBrawlerDamaged OnPlayerBrawlerDamaged;                                                                     //delegato per danno al giocatore (Brawler)
    UPROPERTY(BlueprintAssignable, Category = "Damage")
    FOnPlayerSniperDamaged OnPlayerSniperDamaged;   // Delegate per danno al giocatore (Sniper)

    //condizioni di Vittoria / Pareggio
    void CheckVictoryCondition();                                                                                       //controlla le condizioni di vittoria
    UPROPERTY(BlueprintAssignable, Category = "Victory")
    FOnPlayerVictory OnPlayerVictory;                                                                                   //delegate per la vittoria del giocatore
    UPROPERTY(BlueprintAssignable, Category = "Victory")
    FOnDraw OnDraw;                                                                                                     //delegate per il pareggio

    //gestione Celle e Tile per l'AI
    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsCellOccupiedByOTHERS(int32 X, int32 Y, AActor* CallingActor);                                                //verifica se una cella è occupata dall'AI
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TSubclassOf<AActor> ReachableTileBlueprint;                                                                         //blueprint per i tile raggiungibili
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float ReachableTileHeightOffset = 0.5f;                                                                             //offset di altezza per i tile
    
    //funzione principale per il calcolo del percorso
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")                                                              //mi serve per la hardmode del gioco
    TArray<FVector> CalculatePath(const FVector& StartLocation, const FVector& TargetLocation, int32 DesiredSteps);
    
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetEasyMode(bool bEasy);                                                                                       //funzione per definire la differenza tra easy mode e hard mode

    //booleano per la divisione tra easy mode e nom
    UFUNCTION(BlueprintCallable, Category = "AI")                                                                       //restituisce true se è attiva la modalità Easy, altrimenti false 
    bool IsEasyMode() const;

    AActor* CurrentAIMovingActor = nullptr;

private:
    bool IsCellFree(int32 X, int32 Y);                                                                                  //verifica se una cella è libera

    //funzioni di Movimento dell'AI 
    void StartAIMovement(AActor* AICharacter, TArray<FVector>& Path, TFunction<void()> OnMovementCompleted);            //avvia il movimento dell'AI
    void MoveAICharacterStep(AActor* AICharacter,
                             const TArray<FVector>& Path,
                             int32 CurrentStep,
                             const FVector& OriginalLocation,
                             TFunction<void()> OnMovementCompleted);                                                    //muove l'AI passo per passo

    //visualizzazione del Percorso dell'AI
    void ShowAIPath(const TArray<FVector>& Path);                                                                       //mostra il percorso generale dell'AI
    void ClearAIPath();                                                                                                 //pulisce il percorso visualizzato
    TArray<TPair<int32,int32>> AIHighlightedCells;                                                                      //celle evidenziate

    //blueprint e Selezione delle Unità AI
    UPROPERTY(EditDefaultsOnly, Category = "Blueprints")
    TSubclassOf<AActor> AIBrawlerBlueprintClass;                                                                        //blueprint per il Brawler AI
    UPROPERTY(EditDefaultsOnly, Category = "Blueprints")
    TSubclassOf<AActor> AISniperBlueprintClass;                                                                         //blueprint per lo Sniper AI
    UPROPERTY()
    AActor* SelectedBrawler;                                                                                            //riferimento al Brawler AI selezionato
    UPROPERTY()
    AActor* SelectedSniper;                                                                                             //riferimento allo Sniper AI selezionato
    
    //contatori di Movimento dell'AI
    int32 CompletedMovements;                                                                                           //contatore dei movimenti completati
    int32 TotalAIUnits;                                                                                                 //numero totale di unità AI da muovere

    //funzioni di Utilità e Gestione Stato
    AActor* FindNearestPlayerUnit(const FVector& AILocation);                                                           //trova l'unità giocatore più vicina all'AI
    
    // Variabili per le texture
    UPROPERTY()
    UTexture2D* Soldier1RedTexture;
    
    UPROPERTY()
    UTexture2D* Soldier2RedTexture;                                                                                     //texture

    UPROPERTY()
    UMaterialInterface* AIBrawlerMaterial;                                                                              //materiale per la pedina del brawler
    
    UPROPERTY()
    UMaterialInterface* AISniperMaterial;                                                                               //materiale per la pedina dello sniper

    UPROPERTY(VisibleAnywhere, Category = "AI Mode")
    bool bEasyMode = true;                                                                                              //booleanno per notificare alla partita corrente quale modalità sia stata scelta dal player
};