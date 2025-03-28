#include "../headers/MyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../headers/MainMenuWidget.h"
#include "../headers/BP_Obstacles.h"
#include "../headers/LateralPanelWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "../headers/MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Queue.h"
#include "../headers/AIMode.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"

AMyGameMode::AMyGameMode()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetBPClass(TEXT("/Game/Blueprints/MainMenuWidget"));//ricerca widget menu principale
    if(MainMenuWidgetBPClass.Succeeded())                                                                               //se trovato
    {
        MainMenuWidgetClass = MainMenuWidgetBPClass.Class;                                                              //assegna classe widget
    }
    else UE_LOG(LogTemp,Error,TEXT("MainMenuWidget non trovato! Controlla il percorso."));                      //errore caricamento widget

    static ConstructorHelpers::FClassFinder<UUserWidget> LateralPanelWidgetBPClass(TEXT("/Game/Blueprints/LateralPanelWidget")); //ricerca widget pannello laterale
    if(LateralPanelWidgetBPClass.Succeeded())                                                                           //se trovato
    {
        LateralPanelWidgetClass = LateralPanelWidgetBPClass.Class;                                                      //assegna classe widget
    }
    else UE_LOG(LogTemp,Error,TEXT("LateralPanelWidget NON TROVATO! Controlla il percorso esatto."));           //errore caricamento widget

    static ConstructorHelpers::FClassFinder<AActor> BrawlerBlueprintFinder(TEXT("/Game/Blueprints/BRAWLER"));           //ricerca blueprint brawler
    if(BrawlerBlueprintFinder.Succeeded())                                                                              //se trovato
    {
        BrawlerBlueprintClass = BrawlerBlueprintFinder.Class;                                                           //assegna classe blueprint
    }
    else UE_LOG(LogTemp,Error,TEXT("BrawlerBlueprintClass NON TROVATO! Controlla il percorso."));               //errore caricamento blueprint

    static ConstructorHelpers::FClassFinder<AActor> SniperBlueprintFind(TEXT("/Game/Blueprints/AISniper"));             //ricerca blueprint sniper AI
    if(SniperBlueprintFind.Succeeded())                                                                                 //se trovato
    {
        SniperBlueprintClass = SniperBlueprintFind.Class;                                                               //assegna classe blueprint
        UE_LOG(LogTemp,Warning,TEXT("SniperBlueprintClass caricata correttamente: %s"),*SniperBlueprintClass->GetPathName()); //log percorso blueprint
    }
    else UE_LOG(LogTemp,Error,TEXT("SniperBlueprintClass NON TROVATA! Controlla il percorso."));                //errore caricamento blueprint

    static ConstructorHelpers::FClassFinder<AActor> SniperBlueprintFinder(TEXT("/Game/Blueprints/SNIPER"));             //ricerca blueprint sniper giocatore
    if(SniperBlueprintFinder.Succeeded())                                                                               //se trovato
    {
        SniperBlueprintClass = SniperBlueprintFinder.Class;                                                             //assegna classe blueprint
        UE_LOG(LogTemp,Warning,TEXT("SniperBlueprintClass caricato correttamente: %s"),*SniperBlueprintClass->GetPathName()); //log percorso blueprint
    }
    else UE_LOG(LogTemp,Error,TEXT("SniperBlueprintClass NON TROVATO! Controlla il percorso."));                //errore caricamento blueprint

    static ConstructorHelpers::FClassFinder<AActor> ReachableTileFinder(TEXT("/Game/Blueprints/BP_ReachableTile"));     //ricerca blueprint tile raggiungibile
    if(ReachableTileFinder.Succeeded())                                                                                 //se trovato
    {
        ReachableTileClass = ReachableTileFinder.Class;                                                                 //assegna classe blueprint
    }
    else UE_LOG(LogTemp,Error,TEXT("BP_ReachableTile NON TROVATO! Controlla il percorso."));                    //errore caricamento blueprint

    bAreReachableTilesVisible = false;                                                                                  //inizializza flag visibilità tile
    TargetLocation = FVector::ZeroVector;                                                                               //inizializza posizione target
    bIsMoving = false;                                                                                                  //inizializza flag movimento
    MovementSpeed = 200.0f;                                                                                             //imposta velocità movimento

    PrimaryActorTick.bCanEverTick = true;                                                                               //abilita tick

    SelectedBrawler = nullptr;                                                                                          //reset selezione brawler
    SelectedSniper = nullptr;                                                                                           //reset selezione sniper
    bIsPlacingBrawler = false;                                                                                          //reset posizionamento brawler
    bIsPlacingSniper = false;                                                                                           //reset posizionamento sniper
    bIsMovingBrawler = false;                                                                                           //reset movimento brawler
    bIsMovingSniper = false;                                                                                            //reset movimento sniper
    bPlayerTurn = true;                                                                                                 //inizializza turno giocatore
    bIsFirstTurn = true;                                                                                                //flag primo turno
    bIsAIBrawlerSpawned = false;                                                                                        //flag AI brawler non spawnato
    bIsAISniperSpawned = false;                                                                                         //flag AI sniper non spawnato

    bIsFirstAITurn = true;                                                                                              //flag primo turno AI
    bIsSecondAITurn = false;                                                                                            //flag secondo turno AI
    AITurnCounter = 0;                                                                                                  //reset contatore turni AI

    PlayerControllerClass = AMyPlayerController::StaticClass();                                                         //imposta classe controller giocatore
}

