#include "../headers/AIMode.h"
#include "../headers/BP_Obstacles.h"
#include "../headers/LateralPanelWidget.h"
#include "../headers/GridManagerCPP.h"
#include "../headers/MyPlayerController.h"
#include "UObject/ConstructorHelpers.h" 
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AAIMode::AAIMode()                                                                                                      //definisco nel costruttore ciò che ci servirà, texture, materiali e blueprints
{                                                                                                                       //in questa classe definiremo i comportamenti di tutto ciò che riguarda l'ai
    PrimaryActorTick.bCanEverTick = true;                                 
    static ConstructorHelpers::FClassFinder<AActor> AIBrawlerBlueprintFinder(TEXT("/Game/Blueprints/AIBrawler"));       //carica blueprint AI brawler
    if(AIBrawlerBlueprintFinder.Succeeded())                                                                            //se trovato
    {
        AIBrawlerBlueprintClass = AIBrawlerBlueprintFinder.Class;                                                       //assegna classe blueprint
        UE_LOG(LogTemp, Warning, TEXT("AIBrawlerBlueprintClass caricato correttamente: %s"), *AIBrawlerBlueprintClass->GetPathName());
    }
    else UE_LOG(LogTemp, Error, TEXT("AIBrawlerBlueprintClass NON TROVATO! Controlla il percorso."));           

    static ConstructorHelpers::FClassFinder<AActor> AISniperBlueprintFinder(TEXT("/Game/Blueprints/AISniper"));         //carica blueprint AI sniper
    if(AISniperBlueprintFinder.Succeeded())                                                                             //se trovato
    {
        AISniperBlueprintClass = AISniperBlueprintFinder.Class;                                                         //assegna classe blueprint
        UE_LOG(LogTemp, Warning, TEXT("AISniperBlueprintClass caricato correttamente: %s"), *AISniperBlueprintClass->GetPathName()); 
    }
    else UE_LOG(LogTemp, Error, TEXT("AISniperBlueprintClass NON TROVATO! Controlla il percorso.")); 

    static ConstructorHelpers::FObjectFinder<UTexture2D> Soldier1RedFinder(TEXT("Texture2D'/Game/Blueprints/Soldier1_Red.Soldier1_Red'")); 
    if(Soldier1RedFinder.Succeeded())                                                                                   //se trovato
    {
        Soldier1RedTexture = Soldier1RedFinder.Object;                                                               //assegna texture
        UE_LOG(LogTemp, Warning, TEXT("Texture Soldier1_Red caricata correttamente: %s"), *Soldier1RedTexture->GetName()); 
    }
    else UE_LOG(LogTemp, Error, TEXT("Texture Soldier1_Red NON TROVATA!"));

    static ConstructorHelpers::FObjectFinder<UTexture2D> Soldier2RedFinder(TEXT("Texture2D'/Game/Blueprints/Soldier2_Red.Soldier2_Red'")); //carica texture soldier2 red
    if(Soldier2RedFinder.Succeeded())                                                                                   //se trovato
    {
        Soldier2RedTexture = Soldier2RedFinder.Object;                                                               //assegna texture
        UE_LOG(LogTemp, Warning, TEXT("Texture Soldier2_Red caricata correttamente: %s"), *Soldier2RedTexture->GetName()); 
    }
    else UE_LOG(LogTemp, Error, TEXT("Texture Soldier2_Red NON TROVATA!"));

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> AISniperMatFinder(TEXT("Material'/Game/Blueprints/AISNIPERMATERIAL.AISNIPERMATERIAL'")); //carica materiale sniper AI
    if(AISniperMatFinder.Succeeded())                                                                                   //se trovato
    {
        AISniperMaterial = AISniperMatFinder.Object;                                                                 //assegna materiale
        UE_LOG(LogTemp, Warning, TEXT("Material AISNIPERMATERIAL caricato correttamente: %s"), *AISniperMaterial->GetName());
    }
    else UE_LOG(LogTemp, Error, TEXT("Material AISNIPERMATERIAL NON TROVATO!"));                                
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIBrawlerMatFinder(TEXT("Material'/Game/Blueprints/AIBRAWLERMATERIAL.AIBRAWLERMATERIAL'")); //carica materiale brawler AI
    if(AIBrawlerMatFinder.Succeeded())                                                                                  //se trovato
    {
        AIBrawlerMaterial = AIBrawlerMatFinder.Object;                                                               //assegna materiale
        UE_LOG(LogTemp, Warning, TEXT("Material AIBRAWLERMATERIAL caricato correttamente: %s"), *AIBrawlerMaterial->GetName()); 
    }
    else UE_LOG(LogTemp, Error, TEXT("Material AIBRAWLERMATERIAL NON TROVATO!"));                               //log errore

    bHasSpawnedAIBrawler = false;                                                                                       //reset flag spawn brawler AI, vanno inizializzati a false non essendo ancora stati spawnati
    bHasSpawnedAISniper = false;                                                                                        //reset flag spawn sniper AI
    AIBrawlerHealth = 40.0f;                                                                                            //imposta punti vita brawler
    AISniperHealth = 20.0f;                                                                                             //imposta punti vita sniper 

    static ConstructorHelpers::FClassFinder<AActor> TileFinder(TEXT("/Game/Blueprints/BP_ReachableTile"));              //carica blueprint reachable tile
    if(TileFinder.Succeeded())                                                                                          //se trovato
    {
        ReachableTileBlueprint = TileFinder.Class;                                                                      //assegna classe blueprint
        UE_LOG(LogTemp, Warning, TEXT("BP_ReachableTiles caricato correttamente: %s"), *ReachableTileBlueprint->GetPathName()); 
    }
    else UE_LOG(LogTemp, Error, TEXT("BP_ReachableTiles NON TROVATO! Controlla il percorso."));
}

