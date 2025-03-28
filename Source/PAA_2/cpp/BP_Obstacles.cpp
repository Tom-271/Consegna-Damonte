#include "../headers/BP_Obstacles.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Containers/Queue.h" 

ABP_Obstacles::ABP_Obstacles()                                                                                          //breve contestualizzazione: il motivo per cui ricreo variabili come gridsize e tile è dovuto al fatto che io abbia scelto di 
{                                                                                                                       //sovrapporre due griglie una sopra l'altra, una tutta piena (sotto) e l'altra di ostacoli (sopra), mi sembra più ordinato e non difficile 
    PrimaryActorTick.bCanEverTick = true;                                                                               //da implementare rispetto al fare tutto nella stessa classe

    GridSize = 25;
    GridTile = 104.0f;
    SpawnProbability = 0.3f;                                                                                            //la probabilità di spawn è di default a 30%, numero opportuno che permette buone dinamiche di gioco, sarà modificabile con slider
}

void ABP_Obstacles::SetSpawnProbability(float NewProbability)
{
    UE_LOG(LogTemp, Warning, TEXT("SetSpawnProbability chiamato con: %f"), NewProbability);
    SpawnProbability = NewProbability;                                                                                  //quando percentuale aggiornata passo valore nuovo
    GenerateGrid();
}

void ABP_Obstacles::BeginPlay()
{
    Super::BeginPlay();                                                                                                 

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0);                //ottiene playercontroller
    if(PlayerController)                                                                                                //se controller valido
    {
        if(APawn* DefaultPawn = PlayerController->GetPawn())                                                            //ottiene pawn di default
        {
            DefaultPawn->DetachFromControllerPendingDestroy();                                                          //stacca pawn dal controller, perche se lo distruggo rischio di inficiare anche la telecamera
            DefaultPawn->Destroy();                                                                                     //distrugge pawn, se non lo elimino compare una sfera dello stesso materiale nella cella A1
        }
    }

    UE_LOG(LogTemp,Warning,TEXT("Posizione della griglia (BeginPlay): %s"),*GetActorLocation().ToString());     //log posizione griglia
    GenerateGrid();                                                                                                     //genera la nuova griglia
}

void ABP_Obstacles::GenerateGrid()                                                                                      //ci sono molti controlli sulla generazione e la distruzione di osacoli perchè qui siamo in bp_obstacles, l'idea alla base è che per gestire lo slider, io vaada ad eliminare gli ostacoli precedentemente creati
{                                                                                                                       //per aggiornare prima dell'inizio lo slider con la percentuale, rischiavo che la percentuale scelta si aggiungesse a quella di default, quindi destroyggo la griglia precedentemente creata e creo la nuova con percent aggiornata
    if(bIsGridUpdating)                                                                                                 //evita chiamate concorrenti
    {
        UE_LOG(LogTemp, Warning, TEXT("GenerateGrid già in esecuzione."));
        return;
    }
    bIsGridUpdating = true;                                                                                             //imposta flag aggiornamento

    DestroyGrid();                                                                                                      //distruggi griglia esistente

    if(!TileBlueprintClass)                                                                                             //controlla se il blueprint della tile è impostato
    {
        bIsGridUpdating = false;                                                                                        //resetta flag aggiornamento
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("BP_Obstacles - Posizione griglia: %s"), *GetActorLocation().ToString());     //log posizione griglia

    TArray<TArray<bool>> ObstacleMap;                                                                                   //mappa ostacoli
    CreateObstacleMap(ObstacleMap);                                                                                  //popola la mappa di ostacoli

    for(int32 x = 0; x < GridSize; ++x)                                                                                 //itera sulle colonne
    {
        TArray<FCellInfo> Column;                                                                                       //info colonna corrente
        for(int32 y = 0; y < GridSize; ++y)                                                                             //itera sulle righe
        {
            FVector TilePosition = FVector(                                                                             //calcola posizione world della tile
                FMath::RoundToFloat(x * GridTile),
                FMath::RoundToFloat(y * GridTile),
                0.2f
            );

            FCellInfo CellInfo;                                                                                         //struttura dati cella
            CellInfo.bIsObstacle = ObstacleMap[x][y];                                                                   //imposta se è ostacolo

            if(CellInfo.bIsObstacle)                                                                                    //se cella è ostacolo
            {
                AActor* NewTile = GetWorld()->SpawnActor<AActor>(TileBlueprintClass, TilePosition, FRotator::ZeroRotator); 
                if(NewTile) CellInfo.TileActor = NewTile;                                                               //assegna actor se spawn riuscito
                else UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare tile alla posizione: %s"), *TilePosition.ToString());
            }

            Column.Add(CellInfo);                                                                                       //aggiunge cella alla colonna
        }
        GridCells.Add(Column);                                                                                          //aggiunge colonna alla griglia
    }

    bIsGridUpdating = false;                                                                                            //resetta flag aggiornamento
}

