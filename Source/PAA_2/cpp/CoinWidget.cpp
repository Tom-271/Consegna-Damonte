#include "../headers/CoinWidget.h"
#include "../headers/MyGameMode.h"
#include "../headers/BP_Obstacles.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"  
#include "Components/Border.h"
#include "../headers/AIMode.h"
#include "Components/Slider.h" 
#include "Kismet/GameplayStatics.h"

void UCoinWidget::SetGameMode(AMyGameMode* GameMode)
{
    GameModeRef = GameMode;
}

void UCoinWidget::NativeConstruct()                                                                                     
{
    Super::NativeConstruct();

    if (EasyModeButton)
    {
        EasyModeButton->OnClicked.AddDynamic(this, &UCoinWidget::HandleEasyModeButtonClicked);
        UE_LOG(LogTemp, Warning, TEXT("EasyModeButton collegato correttamente"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EasyModeButton non trovato!"));
    }

    if (HardModeButton)
    {
        HardModeButton->OnClicked.AddDynamic(this, &UCoinWidget::HandleHardModeButtonClicked);
        UE_LOG(LogTemp, Warning, TEXT("HardModeButton collegato correttamente"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HardModeButton non trovato!"));
    }

    UpdateModeButtonColors(false);

    // Imposta Hard Mode come default all'avvio
    if (AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass())))
    {
        AIMode->SetEasyMode(false); // Force Hard Mode
    }
    
    bMenuAperto = false;                                                                                                //assegno false alle due variabili perchè nella versione iniziale una è chiusa e la moneta non ancora lanciata
    bMonetaLanciata = false;

    if (ObstacleProbabilitySlider)                                                                                      //verifichiamo che sia collegato lo slider grazie al quale stabiliremo la prob di spawn
    {                                                                                                                   //degli ostacoli
        UE_LOG(LogTemp, Warning, TEXT("Slider trovato nel Blueprint"));

        ObstacleProbabilitySlider->SetValue(0.3f);                                                                      //impostiamo un valore di default cosicchè anche se il giocatore decidesse di non usare lo slider comunque 
        ObstacleProbabilitySlider->SetMaxValue(0.99f);                                                                  //lo imposto a 99% come valore massimo perchè con una griglia di soli ostacoli posso fare poco...
        ObstacleProbabilitySlider->OnValueChanged.AddDynamic(this, &UCoinWidget::OnObstacleProbabilitySliderChanged);   //impostiamone il movimento

        ApplySliderStyle();                                                                                             //funzione esterna per gestirne qualche piccolo aspetto estetico in c++
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ObstacleProbabilitySlider non trovato nel Blueprint!"));
    }

   

    if (MonetaButton)                                                                                                   //verifichiamo collegamento moneta
    {
        MonetaButton->SetIsEnabled(true);                                                                               //lo abilitiamo a true, è il primo da usare, prima moneta poi si starta
        MonetaButton->SetVisibility(ESlateVisibility::Visible);                                                         //ne forziamo la visibilità 
        MonetaButton->OnClicked.AddDynamic(this, &UCoinWidget::OnMonetaButtonClicked);                                  //click sul pulsante stesso
    }

    if (StartGameButton)
    {
        StartGameButton->SetIsEnabled(false);
        StartGameButton->OnClicked.AddDynamic(this, &UCoinWidget::OnStartGameButtonClicked);                            //idem
    }

    if (BorderTesta) BorderTesta->SetVisibility(ESlateVisibility::Hidden);                                              //definisco i due border su hidden perchè devono entrambi essere hidden e poi 
    if (BorderCroce) BorderCroce->SetVisibility(ESlateVisibility::Hidden);                                              //diventare visible quando estraggo il valore

    if (MenuTendina) MenuTendina->SetVisibility(ESlateVisibility::Hidden);

    if (InformazioniButton)
    {
        InformazioniButton->SetIsEnabled(true);
        InformazioniButton->OnClicked.AddDynamic(this, &UCoinWidget::OnInformazioniButtonClicked);                      //pulsante per le regole di gioco che fa scomparire e comparire border relativo
    }
}
void UCoinWidget::NativeDestruct()
{
    Super::NativeDestruct();
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateGridTimer);
    }
}
void UCoinWidget::OnObstacleProbabilitySliderChanged(float Value)
{
    if (!ObstacleProbabilitySlider || !ObstacleProbabilityLabel)
    {
        UE_LOG(LogTemp, Error, TEXT("ObstacleProbabilitySlider o ObstacleProbabilityLabel è nullptr!"));
        return;
    }
    
    int32 PercentValue = FMath::RoundToInt(Value * 100);
    ObstacleProbabilityLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));

    FLinearColor TextColor(Value, 0.0f, 0.0f, 1.0f);
    ObstacleProbabilityLabel->SetColorAndOpacity(FSlateColor(TextColor));

    GetWorld()->GetTimerManager().ClearTimer(UpdateGridTimer);                                                       //cancella il timer esistente prima di crearne uno nuovo per evitare chiamate concorrenti

    GetWorld()->GetTimerManager().SetTimer(UpdateGridTimer, this, &UCoinWidget::ApplyObstacleProbability, 0.2f, false);
}