void AMyGameMode::BeginPlay()
{
    OnTurnChanged.AddUObject(this, &AMyGameMode::HandleTurnChanged);                                                    //collega evento cambio turno

    bHasBrawlerMoved = false;                                                                                           //reset stato movimento brawler
    bHasSniperMoved = false;                                                                                            //reset stato movimento sniper
    Super::BeginPlay();                                                                                                 //chiama implementazione base
    
    if (!AIModeInstance)                                                                                                //se AIMode non istanziato
    {
        AIModeInstance = GetWorld()->SpawnActor<AAIMode>(AAIMode::StaticClass());                                       //spawn AIMode
        if (AIModeInstance)                                                                                             //se spawn riuscito
        {
            UE_LOG(LogTemp, Warning, TEXT("AAIMode spawnato dinamicamente!"));                                  //log spawn riuscito
        }
        else UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare AAIMode!"));                                     //log errore spawn
    }
    
    AIModeInstance = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(this, AAIMode::StaticClass()));//trova AIMode nel mondo
    if (!AIModeInstance) UE_LOG(LogTemp, Error, TEXT("AIMode non trovato nel mondo!"));                         //log mancata istanza
}

void AMyGameMode::StartPlay()
{
    Super::StartPlay();                                                                       

    ToggleMenu(true);                                                                                                   //mostra menu principale
}

void AMyGameMode::ToggleMenu(bool bShowMenu)                                                                            //gestisco subito, infatti viene chiamato nel beginplay, è la prima cosa che voglio vedere
{
    if (MainMenuWidgetClass)                                                                                            //se classe widget valida
    {
        if (bShowMenu)                                                                                                  
        {
            if (!MainMenuWidget)                                                                                        //evita di creare più istanze
            {
                MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);             //crea widget menu
                if (MainMenuWidget)                                                                                     //se creazione riuscita
                {
                    MainMenuWidget->SetGameMode(this);                                                                  //assegna game mode al widget
                    MainMenuWidget->AddToViewport();                                                                    //aggiunge widget alla viewport
                }
            }
        }
        else 
        {
            if (MainMenuWidget && MainMenuWidget->IsInViewport())                                                       //se widget visibile lo nascondiamo
            {
                MainMenuWidget->RemoveFromParent();                                                                     //rimuove widget dalla viewport
                MainMenuWidget = nullptr;                                                                               //reset puntatore widget
                UE_LOG(LogTemp, Warning, TEXT("Widget rimosso dalla viewport!")); 
            }
        }
    }
}

void AMyGameMode::StartGame()
{
    bHasSpawnedAIBrawler = false;
    bHasSpawnedAISniper = false;                                                                                        //resetta flags per lo spawn delle pedine ai

    ToggleMenu(false);
    ShowLateralPanel();

    AITurnCounter = 0;                                                                                                  //resetta contatore dei turni dell'ai, 
                                                                                                                        //vedero la gestione dei turni come una coda che inizia in base al lancio della moneta ed alterna in perpetuo
    if (bPlayerStarts)                                                                                                  //dal lancio della moneta deduco chi dei due inizia
    {
        //se inizia il giocatore: P -> AI -> P -> AI -> ...
        TurnQueue.Enqueue(true);                                                                                   //è il turno del giocatore
        TurnQueue.Enqueue(false);                                                                                  //NON è il urno dell'AI
    }
    else
    {
        //se inizia l'AI: AI -> P -> AI -> P -> ...
        TurnQueue.Enqueue(false);                                                                                  //NON è il urno dell'AI
        TurnQueue.Enqueue(true);                                                                                   //è il urno del giocatore
    }
    StartNextTurn();                                                                                                    //avvia il primo turno
}