void AAIMode::SpawnAIBrawler()                                                                                          //gestisco ora le funzioni chiamate dall'AiMode
{
    if(bHasSpawnedAIBrawler) return;                                                                                    //evita doppio spawn del brawler

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); 
    if(!Obstacles) return;                                                                                              //se non trova ostacoli esce

    int32 X, Y;
    if(Obstacles->FindRandomEmptyCell(X, Y))                                                                      //trova cella libera in modo randomico
    {
        FVector CellPosition = Obstacles->GetCellWorldPosition(X, Y);                                                   //ottiene posizione tramite world della cella
        CellPosition.Z = 50.0f;                                                                                         //imposta altezza spawn, è una cosa forse superflu ma almeno sappiamo per certo altezza pedina

        FRotator AIBrawlerRotation = FRotator(0, 180, 0);                                              //imposta rotazione spawn - ruoto di 180 gradi le pedine per farle puntare verso di me

        if(AIBrawlerBlueprintClass)                                                                                     //verifichiamo se il blueprint sia valido
        {
            SelectedBrawler = GetWorld()->SpawnActor<AActor>(AIBrawlerBlueprintClass, CellPosition, AIBrawlerRotation); //spawn AI brawler
            if(SelectedBrawler)                                                                                         //se lo spawn è riuscito
            {
                SelectedBrawler->Tags.Add(FName("AIBrawler"));                                                     //aggiunge tag identificativo con cui lo chiameremo, i tag li ho usati anche per le pedine del player e forse nel mio caso sono superflue visto che controllo 2 pedine oltretutto diverse
                                                                                                                        //mi piaceva come metodo all'inizio e l'idea di usare un tag per chiamare su larga scala tanti actor ecc, quindi ho proseguito secondo questa logica
                UStaticMeshComponent* MeshComp = SelectedBrawler->FindComponentByClass<UStaticMeshComponent>();         //ottiene componente mesh
                if(MeshComp)                                                                                            //se mesh trovata
                {
                    if(AIBrawlerMaterial && Soldier2RedTexture) 
                    {
                        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(AIBrawlerMaterial, this); //crea materiale dinamico
                        if(DynMat)
                        {
                            DynMat->SetTextureParameterValue(FName("BaseTexture"), Soldier2RedTexture);                 //imposta texture alla pedina, gestisco tutto in c++
                            MeshComp->SetMaterial(0, DynMat);                                               //applica materiale
                            UE_LOG(LogTemp, Warning, TEXT("AIBrawler: Dynamic Material Instance creato e parametro 'BaseTexture' impostato.")); 
                        }
                        else UE_LOG(LogTemp, Error, TEXT("AIBrawler: Impossibile creare il Dynamic Material Instance.")); 
                    }
                    else UE_LOG(LogTemp, Error, TEXT("AIBrawler: AIBRAWLERMATERIAL o Soldier2RedTexture non disponibili.")); 
                }
                else UE_LOG(LogTemp, Error, TEXT("AIBrawler: StaticMeshComponent non trovato.")); 
            }
        }
    }
    bHasSpawnedAIBrawler = true;                                                                                        //bene ho spawnato la pedina
}

void AAIMode::SpawnAISniper()                                                                                           //replico tutto per aisniper, ogni pedina è un'entità a se stante, così come per player
{
    if(bHasSpawnedAISniper) return; 
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); 
    if(!Obstacles) return; 
    int32 X, Y;
    if(Obstacles->FindRandomEmptyCell(X, Y)) 
    {
        FVector CellPosition = Obstacles->GetCellWorldPosition(X, Y); 
        CellPosition.Z = 50.0f; 
        FRotator AISniperRotation = FRotator(0, 180, 0);
        if(AISniperBlueprintClass)
        {
            SelectedSniper = GetWorld()->SpawnActor<AActor>(AISniperBlueprintClass, CellPosition, AISniperRotation); 
            if(SelectedSniper) //se spawn riuscito
            {
                SelectedSniper->Tags.Add(FName("AISniper")); 
                UStaticMeshComponent* MeshComp = SelectedSniper->FindComponentByClass<UStaticMeshComponent>(); 
                if(MeshComp) 
                {
                    if(AISniperMaterial && Soldier1RedTexture) 
                    {
                        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(AISniperMaterial, this);
                        if(DynMat) 
                        {
                            DynMat->SetTextureParameterValue(FName("BaseTexture"), Soldier1RedTexture); 
                            MeshComp->SetMaterial(0, DynMat);
                            UE_LOG(LogTemp, Warning, TEXT("AISniper: Dynamic Material Instance creato e parametro 'BaseTexture' impostato."));
                        }
                        else UE_LOG(LogTemp, Error, TEXT("AISniper: Impossibile creare il Dynamic Material Instance.")); 
                    }
                    else UE_LOG(LogTemp, Error, TEXT("AISniper: AISNIPERMATERIAL o Soldier1RedTexture non disponibili.")); 
                }
                else UE_LOG(LogTemp, Error, TEXT("AISniper: StaticMeshComponent non trovato.")); 
            }
        }
    }
    bHasSpawnedAISniper = true; 
}