void UCoinWidget::ApplySliderStyle()
{
    if (!ObstacleProbabilitySlider)
        return;

    ObstacleProbabilitySlider->SetSliderBarColor(FLinearColor(1.0f, 0.4f, 0.0f, 1.0f));       //colore dello slider

    ObstacleProbabilitySlider->SetRenderScale(FVector2D(1.2f, 1.2f));

    ObstacleProbabilitySlider->SetSliderHandleColor(FLinearColor::Black);                                        //colore dell'handler dello slider

}


void UCoinWidget::ApplyObstacleProbability()
{
    float Value = ObstacleProbabilitySlider->GetValue();                                                                //impostiamo definitivamente la probabilità per la generazione degli ostacoli
    UE_LOG(LogTemp, Warning, TEXT("Applicazione finale della probabilità: %f"), Value);                         
    SetObstacleProbability(Value);
}

void UCoinWidget::SetObstacleProbability(float Value)
{
    UE_LOG(LogTemp, Warning, TEXT("SetObstacleProbability chiamato con valore: %f"), Value);
    ABP_Obstacles* Obstacles = GetObstacleActor();
    if (IsValid(Obstacles))
    {
        Obstacles->SetSpawnProbability(Value);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Ostacoli non trovati o non validi!"));
    }           
}

ABP_Obstacles* UCoinWidget::GetObstacleActor()                                                                          //restituisce un puntatore all'attore di tipo ABP_Obstacles
{
    UWorld* World = GetOwningPlayer()->GetWorld();                                                                      //ottiene il mondo di gioco dal giocatore
    if (!World)                                                                                                         //controlla se il mondo è valido
    {
        UE_LOG(LogTemp, Error, TEXT("World non trovato in UCoinWidget"));                                       //ottenere il riferimento al word mi serve per interagire direttamente con gli ostacoli, pedine, ecc
        return nullptr; 
    }

    AActor* ObstacleActor = UGameplayStatics::GetActorOfClass(World, ABP_Obstacles::StaticClass());                     //trova il primo attore di tipo ABP_Obstacles nel mondo
    return Cast<ABP_Obstacles>(ObstacleActor);                                                                          //effettua il cast ad ABP_Obstacles e lo restituisce 
}


void UCoinWidget::OnStartGameButtonClicked()
{
    // Log per verificare lo stato della modalità (Easy/Hard) quando il giocatore avvia il gioco
    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));
    if (AIMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnStartGameButtonClicked: bEasyMode = %s"), AIMode->IsEasyMode() ? TEXT("true") : TEXT("false"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnStartGameButtonClicked: AAIMode non trovato!"));
    }
    
    if (GameModeRef)
    {
        GameModeRef->StartGame();                                                                                       //ultimo pulsante prima di iniziare a giocare
    }

    RemoveFromParent();                                                                                                 //lo faccio sparire dopo aver premuto
}

void UCoinWidget::UpdateModeButtonColors(bool bEasySelected)
{
    const FLinearColor ActiveColor(0.0f, 1.0f, 0.0f, 1.0f);    // Rosso
    const FLinearColor InactiveColor(0.5f, 0.5f, 0.5f, 1.0f);  // Grigio

    if (EasyModeButton)
        EasyModeButton->SetBackgroundColor(bEasySelected ? ActiveColor : InactiveColor);

    if (HardModeButton)
        HardModeButton->SetBackgroundColor(bEasySelected ? InactiveColor : ActiveColor);
}

