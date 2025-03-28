#include "../headers/LateralPanelWidget.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "../headers/MyGameMode.h"
#include "../headers/MyPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ULateralPanelWidget::ULateralPanelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsRotating = false;                                                                                                //assegna valore la variabile di rotazione
    MaxAIBrawlerHealth = 40.0f;                                                                                         //assegna la massima vita delle pedine ai, idem per quelle del player
    MaxAISniperHealth = 20.0f;    
    MaxBrawlerHealth = 40.0f; 
    MaxSniperHealth  = 20.0f;                                                                                           //assegna valore a vite personaggi
}

void ULateralPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    AddGameMessage("LA PARTITA HA INIZIO!", FLinearColor::White);                                               //chiama la funzione che definiremo dopo addgamemessage per mostrare subito la presenza della scrollbox con un messaggio base
    AddGameMessage("----------------------", FLinearColor::White);
    AAIMode* AIMode = Cast<AAIMode>(UGameplayStatics::GetActorOfClass(this, AAIMode::StaticClass()));       
    if (AIMode)
    {
        AIMode->OnAIBrawlerHealthChanged.AddDynamic(this, &ULateralPanelWidget::UpdateAIBrawlerHealth);                 //collega i delegati alla funzione, così da avere un interazioni tra classi diverse
        UE_LOG(LogTemp, Warning, TEXT("Delegato collegato correttamente!"));                                    //i delegati in breve sono mezzi che permettono di appunto delegare il compito di una funzione ad un altra, per esempio li invochiamo nell'aimode o nel playercontroller quando le rispettive pedine prendono danno 
        AIMode->OnAISniperHealthChanged.AddDynamic(this, &ULateralPanelWidget::UpdateAISniperHealth);                   //senza però dover definire i comportamenti della funzione in ogni classe
        UE_LOG(LogTemp, Warning, TEXT("Delegato collegato correttamente!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIMode non trovato!"));
    }
    GameModeRef = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));                          //prende riferimento alla gamemode per far spawnare il brawler o lo sniper in base a quale dei due viene premuto

    if (GameModeRef)
    {
        if (BrawlerButton)                                                                                              //controlla e sia stato trovato il brawlerbutton
        {
            BrawlerButton->SetIsEnabled(!GameModeRef->bHasSpawnedBrawler);                                              //se il brawlwer è già stato chiamato allora non puoi spawnare nuovamente, disabilità funzionalità click
            BrawlerButton->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnBrawlerButtonClicked);                   
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Pulsante BRAWLER non trovato!"));
        }
        if (SniperButton)                                                                                               //controlla sniperbutton se sia collegato 
        {
            SniperButton->SetIsEnabled(!GameModeRef->bHasSpawnedSniper);
            SniperButton->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnSniperButtonClicked);                      //idem per l'altro
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Pulsante SNIPER non trovato!"));
        }

        if (InfoBrawlerButton)                                                                                          //controlliamo collegamento all'infobrawler, QUELLO CHE FA OCMPARIRE INFO BRAWLWER E LO FA ROTEARE
        {
            InfoBrawlerButton->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnInfoBrawlerButtonClicked);            //lo collega alla funzionalità
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Pulsante INFOBRAWLER non trovato!"));
        }
        if (InfoSniperButton)                                                                                           //idem per sniper
        {
            InfoSniperButton->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnInfoSniperButtonClicked);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Pulsante INFOSNIPER non trovato!"));
        }
        if (SkipTurnButton)                                                                                             //questo per skippare il turno quando mi serve e posso
        {
            SkipTurnButton->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnSkipTurnButtonClicked);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Pulsante SKIPTURN non trovato!"));
        }
    }
    if (INTRO)
    {
        INTRO->SetVisibility(ESlateVisibility::Visible);
        IntroSlideDistance = 1000.0f;                                                                                   //valore  per far scomparire completamente il border e button durante la traslazione
        IntroSlideDuration = 1.0f;                                                                                      //durata dell'animazione
    }
    else UE_LOG(LogTemp, Error, TEXT("INTRO Border non trovato!"));

    if (INTROBUTTON)
    {
        INTROBUTTON->SetVisibility(ESlateVisibility::Visible);
        INTROBUTTON->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnIntroButtonClicked);
    }
    if (HEART1)                                                                                                         //carica i blueprint dei cuori per le animazioni
    {
        HEART1->SetBrushFromTexture(Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/Blueprints/HEART1.HEART1"))));
        HEART1->SetVisibility(ESlateVisibility::Hidden);
    }
    if (HEART2)
    {
        HEART2->SetBrushFromTexture(Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/Blueprints/HEART2.HEART2"))));
        HEART2->SetVisibility(ESlateVisibility::Hidden);
    }
    if (!HEART1)
    {
        UE_LOG(LogTemp, Error, TEXT("HEART1 non trovato nel widget!"));
    }
    if (!HEART2)
    {
        UE_LOG(LogTemp, Error, TEXT("HEART2 non trovato nel widget!"));
    }                                                                                                                   //mi notifica se non li trova
    if (BrawlerHEALTH && SniperHEALTH && BrawlerHealthText && SniperHealthText && HEART1 && HEART2 && SNIPERIMAGE && BRAWLERIMAGE)
    {
        UTexture2D* Heart1Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Blueprints/HEART1"));
        if (!Heart1Texture)
        {
            UE_LOG(LogTemp, Error, TEXT("HEART1 texture non trovata!"));
            Heart1Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
        }
        HEART1->SetBrushFromTexture(Heart1Texture);

        // Caricamento texture HEART2 con fallback
        UTexture2D* Heart2Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Blueprints/HEART2"));
        if (!Heart2Texture)
        {
            UE_LOG(LogTemp, Error, TEXT("HEART2 texture non trovata!"));
            Heart2Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
        }
        HEART2->SetBrushFromTexture(Heart2Texture);
        BrawlerHEALTH->SetVisibility(ESlateVisibility::Hidden);
        SniperHEALTH->SetVisibility(ESlateVisibility::Hidden);
        BrawlerHealthText->SetVisibility(ESlateVisibility::Hidden);
        SniperHealthText->SetVisibility(ESlateVisibility::Hidden);
        HEART1->SetVisibility(ESlateVisibility::Hidden);
        HEART2->SetVisibility(ESlateVisibility::Hidden);
        SNIPERIMAGE->SetVisibility(ESlateVisibility::Hidden); 
        BRAWLERIMAGE->SetVisibility(ESlateVisibility::Hidden);                                                          //nascondiamo ciò che non ci serve subito tipo l'immagine del cuore che compare solo dopo aver spawnato il brawler/sniper
    }
    if (MESSAGGI)
    {
        MESSAGGI->SetVisibility(ESlateVisibility::Visible);                                                             //collegamento allo scrollbox
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ScrollBox 'MESSAGGI' è NULL in NativeConstruct!"));
    }
    if (AIBRAWLERHEALTH)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIBRAWLERHEALTH trovato!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIBRAWLERHEALTH non trovato!"));
    }

    UpdateBrawlerHealthText(MaxBrawlerHealth);                                                                          //passa alla funzione la vita massima del brawler
    UpdateSniperHealthText(MaxSniperHealth);                                                                            //passa alla funzione la vita massima dello sniper

    if (BrawlerHEALTH)
    {
        BrawlerHEALTH->SetPercent(1.0f);                                                                                //setto la percentuale della scrollbox al valore massimo per farla decrescere in base alla vita attuale
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BrawlerHEALTH è nullo! Verifica il binding nel blueprint."));
    }
    if (SniperHEALTH)
    {
        SniperHEALTH->SetPercent(1.0f);                                                                                 //idem per sniper, le progress bar lavorano in percentuali, non valori, da 0 a 1 o viceversa in base a far crescere o decrescere la barra
    }

    bIsMousePressed = false;    
    LastMousePosition = FVector2D::ZeroVector;                                                  
    
    if (AIMode)
    {
        AIMode->OnAIBrawlerAttack.AddDynamic(this, &ULateralPanelWidget::HandleAIBrawlerAttack);                        //colleghiamo i delegati per gestire ciò che avviene con gli attacchi AI
        AIMode->OnAISniperAttack.AddDynamic(this, &ULateralPanelWidget::HandleAISniperAttack);
    }

    if (AIMode)
    {
        AIMode->OnPlayerBrawlerDamaged.AddDynamic(this, &ULateralPanelWidget::HandlePlayerBrawlerDamaged);              //delegati per gestire se siano danneggiate le pedine
        AIMode->OnPlayerSniperDamaged.AddDynamic(this, &ULateralPanelWidget::HandlePlayerSniperDamaged);
    }

    if (PLAYERVICTORY)                                                                                                  //verifichiamo collegamento e nascondiamo i border
    {
        PLAYERVICTORY->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PLAYERVICTORY Border non trovato!"));
    }

    if (AIVICTORY)
    {
        AIVICTORY->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
    }
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetOwningPlayer());
    if (PlayerController)
    {
        PlayerController->OnAIVictory.AddDynamic(this, &ULateralPanelWidget::HandleAIVictory);                          //delegato per la vittoria dell'AI
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));
    }
    if (AIMode)
    {
        AIMode->OnPlayerVictory.AddDynamic(this, &ULateralPanelWidget::HandlePlayerVictory);                            //delegato per la vittoria del player
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIMode non trovato in LateralPanelWidget!"));
    }
    if (HELP)
    {
        CLICK->SetVisibility(ESlateVisibility::Hidden);
        HELP->SetVisibility(ESlateVisibility::Visible);
        FONDO->SetVisibility(ESlateVisibility::Hidden);
        HELP->OnClicked.AddDynamic(this, &ULateralPanelWidget::OnHelpButtonClicked);                                    //stiamo settando tutte le UI che interagiscono con il button, partono tutte hide a parte il button stesso
    }
    if (HELPTEXT)
    {
        HELPTEXT->SetVisibility(ESlateVisibility::Hidden);                                                              //imposta il testo come nascosto inizialmente
    }
}

