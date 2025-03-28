#include "../headers/MyPlayerController.h"
#include "../headers/MyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../headers/BP_Obstacles.h"
#include "../headers/GridManagerCPP.h"
#include "GameFramework/DamageType.h"
#include "Blueprint/UserWidget.h"
#include <queue>
#include <vector>
#include "Engine/World.h"
#include "../headers/LateralPanelWidget.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"

AMyPlayerController::AMyPlayerController()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> LateralPanelWidgetBPClass(TEXT("/Game/Blueprints/LateralPanelWidget"));  //cerca il Blueprint del widget laterale nella cartella specificata
    if (LateralPanelWidgetBPClass.Succeeded())
    {
        LateralPanelWidgetClass = LateralPanelWidgetBPClass.Class;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile trovare il Blueprint del LateralPanelWidget."));
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Blueprints/HPBRAWLERMATERIAL.HPBRAWLERMATERIAL'"));     //caricamento diretto del materiale HPBrawler
    if (MaterialFinder.Succeeded())
    {
        HPBrawlerMaterial = MaterialFinder.Object;                                                                   //se ce l'ho me lo notifico tramite log
        UE_LOG(LogTemp, Warning, TEXT("Materiale HPBrawler caricato con successo!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED TO LOAD HPBRAWLERMATERIAL! Controlla esattamente il Copy Reference dell'asset."));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMatFinder(TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
        HPBrawlerMaterial = DefaultMatFinder.Object;
    }                                                                                                                   //altrimenti no e gli passo materiale di default
    static ConstructorHelpers::FObjectFinder<UTexture2D> Soldier1BlueFinder(TEXT("Texture2D'/Game/Blueprints/Soldier1_Blue.Soldier1_Blue'"));
    if (Soldier1BlueFinder.Succeeded())                                                                                 //texture
    {
        Soldier1BlueTexture = Soldier1BlueFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("Texture Soldier1_Blue caricata correttamente"));
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SniperMatFinder(TEXT("Material'/Game/Blueprints/HPSNIPERMATERIAL.HPSNIPERMATERIAL'"));
    if (SniperMatFinder.Succeeded())
    {
        HPSniperMaterial = SniperMatFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("Materiale HPSniper caricato con successo!"));                            
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED TO LOAD HPSNIPERMATERIAL! Verifica percorso."));
        HPSniperMaterial = HPBrawlerMaterial;                                                                           
    }
    bIsPlacingBrawler = false;                                                                                          //placement pedine a false
    bIsPlacingSniper = false;
    bIsMovingBrawler = false;                                                                                           //movemente a false
    bIsMovingSniper = false;
    bAreReachableTilesVisible = false;                                                                                  //tiles a false
    bIsMoving = false;
    MovementSpeed = 400.0f;                                                                                             //movimento delle mie pedine
    bBrawlerAlive = true;                                                                                               //sì ad inizio partita sono entrambe vive
    bSniperAlive = true;                                                                                                
    SelectedSniper = nullptr;
    SelectedBrawler = nullptr;                                                                                          //puntatori null
    bHasBrawlerAttacked = false;
    bHasSniperAttacked = false;                                                                                         //impostate tutte le variabili
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetupInput();
    LateralPanelWidgetInstance = GetLateralPanelWidget();

    AMyGameMode* GameMode = GetGameMode();
    if (GameMode && !GameMode->IsPlayerTurn())                                                                          //verifica il turno e mi disabilita i comandi se non è il mio
    {
        SetInputMode(FInputModeUIOnly());
        bShowMouseCursor = false;
        bEnableClickEvents = false;
        bEnableMouseOverEvents = false;
    }
}

void AMyPlayerController::Tick(float DeltaTime)                                                                         //la funzione tick, così come in ogni classe, viene chiamata ad ogni frame e la usiamo per controllare periodicamente alcune cose, come lo stato delle pedine
{
    Super::Tick(DeltaTime);
    CheckPlayerBrawlerHealth();                                                                                         //controlliamo la salute delle pedine
    CheckPlayerSniperHealth();

    if (!bHasAppliedBrawlerMaterial)
    {
        TArray<AActor*> Brawlers;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Brawler"), Brawlers);                           //brawler prendo materiale
        if (Brawlers.Num() > 0)
        {
            for (AActor* Brawler : Brawlers)
            {
                if (UStaticMeshComponent* MeshComp = Brawler->FindComponentByClass<UStaticMeshComponent>())
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(HPBrawlerMaterial, this);
                    if (DynMat && Soldier1BlueTexture)
                        DynMat->SetTextureParameterValue(FName("BaseTexture"), Soldier1BlueTexture);                    //e carico la texture
                    MeshComp->SetMaterial(0, DynMat);
                    UE_LOG(LogTemp, Warning, TEXT("Materiale HPBrawler assegnato a %s"), *Brawler->GetName());
                }
            }
            bHasAppliedBrawlerMaterial = true;                                                                          //bool yes
        }
    }
    if (!bHasAppliedSniperMaterial)                                                                                     //idem sniper
    {
        TArray<AActor*> Snipers;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Sniper"), Snipers);
        if (Snipers.Num() > 0)
        {
            for (AActor* Sniper : Snipers)
            {
                if (UStaticMeshComponent* Mesh = Sniper->FindComponentByClass<UStaticMeshComponent>())                  //applico la mesh alla pedina con tag sniper
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(HPSniperMaterial, this);
                    if (DynMat && Soldier1BlueTexture)                                                                  //e la texture
                    {
                        DynMat->SetTextureParameterValue(FName("BaseTexture"), Soldier1BlueTexture);
                        Mesh->SetMaterial(0, DynMat);
                        UE_LOG(LogTemp, Warning, TEXT("Materiale HPSniper assegnato a %s"), *Sniper->GetName());//me lo notifico da solo
                    }
                }
            }
            bHasAppliedSniperMaterial = true;                                                                           //okay bool sì
        }
    }
    if (bIsMoving && MovementPath.Num() > 0)
    {
        AActor* ActorToMove = nullptr;
        if (bIsMovingBrawler)
        {
            ActorToMove = SelectedBrawler;
        }
        else if (bIsMovingSniper)
        {
            ActorToMove = SelectedSniper;
        }
        if (ActorToMove)
        {
            FVector CurrentLocation = ActorToMove->GetActorLocation();
            FVector NextLocation = MovementPath[0];                                                                     //prossima cella da raggiungere
            
            NextLocation.Z = 50.0f;                                                                                     //mantiene l'attore sopra la mappa, è un controllo aggiuntivo
            
            FVector Direction = (NextLocation - CurrentLocation).GetSafeNormal();                                       //calcola la direzione e la distanza
            float Distance = FVector::Dist(CurrentLocation, NextLocation);
            if (Distance < 5.0f)                                                                                        //se è vicino alla prossima cella, rimuovila dal percorso
            {
                MovementPath.RemoveAt(0);
                if (MovementPath.Num() == 0)                                                                            //se arriviamo all'ultima cella, ferma il movimento
                {
                    bIsMoving = false;
                    bIsMovingBrawler = false;
                    bIsMovingSniper = false;
                    ClearReachableTiles();
                    return;
                }
            }
            FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;                              //muovi l'attore in direzione della prossima cella
            NewLocation.Z = 50.0f;                                                                                      //mantiene l'altezza costante
            ActorToMove->SetActorLocation(NewLocation);
        }
    }
}