void AMyGameMode::StartNextTurn()
{
    bool bIsNowPlayerTurn;
    if (TurnQueue.Dequeue(bIsNowPlayerTurn))                                                                         //tolgo dalla coda creata il turno attuale
    {
        bIsPlayerTurn = bIsNowPlayerTurn;
        OnTurnChanged.Broadcast(bIsPlayerTurn);                                                                         //uso il delegato per la gestione delle altre funzionalità, msg broadcast

        if (bIsPlayerTurn)
        {
            StartPlayerTurn(true);                                                                                      //se il turno corrente è del player
        }
        else
        {
            StartAITurn();                                                                                              //altrimenti ai
        }

        TurnQueue.Enqueue(!bIsPlayerTurn);                                                                              //adesso riaggiungo alla coda il turno, una sorta di evento perpetuo
    }
}

void AMyGameMode::StartPlayerTurn(bool boolIsPlayerTurn)
{
    bHasBrawlerMoved = false;
    bHasSniperMoved = false;
    bHasBrawlerAttacked = false;
    bHasSniperAttacked = false;                                                                                         //riporto a false tutte le azioni ceh possono fare le mie pedine, devono azzerarsi ad ogni turno
    bHasBrawlerMoved = false;
    bHasSniperMoved = false;
    bHasBrawlerAttacked = false;
    bHasSniperAttacked = false;
    PlayerMoves = 0;                                                                                                    //resettiamo le mosse del player a 0
    PlayerSpawnedPieces = 0;
    
    this->bIsPlayerTurn = boolIsPlayerTurn;
    LastPlayerBrawlerTargetCell = FVector2D(0, 0);
    LastPlayerSniperTargetCell = FVector2D(0, 0);
    
    OnTurnChanged.Broadcast(bIsPlayerTurn);                                                                             //delegato per notificare il cambio di turno
    
    if (LateralPanelWidget)                                                                                             //aggiorna la visibilità dei bordi nel LateralPanelWidget
    {
        LateralPanelWidget->UpdateBordersVisibility(bIsPlayerTurn);                                                     //fa vedere PlayerBorder o AIborder, vedi nella classe corrispondente
    }
}

void AMyGameMode::EndPlayerTurn()
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    
    if (PlayerController)
    {
        PlayerController->ResetTurnFlags();                                                                             //finisce il turno del player resetto tutto per sicurezza
    }

    if (LateralPanelWidget)
    {
        LateralPanelWidget->UpdateBordersVisibility(false);                                                  //stessa cosa dello start, ma nasconde PlayerBorder e mostra AIBorder
    }
    
    FTimerHandle TimerHandle;                                                                                           //gestiamo una piccola transizione tra stati per non far partire tutto subito
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyGameMode::StartAITurn, 0.75f, false);
}

void AMyGameMode::StartAITurn()
{
    this->bIsPlayerTurn = false;                                                                                        //imposta stato turno AI
    OnTurnChanged.Broadcast(false);                                                                                     //broadcast cambio turno a AI
                                                                                                                        //il counter inizializzato prima ci serve per capire quando l'ai stia spawnando o giocando, per regolamento i primi due turni sono di spawn
    if (AITurnCounter == 0 && !bHasSpawnedAIBrawler)                                                                    //primo turno AI: spawn brawler
    {
        AIModeInstance->SpawnAIBrawler();                                                                               //chiamo la funzione per spawn AI brawler
        bHasSpawnedAIBrawler = true;                                                                                    //flag spawn brawler si
        EndAITurn();                                                                                                    //termina immediatamente turno AI
    }
    else if (AITurnCounter == 1 && !bHasSpawnedAISniper)                                                                //secondo turno AI: spawn sniper, è la second avolta che entro qui
    {
        AIModeInstance->SpawnAISniper();                                                                                //spawn AI sniper
        bHasSpawnedAISniper = true;                                                                                     //flag spawn sniper
        EndAITurn();                                                                                                    //termina immediatamente turno AI
    }
    else                                                                                                                //turni successivi AI: gioca davvero
    {
        AIModeInstance->OnAITurnFinished.Clear();                                                                       //pulisci delegate precedente
        AIModeInstance->OnAITurnFinished.AddUObject(this, &AMyGameMode::EndAITurn);                                     //collega callback fine turno
        MoveAICharacters();                                                                                             //avvia movimento AI
    }

    AITurnCounter++;                                                                                                    //incrementa contatore turno AI
}