void ULateralPanelWidget::NativeDestruct()                                                                              //distrugge operazioni standard
{
    Super::NativeDestruct();
}

void ULateralPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)                                    //ricordo che il tick sia la funzione tale che si aggiorna ogni frame
{
    Super::NativeTick(MyGeometry, InDeltaTime);                                                 

    static float HeartPulseTime=0.0f;                                                                                   //tempo accumulato per battito
    HeartPulseTime+=InDeltaTime;                                                                                        //incrementa tempo

    float BaseScale=1.0f;                                                                                               //scala base icona cuore
    float Amplitude=0.2f;                                                                                               //ampiezza variazione scala
    float Frequency=1.0f;                                                                                               //frequenza battito
    float ScaleDelta=Amplitude*FMath::Sin(HeartPulseTime*Frequency*2.0f*PI);                                      //calcola delta scala
    float NewScale=BaseScale+ScaleDelta;                                                                                //scala finale

    FWidgetTransform NewTransform;                                                                                      //trasformazione icona cuore
    NewTransform.Scale=FVector2D(NewScale,NewScale);                                                            //queste righe precedenti mi sono servite per simulare il battito del cuore quando viene spawnata la pedina in gioco

    if(HEART1)                                                                                                          //se primo cuore esiste (quello che corrisponde al brawler)
    {
        if(BrawlerHEALTH && BrawlerHEALTH->GetPercent()>0)                                                              //se brawler vivo
            HEART1->SetRenderTransform(NewTransform);                                                                   //applica effetto battito
        else
        {
            FWidgetTransform BaseTransform;                                                                             //trasformazione per quando muore la pedina, non ha senso che rimanga il cuore se la pedina muore
            BaseTransform.Scale=FVector2D(BaseScale-1.0f,BaseScale-1.0f);                                       //scala minima
            HEART1->SetRenderTransform(BaseTransform);                                                                  //applica trasformazione
        }
    }

    if(HEART2)                                                                                                          //se secondo cuore esiste
    {
        if(SniperHEALTH && SniperHEALTH->GetPercent()>0)                                                                //se sniper vivo
            HEART2->SetRenderTransform(NewTransform);                                                                   //applica effetto battito
        else
        {
            FWidgetTransform BaseTransform;                                                                             //trasformazione default
            BaseTransform.Scale=FVector2D(BaseScale-1.0f,BaseScale-1.0f);                                       //scala minima
            HEART2->SetRenderTransform(BaseTransform);                                                                  //applica trasformazione
        }
    }

    if(bIsRotating && SpawnedSniperActor)                                                                               //qeuste mi servono per una feature carina quando premo infobrawlerbutton e snper
        SpawnedSniperActor->AddActorWorldRotation(FRotator(0.0f,0.0f,80.0f*InDeltaTime));              //ruota actor sniper

    if(bIsRotating && SpawnedBrawlerActor)
        SpawnedBrawlerActor->AddActorWorldRotation(FRotator(0.0f,0.0f,80.0f*InDeltaTime));             //ruota actor brawler

    if(bIsRotating)(InDeltaTime); 
    AMyPlayerController* PlayerController=Cast<AMyPlayerController>(GetOwningPlayer());                             //ottiene controller
    if(PlayerController && SkipTurnButton)                                                                              //se controller e bottone esistono
    {
        bool bShouldEnableButton=false;                                                                                 //flag abilitazione bottone

        if(PlayerController->bBrawlerAlive && !PlayerController->bSniperAlive)                                          //solo brawler vivo
            bShouldEnableButton=PlayerController->bHasBrawlerMoved||PlayerController->GetBrawlerAttacked();             //abilita se mossa o attacco completato
        else if(!PlayerController->bBrawlerAlive && PlayerController->bSniperAlive)                                     //solo sniper vivo
            bShouldEnableButton=PlayerController->bHasSniperMoved||PlayerController->GetSniperAttacked();               //abilita se mossa o attacco completato
        else if(PlayerController->bBrawlerAlive && PlayerController->bSniperAlive)                                      //entrambi vivi
        {
            bool bBrawlerDone=PlayerController->bHasBrawlerMoved||PlayerController->GetBrawlerAttacked();               //brawler turno completato
            bool bSniperDone=PlayerController->bHasSniperMoved||PlayerController->GetSniperAttacked();                  //sniper turno completato
            bShouldEnableButton=bBrawlerDone&&bSniperDone;                                                              //abilita se entrambi completati
        }
        SkipTurnButton->SetIsEnabled(bShouldEnableButton);                                                              //aggiorna lo stato bottone ad ogni frame per essere sempre aggiornato sulla situazione
    }                                                                                                                   //dobbiamo abiitare il pulsante in casi specifici
    
    if (bIntroSliding)                                                                                                  //gestiamo scorrimendo border
    {
        IntroSlideElapsed += InDeltaTime;                                                                               //tempo di scorrimento aggiornato
        float Alpha = FMath::Clamp(IntroSlideElapsed / IntroSlideDuration, 0.0f, 1.0f);
        float YOffset = FMath::InterpEaseOut(0.0f, IntroSlideDistance, Alpha, 2.0f);                          //calcola lo spostamento Y usando un'interpolazione con easing (inizia veloce, rallenta alla fine)
        
        if (INTRO)                                                                                                      //applica lo spostamento agli elementi INTRO e INTROBUTTON dopo aver verificato che esistano
            INTRO->SetRenderTranslation(FVector2D(0, YOffset));                                                     //sposta lungo l'asse Y
        if (INTROBUTTON)
            INTROBUTTON->SetRenderTranslation(FVector2D(0, YOffset));                                               //stesso spostamento per il pulsante
        if (Alpha >= 1.0f)                                                                                              //quando l'animazione è completata (Alpha >= 1.0)
        {
            if (INTRO) 
                INTRO->SetVisibility(ESlateVisibility::Hidden);                                                         //disabilita la visibilità
            if (INTROBUTTON) 
                INTROBUTTON->SetVisibility(ESlateVisibility::Hidden);                                                   //nascondi il pulsante
            
            bIntroSliding = false;                                                                                      //termina l'animazione
        }
    }
}