void AMyPlayerController::HandleMouseClick()
{
    AMyGameMode* game_mode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());                                      //controllo se è il turno del giocatore
    if (!game_mode || !game_mode->bIsPlayerTurn)
    {
        return;                                                                                                         //esce dalla funzione
    }

    float mouse_x, mouse_y;                                                                                             //variabili per la posizione del mouse
    if (!GetMousePosition(mouse_x, mouse_y))
    {
        UE_LOG(LogTemp, Error, TEXT("impossibile ottenere la posizione del mouse!"));
        return;                                                                                                         //esce dalla funzione
    }

    FVector world_location, world_direction;                                                                            //variabili per posizione e direzione mondiali
    if (!DeprojectScreenPositionToWorld(mouse_x, mouse_y, world_location, world_direction))
    {
        return;                                                                                                         //esce se la conversione fallisce
    }
    FHitResult hit_result;                                                                                              //variabile per il risultato del line trace
    FVector start = world_location;                                                                                     //inizio del line trace
    FVector end = world_location + (world_direction * 3000.0f);                                                         //fine del line trace
    if (!GetWorld()->LineTraceSingleByChannel(hit_result, start, end, ECC_Visibility))
    {
        return;                                                                                                         //esce dalla funzione
    }
    ABP_Obstacles* obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottengo il riferimento agli ostacoli (griglia)
    if (!obstacles)
    {
        UE_LOG(LogTemp, Error, TEXT("obstacles non trovato!")); 
        return;                                                                                                         //esce dalla funzione
    }
    int32 x, y;                                                                                                         //variabili per le coordinate della cella
    if (!obstacles->GetCellCoordinatesFromWorldPosition(hit_result.Location, x, y))
    {
        UE_LOG(LogTemp, Error, TEXT("coordinate della cella non valide!")); 
        return;                                                                                                         //esce dalla funzione
    }

    FCellInfo cell_info = obstacles->GetCellInfo(x, y);                                                                 //recupero le informazioni della cella
    if (cell_info.bIsObstacle)
    {
        if (LateralPanelWidgetInstance)
        {
            LateralPanelWidgetInstance->AddGameMessage(TEXT("impossibile interagire con la cella: è un ostacolo..."), FLinearColor::Gray); //mostra messaggio: cella è un ostacolo
        }
        return;                                                                                                         //esce dalla funzione
    }

    if (hit_result.GetActor() == SelectedBrawler && bIsMovingBrawler && !bHasBrawlerMoved)
    {
        ClearReachableTiles();                                                                                          //pulisce le celle evidenziate
        bIsMovingBrawler = false;                                                                                       //disabilita il movimento del brawler
        SelectedBrawler = nullptr;                                                                                      //deseleziona il brawler
        return;                                                                                                         //esce dalla funzione
    }
    else if (hit_result.GetActor() == SelectedSniper && bIsMovingSniper && !bHasSniperMoved)
    {
        ClearReachableTiles();                                                                                          //pulisce le celle evidenziate
        bIsMovingSniper = false;                                                                                        //disabilita il movimento dello sniper
        SelectedSniper = nullptr;                                                                                       //deseleziona lo sniper
        return;                                                                                                         //esce dalla funzione
    }
    if (hit_result.GetActor()->ActorHasTag(FName("Brawler")))
    {
        ClearReachableTiles();                                                                                          //pulisce le celle evidenziate
        if (bHasBrawlerMoved)
            return;                                                                                                     //se il brawler ha già mosso, esce dalla funzione

        bIsMovingBrawler = true;                                                                                        //abilita il movimento del brawler
        SelectedBrawler = hit_result.GetActor();                                                                        //seleziona il brawler
        SelectedSniper = nullptr;                                                                                       //annulla ogni riferimento allo sniper
        int32 cell_x, cell_y;                                                                                           //variabili per le coordinate della cella del brawler
        if (obstacles->GetCellCoordinatesFromWorldPosition(SelectedBrawler->GetActorLocation(), cell_x, cell_y))
        {
            if (AGridManagerCPP* grid_manager = GetGridManager())
            {
                grid_manager->HighlightCell(cell_x, cell_y, true);                                              //evidenzia la cella corrente del brawler
                TArray<TPair<int32, int32>> reachable_cells = FindReachableCells(obstacles, cell_x, cell_y, 6, SelectedBrawler); //trova le celle raggiungibili (fino a 6 passi)
                for (const auto& cell : reachable_cells)
                {
                    if (!IsCellBlocked(cell.Key, cell.Value, SelectedBrawler))
                        grid_manager->HighlightCell(cell.Key, cell.Value, true);                          //evidenzia ogni cella raggiungibile non bloccata
                }
                bAreReachableTilesVisible = true;                                                                       //imposta la visibilità delle celle raggiungibili
            }
        }
    }
    else if (hit_result.GetActor()->ActorHasTag(FName("Sniper")))
    {
        ClearReachableTiles();                                                                                          //pulisce le celle evidenziate
        if (bHasSniperMoved)
            return;                                                                                                     //se lo sniper ha già mosso, esce dalla funzione

        bIsMovingSniper = true;                                                                                         //abilita il movimento dello sniper
        SelectedSniper = hit_result.GetActor();                                                                         //seleziona lo sniper
        SelectedBrawler = nullptr;                                                                                      //annulla ogni riferimento al brawler
        int32 cell_x, cell_y; 
        if (obstacles->GetCellCoordinatesFromWorldPosition(SelectedSniper->GetActorLocation(), cell_x, cell_y))
        {
            if (AGridManagerCPP* grid_manager = GetGridManager())
            {
                grid_manager->HighlightCell(cell_x, cell_y, true);                                              //evidenzia la cella corrente dello sniper
                TArray<TPair<int32, int32>> reachable_cells = FindReachableCells(obstacles, cell_x, cell_y, 3, SelectedSniper); //trova le celle raggiungibili (fino a 3 passi)
                for (const auto& cell : reachable_cells)
                {
                    if (!IsCellBlocked(cell.Key, cell.Value, SelectedSniper))
                        grid_manager->HighlightCell(cell.Key, cell.Value, true);                          //evidenzia ogni cella raggiungibile non bloccata
                }
                bAreReachableTilesVisible = true;                                                                       //imposta la visibilità delle celle raggiungibili
            }
        }
    }
    else if (bIsMovingBrawler && SelectedBrawler)
    {
        MoveBrawlerToCell(x, y);                                                                                        //muovo il brawler verso la cella cliccata
    }
    else if (bIsMovingSniper && SelectedSniper)
    {
        MoveSniperToCell(x, y);                                                                                         //muovo lo sniper verso la cella cliccata
    }
    else if (bIsPlacingBrawler)
    {
        GetGameMode()->SetIsPlacingBrawler(true);                                                                       //attiva il posizionamento del brawler
        GetGameMode()->OnCellSelected(x, y);                                                                            //segnala la cella selezionata per il posizionamento del brawler
    }
    else if (bIsPlacingSniper)
    {
        GetGameMode()->SetIsPlacingSniper(true);                                                                        //attiva il posizionamento dello sniper
        GetGameMode()->OnCellSelected(x, y);                                                                            //segnala la cella selezionata per il posizionamento dello sniper
    }

    if (hit_result.GetActor()->ActorHasTag(FName("AISniper")) || hit_result.GetActor()->ActorHasTag(FName("AIBrawler")))
    {
        int32 target_x, target_y;                                                                                       //variabili per le coordinate del bersaglio
        if (!obstacles->GetCellCoordinatesFromWorldPosition(hit_result.GetActor()->GetActorLocation(), target_x, target_y))
        {
            return;                                                                                                     //esce dalla funzione
        }

        if (SelectedBrawler != nullptr)
        {
            SelectedSniper = nullptr;                                                                                   //annullo ogni riferimento allo sniper se il brawler è attivo
            int32 brawler_x, brawler_y;                                                                                 //variabili per le coordinate del brawler
            if (obstacles->GetCellCoordinatesFromWorldPosition(SelectedBrawler->GetActorLocation(), brawler_x, brawler_y))
            {
                int32 distance_brawler = FMath::Abs(brawler_x - target_x) + FMath::Abs(brawler_y - target_y);     //calcolo la distanza del brawler dal bersaglio
                if (distance_brawler <= 1)
                {
                    HandleBrawlerAttack(hit_result.GetActor());                                                   //eseguo l'attacco del brawler se il bersaglio è entro 1 cella
                }
                else
                {
                    FString log_message = FString::Printf(TEXT("Il bersaglio è fuori portata per il brawler (distanza: %d)"), distance_brawler); //preparo il messaggio di errore per il brawler
                    if (LateralPanelWidgetInstance)
                    {
                        LateralPanelWidgetInstance->AddGameMessage(log_message, FLinearColor::Black); //mostro il messaggio sul pannello laterale
                    }
                    
                }
            }
        }
        else if (SelectedSniper != nullptr)
        {
            int32 sniper_x, sniper_y;                                                                                   //variabili per le coordinate dello sniper
            if (obstacles->GetCellCoordinatesFromWorldPosition(SelectedSniper->GetActorLocation(), sniper_x, sniper_y))
            {
                int32 distance_sniper = FMath::Abs(sniper_x - target_x) + FMath::Abs(sniper_y - target_y);        //calcolo la distanza dello sniper dal bersaglio
                if (distance_sniper <= 10)
                {
                    HandleSniperAttack(hit_result.GetActor());                                                    //eseguo l'attacco dello sniper se il bersaglio è entro 10 celle
                }
                else
                {
                    FString log_message = FString::Printf(TEXT("Il bersaglio è fuori portata per lo sniper (distanza: %d)"), distance_sniper); //preparo il messaggio di errore per lo sniper
                    if (LateralPanelWidgetInstance)
                    {
                        LateralPanelWidgetInstance->AddGameMessage(log_message, FLinearColor::Black);                   //mostro il messaggio sul pannello laterale
                    }
                }
            }
        }
    }
}