void AAIMode::MoveAICharacters()                                                                                        //chiamata anche dalla gamemode, gestisce il movimento in ogni turno delle pedine
{
    if(SelectedBrawler)                                                                                                 //se esiste AI brawler
    {
        AActor* TargetForBrawler = FindNearestPlayerUnit(SelectedBrawler->GetActorLocation());                          //trova unità giocatore più vicina
        if(!TargetForBrawler)                                                                                           //se nessun bersaglio trovato
        {
            AttackIfPossible(SelectedBrawler);                                                                          //prova attacco
            MoveAISniper();                                                                                             //passa al movimento dello sniper
            return;                                                                                                     //esce dalla funzione
        }
        ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
        if(Obstacles)                                                                                                   //verifica se il Brawler è già affianco al target
        {
            int32 BrawlerX, BrawlerY, TargetX, TargetY;
            Obstacles->GetCellCoordinatesFromWorldPosition(SelectedBrawler->GetActorLocation(), BrawlerX, BrawlerY);
            Obstacles->GetCellCoordinatesFromWorldPosition(TargetForBrawler->GetActorLocation(), TargetX, TargetY);
            
            int32 Distance = FMath::Abs(BrawlerX - TargetX) + FMath::Abs(BrawlerY - TargetY);                       
            if(Distance <= 1)                                                                                           //se è già affianco al target, attacca direttamente senza muoversi, non ho necessità che ad ogni turno continui a muoversi e poi attaccare
            {
                UE_LOG(LogTemp, Warning, TEXT("AIBrawler è già affianco al target, attacca direttamente"));
                AttackIfPossible(SelectedBrawler);
                MoveAISniper();
                return;
            }
        }
        TArray<FVector> BrawlerPath = CalculatePath(SelectedBrawler->GetActorLocation(), TargetForBrawler->GetActorLocation(), 6); //calcola percorso di 6 passi
        if(BrawlerPath.Num() > 0)                                                                                       //se percorso valido
        {
            StartAIMovement(SelectedBrawler, BrawlerPath, [this]()                       //inizia movimento AI
            {
                AttackIfPossible(SelectedBrawler);                                                                      //attacca dopo movimento
                MoveAISniper();                                                                                         //chiama movimento sniper al termine
            });
        }
        else 
        {
            UE_LOG(LogTemp, Warning, TEXT("Nessun percorso trovato per il Brawler!")); 
            AttackIfPossible(SelectedBrawler);                                                                          //prova attacco
            MoveAISniper();                                                                                             //passa allo sniper
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessun AIBrawler trovato, si passa direttamente ad AISniper."));
        MoveAISniper();                                                                                                 //muove solo sniper
    }
}

void AAIMode::MoveAISniper()                                                                                            //qui si gestisce il movimento dello sniper
{
    if(!SelectedSniper)                                                                                                 //se non esiste lo sniper AI
    {
        OnAITurnFinished.Broadcast();                                                                                   //fine turno AI
        return;
    }
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottiene ostacoli
    if(!Obstacles)                                                                                                      //se ostacoli non trovati
    {
        UE_LOG(LogTemp, Error, TEXT("MoveAISniper: Obstacles non trovato!"));                                   //me lo dice
        OnAITurnFinished.Broadcast();                                                                                   //fine turno AI
        return; 
    }
    int32 StartX, StartY;
    Obstacles->GetCellCoordinatesFromWorldPosition(SelectedSniper->GetActorLocation(), StartX, StartY);           //ottiene coordinate start
    SelectedSniper->SetActorLocation(Obstacles->GetCellWorldPosition(StartX, StartY));                                  //allinea posizione actor alla cella

    const int32 Steps = 3;                                                                                              //numero di passi massimo per lo sniper
    TArray<FVector> Path;                                                                                               //salva array percorso
    if(bEasyMode)                                                                                                       //se modalità facile
    {
        TArray<TPair<int32,int32>> Frontier = {{StartX, StartY}}, Next;                                        
        TMap<TPair<int32,int32>, TPair<int32,int32>> Predecessor;                                                       //mappa predecessori - BFS  
        TArray<TArray<bool>> Visited;                                                                                   //mappa visite
        Visited.Init(TArray<bool>(), Obstacles->GridSize);                                                       //inizializza righe
        for(int i=0;i<Obstacles->GridSize;i++) Visited[i].Init(false, Obstacles->GridSize);                      //inizializza colonne
        Visited[StartX][StartY] = true;                                                                                 //segna start visitato

        static const int Dir[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};                                           //direzioni movimento
        for(int step=0; step<Steps && Frontier.Num(); step++)                                                           //BFS fino a Steps
        {
            Next.Empty(); 
            for(auto Cell : Frontier)
            {
                for(auto& d : Dir)                                                                          //itera direzioni
                {
                    int NX = Cell.Key + d[0], NY = Cell.Value + d[1];                                                   //calcola nuova cella
                    if(NX>=0 && NY>=0 && NX<Obstacles->GridSize && NY<Obstacles->GridSize && !Visited[NX][NY])          //se cella valida e non visitata
                    {
                        FCellInfo Info = Obstacles->GetCellInfo(NX, NY);                                          //ottiene info cella
                        if(!Info.bIsObstacle && !IsCellOccupiedByOTHERS(NX, NY, this))                  //se cella libera
                        {
                            Visited[NX][NY] = true;                                                                     //segna visitata
                            Predecessor.Add({NX,NY}, Cell);                                       //registra predecessore
                            Next.Add({NX,NY});                                                            //aggiunge a next frontier
                        }
                    }
                }
            }
            Frontier = Next; 
        }
        if(Frontier.Num())                                                                                              //se frontier non vuota
        {
            TPair<int32,int32> Target = Frontier[FMath::RandRange(0, Frontier.Num()-1)];                       //sceglie cella casuale
            TArray<FVector> TempPath;                                                                                   
            for(auto Node = Target; !(Node.Key==StartX && Node.Value==StartY); Node = Predecessor[Node]) //ricostruisce percorso
            {
                TempPath.Insert(Obstacles->GetCellWorldPosition(Node.Key, Node.Value), 0);              //inserisce cella in testa
            }
            Path = TempPath;                                                                                            //assegna percorso finale
        }
    }
    else if(AActor* TargetUnit = FindNearestPlayerUnit(SelectedSniper->GetActorLocation()))                             //modalità normale: trova bersaglio più vicino
    {
        Path = CalculatePath(SelectedSniper->GetActorLocation(), TargetUnit->GetActorLocation(), Steps); //calcola percorso tramite la funzione seguente ...
    }
    if(Path.Num() > 0)                                                                                                  //se percorso validom quindi infatti path > 0
    {
        StartAIMovement(SelectedSniper, Path, [this]()                                   //inizia movimento AI
        {
            AttackIfPossible(SelectedSniper);                                                                           //attacca dopo movimento
            OnAITurnFinished.Broadcast();                                                                               //fine turno AI
        });
    }
    else 
    {
        AttackIfPossible(SelectedSniper);                                                                               //prova attacco
        OnAITurnFinished.Broadcast();                                                                                   //fine turno AI
    }
}                                                                                                                       //HO USATO BFS

TArray<FVector> AAIMode::CalculatePath(const FVector& StartLocation, const FVector& TargetLocation, int32 MaxSteps)     //funzione dedita al calcolo del percorso
{
    TArray<FVector> Path;                                                                                               //array che conterrà il percorso finale
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottieni riferimento alla griglia per le conversioni di posizione
    if(!Obstacles) 
    {
        return Path;                                                                                                    //restituisce percorso vuoto se non trova la griglia
    }
    int32 StartX, StartY;
    if(!Obstacles->GetCellCoordinatesFromWorldPosition(StartLocation, StartX, StartY))                            //converte la posizione iniziale in coordinate griglia
    {
        UE_LOG(LogTemp, Error, TEXT("CalculatePath: Start position invalid!")); 
        return Path;
    }
    auto IsValidCell = [Obstacles, this, StartX, StartY](int32 X, int32 Y, AActor* MovingActor) -> bool                 //funzione lambda per validare una cella (controlla bordi, ostacoli e occupazione)
    {
        if(X < 0 || Y < 0 || X >= Obstacles->GridSize || Y >= Obstacles->GridSize)                                      //controlla che la cella sia dentro i bordi della griglia
            return false;
        FCellInfo Info = Obstacles->GetCellInfo(X, Y);                                                                  //ottiene info cella e verifica che non sia ostacolo o occupata da altri
        bool bIsFree = !Info.bIsObstacle && 
                       !IsCellOccupiedByOTHERS(X, Y, MovingActor) && 
                       !(X == StartX && Y == StartY);                                                                   //evita di tornare al punto di partenza
        
        UE_LOG(LogTemp, Verbose, TEXT("Cell [%d,%d] validation: %d"), X, Y, bIsFree);
        return bIsFree;
    };
    if(bEasyMode)                                                                                                       //modalità facile: movimento casuale senza pathfinding verso target, cosa che invece facciamo se non entriamo nell'if
    {
        static const int Directions[4][2]={{-1,0},{1,0},{0,-1},{0,1}};                                      //direzioni possibili, su giu destra sinistra
        const int32 MaxAttempts = 10;                                                                                   //massimo tentativi per generare percorso
        for(int32 Attempt=0; Attempt<MaxAttempts; ++Attempt)                                                            //prova a generare un percorso casuale valido
        {
            Path.Empty();
            int32 CurrX = StartX, CurrY = StartY;
            int32 PrevX=-1, PrevY=-1;                                                                                   //memorizza cella precedente per evitare backtracking

            for(int32 Step=0; Step<MaxSteps; ++Step)                                                                    //passo dopo passo
            {
                TArray<TPair<int32,int32>> Options;                                                                     //celle disponibili per il prossimo passo
                for(auto& d : Directions)                                                                   //controlla tutte le direzioni possibili
                {
                    int NX = CurrX + d[0], NY = CurrY + d[1];
                    if(IsValidCell(NX, NY, CurrentAIMovingActor) && (NX!=PrevX || NY!=PrevY))                     //aggiunge solo celle valide e diverse dalla precedente
                    {
                        Options.Add({NX,NY});
                    }
                }
                if(Options.Num()==0) break;                                                                             //nessuna direzione valida, interrompe
                auto Choice = Options[FMath::RandRange(0, Options.Num()-1)];                        //sceglie una direzione casuale tra quelle disponibili
                PrevX=CurrX; PrevY=CurrY;                                                                               //memorizza la posizione corrente
                CurrX=Choice.Key; CurrY=Choice.Value;                                                                   //muovi alla nuova cella
                Path.Add(Obstacles->GetCellWorldPosition(CurrX, CurrY));                                           //aggiungi al percorso
            }
            if(Path.Num()==MaxSteps)                                                                                    //percorso completo trovato
            {
                UE_LOG(LogTemp, Warning, TEXT("Easy mode: generated full path"));                               //mi restituisce log e path finito 
                return Path;
            }
        }
        return Path;                                                                                                    //restituisce il percorso
    }
    int32 TargetX, TargetY;                                                                                             //non sono entrato nell'if, quindi è la HARDMODE
    if(!Obstacles->GetCellCoordinatesFromWorldPosition(TargetLocation, TargetX, TargetY))                         //converte la posizione target in coordinate griglia
    {
        UE_LOG(LogTemp, Error, TEXT("CalculatePath: Target position invalid!")); 
        return Path;
    }
    int32 StopDistance = CurrentAIMovingActor && CurrentAIMovingActor->ActorHasTag(FName("AISniper")) ? 2 : 1;          //distanza di stop: 2 per sniper (attacco a distanza), 1 per brawler (mischia)
    
    TArray<TArray<bool>> Visited;                                                                                       //prepara strutture per l'algoritmo BFS (visite e predecessori)
    Visited.Init(TArray<bool>(), Obstacles->GridSize);                                                      
    TArray<TArray<TPair<int32,int32>>> Predecessors;                                                                    //l'algoritmo si basa proprio su BFS
    Predecessors.Init(TArray<TPair<int32,int32>>(), Obstacles->GridSize);

    for(int i=0;i<Obstacles->GridSize;i++)
    {
        Visited[i].Init(false, Obstacles->GridSize); 
        Predecessors[i].Init({-1,-1}, Obstacles->GridSize); 
    }
    TArray<TPair<int32,int32>> Queue;                                                                                   //coda per la BFS
    Queue.Add({StartX, StartY});                                                                          //parte dalla posizione iniziale
    Visited[StartX][StartY] = true;                                                                                     //marca come visitata
    
    static const int DirectionsHard[4][2]={{-1,0},{1,0},{0,-1},{0,1}};                                      //direzioni
    bool bFoundPath=false;
    int32 FinalX=StartX, FinalY=StartY;                                                                                 //cella finale del percorso
    while(Queue.Num() && !bFoundPath)                                                                                   //esegue la BFS
    {
        auto Current = Queue[0]; 
        Queue.RemoveAt(0);                                                                                         //prende il primo elemento della coda
        
        for(auto& d : DirectionsHard)                                                                       //esplora tutte le direzioni
        {
            int NX = Current.Key + d[0], NY = Current.Value + d[1];
            if(IsValidCell(NX, NY, CurrentAIMovingActor) && !Visited[NX][NY])                                     //se la cella è valida e non visitata
            {
                Visited[NX][NY] = true;                                                                                 //marca come visitata
                Predecessors[NX][NY] = Current;                                                                         //memorizza il predecessore
                Queue.Add({NX,NY});                                                                       //aggiunge alla coda
                
                if(FMath::Abs(NX - TargetX) + FMath::Abs(NY - TargetY) == StopDistance)                           //controlla se ha raggiunto la distanza di stop dal target
                {
                    FinalX = NX; FinalY = NY;                                                                           //cella finale
                    bFoundPath = true;                                                                                  //percorso trovato
                    break;
                }
            }
        }
    }
    if(bFoundPath)                                                                                                      //se ha trovato un percorso, lo ricostruisce all'indietro
    {                                                                                                                   //mi serve tornare indietro e aggiungere alla coda altrimetni non saprei come ci sono arrivato, saprei solo la destinazione finale, valida si ma non come ci sono arrivato
        int32 X = FinalX, Y = FinalY;
        while(X != StartX || Y != StartY)                                                                               //finché non torna al punto di partenza
        {
            Path.Insert(Obstacles->GetCellWorldPosition(X, Y), 0);                                            //inserisce la posizione all'inizio dell'array
            auto Prev = Predecessors[X][Y]; 
            X = Prev.Key; Y = Prev.Value;                                                                               //aggiorna le coordinate
        }
        if(Path.Num() > MaxSteps) 
            Path.SetNum(MaxSteps);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculatePath: Nessun percorso trovato verso il target!"));
    }

    return Path;                                                                                                        //restituisce il percorso trovato (o vuoto)
}

void AAIMode::AttackIfPossible(AActor* AICharacter)                                                                     //funzione per la gestione dell'attacco delle pedine ai, chiamata sia per sniper che bralwer
{
    if(!AICharacter)return;                                                                                             //controlla se l'actor è valido
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); 
    if(!Obstacles)return;                                                                                               //verifica se la griglia esiste

    int32 AIX, AIY;
    if(!Obstacles->GetCellCoordinatesFromWorldPosition(AICharacter->GetActorLocation(), AIX, AIY))return;
    AActor* TargetUnit = FindNearestPlayerUnit(AICharacter->GetActorLocation());                                        //trova unità player più vicina                                        
    if(!TargetUnit)return;                                                                                              //nessun bersaglio trovato
    
    int32 TargetX, TargetY;
    if(!Obstacles->GetCellCoordinatesFromWorldPosition(TargetUnit->GetActorLocation(), TargetX, TargetY))return;  //ottiene coordinate bersaglio
    int32 Distance = FMath::Abs(AIX - TargetX) + FMath::Abs(AIY - TargetY);                                       //calcola distanza manhattan 

    FString AIName = AICharacter->ActorHasTag(FName("AISniper")) ? TEXT("S") : TEXT("B");
    FString AICell = FString::Printf(TEXT("%c%d"), 'A' + AIY, AIX + 1);
    FString TargetCell = FString::Printf(TEXT("%c%d"), 'A' + TargetY, TargetX + 1);

    if(AICharacter->ActorHasTag(FName("AIBrawler")) && Distance <= 1)                                                   //controlla se brawler in range
    {
        int32 Damage = FMath::RandRange(1,6);                                                                  //calcola danno randomico
        ULateralPanelWidget* PanelWidget = GetLateralPanelWidget();                                                     
        if(PanelWidget)                                                                                                 //mostra messaggio attacco
        {
            FString Message = FString::Printf(TEXT("AI: B %s -> %s %d danni"), *AICell, *TargetCell, Damage);       //formato messaggio
            PanelWidget->AddGameMessage(Message, FLinearColor::Red);
        }
        AMyPlayerController* PC = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(),0)); 
        if(PC)                                                                                                          //gestisce danno
        {
            if(TargetUnit->ActorHasTag(FName("Sniper")))OnPlayerSniperDamaged.Broadcast(Damage);
            else if(TargetUnit->ActorHasTag(FName("Brawler")))OnPlayerBrawlerDamaged.Broadcast(Damage);                 //in base al tag
            PC->HandlePlayerCounterAttack(AICharacter, TargetUnit);                                     //gestisce contrattacco
        }
    }
    else if(AICharacter->ActorHasTag(FName("AISniper")) && Distance <= 10)                                              //controlla range per sniper
    {
        int32 Damage = FMath::RandRange(4,8);                                                                  //calcola danno random
        ULateralPanelWidget* PanelWidget = GetLateralPanelWidget();                                                    
        if(PanelWidget)                                                                                                 //mostra messaggio attacco
        {
            FString Message = FString::Printf(TEXT("AI: S %s -> %s %d danni"), *AICell, *TargetCell, Damage);
            PanelWidget->AddGameMessage(Message, FLinearColor::Red);
        }
        AMyPlayerController* PC = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(),0));
        if(PC)                                                                                                          //gestisce danno
        {
            if(TargetUnit->ActorHasTag(FName("Brawler")))OnPlayerBrawlerDamaged.Broadcast(Damage);
            else if(TargetUnit->ActorHasTag(FName("Sniper")))OnPlayerSniperDamaged.Broadcast(Damage);                   //delegati
            PC->HandlePlayerCounterAttack(AICharacter, TargetUnit);                                     //gestisce contrattacco
        }   
    }
}
    