void ULateralPanelWidget::HandleAIVictory()                                                                             //questa è proprio per far comparire il border per l' AI victory, nella classe AIMode e PlayerController
{                                                                                                                       //viene chiamata al di fuori delle vittorie 
    if (AIVICTORY)
    {
        AIVICTORY->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: AIVICTORY è nullptr!"));
    }
}
void ULateralPanelWidget::HandlePlayerVictory()                                                                         //stessa cosa, quando arriva il delegato chiamato viene posto come visibile il border
{
    if (PLAYERVICTORY)
    {
        PLAYERVICTORY->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VictoryBorder non assegnato nel widget!"));
    }
}
void ULateralPanelWidget::HandlePlayerBrawlerDamaged(int32 Damage)                                                      //ecco cosa succede quando una pedina del player prende danno
{
    if (BrawlerHEALTH)
    {
        float CurrentHealth = BrawlerHEALTH->GetPercent() * MaxBrawlerHealth;                                           //calcola la vita attuale
        CurrentHealth -= Damage;
        CurrentHealth = FMath::Max(CurrentHealth, 0.0f);                                                          //la percentuale non può scendere in negativo
        UpdateBrawlerHealth(CurrentHealth);
        UpdateBrawlerHealthText(CurrentHealth);                                                                         //passa tale valore per aggiornare sia text che vita
        FString DamageMessage = FString::Printf(TEXT("HP: Il Brawler ha subito %d danni!"), Damage);               //salva stringa nella DamageMessage un valore che ogni volta viene aggiornato in base al danno
        AddGameMessage(DamageMessage, FLinearColor::Yellow);                                                            //e lo stampo nella srollbox di colore giallo

        if (CurrentHealth <= 0.0f)                                                                                      //questo è il controllo per verificare che la vita della pedina sia <= 0,
        {
            AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());                                 //questa funzione restituisce un puntatore all'oggetto APlayerController che possiede il widget
            if (PC)
            {
                PC->RemovePlayerBrawler();                                                                              //questa funzione propria del playercontroller fa scomparire il personaggio quando la sua vita è <= 0
            }
        }
    }
}