void AMyPlayerController::ClearReachableTiles()                                                                         //ripulisce visualizzazione celle
{
    AGridManagerCPP* GridManager = GetGridManager();
    if (GridManager)
    {
        for (int32 X = 0; X < GridManager->GridSize; ++X)                                                               
        {
            for (int32 Y = 0; Y < GridManager->GridSize; ++Y)
            {
                GridManager->HighlightCell(X, Y, false);                                                        //itera su tutte le celle della griglia e resetta l'evidenziazione
            }
        }
    }
    ReachableTiles.Empty();
    bIsMovingBrawler = false;

    if ((bBrawlerAlive && !bSniperAlive) || (!bBrawlerAlive && bSniperAlive))                                           //controlliamo di poter gestire l'input dello skip turn, potrebbe venir disattivato involontariamente dal click involontariamente
    {                                                                                                                   //anche in casi in cui si avrebbe la facoltà di usarlo
        ULateralPanelWidget* Widget = GetLateralPanelWidget();
        if (Widget && Widget->SkipTurnButton)
        {
            Widget->SkipTurnButton->SetIsEnabled(true);
            UE_LOG(LogTemp, Warning, TEXT("Skip turn button abilitato: solo la pedina viva ha mosso."));
        }
    }
}

void AMyPlayerController::HandleAddGameMessage(const FString& Message, const FLinearColor& Color)                       //questa funzione è un diretto riferimento a quella del lateralpanelwidget, dove è implementata
{
    if (!LateralPanelWidgetInstance)                                                                                    //se il widget non è ancora stato trovato lo cerco
    {
        GetLateralPanelWidget();
    }
    if (LateralPanelWidgetInstance)                                                                                     //se ce l'ho allora...
    {
        LateralPanelWidgetInstance->AddGameMessage(Message, Color);                                                     
    }
}

void AMyPlayerController::MoveBrawlerToCell(int32 X, int32 Y)                                                           //funzioni essenziali per il movimento di brawler e sniper con i relativi controlli sul path e la distanza
{
    if (bHasBrawlerMoved)                                                                                               //verifica se il brawler ha già mosso in questo turno
    {
        return;
    }
    if (!SelectedBrawler || !bIsMovingBrawler) return;                                                                  //verifica se il brawler è selezionato e se è in movimento

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if (!Obstacles) return;                                                                                             //ottiene l'ostacolo e verifica la sua esistenza

    int32 StartX, StartY;
    Obstacles->GetCellCoordinatesFromWorldPosition(SelectedBrawler->GetActorLocation(), StartX, StartY);          //ottiene le coordinate iniziali del brawler

    TArray<FVector> Path = CalculatePath(Obstacles, SelectedBrawler->GetActorLocation(), Obstacles->GetCellWorldPosition(X, Y)); 
    if (Path.Num() > 0)
    {
        MovementPath = Path;
        bIsMoving = true;
        bIsMovingBrawler = true;
        bHasBrawlerMoved = true;
        bHasBrawlerCompletedActions = true;                                                                             //imposta i flag di movimento e azione completata

        AMyGameMode* GameMode = GetGameMode();
        if (GameMode)
        {
            GameMode->SetPlayerTargetCells(X, Y, GameMode->LastPlayerSniperTargetCell.X, GameMode->LastPlayerSniperTargetCell.Y); 
            GameMode->PlayerMoves++;                                                                                    //incrementa il contatore di mosse
        }

        FString StartLabel = FString::Printf(TEXT("%c%d"), 'A' + StartY, StartX + 1);
        FString EndLabel = FString::Printf(TEXT("%c%d"), 'A' + Y, X + 1);
        LateralPanelWidgetInstance->AddGameMessage(FString::Printf(TEXT("HP: B %s -> %s"), *StartLabel, *EndLabel), FLinearColor::Blue); //aggiunge messaggio di movimento
    }
}

