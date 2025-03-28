#include "../headers/GridManagerCPP.h"
#include "Camera/CameraActor.h"   
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Components/LightComponent.h"  
#include "Engine/DirectionalLight.h" 
#include "EngineUtils.h"

AGridManagerCPP::AGridManagerCPP()                                                                                      //costruttore nel quale dichiaro le variabili di base da usare nel codice                                                                              
{
    PrimaryActorTick.bCanEverTick = true;
    GridSize = 25;                                                                                                      //specifiche
    TileSize = 104.0f;                                                                                                  //per grafica e riallineamento con planes nel blueprint
}

void AGridManagerCPP::BeginPlay()
{
    Super::BeginPlay();
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetName().Contains("Floor"))
        {
            UStaticMeshComponent* Mesh = Actor->FindComponentByClass<UStaticMeshComponent>();                           //cerca il floor e vi assegna la mesh
            if (Mesh)                                                                                                   //uso LEGNO, l'ho importato io scaricandolo da internet da store della epic
            {
                UMaterialInterface* LegnoMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Blueprints/LEGNO"));
                if (LegnoMaterial)
                {
                    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(LegnoMaterial, this); //Crea un materiale dinamico istanza per modificare i parametri
                    DynamicMaterial->SetScalarParameterValue(FName("Specular"), 0.0f);                            //imposta lo specular a 0
                    Mesh->SetMaterial(0, DynamicMaterial);                                                   //applica il materiale modificato

                }
            }
        }
    }
    
    FActorSpawnParameters SpawnParams;                                                                                  //inizia la generazione dinamica della telecamera in base alla posizione della griglia
    float CameraX = (GridSize * TileSize) / 2.0f;                                                                       //la centra tramite X ed Y
    float CameraY = (GridSize * TileSize) / 2.0f; 
    FVector CameraPosition = FVector(CameraX, CameraY-20.0f, 2500.0f);                                              //altezza bloccata in alto. deve essere gioco 2D spefiche, non visuale distorta

   
    ACameraActor* TopDownCamera = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(), CameraPosition, FRotator(-90, 0, 0), SpawnParams);         //angolazione per la telecamera che punta verso il basso

    if (TopDownCamera)                                                                                                  //questo obbliga la telecamera a diventare il punto di vista principale del gioco
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PlayerController)                                                                                           //gli if controllano che siano collegati e reperibili 
        {
            PlayerController->SetViewTarget(TopDownCamera);
        }
    }

    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)                                                    //per evitare conflitti con l'engine preferisco annullare qualsiasi luce che venga posta da esso
    {                                                                                                                   //così da impostarne io tutti i valori
        (*It)->Destroy();
    }

    ADirectionalLight* DirectionalLight = GetWorld()->SpawnActor<ADirectionalLight>(
        ADirectionalLight::StaticClass(), FVector(1000, 3000, 1000), FRotator(-90, 0, 0), SpawnParams);

    if (DirectionalLight)                                                                       
    {
        ULightComponent* LightComp = DirectionalLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(3.0f);                                                                              //intensità luce
            LightComp->SetLightColor(FLinearColor(0.9f, 0.8f, 0.9f));                                    //colore della luce
            LightComp->SetCastShadows(false);                                                                 //no alle ombre, un minimo spessore può far variare un minimo la percezione visiva delle forme
        }
    }

    GenerateGrid();                                                                                                     //ora che tutto è impostato il beginplay chiama la cosa principale: generazione della griglia
}

void AGridManagerCPP::GenerateGrid()                                                                                    //chiamata alla funzione base
{
    if (!TileBlueprintClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TileBlueprintClass is not set!"));                                         //log unreal per verificare che il collegamento sia avvenuto
        return;
    }
    GridTiles.Empty();                                                                                                  //svuota l'array bidimensionale

    for (int32 x = 0; x < GridSize; ++x)                                                                                //doppio for per scannare ogni cella della griglia 25x25
    {
        TArray<AActor*> Column;
        for (int32 y = 0; y < GridSize; ++y)
        {
            FVector TilePosition = FVector(                                                                             //vettore per la posizione delle singole celle nello spazio: X Y Z
                x * TileSize,
                y * TileSize,
                0.0f 
            );
                        
            FString RowLabel = FString::Chr('A' + y);                                                               //codice per consentirmi di programmare facilmente la griglia tramite due for con int
            UE_LOG(LogTemp, Warning, TEXT("Tile %d%s position: %s"), x + 1, *RowLabel, *TilePosition.ToString());   
                                                                                                                        //ma comunque di chiamare una cella A1, Y25
                                                                                                                        //ricordiamo che da specifiche l'asse orizzontale della griglia deve corrispondere alle lettere mentre quello verticale ai numeri
                                                                                                                        //prima cella in basso a sinistra A1
            AActor* NewTile = GetWorld()->SpawnActor<AActor>(TileBlueprintClass, TilePosition, FRotator::ZeroRotator);
                                                                                                                        
            if (NewTile)                                                                                                //aggiunta fisicamente e logicamente delle singole celle per comporre la griglia
            {
                Column.Add(NewTile);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn tile at position: %s"), *TilePosition.ToString());
            }
        }
        GridTiles.Add(Column);
    }
}

    
void AGridManagerCPP::HighlightCell(int32 X, int32 Y, bool bHighlight)                                                  //funzione che se chiamata dal playercontroller tramite click destro
{                                                                                                                       //cambia materiale delle celle raggiungibili da 6 passi per brawler e 3 per sniper
    if (!GridTiles.IsValidIndex(X) || !GridTiles[X].IsValidIndex(Y))
    {
        return;                                                                                                         //verifica che le coordinate siano valide
    }

    AActor* TileActor = GridTiles[X][Y];                                                                                //coordinate 
    if (!TileActor) return;

    UStaticMeshComponent* Mesh = TileActor->FindComponentByClass<UStaticMeshComponent>();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tile senza mesh a [%d,%d]"), X, Y);
        return;
    }

    UMaterialInterface* NewMat = bHighlight ? HighlightTileMaterial : DefaultTileMaterial;                              //qua si imposta il materiale corretto per l'evidenziazione delle celle quando e se richiesto
    if (NewMat)
    {
        Mesh->SetMaterial(0, NewMat);
        UE_LOG(LogTemp, Warning, TEXT("Tile [%d,%d] %s"), X, Y, bHighlight ? TEXT("evidenziato") : TEXT("resettato"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HighlightTileMaterial o DefaultTileMaterial non assegnati in GridManager"));
    }
}