void ABP_Obstacles::DestroyGrid()                                                                                       //dobbiamo ora gestire la distruzione delle celle quando viene chiamato lo slider, perchè senza di questo avremmo una sovrapposizione degli ostacoli di default con 
{                                                                                                                       //quelli sceldi dallo slider, ex anzichè avere un 50% di ostacoli scelto tramite slider avremmo un 30% (default) + 50%
    if (GridCells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridCells è vuoto, nessun ostacolo da distruggere."));
        return;
    }

    for (TArray<FCellInfo>& Column : GridCells)
    {
        for (FCellInfo& Cell : Column)
        {
            if (Cell.TileActor.IsValid())
            {
                AActor* Actor = Cell.TileActor.Get();
                UE_LOG(LogTemp, Warning, TEXT("Distruggo tile: %s"), *Actor->GetName());
                Actor->Destroy();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Tile non valido o già distrutto."));
            }
            Cell.TileActor = nullptr;
        }
    }
    GridCells.Empty();
}



FCellInfo ABP_Obstacles::GetCellInfo(int32 X, int32 Y) const                                                            //funzione utile a fornire le celle della griglia
{
    if (X >= 0 && X < GridSize && Y >= 0 && Y < GridSize)
    {
        return GridCells[X][Y];                                                                                         //letteralmente un return
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Coordinate della cella non valide: X=%d, Y=%d"), X, Y);
        return FCellInfo();
    }
}

void ABP_Obstacles::CreateObstacleMap(TArray<TArray<bool>>& ObstacleMap)
{
    ObstacleMap.SetNum(GridSize);                                                                                       //inizializza la mappa degli ostacoli
    for (int32 x = 0; x < GridSize; ++x)
    {
        ObstacleMap[x].SetNum(GridSize);
        for (int32 y = 0; y < GridSize; ++y)
        {
            ObstacleMap[x][y] = false;                                                                                  //tutte le celle all'inizio sono vuote e via via vanno riempite
        }
    }

    for (int32 x = 0; x < GridSize; ++x)                                                                                //aggiungiamo ostacoli casuali rispettando la probabilità scelta MA VERIFICANDO CHE NON CI SIANO ZONE CHIUSE
    {
        for (int32 y = 0; y < GridSize; ++y)
        {
            if (FMath::FRand() <= SpawnProbability)
            {
                ObstacleMap[x][y] = true;                                                                               //tenta di spawnare un'ostacolo, l'aggiunta potrebbe fallire come no
                                                                                                                        //PERCHE'? commento nella funzione AreAllCellsReachable
                
                if (!AreAllCellsReachable(ObstacleMap))
                {
                                                                                                                        //se la mappa non è valida, rimuovi l'ostacolo
                    ObstacleMap[x][y] = false;
                }
            }
        }
    }
}

bool ABP_Obstacles::AreAllCellsReachable(const TArray<TArray<bool>>& ObstacleMap)                                       //l'idea su cui si basa la creazione della griglia degli ostacoli è molto interessante, pongo un ostacolo casualmente sulla griglia e valuto che tutti i percorsi siano
{                                                                                                                       //raggiungibili almeno in un percorso, se sì allora posso lasciare l'ostacolo, altrimenti lo rimuovo, questo perchè significa che si sta creando una zona accerchiata da ostacoli, irraggiungibile
    
    TArray<TArray<bool>> Visited;                                                                                       //mappa da visitare tramite BFS con questa logica appena descritta
    Visited.SetNum(GridSize);
    for (int32 i = 0; i < GridSize; ++i)
    {
        Visited[i].SetNum(GridSize, false);
    }
    
    bool FoundStart = false;                                                                                            //trova una cella vuota da cui partire                        
    int32 StartX = 0;
    int32 StartY = 0;

    for (int32 x = 0; x < GridSize; ++x)
    {
        for (int32 y = 0; y < GridSize; ++y)
        {
            if (!ObstacleMap[x][y])
            {
                StartX = x;
                StartY = y;
                FoundStart = true;
                break;
            }
        }
        if (FoundStart) break;
    }

    if (!FoundStart)
    {
        return true;                                                                                                    //se non ci sono celle vuote la mappa è valida
    }
    
    BFS(ObstacleMap, Visited, StartX, StartY);                                                                       //dopo che abbiamo calcolato la cella da cui partire per effettuare la ricerca, applichiamo BFS

    for (int32 x = 0; x < GridSize; ++x)
    {
        for (int32 y = 0; y < GridSize; ++y)
        {   
            if (!ObstacleMap[x][y] && !Visited[x][y])                                                                   //verifica che tutte le celle siano state visitate
            {
                return false;                                           
            }
        }
    }

    return true;
}