bool AAIMode::IsCellOccupiedByOTHERS(int32 X, int32 Y, AActor* CallingActor)                                            //l'obiettivo della funzione è vedere se la cella nel path che sta calcolando la funzione sia oocupata da altre pedine
{
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottieni riferimento alla griglia degli ostacoli per verificare le posizioni
    if(!Obstacles)                                                                                                      //se non trova la griglia restituisce falso
    {
        UE_LOG(LogTemp, Warning, TEXT("IsCellOccupiedByOTHERS: Obstacles non trovato!")); 
        return false;
    }
    TArray<AActor*> AllUnits;                                                                                           //prepara array per raccogliere tutte le unità presenti nel livello
    TArray<AActor*> Temp;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("AIBrawler"), Temp);                                 //popola l'array con tutte le pedine AI brawler
    AllUnits.Append(Temp);
    Temp.Empty();
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("AISniper"), Temp);                                  //popola l'array con tutte le pedine AI sniper
    AllUnits.Append(Temp); 
    Temp.Empty();
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Brawler"), Temp);                                   //popola l'array con tutte le pedine player brawler
    AllUnits.Append(Temp); 
    Temp.Empty();
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Sniper"), Temp);                                    //popola l'array con tutte le pedine player sniper
    AllUnits.Append(Temp);
    
    for(AActor* Unit : AllUnits)                                                                                        //scansiona tutte le unità trovate
    {
        if(Unit && Unit != CallingActor)                                                                                //ignora se è l'unità chiamante o è null
        {
            int32 UX, UY;
            if(Obstacles->GetCellCoordinatesFromWorldPosition(Unit->GetActorLocation(), UX, UY) &&      //converte posizione world in coordinate griglia e controlla se occupa la cella target
               UX == X && UY == Y) 
            {
                UE_LOG(LogTemp, Warning, TEXT("Cella [%d,%d] occupata da %s (chiamante: %s)"), 
                    X, Y, *Unit->GetName(), CallingActor ? *CallingActor->GetName() : TEXT("None"));                    //lo stampa in in log
                return true;                                                                                            //cella occupata
            }
        }
    }
    return false;                                                                                                       //cella libera, perfetto questa cella può contribuire al path
}

