#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LateralPanelWidget.h"
#include "GameFramework/HUD.h"
#include "AIMode.h"
#include "MyPlayerController.generated.h"

class UMyClass;
class ABP_Obstacles;
class AMyGameMode;

DECLARE_DELEGATE_TwoParams(FAddGameMessageDelegate, const FString&, const FLinearColor&);

UCLASS()
class PAA_2_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMyPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInput();

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<ULateralPanelWidget> LateralPanelWidgetClass;

public:
    virtual void Tick(float DeltaTime) override;
    
    //funzioni di input 
    void HandleMouseClick();                                                                                            //gestione click sinistro

    //funzioni di posizionamento 
    void StartBrawlerPlacement();                                                                                       //inizia posizionamento brawler
    void StartSniperPlacement();                                                                                        //inizia posizionamento sniper

    //funzioni di movimento
    void MoveBrawlerToCell(int32 X, int32 Y);                                                                           //muove il brawler nella cella indicata
    void MoveSniperToCell(int32 X, int32 Y);                                                                            //muove lo sniper nella cella indicata
    void ClearReachableTiles();                                                                                         //rimuove le celle raggiungibili evidenziate
    void EndPlayerTurn();                                                                                               //termina il turno del giocatore

    //funzioni di pathfinding
    TArray<TPair<int32, int32>> FindReachableCells(ABP_Obstacles* Obstacles, int32 StartX, int32 StartY, int32 Steps, AActor* MovingActor); //trova celle raggiungibili
    TArray<FVector> CalculatePath(ABP_Obstacles* Obstacles, FVector StartLocation, FVector DestinationLocation);        //calcola il percorso

    //funzioni di attacco
    bool HandleBrawlerAttack(AActor* Target);                                                                           //gestisce attacco del brawler
    bool HandleSniperAttack(AActor* Target);                                                                            //gestisce attacco dello sniper

    //funzioni relative al widget
    ULateralPanelWidget* GetLateralPanelWidget();                                                                       //restituisce l'istanza del widget
    void UpdateHealthBar(float NewHealth);                                                                              //aggiorna la progress bar della salute

    //istanza del widget creato in runtime
    UPROPERTY()
    ULateralPanelWidget* LateralPanelWidget;

    //istanza del widget da blueprint
    UPROPERTY(BlueprintReadOnly, Category = "UI")
    ULateralPanelWidget* LateralPanelWidgetInstance;

    //funzioni di stato e controlli turni
    void CheckEndTurn();                                                                                                //controlla se il turno può terminare

    //variabili che indicano se le pedine hanno mosso
    bool bHasBrawlerMoved;
    bool bHasSniperMoved;

    //gestione messaggi di gioco
    UFUNCTION()
    void HandleAddGameMessage(const FString& Message, const FLinearColor& Color);                                       //gestisce l'aggiunta di messaggi

    //getter per le pedine selezionate
    AActor* GetSelectedBrawler() const { return SelectedBrawler; }                                                      //restituisce il brawler selezionato
    AActor* GetSelectedSniper() const { return SelectedSniper; }                                                        //restituisce lo sniper selezionato

    //funzioni relative alla salute del giocatore
    void CheckPlayerBrawlerHealth();                                                                                    //controlla la salute del brawler
    void CheckPlayerSniperHealth();                                                                                     //controlla la salute dello sniper
    void RemovePlayerBrawler();                                                                                         //rimuove il brawler (es. in caso di morte)
    void RemovePlayerSniper();                                                                                          //rimuove lo sniper (es. in caso di morte)
    void ResetTurnFlags();                                                                                              //resetta i flag di azione per il turno

    //stato di vita delle pedine del giocatore
    bool bBrawlerAlive;                                                                                                 //true se il brawler è vivo
    bool bSniperAlive;                                                                                                  //true se lo sniper è vivo

    // elementi UI 
    UPROPERTY()
    class UBorder* AIVICTORY;                                                                                           //border per la vittoria dell'AI

    //delegato per la vittoria dell'AI
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnAIVictory OnAIVictory;

    //funzioni blueprint per verificare lo stato di vita delle pedine
    UFUNCTION(BlueprintCallable, Category = "Player")
    bool IsBrawlerAlive() const { return bBrawlerAlive; }                                                               //restituisce true se il brawler è vivo
    UFUNCTION(BlueprintCallable, Category = "Player")
    bool IsSniperAlive() const { return bSniperAlive; }                                                                 //restituisce true se lo sniper è vivo

    // variabili di salute massima
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxBrawlerHealth;                                                                                             //salute massima del brawler
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxSniperHealth;                                                                                              //salute massima dello sniper

    //funzioni per gestione della griglia e contrattacchi
    UFUNCTION(BlueprintCallable, Category = "Grid")
    AGridManagerCPP* GetGridManager() const;                                                                            //restituisce il grid manager dalla scena
    void HandlePlayerCounterAttack(AActor* Attacker, AActor* Defender);                                                 //gestisce il contrattacco del giocatore

    //getter per lo stato di attacco
    bool GetBrawlerAttacked() const { return bHasBrawlerAttacked; }                                                     //restituisce true se il brawler ha attaccato
    bool GetSniperAttacked() const { return bHasSniperAttacked; }                                                       //restituisce true se lo sniper ha attaccato
    void OnCellSelected(int32 X, int32 Y);                                                                              //gestisce la selezione di una cella
    
    UPROPERTY()
    UMaterialInterface* HPBrawlerMaterial;
    UPROPERTY()
    UMaterialInterface* HPSniperMaterial;
    bool IsCellBlocked(int32 X, int32 Y, AActor* IgnoreActor) const;                                                    //booleano per aiutarmi a gestire quali celle siano occupate da altre pedine sia HP che AI