void AMyPlayerController::MoveSniperToCell(int32 X, int32 Y)
{
    if (bHasSniperMoved)                                                                                                //controlla se lo sniper ha già mosso
    {
        return;
    }
    if (!SelectedSniper || !bIsMovingSniper) return;                                                                    //verifica se lo sniper è selezionato e se è in movimento

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if (!Obstacles) return;                                                                                             //ottiene l'ostacolo e verifica la sua esistenza

    int32 StartX, StartY;
    Obstacles->GetCellCoordinatesFromWorldPosition(SelectedSniper->GetActorLocation(), StartX, StartY);           //ottiene le coordinate iniziali dello sniper

    TArray<FVector> Path = CalculatePath(Obstacles, SelectedSniper->GetActorLocation(), Obstacles->GetCellWorldPosition(X, Y)); //calcola il percorso
    if (Path.Num() > 0)
    {
        MovementPath = Path;
        bIsMoving = true;
        bIsMovingSniper = true;
        bHasSniperMoved = true;
        bHasSniperCompletedActions = true;                                                                              //imposta i flag di movimento e azione completata

        AMyGameMode* GameMode = GetGameMode();
        if (GameMode)
        {
            GameMode->SetPlayerTargetCells(GameMode->LastPlayerBrawlerTargetCell.X, GameMode->LastPlayerBrawlerTargetCell.Y, X, Y); 
            GameMode->PlayerMoves++;                                                                                    //incrementa il contatore di mosse
        }

        FString StartLabel = FString::Printf(TEXT("%c%d"), 'A' + StartY, StartX + 1);
        FString EndLabel = FString::Printf(TEXT("%c%d"), 'A' + Y, X + 1);
        LateralPanelWidgetInstance->AddGameMessage(FString::Printf(TEXT("HP: S %s -> %s"), *StartLabel, *EndLabel), FLinearColor::Blue);
    }
}

bool AMyPlayerController::IsCellBlocked(int32 X, int32 Y, AActor* IgnoreActor) const                                    //questa funzione mi è essenziale per far raggirare tra loro pedine del player ed ostacoli, senza si passerebber
{                                                                                                                       //una sopra l'altra
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())
    );
    if (!Obstacles) return true;                                                                                        //se non ci sono ostacoli, considera la cella bloccata per sicurezza

    FCellInfo Info = Obstacles->GetCellInfo(X, Y);
    if (Info.bIsObstacle) return true;                                                                                  //controlla se la cella è un ostacolo statico

    FVector CellCenter = Obstacles->GetCellWorldPosition(X, Y);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(IgnoreActor);                                                                                //ignora l'attore specificato nel trace

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        CellCenter + FVector(0,0,50),                                                                       //inizia il trace leggermente sopra la cella
        CellCenter - FVector(0,0,50),                                                                       //termina il trace leggermente sotto la cella
        ECC_Visibility,                                                                                                 //usa il canale di visibilità
        Params
    );

    if (!bHit || !Hit.GetActor()) return false;                                                                         //se non c'è collisione, la cella è libera

    FName Tag = Hit.GetActor()->Tags.Num() ? Hit.GetActor()->Tags[0] : NAME_None;
    return Tag == FName("Brawler") || Tag == FName("Sniper") || Tag == FName("AIBrawler") || Tag == FName("AISniper");  //considera bloccata solo se contiene personaggi (amici o nemici)
}

AMyGameMode* AMyPlayerController::GetGameMode() const                                                                   //otteniamo riferimento alla gamemode
{
    return Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
}

void AMyPlayerController::StartBrawlerPlacement()                                                                       
{
    AMyGameMode* GameMode = GetGameMode();
    
    if (GameMode && GameMode->IsPlayerTurn() && !GameMode->bHasSpawnedBrawler && GameMode->PlayerTurnCounter <= 2)      //se è il turno del player e se non le ho spawnate entrambe, allora dobbiamo ancora farlo
    {                                                                                                                   //spawniamo brawler
        bIsPlacingBrawler = true;
        bIsPlacingSniper = false;                                                                                       
        UE_LOG(LogTemp, Warning, TEXT("Modalità di posizionamento del Brawler attivata!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Non è il turno del giocatore, il Brawler e' già stato spawnato o il limite di turni è stato superato!"));
    }
}

void AMyPlayerController::StartSniperPlacement()                                                                        //idem
{
    AMyGameMode* GameMode = GetGameMode();
    if (GameMode && GameMode->IsPlayerTurn() && !GameMode->bHasSpawnedSniper && GameMode->PlayerTurnCounter <= 2)
    {
        bIsPlacingSniper = true;
        bIsPlacingBrawler = false;
        UE_LOG(LogTemp, Warning, TEXT("Modalità di posizionamento dello Sniper attivata!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Non è il turno del giocatore, lo Sniper è già stato spawnato o il limite di turni è stato superato!"));
    }
}