void ULateralPanelWidget::HandlePlayerSniperDamaged(int32 Damage)                                                       //questo fa letteralmente la stessa cosa ma per lo sniper
{
    if (SniperHEALTH)
    {
        float CurrentHealth = SniperHEALTH->GetPercent() * MaxSniperHealth;
        CurrentHealth -= Damage;
        CurrentHealth = FMath::Max(CurrentHealth, 0.0f);
        UpdateSniperHealth(CurrentHealth);
        UpdateSniperHealthText(CurrentHealth);
        FString DamageMessage = FString::Printf(TEXT("HP: Lo Sniper ha subito %d danni!"), Damage);
        AddGameMessage(DamageMessage, FLinearColor::Yellow);
        if (CurrentHealth <= 0.0f)
        {
            AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
            if (PC)
            {
                PC->RemovePlayerSniper();                                                                               //funzione che distrugge lo Sniper
            }
        }
    }
}

void ULateralPanelWidget::OnSkipTurnButtonClicked()                                                                     //questa funzione deve capire quando terminare il turno alla presssione del button, ovviamente sotto specifiche condizioni e non sempre
{
    if (GameModeRef)
    {
        AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());                                     //ottengo il player controller
        if (!PC)
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));                                      //log: player controller non trovato
            return;
        }
        if (PC->bBrawlerAlive && !PC->bSniperAlive)                                                                     //caso in cui solo il brawler è vivo
        {
            if (PC->bHasBrawlerMoved || PC->GetBrawlerAttacked())                                                       //se il brawler ha mosso oppure ha attaccato
            {
                UE_LOG(LogTemp, Warning, TEXT("Solo il Brawler è vivo e ha completato l'azione, turno terminato.")); 
                GameModeRef->EndPlayerTurn();                                                                           //termina il turno
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Il Brawler non ha completato l'azione.")); 
            }
            return;
        }
        else if (!PC->bBrawlerAlive && PC->bSniperAlive)                                                                //caso in cui solo lo sniper è vivo
        {
            if (PC->bHasSniperMoved || PC->GetSniperAttacked())                                                         //se lo sniper ha mosso oppure ha attaccato
            {
                UE_LOG(LogTemp, Warning, TEXT("Solo lo Sniper è vivo e ha completato l'azione, turno terminato.")); 
                GameModeRef->EndPlayerTurn();                                                                           //termina il turno
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Lo Sniper non ha completato l'azione."));                        //log: sniper non ha completato l'azione
            }
            return;
        }
        if (PC->bBrawlerAlive && PC->bSniperAlive)                                                                      //caso in cui entrambe le pedine sono vive
        {
            bool bBrawlerDone = PC->bHasBrawlerMoved || PC->GetBrawlerAttacked();                                       //controllo se il brawler ha completato un'azione
            bool bSniperDone  = PC->bHasSniperMoved  || PC->GetSniperAttacked();                                        //controllo se lo sniper ha completato un'azione
            if (bBrawlerDone && bSniperDone)
            {
                UE_LOG(LogTemp, Warning, TEXT("Entrambe le pedine hanno completato un'azione, turno terminato."));
                GameModeRef->EndPlayerTurn(); // termina il turno
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Devi muovere o attaccare con entrambe le pedine per terminare il turno.")); 
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeRef non trovato!"));
    }
}