void AMyGameMode::EndAITurn()                                                                                           //gestiamone la fine
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);                   //ottiene player controller
    if (PC)
    {
        if (APawn* Pawn = PC->GetPawn())                                                                                //ottiene pawn controllato
        {
            Pawn->EnableInput(PC);                                                                                      //riabilita input: ho fatto in modo che ad ogni turno dell'ai io perda fisicamente la possibilità di cliccare, mi scompare persino il cursore
        }
        PC->SetInputMode(FInputModeGameAndUI());                                                                  //imposta modalità input game+UI
        PC->bShowMouseCursor = true;                                                                                    //mostra cursore
        PC->bEnableClickEvents = true;                                                                                  //abilita click events
        PC->bEnableMouseOverEvents = true;                                                                              //abilita mouse over events
    }
    if (LateralPanelWidget)                                                                                             //se pannello laterale esiste
    {
        LateralPanelWidget->UpdateBordersVisibility(true);                                                   //mostra bordi pannello
    }
    StartPlayerTurn(true);                                                                                              //avvia turno giocatore
}

void AMyGameMode::SetIsPlacingBrawler(bool bIsPlacing)                                                                  //siamo nella gestione del piazzamento delle pedine
{
    if(PlayerSpawnedPieces < 1)                                                                                         //se non è stata piazzata nessuna pedina questo turno
    {
        bIsPlacingBrawler = bIsPlacing;                                                                                 //imposta flag piazzamento brawler
        UE_LOG(LogTemp, Warning, TEXT("bIsPlacingBrawler impostato su: %s"), bIsPlacing ? TEXT("True") : TEXT("False"));
    }
    else UE_LOG(LogTemp, Warning, TEXT("Impossibile piazzare il Brawler: pedina già piazzata in questo turno."));
}

void AMyGameMode::SetIsPlacingSniper(bool bIsPlacing)
{
    if(PlayerSpawnedPieces < 1) 
    {
        bIsPlacingSniper = bIsPlacing; 
        UE_LOG(LogTemp, Warning, TEXT("bIsPlacingSniper impostato su: %s"), bIsPlacing ? TEXT("True") : TEXT("False")); 
    }
    else UE_LOG(LogTemp, Warning, TEXT("Impossibile piazzare lo Sniper: pedina già piazzata in questo turno."));
}

void AMyGameMode::SetPlayerStarts(bool bStarts)
{
    bPlayerStarts = bStarts;                                                                                            //imposta flag turno iniziale
    UE_LOG(LogTemp, Warning, TEXT("Player starts: %s"), bStarts ? TEXT("True") : TEXT("False")); 

    if(!bStarts) 
    {
        bIsPlacingBrawler = false; 
        bIsPlacingSniper = false; 
        UE_LOG(LogTemp, Warning, TEXT("AI inizia, disabilitato spawn per il giocatore.")); 
    }
}

void AMyGameMode::HandleTurnChanged(bool bNewIsPlayerTurn)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);                   //ottiene playercontroller
    if (!PC)                                                                                                            //se controller non trovato
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));                                          //log di errore
        return;                                                                                                         //esce dalla funzione
    }
    if (bNewIsPlayerTurn)                                                                                               //se è il turno del giocatore
    {
        UE_LOG(LogTemp, Warning, TEXT("Turno del giocatore: abilito input e cursore."));                        //log abilitazione input
        PC->SetInputMode(FInputModeGameAndUI());                                                                  //imposta modalità input game+UI
        PC->bShowMouseCursor = true;                                                                                    //mostra cursore
        PC->bEnableClickEvents = true;                                                                                  //abilita eventi click
        PC->bEnableMouseOverEvents = true;                                                                              //abilita eventi mouse over
    }
    else                                                                                                                //contrario
    {
        PC->SetInputMode(FInputModeUIOnly()); 
        PC->bShowMouseCursor = false; 
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false; 
        if (APawn* Pawn = PC->GetPawn()) 
        {
            Pawn->DisableInput(PC); 
        }
    }
}

void AMyGameMode::ShowLateralPanel()
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());      //ottiene player controller
    if(PlayerController)                                                                                                //se controller valido
    {
        LateralPanelWidget = PlayerController->GetLateralPanelWidget();                                                 //ottiene widget pannello laterale
        
        if(LateralPanelWidget)                                                                                          //se widget valido
        {
            LateralPanelWidget->AddToViewport();                                                                        //aggiunge widget alla viewport
            OnTurnChanged.AddUObject(this, &AMyGameMode::HandleTurnChanged);                                            //collega evento cambio turno
            LateralPanelWidget->UpdateBordersVisibility(bPlayerTurn);                                                   //aggiorna visibilità bordi in base al turno
        }
        else UE_LOG(LogTemp, Error, TEXT("Impossibile ottenere il LateralPanelWidget!"));                       //log errore widget
    }
}