TArray<TPair<int32, int32>> AMyPlayerController::FindReachableCells(ABP_Obstacles* Obstacles, int32 StartX, int32 StartY, int32 Steps, AActor* MovingActor)
{
    TArray<TPair<int32, int32>> ReachableCells;                                                                         //array per memorizzare le celle raggiungibili (coordinate X e Y)
    TQueue<TPair<int32, int32>> Queue;                                                                                  //coda per la ricerca in ampiezza (BFS)
    
    TArray<TArray<bool>> Visited;                                                                                       //inizializza la matrice per tenere traccia delle celle già visitate (GridSize x GridSize)
    Visited.Init(TArray<bool>(), Obstacles->GridSize);
    for (int32 i = 0; i < Obstacles->GridSize; ++i)
    {
        Visited[i].Init(false, Obstacles->GridSize);                                                             //inizializza ogni riga con 'false' (nessuna cella visitata)
    }
    
    Queue.Enqueue(TPair<int32, int32>(StartX, StartY));                                                      //inserisce la cella di partenza nella coda
    Visited[StartX][StartY] = true;                                                                                     //segna la cella di partenza come visitata

    int32 Directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };                                          //definisce le 4 direzioni (sinistra, destra, su, giù)
    
    while (!Queue.IsEmpty() && Steps > 0)                                                                               //esegue la BFS livello per livello (ogni "passo")
    {
        TArray<TPair<int32, int32>> CurrentLevel;                                                                       //vettore temporaneo per le celle del livello corrente
                                                                                                                        //estrae tutte le celle attuali nella coda
        while (!Queue.IsEmpty())
        {
            TPair<int32, int32> Cell;
            Queue.Dequeue(Cell);
            CurrentLevel.Add(Cell);
        }
        for (const TPair<int32, int32>& CurrentCell : CurrentLevel)                                                     //per ogni cella del livello corrente, esplora i vicini
        {
            for (int32 j = 0; j < 4; ++j)
            {
                int32 NewX = CurrentCell.Key + Directions[j][0];                                                        //ccalcola la nuova coordinata X
                int32 NewY = CurrentCell.Value + Directions[j][1];                                                      //calcola la nuova coordinata Y
                
                                                                                                                        //verifica che le coordinate siano valide e non siano già state visitate
                if (NewX >= 0 && NewX < Obstacles->GridSize && NewY >= 0 && NewY < Obstacles->GridSize && !Visited[NewX][NewY])
                {
                    FCellInfo CellInfo = Obstacles->GetCellInfo(NewX, NewY);                                            //ottiene informazioni sulla cella
                                                                                                                        //se la cella non è un ostacolo statico e non è bloccata da un nemico lungo il percorso...
                    if (!CellInfo.bIsObstacle && !IsCellBlocked(NewX, NewY, MovingActor))
                    {
                        Visited[NewX][NewY] = true;                                                                     //segna la cella come visitata
                        Queue.Enqueue(TPair<int32, int32>(NewX, NewY));                                      //aggiunge la cella alla coda per ulteriori esplorazioni
                        ReachableCells.Add(TPair<int32, int32>(NewX, NewY));                                 //aggiunge la cella all'array dei raggiungibili
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Cella [%d, %d] bloccata (ostacolo o nemico)"), NewX, NewY);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cella [%d, %d] non valida o già visitata!"), NewX, NewY);
                }
            }
        }
        Steps--;                                                                                                        //decrementa i passi disponibili dopo aver elaborato un livello
    }
    return ReachableCells;                                                                                              //restituisce l'array delle celle raggiungibili
}


TArray<FVector> AMyPlayerController::CalculatePath(ABP_Obstacles* Obstacles, FVector StartLocation, FVector DestinationLocation)    //si usa BFS qui, infatti impostiamo una linea simile a come l'ho studiato
{
    TArray<FVector> Path;                                                                                               //array per salvare percorso calcolato
    int32 StartX, StartY, TargetX, TargetY;
    if (!Obstacles->GetCellCoordinatesFromWorldPosition(StartLocation, StartX, StartY) ||                         //ottieni coordinate start
        !Obstacles->GetCellCoordinatesFromWorldPosition(DestinationLocation, TargetX, TargetY))                   //ottieni coordinate destinazione
    {
        return Path;                                                                                                    //ritorna percorso vuoto se coordinate non valide
    }
    const int32 MaxSteps = bIsMovingBrawler ? 6 : 3;                                                                    //numero massimo di passi in base al tipo di unità da specifiche
    struct FNode { TPair<int32,int32> Cell; int32 Cost; };                                                              //nodo per a*
    struct FCompare { bool operator()(const FNode& A, const FNode& B) const { return A.Cost > B.Cost; } };              //comparatore costo minore

    auto Heuristic = [&](int32 X,int32 Y){ return FMath::Abs(X - TargetX) + FMath::Abs(Y - TargetY); };      //euristica manhattan

    std::priority_queue<FNode, std::vector<FNode>, FCompare> Open;                                                      //coda prioritaria aperta
    TMap<TPair<int32,int32>,int32> GScore;                                                                              //costo da start a cella
    TMap<TPair<int32,int32>,TPair<int32,int32>> Parent;                                                                 //mappa predecessori
    TSet<TPair<int32,int32>> Closed;                                                                                    //insieme chiuso visitato

    TPair<int32,int32> Start = {StartX,StartY};                                                                   //coordinate iniziali
    GScore.Add(Start,0);                                                                                         //costo start = 0
    Open.push({Start, Heuristic(StartX,StartY)});                                                      //aggiungi start a open

    int32 Dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};                                                           //direzioni movimento
    while(!Open.empty())                                                                                                //finché ci sono nodi da esplorare
    {
        auto Current = Open.top(); Open.pop(); 
        auto [CX,CY] = Current.Cell;                                                                              //coordinate correnti

        if(Current.Cell == TPair<int32,int32>{TargetX,TargetY}) break;                                            //se raggiunta destinazione esci
        Closed.Add(Current.Cell);                                                                                       //aggiungi a chiuso

        for(auto& D:Dirs)                                                                                      //per ogni direzione
        {
            int32 NX=CX+D[0], NY=CY+D[1];                                                                               //calcola nuova cella
            TPair<int32,int32> Next={NX,NY};
            if(NX<0||NX>=Obstacles->GridSize||NY<0||NY>=Obstacles->GridSize) continue; 
            if(Closed.Contains(Next) || IsCellBlocked(NX,NY, bIsMovingBrawler? SelectedBrawler: SelectedSniper)) continue; //salta celle occupate o già visitate

            int32 TentG = GScore[Current.Cell] + 1;                                                                     //costo tentativo
            if(TentG > MaxSteps) continue;                                                                              //supera passi massimi
            if(!GScore.Contains(Next) || TentG < GScore[Next])                                                          //se percorso migliore
            {
                GScore.Add(Next, TentG);                                                                                //aggiorna costo
                Parent.Add(Next, Current.Cell);                                                            //salva predecessore
                Open.push({Next, TentG + Heuristic(NX,NY)});                                     //aggiungi a open con costo totale
            }
        }
    }
    TPair<int32,int32> Step={TargetX,TargetY};                                                                    //inizia ricostruzione percorso
    if(!Parent.Contains(Step)) return {};                                                                            //nessun percorso trovato

    while(!(Step == Start))                                                                                             //ricostruisci fino allo start
    {
        Path.Insert(Obstacles->GetCellWorldPosition(Step.Key, Step.Value),0);                           //aggiungi passo in testa
        Step = Parent[Step];                                                                                            //vai al predecessore
    }
    return Path;                                                                                                        //ritorna percorso finale
}

void AMyPlayerController::SetupInput()
{
    InputComponent = this->InputComponent;
    if (InputComponent)
    {                                                                                                                   //configuriamo click sinistro e destro
        InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMyPlayerController::HandleMouseClick);

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent non trovato!"));                                            //se non confiugra inputcomponet
    }
}

void AMyPlayerController::EndPlayerTurn()                                                                               //lavoriamo sulla chiusura del turno di gioco del player, per quando passare la palla all'AI
{
    AMyGameMode* GameMode = GetGameMode();
    if (GameMode)
    {
        ULateralPanelWidget* LateralPanel = GetLateralPanelWidget();
        if (LateralPanel && LateralPanel->SkipTurnButton)
        {
            LateralPanel->SkipTurnButton->SetIsEnabled(false);                                                          //disabilita il pulsante ( se non sono nel mio turno non ha nemmeno senso che io possa premerlo )
        }
        GameMode->EndPlayerTurn();                                                                                      //reference alla gamemode
        ResetTurnFlags();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode non trovato!"));
    }
}