void AAIMode::StartAIMovement(AActor* AICharacter, TArray<FVector>& Path, TFunction<void()> OnMovementCompleted)        //mi gestisce i movimenti delle pedine ai
{
    if(!AICharacter || Path.Num() == 0)                                                                                 //controlla parametri di input
    {
        UE_LOG(LogTemp, Warning, TEXT("StartAIMovement: Parametri non validi!"));
        OnMovementCompleted();                                                                                          //chiama il callback comunque
        return;
    }
    CurrentAIMovingActor = AICharacter;                                                                                 //memorizza l'actor che sta per muoversi perche chiamiamo la funzione sia per sniper che brawler
    UE_LOG(LogTemp, Warning, TEXT("StartAIMovement: %s inizia a muoversi"), *AICharacter->GetName());

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); //ottieni riferimento alla griglia per verificare le celle
    for(int32 i = 0; i < Path.Num(); i++)
    {
        int32 CellX, CellY;
        if(Obstacles && Obstacles->GetCellCoordinatesFromWorldPosition(Path[i], CellX, CellY))
        {
            if(IsCellOccupiedByOTHERS(CellX, CellY, AICharacter))                                                       //se trova una cella occupata da pedine o altro, tronca il percorso prima di quella cella
            {
                UE_LOG(LogTemp, Warning, TEXT("StartAIMovement: Cella [%d,%d] occupata, tronco il percorso"), CellX, CellY);
                Path.SetNum(i);
                break;
            }
        }
    }
    if(Path.Num() == 0)                                                                                                 //se il percorso è vuoto dopo la verifica, interrompe
    {
        UE_LOG(LogTemp, Warning, TEXT("StartAIMovement: Nessun percorso valido trovato!"));
        OnMovementCompleted();
        CurrentAIMovingActor = nullptr;                                                                                 //resetta l'actor corrente
        return;
    }
    int32 MaxSteps = AICharacter->ActorHasTag(FName("AIBrawler")) ? 6 : 3;                                              //determina il numero massimo di passi in base al tipo di unità
    if(Path.Num() > MaxSteps) 
        Path.SetNum(MaxSteps);                                                                                          //tronca se il percorso supera max passi
    
    FVector FinalDestination = Path.Last();                                                                             //verifica la cella di destinazione finale
    int32 FinalX, FinalY;
    if(Obstacles && Obstacles->GetCellCoordinatesFromWorldPosition(FinalDestination, FinalX, FinalY))
    {
        if(IsCellOccupiedByOTHERS(FinalX, FinalY, AICharacter))                                                         //se la cella finale è occupata, interrompe
        {
            UE_LOG(LogTemp, Warning, TEXT("StartAIMovement: Cella di destinazione [%d,%d] occupata!"), FinalX, FinalY);
            OnMovementCompleted();
            CurrentAIMovingActor = nullptr;
            return;
        }
    }
    ShowAIPath(Path);                                                                                                   //mostra il percorso sulla griglia
    FTimerHandle TimerHandle;                                                                                           //imposta un timer per iniziare il movimento dopo un breve ritardo
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AICharacter, Path, OnMovementCompleted]()
    {
        ClearAIPath();                                                                                                  //rimuove l'evidenziazione
        MoveAICharacterStep(AICharacter, Path, 0, AICharacter->GetActorLocation(), [this, OnMovementCompleted]()//avvia il movimento passo-passo
        {
            CurrentAIMovingActor = nullptr; //resetta l'actor corrente
            OnMovementCompleted();                                                                                      //termine movimento                                                   
        });
    }, 1.5f, false);                                                                                       //ritardo di 1.5 secondi per estetica
}