void AMyGameMode::OnCellSelected(int32 X, int32 Y)                                                                      //gestisco direttamente qui la pressione di una cella
{
    UE_LOG(LogTemp, Warning, TEXT("OnCellSelected chiamato con X=%d, Y=%d"), X, Y);                             //log che mi dice le coordinate cella selezionata

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if(!Obstacles)                                                                                                      //se ostacoli non trovati
    {
        return;                                                                                                         //esce dalla funzione
    }

    FVector CellPosition = Obstacles->GetCellWorldPosition(X, Y);                                                       //calcola posizione world della cella
    CellPosition.Z = 50.0f;                                                                                             //imposta altezza spawn
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);     //ottiene player controller

    if(bIsPlacingBrawler && !bHasSpawnedBrawler)                                                                        //se si sta piazzando il brawler e non è ancora spawnato
    {
        PlayerSpawnedPieces++;                                                                                          //incrementa contatore pezzi spawnati

        if(BrawlerBlueprintClass)                                                                                       //se blueprint valido
        {
            AActor* Brawler = GetWorld()->SpawnActor<AActor>(BrawlerBlueprintClass, CellPosition, FRotator::ZeroRotator);
            if(Brawler)                                                                                                 //se spawn riuscito
            {
                SelectedBrawler = Brawler;                                                                              //salva riferimento brawler
                bHasSpawnedBrawler = true;                                                                              //flag brawler spawnato
                UE_LOG(LogTemp, Warning, TEXT("Brawler spawnato in posizione: %s"), *CellPosition.ToString());  //log successo
                EndPlayerTurn();                                                                                        //termina turno giocatore
            }
            else UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare il Brawler!"));                              //log errore spawn
        }
        else UE_LOG(LogTemp, Error, TEXT("BrawlerBlueprintClass non valida!"));                                 //log blueprint non valido

        bIsPlacingBrawler = false;                                                                                      //reset flag piazzamento
    }
    else if(bIsPlacingSniper && !bHasSpawnedSniper)                                                                     //se si sta piazzando lo sniper e non è ancora spawnato
    {
        PlayerSpawnedPieces++;
        if(SniperBlueprintClass)                                                                                        //se blueprint valido
        {
            AActor* Sniper = GetWorld()->SpawnActor<AActor>(SniperBlueprintClass, CellPosition, FRotator::ZeroRotator); 
            if(Sniper)                                                                                                  //se spawn riuscito
            {
                SelectedSniper = Sniper;                                                                                //salva riferimento sniper
                bHasSpawnedSniper = true;                                                                               //flag sniper spawnato
                UE_LOG(LogTemp, Warning, TEXT("Sniper spawnato in posizione: %s"), *CellPosition.ToString());   
                EndPlayerTurn();                                                                                        //termina turno giocatore
            }
            else UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare lo Sniper!"));                               //log errore spawn
        }
        else UE_LOG(LogTemp, Error, TEXT("SniperBlueprintClass non valida!")); 

        bIsPlacingSniper = false; 
    }
}

void AMyGameMode::MoveAICharacters()                                                                                    //per gestire bene la progressione dei turni ho qui un riferimento al movimento delle pedine ai
{
    if (AIModeInstance)
    {
        AIModeInstance->MoveAICharacters();                                                                             //vedi riferimento nella calsse corrispondente
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIModeInstance non è valido!"));
    }
}

void AMyGameMode::SetPlayerTargetCells(int32 BrawlerX, int32 BrawlerY, int32 SniperX, int32 SniperY)                    //per movimento delle pedine e per salvarmi effettivamente le celle che le mie pedine hanno visitato
{                                                                                                                       
    LastPlayerBrawlerTargetCell = FVector2D(BrawlerX, BrawlerY);                                                        //imposta ultima cella target del brawler
    LastPlayerSniperTargetCell = FVector2D(SniperX, SniperY);                                                           //imposta ultima cella target dello sniper
    FString BrawlerCellLabel = FString::Printf(TEXT("%c%d"), 'A' + BrawlerX, BrawlerY + 1);                         //crea etichetta cella brawler
    FString SniperCellLabel = FString::Printf(TEXT("%c%d"), 'A' + SniperX, SniperY + 1);                            //crea etichetta cella sniper
    UE_LOG(LogTemp, Warning, TEXT("Celle di destinazione del giocatore impostate: Brawler %s, Sniper %s"), *BrawlerCellLabel, *SniperCellLabel); 
    FString BrawlerMessage = FString::Printf(TEXT("Il brawler si è mosso in %s"), *BrawlerCellLabel);               
}