void ULateralPanelWidget::OnBrawlerButtonClicked()                                                                      //click bottone brawler
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());      
    if (PlayerController)
    {
        PlayerController->StartBrawlerPlacement();                                                                      //riferimento al pc per far spawnare il brawler dove voglio
        UpdateHealthBrawlerBarsVisibility(true);                                                                        //facciamo comparire la barra di vita pedine
        UE_LOG(LogTemp, Warning, TEXT("HEART1 trovato, visibilità: %d"), (int)HEART1->GetVisibility());

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));
        
    }
}

void ULateralPanelWidget::OnSniperButtonClicked()                                                                       //idem per il precedente
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (PlayerController)
    {
        PlayerController->StartSniperPlacement();
        UpdateHealthSniperBarsVisibility(true);

        FInputModeGameAndUI InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));
    }
    this->SetFocus();
}


void ULateralPanelWidget::OnInfoBrawlerButtonClicked()                                                                  //bottone per far spawnare visibilmente il render 3D
{
    if (SpawnedSniperActor)
    {
        SpawnedSniperActor->Destroy();                                                                                  //mi servono controlli per rendere mutuo esclusivi i pulsanti: se premo su infosniperbutton questo compare e rotea
        SpawnedSniperActor = nullptr;                                                                                   //ma se premo sull'altro quello compare e toglie quello precedente
        UE_LOG(LogTemp, Warning, TEXT("SNIPER3D distrutto!"));
        bIsRotating = false;
        BRAWLERIMAGE->SetVisibility(ESlateVisibility::Visible);                                                         //fa comparire anche l'immagine descrittiva insieme al render 3D
        if (SNIPERIMAGE)
        {
            SNIPERIMAGE->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (SpawnedBrawlerActor)                                                                                            //se lo ripremo scompare
    {
        BRAWLERIMAGE->SetVisibility(ESlateVisibility::Hidden);
        SpawnedBrawlerActor->Destroy();                                                                                 //distruggo
        SpawnedBrawlerActor = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("BRAWLER3D distrutto!"));
        bIsRotating = false;                                                                                            //ne blocco la rotazione
                                                                                                                        
    }                                                                                                                   //sto gestendo anche la scomparsa sia quando premo lo stesso pulsante info che l'altro, mutui esclusivi
    else
    {
        BRAWLERIMAGE->SetVisibility(ESlateVisibility::Visible);

        SpawnBrawler3DInForeground();
        bIsRotating = true;
    }                                                                                                                   //feature carina creata con un fbx importato su UE
}

void ULateralPanelWidget::OnInfoSniperButtonClicked()                                                                   //idem del precedente
{
    BRAWLERIMAGE->SetVisibility(ESlateVisibility::Hidden);

    if (SpawnedBrawlerActor)                                                                                            
    {
        SpawnedBrawlerActor->Destroy();
        SpawnedBrawlerActor = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("BRAWLER3D distrutto!"));
        bIsRotating = false;
    }

    if (SpawnedSniperActor)
    {
        this->SetRenderTranslation(FVector2D(0.0f, 0.0f));

        SpawnedSniperActor->Destroy();
        SpawnedSniperActor = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("SNIPER3D distrutto!"));
        bIsRotating = false;

        if (SNIPERIMAGE)
        {
            SNIPERIMAGE->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        SpawnSniper3DInForeground();
        bIsRotating = true;

        if (SNIPERIMAGE)
        {
            SNIPERIMAGE->SetVisibility(ESlateVisibility::Visible);
            SNIPERIMAGE->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
            SNIPERIMAGE->SetRenderTranslation(FVector2D(0.0f, 0.0f));
        }
    }
}