void AAIMode::ShowAIPath(const TArray<FVector>& Path)                                                                   //come illuminiamo le celle?
{
    AIHighlightedCells.Empty();                                                                                         //reset lista evidenziate

    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass())); 
    AGridManagerCPP* GridManager = Cast<AGridManagerCPP>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerCPP::StaticClass())); //ottiene grid manager, ESSENZIALE stiamo cambiano le SUE celle
    if(!Obstacles || !GridManager)                                                                                      //controllo validità
    {
        UE_LOG(LogTemp, Error, TEXT("ShowAIPath: Obstacles o GridManager non trovato!")); 
        return; 
    }
    for(const FVector& CellPos : Path)                                                                                  //itera sul percorso
    {
        int32 X, Y;
        if(Obstacles->GetCellCoordinatesFromWorldPosition(CellPos, X, Y))                                         //ottiene coordinate
        {
            AIHighlightedCells.Add({X, Y});                                                               //aggiunge a evidenziate
            GridManager->HighlightCell(X, Y, true);                                                             //evidenzia cella
        }
    }
}

void AAIMode::ClearAIPath()                                                                                             //le fa tornare come prima
{
    AGridManagerCPP* GridManager = Cast<AGridManagerCPP>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerCPP::StaticClass())); 
    if(!GridManager) 
    {
        UE_LOG(LogTemp, Error, TEXT("ClearAIPath: GridManager non trovato!")); 
        return;
    }
    for(auto& Cell : AIHighlightedCells)                                                                    //itera celle evidenziate
    {
        GridManager->HighlightCell(Cell.Key, Cell.Value, false);                                          //rimuove evidenziazione
    }
    AIHighlightedCells.Empty();                                                                                         //svuota lista
}

void AAIMode::MoveAICharacterStep(
    AActor* AICharacter, 
    const TArray<FVector>& Path, 
    int32 CurrentStep, 
    const FVector& OriginalLocation, 
    TFunction<void()> OnMovementCompleted)
{
    if (!AICharacter || CurrentStep >= Path.Num())                                                                      //controlla se ha completato il percorso o se l'actor è nullo
    {
        if (AICharacter)                                                                                                //se l'actor esiste ancora
        {
            ULateralPanelWidget* LateralPanelWidgetInstance = GetLateralPanelWidget();                                  //ottieni il widget per mostrare i messaggi
            if (LateralPanelWidgetInstance)
            {
                ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(
                    UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
                
                if (Obstacles)
                {
                    int32 StartColumn, StartRow, EndColumn, EndRow;                                                     //converte le posizioni in coordinate griglia
                    if (Obstacles->GetCellCoordinatesFromWorldPosition(OriginalLocation, StartColumn, StartRow) &&
                        Obstacles->GetCellCoordinatesFromWorldPosition(Path.Last(), EndColumn, EndRow))
                    {
                        FString AIName = AICharacter->ActorHasTag(FName("AISniper")) ? TEXT("S") : TEXT("B");           //determina il tipo di unità (S per sniper, B per brawler) che si muove
                        FString Message = FString::Printf(
                            TEXT("AI: %s %c%d -> %c%d"),
                            *AIName,
                            'A' + StartRow, StartColumn + 1,
                            'A' + EndRow, EndColumn + 1);                                                               //crea messaggio di movimento (es: "AI: B A1 -> B3")
                        LateralPanelWidgetInstance->AddGameMessage(Message, FLinearColor::Red);                         //stampa il messaggio nel widget
                    }
                }
            }
        }
        OnMovementCompleted();                                                                                          //termine movimento ed esce
        return;
    }
    
    FVector TargetLocation = Path[CurrentStep];                                                                         //calcola il movimento verso la prossima cella
    FVector CurrentLocation = AICharacter->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);                                                    //distanza dalla cella target
    float MovementSpeed = 600.0f;                                                                                       //velocità di movimento, estetica, alza o abbasso 
    
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * GetWorld()->GetDeltaSeconds();                  //calcola la nuova posizione
    AICharacter->SetActorLocation(NewLocation);
    if (Distance < 5.0f)                                                                                                //se è vicino alla cella target (entro 5 unità)
    {   
        ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));    //verifica la prossima cella prima di muoversi
        if(Obstacles && CurrentStep+1 < Path.Num())                                                                     //se c'è una prossima cella
        {
            int32 NextX, NextY;
            if(Obstacles->GetCellCoordinatesFromWorldPosition(Path[CurrentStep+1], NextX, NextY))
            {
                if(IsCellOccupiedByOTHERS(NextX, NextY, AICharacter))                                                   //se la prossima cella è occupata, interrompe il movimento
                {
                    OnMovementCompleted();
                    return;
                }
            }
        }
        CurrentStep++;                                                                                                  //passa alla prossima cella del percorso
    }
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, AICharacter, Path, CurrentStep, OriginalLocation, OnMovementCompleted]()
    {
        MoveAICharacterStep(AICharacter, Path, CurrentStep, OriginalLocation, OnMovementCompleted);                  //programma il prossimo frame di movimento
    });
}

