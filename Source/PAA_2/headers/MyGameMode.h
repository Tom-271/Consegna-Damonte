#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AIMode.h"
#include "MyGameMode.generated.h"

class URichTextBlock;
class ULateralPanelWidget;

class UMainMenuWidget;                                                                                                  //dichiarazione forward per il menu principale
class ULateralPanelWidget;                                                                                              //dichiarazione forward per il widget laterale
class ABP_Obstacles;                                                                                                    //dichiarazione forward per gli ostacoli
class AMyPlayerController;                                                                                              //dichiarazione forward per il controller del giocatore

UCLASS()
class PAA_2_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMyGameMode(); 

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> ReachableTileClass;                                                                             //classe dei tile raggiungibili

    UPROPERTY()
    ULateralPanelWidget* LateralPanelWidgetInstance;                                                                    //istanza del widget laterale

protected:
    //inizializzazione del gioco
    virtual void BeginPlay() override;                                                                                  //chiamato all'inizio del gioco o quando l'attore viene spawnato
    virtual void StartPlay() override;                                                                                  //chiamato quando il gioco inizia

    UPROPERTY()
    AAIMode* AIModeInstance;                                                                                            //istanza della modalità AI

public:
    //funzioni di controllo e menu
    void StartGame();                                                                                                   //avvia il gioco
    void ToggleMenu(bool bShowMenu);                                                                                    //mostra o nasconde il menu
    void ShowLateralPanel();                                                                                            //mostra il pannello laterale
    void OnCellSelected(int32 X, int32 Y);                                                                              //gestisce la selezione di una cella
    void SetIsPlacingBrawler(bool bIsPlacing);                                                                          //imposta il posizionamento del brawler
    void SetIsPlacingSniper(bool bIsPlacing);                                                                           //imposta il posizionamento dello sniper
    void SetPlayerStarts(bool bStarts);                                                                                 //imposta chi inizia (giocatore)

    //gestione dei turni
    void StartPlayerTurn(bool bIsPlayerTurn);                                                                           //inizia il turno del giocatore
    void EndPlayerTurn();                                                                                               //termina il turno del giocatore
    void StartAITurn();                                                                                                 //inizia il turno dell'AI
    void EndAITurn();                                                                                                   //termina il turno dell'AI

    bool bIsPlayerTurn;                                                                                                 //flag che indica se è il turno del giocatore, lo usiamo ad esempio nel lateralpanelwidget per mostrare i border del turno corrispondente

    //blueprint delle unità AI
    UPROPERTY(EditDefaultsOnly, Category = "Blueprints")
    TSubclassOf<AActor> AIBrawlerBlueprintClass;                                                                        //blueprint per il brawler AI

    UPROPERTY(EditDefaultsOnly, Category = "Blueprints")
    TSubclassOf<AActor> AISniperBlueprintClass;                                                                         //blueprint per lo sniper AI

    //delegato e variabili del turno
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, bool);                                                          //delegato per il cambio turno
    FOnTurnChanged OnTurnChanged;                                                                                       //istanza del delegato per il cambio turno

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    int32 PlayerMoves;                                                                                                  //numero di mosse del giocatore

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasSpawnedBrawler;                                                                                            //flag per indicare se il brawler è stato spawnato

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasSpawnedSniper;                                                                                             //flag per indicare se lo sniper è stato spawnato

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bIsMovingBrawler;                                                                                              //flag per il movimento del brawler

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bIsMovingSniper;                                                                                               //flag per il movimento dello sniper

    bool IsPlayerTurn() const { return bPlayerTurn; }                                                                   //ritorna true se è il turno del giocatore

    //gestione delle celle e posizionamento
    FVector2D LastPlayerBrawlerTargetCell;                                                                              //ultima cella target del brawler del giocatore
    FVector2D LastPlayerSniperTargetCell;                                                                               //ultima cella target dello sniper del giocatore
    void SetPlayerTargetCells(int32 BrawlerX, int32 BrawlerY, int32 SniperX, int32 SniperY);                            //imposta le celle target del giocatore

    //stato dei turni e log
    bool bIsFirstTurn = true;                                                                                           //flag per il primo turno del giocatore
    bool bIsSecondTurn = false;                                                                                         //flag per il secondo turno del giocatore
    bool bIsFirstAITurn = true;                                                                                         //flag per il primo turno dell'AI
    bool bIsSecondAITurn = false;                                                                                       //flag per il secondo turno dell'AI
                                            
    UPROPERTY()
    URichTextBlock* GameLogText;                                                                                        //riferimento al log di gioco

    void MoveAICharacters();                                                                                            //muove i personaggi AI
    void StartNextTurn();                                                                                               //avvia il turno successivo

    bool bPlayerStarts;                                                                                                 //flag per indicare se il giocatore inizia
    bool GetPlayerStarts() const { return bPlayerStarts; }                                                              //ritorna true se il giocatore inizia

    int32 AITurnCounter;                                                                                                //contatore dei turni dell'AI
    int32 PlayerTurnCounter;                                                                                            //contatore dei turni del giocatore
    int32 PlayerSpawnedPieces;                                                                                          //numero di pedine spawnate dal giocatore

    UPROPERTY()
    ULateralPanelWidget* LateralPanelWidget;                                                                            //riferimento al widget laterale
    
    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasBrawlerMoved;                                                                                              //flag per indicare se il brawler ha mosso

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasSniperMoved;                                                                                               //flag per indicare se lo sniper ha mosso

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasBrawlerAttacked;                                                                                           //flag per indicare se il brawler ha attaccato

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    bool bHasSniperAttacked;                                                                                            //flag per indicare se lo sniper ha attaccato

    UFUNCTION()
    void HandleTurnChanged(bool bNewIsPlayerTurn);