void ULateralPanelWidget::SpawnBrawler3DInForeground()
{                                                                                                                       //carichiamo la classe blueprint relativa al render 3D del brawler
    UClass* BrawlerClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Game/Blueprints/BRAWLER3D.BRAWLER3D_C"));
    if (!BrawlerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile caricare la classe BRAWLER3D!"));
        return;
    }
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector SpawnLocation = CameraLocation + CameraRotation.Vector() * 250.0f;                                          //distanza dalla telecamera
    FRotator SpawnRotation = CameraRotation;
                                                                                                                        //in queste precedenti righe si configura camera rotazione dell render
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;                       //adesso spawniamo il render 3D fisicamente nella scena

    SpawnedBrawlerActor = GetWorld()->SpawnActor<AActor>(BrawlerClass, SpawnLocation, SpawnRotation, SpawnParams);      //spawna
    if (SpawnedBrawlerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BRAWLER3D spawnato con successo in primo piano!"));                      //semplici log per dirmi se lo spawn sia avvenuto
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare BRAWLER3D!"));
    }
}

void ULateralPanelWidget::SpawnSniper3DInForeground()                                                                   //commenti analoghi alla funzione precedente
{
    UClass* SniperClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Game/Blueprints/SNIPER3D.SNIPER3D_C"));
    if (!SniperClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile caricare la classe SNIPER3D!"));
        return;
    }
    
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController non trovato!"));
        return;
    }
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector SpawnLocation = CameraLocation + CameraRotation.Vector() * 250.0f;
    FRotator SpawnRotation = CameraRotation;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SpawnedSniperActor = GetWorld()->SpawnActor<AActor>(SniperClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (SpawnedSniperActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("SNIPER3D spawnato con successo in primo piano!"));

        if (UStaticMeshComponent* MeshComponent = SpawnedSniperActor->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComponent->SetRenderCustomDepth(true); 
            MeshComponent->SetCustomDepthStencilValue(255); 
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile spawnare SNIPER3D!"));
    }
}

void ULateralPanelWidget::OnHelpButtonClicked()                                                                         //questo invece è letteralmente l'help button visibile in basso a sinistra utile a far comparire                                       
{                                                                                                                       //alcune info di gioco durante la partita

    if (HELPTEXT && FONDO && CLICK)                 
    {
        if (HELPTEXT->GetVisibility() == ESlateVisibility::Visible)
        {
            HELPTEXT->SetVisibility(ESlateVisibility::Hidden);                                                          //se è già visibile metto border text ecc hidden
            FONDO->SetVisibility(ESlateVisibility::Hidden);
            CLICK->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            HELPTEXT->SetVisibility(ESlateVisibility::Visible);                                                         //se invece sono hidden li metto visible
            FONDO->SetVisibility(ESlateVisibility::Visible);
            CLICK->SetVisibility(ESlateVisibility::Visible);
        }
    }
}                                                                               