bool AAIMode::IsCellFree(int32 X, int32 Y)                                                                              //controlla se cella è libera, funzione necessaria per il movimento
{
    ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
    if (!Obstacles)                                                                                                     //se non trovati
    {
        return false;                                                                                                   //ritorna falso
    }

    FCellInfo CellInfo = Obstacles->GetCellInfo(X, Y);                                                                  //ottiene info cella
    if (CellInfo.bIsObstacle)                                                                                           //se è ostacolo
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella [%d, %d] è un ostacolo!"), X, Y);                                  //log ostacolo
        return false;                                                                                                   //ritorna falso
    }

    FVector CellWorldPosition = Obstacles->GetCellWorldPosition(X, Y);                                                  //ottiene posizione mondo
    FHitResult HitResult;                                                                                               //risultato collisione
    FCollisionQueryParams CollisionParams;                                                                              //parametri collisione

    TArray<AActor*> AIBrawlers;                                                                                         //array 
    TArray<AActor*> AISnipers;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("AIBrawler"), AIBrawlers);                           //riempie array brawlers
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("AISniper"), AISnipers);                             //riempie array snipers
    for (AActor* AIBrawler : AIBrawlers)                                                                                //per ogni brawler
    {
        CollisionParams.AddIgnoredActor(AIBrawler);                                                                     //ignora in collisione
    }
    for (AActor* AISniper : AISnipers)                                                                                  //per ogni sniper
    {
        CollisionParams.AddIgnoredActor(AISniper);                                                                      //ignora in collisione
    }
    bool bIsOccupied = GetWorld()->LineTraceSingleByChannel(                                                            //esegue trace, verifichiamo se cella sia occupata
        HitResult,
        CellWorldPosition,
        CellWorldPosition + FVector(0, 0, 1),                                                               //trace verticale
        ECC_Visibility,
        CollisionParams
    );

    if (bIsOccupied)                                                                                                    //se lo è me ll dice
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella [%d, %d] è occupata da: %s"), X, Y, *HitResult.GetActor()->GetName());
    }
    return !bIsOccupied;                                                                                                //ritorna se libera
}

AActor* AAIMode::FindNearestPlayerUnit(const FVector& AILocation)                                                       //funzione essenziale per trovare la pedina del player più vicina a se stessa, aggiornata ad ogni turno così può vedere se nel precedente le pedine sono cambiate in posizione
{
    TArray<AActor*> PlayerUnits;                                                                                        //array unità player
    TArray<AActor*> Brawlers;                                                                                           //array brawlers
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Brawler"), Brawlers);                               //riempie array
    PlayerUnits.Append(Brawlers);                                                                                       //aggiunge brawlers

    TArray<AActor*> Snipers;                                                                                            //array snipers
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Sniper"), Snipers);                                 //riempie array
    PlayerUnits.Append(Snipers);                                                                                        //aggiunge snipers

    AActor* NearestUnit = nullptr;                                                                                      //inizializza l'unità più vicina
    float NearestDistance = FLT_MAX;                                                                                    //distanza minima

    for (AActor* PlayerUnit : PlayerUnits)                                                                              //per ogni unità
    {
        if (!PlayerUnit)                                                                                                //se non la trova
        {
            continue;                                                                                                   //salta
        }
        float Distance = FVector::Dist(AILocation, PlayerUnit->GetActorLocation());                                     //calcola distanza
        if (Distance < NearestDistance)                                                                                 //se più vicina
        {
            NearestDistance = Distance;                                                                                 //aggiorna distanza
            NearestUnit = PlayerUnit;                                                                                   //aggiorna unità
        }
    }
    return NearestUnit;                                                                                                 //ritorna unità trovata
}

void AAIMode::ApplyDamageToAIBrawler(int32 Damage)                                                                      //devo qui gestire il danno che le pedine dell'ai subiscono dal nemico
{
    ULateralPanelWidget* LateralPanelWidgetInstance = GetLateralPanelWidget();                                          //ottiene il widget del pannello laterale

    AIBrawlerHealth -= Damage;                                                                                          //applica il danno alla vita
    AIBrawlerHealth = FMath::Max(AIBrawlerHealth, 0.0f);                                                          //mi assicuro che la vita non sia negativa

    UE_LOG(LogTemp, Warning, TEXT("[ApplyDamageToAIBrawler] Vita rimanente: %f"), AIBrawlerHealth);             //log mentre progettavo

    OnAIBrawlerHealthChanged.Broadcast(AIBrawlerHealth / 40.0f);                                                        //notifica il cambiamento di vita, delegato

    if (AIBrawlerHealth <= 0)                                                                                           //gestiamo cosa succede se la vita del brawler scende sotto lo 0 (muore)
    {
        LateralPanelWidgetInstance->AddGameMessage("AI: L' AIBrawler e' stato eliminato", FColor::Black);    //mostra messaggio di eliminazione nella history box

        UE_LOG(LogTemp, Warning, TEXT("AIBrawler è morto!"));
        if (SelectedBrawler)                                                                                            //se il brawler esiste
        {
            SelectedBrawler->Destroy();                                                                                 //distrugge l'actor
            SelectedBrawler = nullptr;                                                                                  //azzera il puntatore
            CheckVictoryCondition();                                                                                    //controlla le condizioni di vittoria
        }
    }
}

void AAIMode::ApplyDamageToAISniper(int32 Damage)                                                                       //idem per sniper
{    
    ULateralPanelWidget* LateralPanelWidgetInstance = GetLateralPanelWidget();

    AISniperHealth -= Damage;
    AISniperHealth = FMath::Max(AISniperHealth, 0.0f);
    OnAISniperHealthChanged.Broadcast(AISniperHealth / 20.0f);

    if (AISniperHealth <= 0)
    {
        LateralPanelWidgetInstance->AddGameMessage("AI: L' AISniper e' stato eliminato", FColor::Black);

        UE_LOG(LogTemp, Warning, TEXT("AISniper è morto!"));
        if (SelectedSniper)
        {
            SelectedSniper->Destroy();
            SelectedSniper = nullptr;
            CheckVictoryCondition();
        }
    }
}

void AAIMode::CheckVictoryCondition()                                                                                   //dobbiamo verifcare in quale condizione decretare la vittoria dell'ai o del player
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PlayerController)
    {
        return;                                                                                                         //se non c'è un PlayerController valido, esci, mi serve il riferimento
    }
    bool bAllAIDead = (AIBrawlerHealth <= 0 && AISniperHealth <= 0);                                                    //verifica se tutte le pedine dell'AI sono morte
    
    bool bBrawlerAlive = PlayerController->IsBrawlerAlive();                                                            //verifica lo stato delle pedine del giocatore
    bool bSniperAlive = PlayerController->IsSniperAlive();

    if (bAllAIDead && (bBrawlerAlive || bSniperAlive))                                                                  //se sono morte entrambe le pedine dell'ai vince il player
    {
        UE_LOG(LogTemp, Warning, TEXT("Il player ha vinto! Entrambe le pedine AI sono state sconfitte."));
        
        PlayerController->SetPause(true);                                                                               //pausa la partita

        OnPlayerVictory.Broadcast();                                                                                    //delegato broadcast
    }
}