private:
    //variabili e classi dei widget
    UPROPERTY() UMainMenuWidget* MainMenuWidget;                                                                        //istanza del widget del menu principale
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;                      //classe del widget del menu principale
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<ULateralPanelWidget> LateralPanelWidgetClass;              //classe del widget laterale
    
    //blueprint delle unità gioco
    UPROPERTY(EditDefaultsOnly, Category = "Blueprints") TSubclassOf<AActor> BrawlerBlueprintClass;                     //blueprint per il brawler del giocatore
    UPROPERTY(EditDefaultsOnly, Category = "Blueprints") TSubclassOf<AActor> SniperBlueprintClass;                      //blueprint per lo sniper del giocatore

    //flags di posizionamento e visibilità
    UPROPERTY() bool bIsPlacingBrawler;                                                                                 //flag per il posizionamento del brawler
    UPROPERTY() bool bIsPlacingSniper;                                                                                  //flag per il posizionamento dello sniper
    UPROPERTY() bool bAreReachableTilesVisible;                                                                         //flag per la visibilità dei tile raggiungibili
    UPROPERTY() bool bPlayerTurn;                                                                                       //flag per il turno del giocatore (privato)

    // selezione e movimento delle unità
    UPROPERTY() AActor* SelectedBrawler;                                                                                //riferimento al brawler selezionato
    UPROPERTY() AActor* SelectedSniper;                                                                                 //riferimento allo sniper selezionato
    UPROPERTY() FVector TargetLocation;                                                                                 //posizione di destinazione
    UPROPERTY() bool bIsMoving;                                                                                         //flag per indicare se un'unità è in movimento
    UPROPERTY() float MovementSpeed;                                                                                    //velocità di movimento

    // gestione dei turni (privato)
    TQueue<bool> TurnQueue;                                                                                             //coda di turni (true = turno giocatore, false = turno AI)

    // flags di spawn per le unità AI
    bool bHasSpawnedAIBrawler;                                                                                          //flag per tracciare il brawler AI spawnato
    bool bHasSpawnedAISniper;                                                                                           //flag per tracciare lo sniper AI spawnato

    bool bIsAIBrawlerSpawned = false;                                                                                   //flag per indicare se il brawler AI è stato spawnato
    bool bIsAISniperSpawned = false;                                                                                    //flag per indicare se lo sniper AI è stato spawnato
};