void ULateralPanelWidget::UpdateHealthBrawlerBarsVisibility(bool bIsBrawlerSelected)                                    //qui si gestisce la progressbar delle pedine del player
{
    if (BrawlerHEALTH && BrawlerHealthText && HEART1)
    {
        BrawlerHEALTH->SetVisibility(bIsBrawlerSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);        //le si imposta su hidden perchè quando il gioco ha inizio le progress bars non sono ancora visibili
        BrawlerHealthText->SetVisibility(bIsBrawlerSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);    //lo diventeranno in concomitanza dello spawn delle relative pedine
        HEART1->SetVisibility(bIsBrawlerSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BrawlerHEALTH, BrawlerHealthText o HEART1 è nullo!"));
    }
}

void ULateralPanelWidget::UpdateHealthSniperBarsVisibility(bool bIsSniperSelected)                                      //stessa cosa del precedente
{
    if (SniperHEALTH && SniperHealthText && HEART2)
    {                                                                                                                   //mostra/nascondi la barra di salute, il testo e l'immagine del cuore dello Sniper
        SniperHEALTH->SetVisibility(bIsSniperSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        SniperHealthText->SetVisibility(bIsSniperSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        HEART2->SetVisibility(bIsSniperSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SniperHEALTH, SniperHealthText o HEART2 è nullo!"));
    }
}

void ULateralPanelWidget::UpdateBrawlerHealth(float CurrentHealth)                                                      //adesso invece gestiamo proprio fisicamente la progress bar passandone i valori anche in base al danno ricevuto
{
    if (BrawlerHEALTH)
    {
        float HealthPercentage = CurrentHealth / MaxBrawlerHealth;
        BrawlerHEALTH->SetPercent(HealthPercentage);                                                                    //ricordiamo che la percentuale sia la variabile cardine per funzionamento progress
        FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Green, HealthPercentage);
        BrawlerHEALTH->SetFillColorAndOpacity(BarColor);                                                                //queste funzioni sono un aggiunta grafica per var passare la vita da max = verde a minima = rossa
        UpdateBrawlerHealthText(CurrentHealth);
    }
}

void ULateralPanelWidget::UpdateSniperHealth(float CurrentHealth)                                                       //stessa cosa per lo sniper
{
    if (SniperHEALTH)
    {
        float HealthPercentage = CurrentHealth / MaxSniperHealth;
        SniperHEALTH->SetPercent(HealthPercentage);
        // Interpolazione: 100% salute → verde, 0% salute → rosso
        FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Green, HealthPercentage);
        SniperHEALTH->SetFillColorAndOpacity(BarColor);
        UpdateSniperHealthText(CurrentHealth);
    }
}

void ULateralPanelWidget::UpdateBrawlerHealthText(float CurrentHealth)                                                  //regoliamo di conseguenza il testo sulla progress bar per dare un riscontro numerico della vita
{
    if (BrawlerHealthText)
    {
        // Formatta il testo come "CurrentHealth / MaxHealth"
        FString HealthString = FString::Printf(TEXT("[ %d / %d ]"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxBrawlerHealth));
        BrawlerHealthText->SetText(FText::FromString(HealthString));
    }
}

void ULateralPanelWidget::UpdateSniperHealthText(float CurrentHealth)
{
    if (SniperHealthText)
    {
        // Formatta il testo come "CurrentHealth / MaxHealth"
        FString HealthString = FString::Printf(TEXT("[ %d / %d ]"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxSniperHealth));
        SniperHealthText->SetText(FText::FromString(HealthString));
    }
}

void ULateralPanelWidget::UpdateAIBrawlerHealth(float HealthPercentage)
{
    if (AIBRAWLERHEALTH)
    {
        HealthPercentage = FMath::Clamp(HealthPercentage, 0.0f, 1.0f);                                      //si assicura che la percentuale sia compresa tra 0 e 1

        AIBRAWLERHEALTH->SetPercent(HealthPercentage);                                                                  //aggiorno la progressbar tramite la sua percentuale
        FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Blue, HealthPercentage);
        AIBRAWLERHEALTH->SetFillColorAndOpacity(BarColor);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIBRAWLERHEALTH è nullo!"));
    }
}

void ULateralPanelWidget::UpdateAISniperHealth(float HealthPercentage)                                                  //idem
{
    if (AISNIPERHEALTH)
    {
        HealthPercentage = FMath::Clamp(HealthPercentage, 0.0f, 1.0f);                                      //mi assicurato che la percentuale sia compresa tra 0 e 1
        AISNIPERHEALTH->SetPercent(HealthPercentage);
        FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Blue, HealthPercentage);
        AISNIPERHEALTH->SetFillColorAndOpacity(BarColor);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SniperHEALTH è nullo!"));
    }
}

void ULateralPanelWidget::UpdateBordersVisibility(bool bIsPlayerTurn)                                                   //queste mi servono per la gestione dei turni, se è turno player compare playerborder
{                                                                                                                       //se è turno AI è border ai
    if (PlayerBorder && AIBorder)
    {
        if (bIsPlayerTurn)
        {
            PlayerBorder->SetVisibility(ESlateVisibility::Visible);
            AIBorder->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            PlayerBorder->SetVisibility(ESlateVisibility::Hidden);
            AIBorder->SetVisibility(ESlateVisibility::Visible);                                                         //li pone visibili o no in base alla situazione in maniera mutua esclusiva
        }
        
        UE_LOG(LogTemp, Warning, TEXT("UpdateBordersVisibility: Turno %s, PlayerBorder %s, AIBorder %s"),
            bIsPlayerTurn ? TEXT("Giocatore") : TEXT("AI"),
            PlayerBorder->IsVisible() ? TEXT("Visibile") : TEXT("Nascosto"),
            AIBorder->IsVisible() ? TEXT("Visibile") : TEXT("Nascosto"));                                                   
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerBorder o AIBorder è nullo!"));
    }
}