bool AMyPlayerController::HandleBrawlerAttack(AActor* Target)                                                           //gestiamo attacco pedine player, ora brawler
{
    if (!SelectedBrawler || !Target)                                                                                    //se non selezioni il brawler che deve attaccare o tantomeno la pedina ...
    {
        return false;
    }
    if (bHasBrawlerAttacked)                                                                                            //hai già attaccato questo turno? allora niente
    {
        if (LateralPanelWidgetInstance)
        {
            LateralPanelWidgetInstance->AddGameMessage("HP: Il brawler ha gia' attaccato in questo turno...", FLinearColor::Red);
        }                                                                                                               //lo comunico
        return false;
    }
    if (!Target->ActorHasTag(FName("AISniper")) && !Target->ActorHasTag(FName("AIBrawler")))                            //controlliamo chi stiamo premendo per attaccare
    {
        UE_LOG(LogTemp, Error, TEXT("Il bersaglio non è un AISniper o un AIBrawler!"));
        return false;
    }
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if (!Obstacles)
    {
        UE_LOG(LogTemp, Error, TEXT("Obstacles non trovato!"));                                                 
        return false;
    }

    int32 BrawlerX, BrawlerY;
    int32 TargetX, TargetY;                                                                                             
    if (!Obstacles->GetCellCoordinatesFromWorldPosition(SelectedBrawler->GetActorLocation(), BrawlerX, BrawlerY) ||
        !Obstacles->GetCellCoordinatesFromWorldPosition(Target->GetActorLocation(), TargetX, TargetY))
    {
        UE_LOG(LogTemp, Error, TEXT("Coordinate della cella non valide!"));                                     //proviamo ad ottenere le coordinate
        return false;
    }
    int32 Distance = FMath::Abs(BrawlerX - TargetX) + FMath::Abs(BrawlerY - TargetY);                             //distana tra brawler e bersaglio, essenziale per rispettare il concetto di attacco ravvicinato
    if (Distance > 1)                                                                                                   //se è fuori portata...                     
    {
        FString LogMessage = FString::Printf(TEXT("Il bersaglio e' fuori portata! Distanza: %d"), Distance);
        if (!LateralPanelWidgetInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("LateralPanelWidgetInstance è nullo!"));
        }
        else
        {
            LateralPanelWidgetInstance->AddGameMessage(LogMessage, FLinearColor::Black);                                  //diciamolo al giocatore
        }
        return false;
    }

    int32 Damage = FMath::RandRange(1, 6);                                                                     //il danno è un random tr 1 e 6
    if (LateralPanelWidgetInstance)
    {
        FString AIName = Target->ActorHasTag(FName("AISniper")) ? TEXT("S") : TEXT("B");
        FString BrawlerCell = FString::Printf(TEXT("%c%d"), 'A' + BrawlerY, BrawlerX + 1);  // Correzione: Y per la lettera, X per il numero
        FString TargetCell = FString::Printf(TEXT("%c%d"), 'A' + TargetY, TargetX + 1);     // Correzione: Y per la lettera, X per il numero
        FString Message = FString::Printf(TEXT("HP: B %s -> %s %d danni"), *BrawlerCell, *TargetCell, Damage); // Usa la cella del bersaglio invece del nome
        LateralPanelWidgetInstance->AddGameMessage(Message, FLinearColor::Blue);
    }

    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));
    if (AIMode)
    {
        if (Target->ActorHasTag(FName("AIBrawler")))
        {
            AIMode->ApplyDamageToAIBrawler(Damage);                                                                     //se ho attaccato il brawler nemico allora aggiornimamone la vita
            AIMode->HandleAICounterAttack(SelectedBrawler, Target);                                     //e ovviamente questa pedina se può deve potermi contrattaccare
        }
        else if (Target->ActorHasTag(FName("AISniper")))
        {
            AIMode->ApplyDamageToAISniper(Damage);
            AIMode->HandleAICounterAttack(SelectedBrawler, Target);
        }
    }
    ClearReachableTiles();
    bHasBrawlerAttacked = true;                                                                                         //il brawler ha attaccato yes
    CheckEndTurn();                                                                                                     //verifichiamo condizioni per terminare il turno
    return true;
}

bool AMyPlayerController::HandleSniperAttack(AActor* Target)    // Gestisce l'attacco dello Sniper
{
    // Verifica che lo sniper e il bersaglio siano validi
    if (!SelectedSniper || !Target)
    {
        return false;
    }
    
    // Verifica che il bersaglio sia effettivamente una unità nemica (AISniper o AIBrawler)
    if (!Target->ActorHasTag(FName("AISniper")) && !Target->ActorHasTag(FName("AIBrawler")))
    {
        UE_LOG(LogTemp, Error, TEXT("Il bersaglio non è un AISniper o un AIBrawler!"));
        return false;
    }
    
    // Ottieni l'oggetto degli ostacoli (griglia)
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if (!Obstacles)
    {
        UE_LOG(LogTemp, Error, TEXT("Obstacles non trovato!"));
        return false;
    }
    
    // Ottieni le coordinate della posizione dello sniper e del bersaglio
    int32 SniperX, SniperY;
    int32 TargetX, TargetY;
    if (!Obstacles->GetCellCoordinatesFromWorldPosition(SelectedSniper->GetActorLocation(), SniperX, SniperY) ||
        !Obstacles->GetCellCoordinatesFromWorldPosition(Target->GetActorLocation(), TargetX, TargetY))
    {
        UE_LOG(LogTemp, Error, TEXT("Coordinate della cella non valide!"));
        return false;
    }
    
    // Calcola la distanza (usando la somma dei valori assoluti, tipica per una griglia)
    int32 Distance = FMath::Abs(SniperX - TargetX) + FMath::Abs(SniperY - TargetY);
    
    // Verifica se il bersaglio è fuori portata (range massimo per lo sniper = 10 celle)
    if (Distance > 10)
    {
        FString LogMessage = FString::Printf(TEXT("Il bersaglio è fuori portata! Distanza: %d"), Distance);
        if (LateralPanelWidgetInstance)
        {
            LateralPanelWidgetInstance->AddGameMessage(LogMessage, FLinearColor::Black);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("LateralPanelWidgetInstance è nullo!"));
        }
        return false;   // Esci senza controllare se lo sniper ha già attaccato
    }
    
    // Ora, se il bersaglio è in range, verifica se lo sniper ha già attaccato in questo turno.
    // In questo modo, se lo sniper non ha attaccato, si procede; se ha già attaccato,
    // semplicemente restituisce false (senza mostrare il messaggio "ha già attaccato")
    if (bHasSniperAttacked)
    {
        return false;
    }
    
    // Calcola il danno (range da 4 a 8)
    int32 Damage = FMath::RandRange(4, 8);
    UE_LOG(LogTemp, Warning, TEXT("Danno inflitto dallo Sniper: %d"), Damage);
    
    // Prepara il messaggio da mostrare sul pannello laterale
    if (LateralPanelWidgetInstance)
    {
        FString SniperCell = FString::Printf(TEXT("%c%d"), 'A' + SniperY, SniperX + 1);  // Formatta la cella dello sniper
        FString TargetCell = FString::Printf(TEXT("%c%d"), 'A' + TargetY, TargetX + 1);   // Formatta la cella del bersaglio
        FString Message = FString::Printf(TEXT("HP: S %s -> %s %d danni"), *SniperCell, *TargetCell, Damage);
        LateralPanelWidgetInstance->AddGameMessage(Message, FLinearColor::Blue);
    }
    
    // Gestione dell'attacco e contrattacco tramite l'istanza AI
    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));
    if (AIMode)
    {
        if (Target->ActorHasTag(FName("AIBrawler")))
        {
            AIMode->ApplyDamageToAIBrawler(Damage);
            AIMode->HandleAICounterAttack(SelectedSniper, Target);  // Esegue il contrattacco se applicabile
        }
        else if (Target->ActorHasTag(FName("AISniper")))
        {
            AIMode->ApplyDamageToAISniper(Damage);
            AIMode->HandleAICounterAttack(SelectedSniper, Target);
        }
    }
    
    // Pulisci le celle evidenziate e imposta lo stato di attacco eseguito
    ClearReachableTiles();
    bHasSniperAttacked = true;
    
    // Verifica eventuali condizioni di fine turno
    CheckEndTurn();
    return true;
}