ULateralPanelWidget* AAIMode::GetLateralPanelWidget() const
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());      //ottiene il player controller
    if (PlayerController)                                                                                               //se il player controller esiste
    {
        ULateralPanelWidget* Widget = PlayerController->GetLateralPanelWidget();                                        //ottiene il widget dal controller
        if (Widget)                                                                                                     //se il widget esiste
        {
            return Widget;                                                                                              //ritorna il widget
        }
    }
    return nullptr;
}

void AAIMode::CounterAttack(AActor* Attacker, AActor* Defender)                                                         //invece questa gestisce il contrattacco dell'ai
{
    if (!Attacker || !Defender)                                                                                         //controlla validità attaccante e difensore
    {
        return;                                                                                                         //se non lo sono esce
    }
    bool bIsBrawler = Defender->ActorHasTag(FName("AIBrawler"));                                                        //bool per sapere se difensore è brawler
    bool bIsSniper = Defender->ActorHasTag(FName("AISniper"));                                                          //bool per sapere se difensore è sniper
    if (!bIsBrawler && !bIsSniper)
    {
        return;
    }
    if (Attacker->ActorHasTag(FName("Brawler")))                                                                        //se attaccante è brawler
    {
        ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));
        if (Obstacles)
        {
            int32 AttackerX, AttackerY, DefenderX, DefenderY;
            if (Obstacles->GetCellCoordinatesFromWorldPosition(Attacker->GetActorLocation(), AttackerX, AttackerY) &&
                Obstacles->GetCellCoordinatesFromWorldPosition(Defender->GetActorLocation(), DefenderX, DefenderY))
            {
                int32 ManhattanDistance = FMath::Abs(AttackerX - DefenderX) + FMath::Abs(AttackerY - DefenderY);
                if (ManhattanDistance > 1)                                                                              //se il brawler non è adiacente non può contrattaccare così come non può attaccare
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
    }

    int32 Damage = FMath::RandRange(1, 3);                                                                     //genera danno casuale da 1 a 3, vedi specifiche
    UE_LOG(LogTemp, Warning, TEXT("%s contrattacca %s con %d danni!"), *Defender->GetName(), *Attacker->GetName(), Damage);

    if (Attacker->ActorHasTag(FName("Brawler")))                                                                        //se attaccante è brawler
    {
        OnPlayerBrawlerDamaged.Broadcast(Damage);                                                                       //notifica danno broadc
        AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
        if (PlayerController)
        {
            PlayerController->CheckPlayerBrawlerHealth();                                                               //controlla vita
        }
    }
    else if (Attacker->ActorHasTag(FName("Sniper")))                                                                    //se attaccante è sniper
    {
        OnPlayerSniperDamaged.Broadcast(Damage);
        AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
        if (PlayerController)
        {
            PlayerController->CheckPlayerSniperHealth();                                                                //controlla vita
        }
    }
}

void AAIMode::HandleAICounterAttack(AActor* Attacker, AActor* Defender)                                                 //sfruttata dal playercontroller
{
    if (!Attacker || !Defender)                                                                                         //controlla validità attori
    {
        UE_LOG(LogTemp, Error, TEXT("HandleAICounterAttack: Attacker o Defender sono nulli!"));
        return;
    }
    if (Defender->ActorHasTag(FName("AISniper")))                                                                       //se difensore è sniper
    {
        int32 CounterDamage = FMath::RandRange(1, 3);                                                          //genera danno casuale
        UE_LOG(LogTemp, Warning, TEXT("AISniper contrattacca %s con %d danni!"), *Attacker->GetName(), CounterDamage);

        if (Attacker->ActorHasTag(FName("Brawler")))                                                                    //applica danno a brawler
            OnPlayerBrawlerDamaged.Broadcast(CounterDamage);
        else if (Attacker->ActorHasTag(FName("Sniper")))                                                                //applica danno a sniper
            OnPlayerSniperDamaged.Broadcast(CounterDamage);
        if (ULateralPanelWidget* Panel = GetLateralPanelWidget())                                                       //ottiene widget
        {
            FString Message = FString::Printf(TEXT("Contrattacco AI: S -> %s %d danni"),
                Attacker->ActorHasTag(FName("Brawler")) ? TEXT("B") : TEXT("S"),
                CounterDamage);                                                                                         //crea messaggio
            Panel->AddGameMessage(Message, FLinearColor::Red);                                                          //mostra messaggio
        }
        return;
    }
    if (Defender->ActorHasTag(FName("AIBrawler")))                                                                      //se difensore è brawler
    {
        ABP_Obstacles* Obstacles = Cast<ABP_Obstacles>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ABP_Obstacles::StaticClass()));                           //ottiene ostacoli
        int32 AX, AY, PX, PY;
        Obstacles->GetCellCoordinatesFromWorldPosition(Defender->GetActorLocation(), AX, AY);           //coordinate difensore
        Obstacles->GetCellCoordinatesFromWorldPosition(Attacker->GetActorLocation(), PX, PY);           //coordinate attaccante
        int32 Dist = FMath::Abs(AX - PX) + FMath::Abs(AY - PY);                                                   //calcola distanza
        if (Dist <= 1)                                                                                                  //se adiacente, allora può contrattaccare
        {
            int32 CounterDamage = FMath::RandRange(1, 3);
            UE_LOG(LogTemp, Warning, TEXT("AIBrawler contrattacca %s con %d danni!"), *Attacker->GetName(), CounterDamage);
            if (Attacker->ActorHasTag(FName("Brawler")))                                                                //applica danno a brawler
                OnPlayerBrawlerDamaged.Broadcast(CounterDamage);
            else if (Attacker->ActorHasTag(FName("Sniper")))                                                            //applica danno a sniper
                OnPlayerSniperDamaged.Broadcast(CounterDamage);
            if (ULateralPanelWidget* Panel = GetLateralPanelWidget())                                                   //ottiene widget
            {
                FString Message = FString::Printf(TEXT("Contrattacco AI: B -> %s %d danni"),
                    Attacker->ActorHasTag(FName("Brawler")) ? TEXT("B") : TEXT("S"),
                    CounterDamage);                                                                                     //crea messaggio per il contrattacco
                Panel->AddGameMessage(Message, FLinearColor::Red);                                                      //mostra messaggio
            }
        }
    }
}

void AAIMode::SetEasyMode(bool bEasy)                                                                                   //imposta modalità facile
{
    bEasyMode = bEasy;
    UE_LOG(LogTemp, Warning, TEXT("AAIMode::SetEasyMode: bEasyMode impostato su %s"), bEasy ? TEXT("true") : TEXT("false"));
}

bool AAIMode::IsEasyMode() const                                                                                        //controlla se lo sia modalità facile
{
    return bEasyMode;
}