void ULateralPanelWidget::SetHealthBarPercent(float Percent)                                                            //mi serve come riferimento per le classi esterne tipo playercontroller
{
    if (AIBRAWLERHEALTH)
    {
        AIBRAWLERHEALTH->SetPercent(Percent);
        UE_LOG(LogTemp, Warning, TEXT("HealthBar aggiornata a: %f"), Percent);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIBRAWLERHEALTH è nullo!"));
    }
}

void ULateralPanelWidget::HandleAIBrawlerAttack(int32 Damage)
{
    if (BrawlerHEALTH)
    {
        float CurrentHealth = BrawlerHEALTH->GetPercent() * MaxBrawlerHealth;                                           //legge il valore corrente della vita

        CurrentHealth -= Damage;                                                                                        //applica il danno alla vita del brawler
        CurrentHealth = FMath::Max(CurrentHealth, 0.0f);                                                          //si assicurati che la salute non sia negativa

        BrawlerHEALTH->SetPercent(CurrentHealth / MaxBrawlerHealth);                                                    //ed infine si aggiornano progress bar

        UpdateBrawlerHealthText(CurrentHealth);
    }
}
void ULateralPanelWidget::HandleAISniperAttack(int32 Damage)                                                            //ide,
{
    if (SniperHEALTH)
    {
        float CurrentHealth = SniperHEALTH->GetPercent() * MaxSniperHealth;

        CurrentHealth -= Damage;
        CurrentHealth = FMath::Max(CurrentHealth, 0.0f); 

        SniperHEALTH->SetPercent(CurrentHealth / MaxSniperHealth);

        UpdateSniperHealthText(CurrentHealth);
    }
}

float ULateralPanelWidget::GetBrawlerHealth() const                                                                     //si limita a restituire il valore della vita delle pedine                                                 
{                                                                                                                       //mi servono per interagire con playercontroller
    if (BrawlerHEALTH)
    {
        float Health = BrawlerHEALTH->GetPercent() * MaxBrawlerHealth;
        return Health;
    }
    return 0.0f;
}

float ULateralPanelWidget::GetSniperHealth() const                                                                      //idem
{
    if (SniperHEALTH)
    {
        float Health = SniperHEALTH->GetPercent() * MaxSniperHealth;
        return Health;
    }
    return 0.0f;
}

void ULateralPanelWidget::AddGameMessage(const FString& Message, const FLinearColor& MessageColor)                      //funzione essenziale per la gestione della historybox, per aggiungervi i messaggi di gioco
{
    if (!MESSAGGI)
    {
        UE_LOG(LogTemp, Error, TEXT("ScrollBox 'MESSAGGI' è NULL! Controlla il binding nel Blueprint."));       //log che verifica il collegamento
        return;
    }
    
    UTextBlock* NewTextBlock = NewObject<UTextBlock>(this);
    NewTextBlock->SetText(FText::FromString(Message));                                                                  //crea un nuovo textblock ad ogni interazione, textblock che verranno incasellati uno sotto l'altro
    NewTextBlock->SetColorAndOpacity(FSlateColor(MessageColor));                                                        //impone colore ed opacità
    NewTextBlock->SetAutoWrapText(true);

    FSlateFontInfo FontInfo = NewTextBlock->GetFont();
    FontInfo.TypefaceFontName = FName("Regular");                                                                       //usa un font senza grassetto
    FontInfo.Size = 20;                                                                                                 //imposta la dimensione del font 
    NewTextBlock->SetFont(FontInfo);
    
    FWidgetTransform Transform;                                                                                         //applico una trasformazione di inclinazione (simile al corsivo) usando FWidgetTransform
    Transform.Translation = FVector2D::ZeroVector;
    Transform.Scale = FVector2D(1.0f, 1.0f);                                                                    //scala di base, qualora mi andasse di modificare i parametri di dimensione sarebbe qui
    Transform.Shear = FVector2D(-0.2f, 0.0f);                                                                   //ecco l'inclinazione leggera (puoi regolare il valore -0.2f)
    NewTextBlock->SetRenderTransform(Transform);                                                                        //applica la trasformazione al TextBlock
    MESSAGGI->AddChild(NewTextBlock);                                                                                   //ed ora nell'effettivo andiamo ad aggiungere il messaggio alla scrollbox creando un child, derivato textblock della scrollbox
    MESSAGGI->ScrollToEnd();                                                                                            //questo invece mi impone invece che si possa scrollare fino in fondo ai messaggi per leggelri
}

void ULateralPanelWidget::OnIntroButtonClicked()
{
    bIntroSliding = true;
    IntroSlideElapsed = 0.0f;
    if (INTROBUTTON)                                                                                                    //disabilita il pulsante durante l'animazione per evitare click multipli
    {
        INTROBUTTON->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}