ULateralPanelWidget* AMyPlayerController::GetLateralPanelWidget()
{
    if (!LateralPanelWidgetInstance)
    {
        if (LateralPanelWidgetClass)
        {
            LateralPanelWidgetInstance = CreateWidget<ULateralPanelWidget>(this, LateralPanelWidgetClass);  //crea il widget se la classe è valida
            if (LateralPanelWidgetInstance)
            {
                LateralPanelWidgetInstance->AddToViewport();                                                            //aggiunge il widget al viewport
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Errore nella creazione di LateralPanelWidget!"));                  //errore: widget non creato
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("LateralPanelWidgetClass non valida!"));                                //errore: classe widget non valida
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Restituzione di LateralPanelWidget esistente."));                        //warning: widget già esistente
    }
    return LateralPanelWidgetInstance;                                                                                  //ritorna l'istanza del widget
}
    
void AMyPlayerController::CheckEndTurn()                                                                                //funzione cardine per la gestione della fine del turno e quando sancirla
{                                                                                                                       //di base ho fatto sì che terminasse dopo che entrambe le pedine abbiano attaccato
    AMyGameMode* GameMode = GetGameMode();                                                                              //se il brawler non ha ancora attaccato seppur non abbia range valido devo premere skip
    if (!bBrawlerAlive && !bSniperAlive)                                                                                //se entrambe le pedine sono morte il turno è obbligato a terminare
    {
        UE_LOG(LogTemp, Error, TEXT("⚠️ ATTENZIONE: Entrambe le pedine sono morte! Turno FORZATAMENTE terminato."));
        GameMode->EndPlayerTurn();
        OnAIVictory.Broadcast();                                                                                        //infatti con un delegato comunica la vittoria dell'ai
        ResetTurnFlags();
        return;
    }
    if (!bBrawlerAlive && bSniperAlive)                                                                                 //solo brawler morto, sniper no
    {
        if (bHasSniperAttacked) 
        {
            UE_LOG(LogTemp, Warning, TEXT("Brawler è morto e lo Sniper ha attaccato. Turno terminato."));
            GameMode->EndPlayerTurn();
            ResetTurnFlags();                                                                                           //alla fine del turno resetta variabili pronte per turno dopo
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Il Brawler è morto, attendi che lo Sniper attacchi."));
        }
        return;
    }
    if (bBrawlerAlive && !bSniperAlive)                                                                                 //brawler vivo ma sniper no
    {
        if (bHasBrawlerAttacked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Lo Sniper è morto e il Brawler ha attaccato. Turno terminato."));
            GameMode->EndPlayerTurn();
            ResetTurnFlags();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Lo Sniper è morto, attendi che il Brawler attacchi."));
        }
        return;
    }
    if (bBrawlerAlive && bSniperAlive)                                                                                  //se entrambe le pedine sono vive il turno termina solo quando entrambe hanno attaccato
    {                                                                                                                   
        if (bHasBrawlerAttacked && bHasSniperAttacked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Entrambe le pedine hanno attaccato. Turno terminato."));
            GameMode->EndPlayerTurn();
            ResetTurnFlags();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Attendi che entrambe le pedine attacchino oppure usa lo Skip Turn."));
        }
    }
}


void AMyPlayerController::ResetTurnFlags()
{
    bHasBrawlerMoved = false;
    bHasSniperMoved = false;
    bHasBrawlerAttacked = false; 
    bHasSniperAttacked = false;  
    bHasSniperCompletedActions = false;
    bHasBrawlerCompletedAttack = false;
    bHasSniperCompletedAttack = false;
}                                                                                                                       //prepariamo variabili per il turno dopo


void AMyPlayerController::CheckPlayerBrawlerHealth()                                                                    //queste due mi servono per l'ai e per verificare se sono morte così da decreatare la vittoria dell'ai
{
    if (SelectedBrawler && LateralPanelWidgetInstance)
    {
        float CurrentHealth = LateralPanelWidgetInstance->GetBrawlerHealth();
        if (CurrentHealth <= 0.01f)
        {
            UE_LOG(LogTemp, Error, TEXT("Salute attuale: %f"), CurrentHealth);
            bBrawlerAlive = false; 
            LateralPanelWidgetInstance->AddGameMessage("HP: Il brawler è stato eliminato", FColor::Black);

            if (SelectedBrawler) 
            {
                SelectedBrawler->Destroy();
                SelectedBrawler = nullptr;
            }
            bHasBrawlerCompletedActions = true;
            bHasBrawlerCompletedAttack = true;

            CheckEndTurn();
        }
    }
}

void AMyPlayerController::CheckPlayerSniperHealth()                                                                     //queste due mi servono per l'ai e per verificare se sono morte così da decreatare la vittoria dell'ai
{
    if (SelectedSniper && LateralPanelWidgetInstance)
    {
        float CurrentHealth = LateralPanelWidgetInstance->GetSniperHealth();                                            //restituisce vita attuale sniper

        if (CurrentHealth <= 0.01f)
        {
            bSniperAlive = false; 
            LateralPanelWidgetInstance->AddGameMessage("HP: Lo Sniper è stato eliminato", FColor::Black);    //se la vita scende sotto lo 0, (0.1 per accettare un minimo errore?) allora muore 

            if (SelectedSniper) 
            {
                SelectedSniper->Destroy();                                                                              //e lo faccio scomparire dalla griglia
                SelectedSniper = nullptr;
            }
            bHasSniperCompletedActions = true;
            bHasSniperCompletedAttack = true;
            CheckEndTurn();                                                                                             //possiamo finire?
        }
    }
}

void AMyPlayerController::RemovePlayerBrawler()                                                                         //rimuovo
{
    TArray<AActor*> Brawlers;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Brawler"), Brawlers);
    for (AActor* Brawler : Brawlers)
    {
        if (Brawler)
        {
            LateralPanelWidgetInstance->AddGameMessage("HP: Il brawler e' stato eliminato", FColor::Black);  //stampato
            Brawler->Destroy();
        }
    }
    SelectedBrawler = nullptr;
    bBrawlerAlive = false;                                                                                              //il brawler è morto

    if (!bSniperAlive)
    {
        OnAIVictory.Broadcast();
    }
}

void AMyPlayerController::RemovePlayerSniper()
{
    TArray<AActor*> Snipers;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Sniper"), Snipers);
    for (AActor* Sniper : Snipers)
    {
        if (Sniper)
        {
            LateralPanelWidgetInstance->AddGameMessage("HP: Lo Sniper e' stato eliminato", FColor::Black);
            Sniper->Destroy();
        }
    }
    SelectedSniper = nullptr;
    bSniperAlive = false;                                                                                               //lo sniper è morto
    if (!bBrawlerAlive)
    {
        UE_LOG(LogTemp, Error, TEXT("Entrambe le pedine del giocatore sono morte! L'AI ha vinto la partita."));
        GetGameMode()->EndPlayerTurn(); 
    }   
}                                                                                                                       //idem a prima

void AMyPlayerController::HandlePlayerCounterAttack(AActor* Attacker, AActor* Defender)                                 //ora siamo noi a poter e dover contrattaccare le pedine dell'ai quando ci attaccano
{
    if (!Attacker || !Defender)
    {
        UE_LOG(LogTemp, Error, TEXT("Attacker o Defender è nullo!")); 
        return;                                                                                                         //termina la funzione se uno dei due è nullo
    }

    if (!Defender->ActorHasTag(FName("Brawler")) && !Defender->ActorHasTag(FName("Sniper")))
    {
        UE_LOG(LogTemp, Error, TEXT("Il difensore non è un'unità del giocatore!")); 
        return;                                                                                                         //termina la funzione se il difensore non è un'unità del giocatore
    }

    if (Defender->ActorHasTag(FName("Sniper")))                                                                         //se colui che difende è sniper
    {
        int32 CounterDamage = FMath::RandRange(1, 3);                                                          //calcola casualmente il danno del contrattacco ( vedi specifiche )
        UE_LOG(LogTemp, Warning, TEXT("%s contrattacca %s con %d danni!"), *Defender->GetName(), *Attacker->GetName(), CounterDamage); 

        AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));   
        if (AIMode)
        {
            if (Attacker->ActorHasTag(FName("AIBrawler")))
            {
                AIMode->ApplyDamageToAIBrawler(CounterDamage);                                                          //applica danno al AIBrawler se l'attaccante è AIBrawler
            }
            else if (Attacker->ActorHasTag(FName("AISniper")))
            {
                AIMode->ApplyDamageToAISniper(CounterDamage);                                                           //applica danno al AISniper se l'attaccante è AISniper
            }
        }
        if (LateralPanelWidgetInstance)
        {
            FString DefenderName = Defender->ActorHasTag(FName("Brawler")) ? TEXT("B") : TEXT("S");                     //imposta il nome del difensore: "B" se Brawler, altrimenti "S"
            FString AttackerName = Attacker->ActorHasTag(FName("AIBrawler")) ? TEXT("B") : TEXT("S");                   //imposta il nome dell'attaccante: "B" se AIBrawler, altrimenti "S"
            FString Message = FString::Printf(TEXT("Contrattacco HP: %s -> %s %d danni"), *DefenderName, *AttackerName, CounterDamage); //crea il messaggio di contrattacco
            LateralPanelWidgetInstance->AddGameMessage(Message, FLinearColor::Blue);                                     //mostra il messaggio sul pannello laterale
        }
    }
    else if (Defender->ActorHasTag(FName("Brawler")))                                                                   //se colui che difente è brawler
    {
        ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottiene l'oggetto degli ostacoli dalla scena
        if (Obstacles)
        {
            int32 AttackerX, AttackerY, DefenderX, DefenderY; //variabili per le coordinate delle celle
            if (Obstacles->GetCellCoordinatesFromWorldPosition(Attacker->GetActorLocation(), AttackerX, AttackerY) &&
                Obstacles->GetCellCoordinatesFromWorldPosition(Defender->GetActorLocation(), DefenderX, DefenderY))
            {
                int32 Distance = FMath::Abs(AttackerX - DefenderX) + FMath::Abs(AttackerY - DefenderY);           //calcola la distanza in termini di celle

                if (Distance <= 1)
                {
                    int32 CounterDamage = FMath::RandRange(1, 3);                                              //calcola casualmente il danno del contrattacco
                    UE_LOG(LogTemp, Warning, TEXT("%s contrattacca %s con %d danni!"), *Defender->GetName(), *Attacker->GetName(), CounterDamage);

                    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass())); 
                    if (AIMode)
                    {
                        if (Attacker->ActorHasTag(FName("AIBrawler")))
                        {
                            AIMode->ApplyDamageToAIBrawler(CounterDamage);                                              //applica danno al AIBrawler se l'attaccante è AIBrawler
                        }
                        else if (Attacker->ActorHasTag(FName("AISniper")))
                        {
                            AIMode->ApplyDamageToAISniper(CounterDamage);                                               //applica danno al AISniper se l'attaccante è AISniper
                        }
                    }
                    if (LateralPanelWidgetInstance)
                    {
                        FString DefenderName = TEXT("B");                                                               //imposta il nome del difensore come "B" (Brawler)
                        FString AttackerName = Attacker->ActorHasTag(FName("AIBrawler")) ? TEXT("B") : TEXT("S");       //imposta il nome dell'attaccante: "B" se AIBrawler, altrimenti "S"
                        FString Message = FString::Printf(TEXT("Contrattacco HP: %s -> %s %d danni"), *DefenderName, *AttackerName, CounterDamage); 
                        LateralPanelWidgetInstance->AddGameMessage(Message, FLinearColor::Blue);                         //mostra il messaggio sul pannello laterale
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Nessun contrattacco: l'attaccante non è adiacente!")); 
                }
            }
        }
    }
}

AGridManagerCPP* AMyPlayerController::GetGridManager() const
{
    return Cast<AGridManagerCPP>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerCPP::StaticClass()));
}