void ABP_Obstacles::BFS(const TArray<TArray<bool>>& ObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY)
{
    TQueue<TPair<int32, int32>> Queue;                                                                                  //creiamo una coda per il BFS

    Queue.Enqueue(TPair<int32, int32>(StartX, StartY));                                                      //aggiungiamo la cella di partenza del controllo alla coda
    Visited[StartX][StartY] = true;
    
    int32 Directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };                                          //coordinate per specificare le direzioni della ricerca

    while (!Queue.IsEmpty())                                                                                            //finchè la coda non è vuota...
    {
        TPair<int32, int32> CurrentCell;
        Queue.Dequeue(CurrentCell);                                                                                  //prendo dalla coda

        int32 x = CurrentCell.Key;
        int32 y = CurrentCell.Value;

        for (int32 i = 0; i < 4; ++i)                                                                                   //celle adiacenti
        {
            int32 NewX = x + Directions[i][0];
            int32 NewY = y + Directions[i][1];

            if (NewX >= 0 && NewX < GridSize && NewY >= 0 && NewY < GridSize && !ObstacleMap[NewX][NewY] && !Visited[NewX][NewY])   
            {                                                                                                           //se dentro la griglia, non un ostacolo e non ancora visitato allora esploro
                Visited[NewX][NewY] = true;
                Queue.Enqueue(TPair<int32, int32>(NewX, NewY));                                              //aggiungo alla coda
            }
        }                                                                                                               
    }
}

bool ABP_Obstacles::GetCellCoordinatesFromWorldPosition(const FVector& WorldPosition, int32& OutX, int32& OutY) const
{
    if (!this)                                                                                                          //controllo di sicurezza (dovrebbe essere superfluo)
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: Il puntatore a BP_Obstacles è NULL!"));
        return false;
    }
    
    if (GridSize <= 0 || GridTile <= 0.0f)                                                                              //verifica che la griglia sia configurata correttamente
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: GridSize=%d o GridTile=%f non validi!"), GridSize, GridTile);
        return false;
    }

    FVector GridOrigin = GetActorLocation();                                                                            //prende la osizione dell’origine (angolo inferiore sinistro) della griglia

    if (!FMath::IsFinite(WorldPosition.X) || !FMath::IsFinite(WorldPosition.Y))
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: WorldPosition non valido! X=%f, Y=%f"), WorldPosition.X, WorldPosition.Y);
        return false;
    }
    
    OutX = FMath::FloorToInt((WorldPosition.X - GridOrigin.X + GridTile * 0.5f) / GridTile);                         //converte la posizione world in indice di colonna/row
    OutY = FMath::FloorToInt((WorldPosition.Y - GridOrigin.Y + GridTile * 0.5f) / GridTile);
    
    OutX = FMath::Clamp(OutX, 0, GridSize - 1);                                                        //clampare è un mezzo con il quale portiamo a zero la coordinata se è minore di 0, e porta a Gridsize-1 se eccede dall'altra parte
    OutY = FMath::Clamp(OutY, 0, GridSize - 1);

    return true;
}

bool ABP_Obstacles::FindRandomEmptyCell(int32& OutX, int32& OutY)                                                       //troviamo celle libere per il percorso
{
    TArray<TPair<int32, int32>> EmptyCells;                                                                             //array

    for (int32 x = 0; x < GridSize; ++x)
    {
        for (int32 y = 0; y < GridSize; ++y)                                                                            //doppio for array bidimensionale
        {
            if (!GridCells[x][y].bIsObstacle)                                                                           //se non è un ostacolo, aggiungo la cella 
            {
                EmptyCells.Add({ x, y });
            }
        }
    }
    
    if (EmptyCells.Num() == 0)                                                                                          //se nessuna cella è libera, assurdo ma controllo, restituisce errore
    {
        UE_LOG(LogTemp, Error, TEXT("Nessuna cella libera disponibile per lo spawn!"));
        return false;
    }
    
    int32 RandomIndex = FMath::RandRange(0, EmptyCells.Num() - 1);                                             //scegliamo casualmente una cella con riga e colonna da quelle decretate come libere
    OutX = EmptyCells[RandomIndex].Key;     
    OutY = EmptyCells[RandomIndex].Value;   
    return true;
}

FVector ABP_Obstacles::GetCellWorldPosition(int32 X, int32 Y) const
{
    FVector GridOrigin = GetActorLocation();                                                                            //prende la osizione dell’origine (angolo inferiore sinistro) della griglia

    FVector WorldPos = FVector(                                                                                         //posizione della cella X,Y
        GridOrigin.X + (X * GridTile),                                                                                  //X, Y, Z
        GridOrigin.Y + (Y * GridTile),                                               
        1.0f                                                                          
    );

    UE_LOG(LogTemp, Warning, TEXT("GetCellWorldPosition per [%d, %d] -> %s"), X, Y, *WorldPos.ToString());
    return WorldPos;
}