private:
    //funzione helper per ottenere la gamemode
    AMyGameMode* GetGameMode() const;                                                                                   //restituisce la gamemode

    //variabili bool
    bool bHasBrawlerAttacked;                                                                                           //true se il brawler ha già attaccato
    bool bHasSniperAttacked;                                                                                            //true se lo sniper ha già attaccato
    bool bIsPlacingBrawler;                                                                                             //true se in fase di posizionamento del brawler
    bool bIsPlacingSniper;                                                                                              //true se in fase di posizionamento dello sniper
    bool bIsMovingBrawler;                                                                                              //true se il brawler è in movimento
    bool bIsMovingSniper;                                                                                               //true se lo sniper è in movimento

    AActor* SelectedBrawler;                                                                                            //riferimento al brawler selezionato. lo usiamo nella funzione nel public
    AActor* SelectedSniper;                                                                                             //riferimento allo sniper selezionato

    TArray<AActor*> ReachableTiles;                                                                                     //array delle celle raggiungibili
    bool bAreReachableTilesVisible;                                                                                     //true se le celle raggiungibili sono evidenziate
    FVector TargetLocation;                                                                                             //destinazione del movimento
    bool bIsMoving;                                                                                                     //true se un movimento è in corso
    float MovementSpeed;                                                                                                //velocità di movimento

    TArray<FVector> MovementPath;                                                                                       //percorso di movimento calcolato

    bool bHasBrawlerCompletedActions = false;                                                                           //true se il brawler ha completato tutte le azioni
    bool bHasSniperCompletedActions = false;                                                                            //true se lo sniper ha completato tutte le azioni
    bool bHasBrawlerCompletedAttack = false;                                                                            //true se il brawler ha completato l'attacco
    bool bHasSniperCompletedAttack = false;                                                                             //true se lo sniper ha completato l'attacco
    
    UPROPERTY()
    UTexture2D* Soldier1BlueTexture;                                                                                    //texture per verificare se il materiale sia stato correttamente applicato alle pedine
    UPROPERTY()
    UTexture2D* Soldier2BlueTexture;
    bool bHasAppliedBrawlerMaterial = false;                                                                            //bool per verificare se il materiale sia stato correttamente applicato alle pedine
    bool bHasAppliedSniperMaterial = false;
    void HandleSniperSelection(AActor* Sniper, ABP_Obstacles* Obstacles);
    void HandleBrawlerSelection(AActor* Sniper, ABP_Obstacles* Obstacles);
};