bool UCoinWidget::Coinflip()
{
    bool bIsHead = FMath::RandBool();                                                                                   //frand tra booleani, o uno o l'altro, fosse stato un valore avrei dovuto passare i valori di limite
    UE_LOG(LogTemp, Warning, TEXT("Lancio della moneta: %s"), bIsHead ? TEXT("Testa") : TEXT("Croce"));
    return bIsHead;
}

void UCoinWidget::OnMonetaButtonClicked()                                                                               //il moneta button è il prinmo tra moneta e start button con cui interagire, prima DEVO estrarre il turno di gioco
{                                                                                                                       //e poi devo startare il gioco
    bool bIsHeads = Coinflip();
    MonetaButton->SetVisibility(ESlateVisibility::Hidden);                                                              //se l'ho premuto non mi serve più, lo nascondo
    MonetaButton->SetIsEnabled(false);                                                                                  //e per sicurezza non lo rendo nemmeno interagibile

    AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        GameMode->SetPlayerStarts(bIsHeads);
        if (!bIsHeads)
        {
            UE_LOG(LogTemp, Warning, TEXT("Lancio della moneta: Croce. Avvio del turno dell'AI."));
            GameMode->StartAITurn();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Lancio della moneta: Testa. Avvio del turno del giocatore."));
        }
    }                                                                                                                   //se è testa inizia PLAYER altrimenti AI, ora chiamo startaiturn dalla gamemode, che gestisce lo scheletro dei turni

    if (BorderTesta) BorderTesta->SetVisibility(bIsHeads ? ESlateVisibility::Visible : ESlateVisibility::Hidden);       //in base a se è uscito testa o croce faccio comparire il relativo border
    if (BorderCroce) BorderCroce->SetVisibility(!bIsHeads ? ESlateVisibility::Visible : ESlateVisibility::Hidden);      //banale visualizzazione utile al player

    bMonetaLanciata = true;
    if (StartGameButton)
    {
        StartGameButton->SetIsEnabled(true);                                                                            //ora finalmente abilito il pulsante per l'inizio della partita, non posso startare il gioco se prima non ho estratto colui che inizierà a giocare
    }
}

void UCoinWidget::OnInformazioniButtonClicked()                                                                         //se lo premo compare il border per le regole di gioco, se lo ripremo scompare
{                                                                                                                       //il gioco inizia con quest non visibile
    if (MenuTendina)
    {
        bMenuAperto = !bMenuAperto;
        MenuTendina->SetVisibility(bMenuAperto ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UCoinWidget::OnEasyModeClicked()
{
    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));         //funzioni che gestiscono i click nella coinwidget per la difficolta di gioco
    if (AIMode)
    {
        AIMode->SetEasyMode(true);                                                                                      //collegamento all'aimode per impostare il tracciamento delle pedine disabilitato
    }
    else
    {                                                                                                                   //se è la easymode le pedine dell'ai non si muovono secondo algoritmi specifici ma casualmente
        UE_LOG(LogTemp, Error, TEXT("OnEasyModeClicked: AAIMode non trovato!"));
    }
}

void UCoinWidget::OnHardModeClicked()                                                                                   //idem ma abilita la hardmode e passa il booleano all'aimode
{
    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIMode::StaticClass()));
    if (AIMode)
    {
        AIMode->SetEasyMode(false);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnHardModeClicked: AAIMode non trovato!"));
    }
}


void UCoinWidget::HandleEasyModeButtonClicked()                                                                         //qeuste invece le uso per gestire la mutua esclusività dei colori. di default l'hard mode                                                        
{                                                                                                                       //la voglio attiva, ma se premo sulla easy lo rendo graficamente evidente, il colore rosso passa all'altro
    OnEasyModeClicked();        
    UpdateModeButtonColors(true);
}

void UCoinWidget::HandleHardModeButtonClicked()
{
    OnHardModeClicked();
    UpdateModeButtonColors(false);
}

