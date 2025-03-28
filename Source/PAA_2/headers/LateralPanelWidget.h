#pragma once 

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "LateralPanelWidget.generated.h"

class UProgressBar;
class UButton;
class UTextBlock;
class UBorder;
class UImage;
class UScrollBox;
class AMyGameMode;
class AMyPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAIVictory);                                                                       //delegato per gestire la vittoria dell'AI

UCLASS()
class PAA_2_API ULateralPanelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    ULateralPanelWidget(const FObjectInitializer& ObjectInitializer);

    //metodi di inizializzazione e aggiornamento del widget
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    //funzioni per la gestione della UI
    UFUNCTION() void UpdateBordersVisibility(bool bIsPlayerTurn);                                                       //fa comparire o no i border
    
    //funzioni per la gestione dei pulsanti
    UFUNCTION() void OnBrawlerButtonClicked();                                                                          //funzioni per gestire fisicamente i bottoni per selezionare lo spawn delle pedine
    UFUNCTION() void OnSniperButtonClicked();   
    UFUNCTION() void OnInfoBrawlerButtonClicked();                                                                      //questi due invece fanno comparire le info delle pedine in gioco
    UFUNCTION() void OnInfoSniperButtonClicked();                                                                       //c'è anche una cosa carina che fa roteare i modelli 3D lavorati con blender
    UFUNCTION() void OnSkipTurnButtonClicked();

    //funzioni per la gestione della salute e delle barre di progresso
    UFUNCTION() void UpdateHealthBrawlerBarsVisibility(bool bIsBrawlerSelected);                                        //le seguenti sono per gestire le progressbar delle mie pedine
    UFUNCTION() void UpdateHealthSniperBarsVisibility(bool bIsSniperSelected);
    UFUNCTION() void UpdateBrawlerHealth(float CurrentHealth);                                                          //queste per la gestione fisica della loro vita
    UFUNCTION() void UpdateSniperHealth(float CurrentHealth);
    UFUNCTION() void UpdateBrawlerHealthText(float CurrentHealth);                                                      //questa per rendere coerente testo vita posto sulla progress bar vita attuale/massima
    UFUNCTION() void UpdateSniperHealthText(float CurrentHealth);
    UFUNCTION() void UpdateAIBrawlerHealth(float CurrentHealth);                                                        //per le pedine dell'ai
    UFUNCTION() void UpdateAISniperHealth(float CurrentHealth);
    UFUNCTION() void OnIntroButtonClicked();                                                                            //gestisce la scomparsa delle pedine
    UFUNCTION(BlueprintCallable, Category = "UI") void SetHealthBarPercent(float Percent);                              
    UFUNCTION(BlueprintCallable, Category = "Health") float GetBrawlerHealth() const;
    UFUNCTION(BlueprintCallable, Category = "Health") float GetSniperHealth() const;

    //funzioni per la gestione delle conseguenze del combattimento
    UFUNCTION() void HandleAIBrawlerAttack(int32 Damage);                                                               //aggiornano la vita delle pedine in base ai danni, con le relative bars
    UFUNCTION() void HandleAISniperAttack(int32 Damage);
    UFUNCTION() void HandlePlayerBrawlerDamaged(int32 Damage);
    UFUNCTION() void HandlePlayerSniperDamaged(int32 Damage);
    
    //funzioni per i messaggi di gioco                                                                                  //essenziali per la scrollbox con la history box
    UFUNCTION(BlueprintCallable, Category = "Game Messages") void AddGameMessage(const FString& Message, const FLinearColor& MessageColor = FLinearColor::Black);

    //widget UI (Messaggi)
    UPROPERTY(meta = (BindWidget)) UScrollBox* MESSAGGI;

    //funzioni per il rendering dei personaggi in primo piano
    UFUNCTION() void SpawnSniper3DInForeground();                                                                       //queste due funzioni fanno girare in 3D sull'asse verticale i modelli relativi a sniper e brawler
    UFUNCTION() void SpawnBrawler3DInForeground();

    //timer e gestione della rotazione
    FTimerHandle RotationTimerHandle;
    FRotator RotationSpeed = FRotator(20.0f, 30.0f, 15.0f);
    
    //widget UI 
    UPROPERTY(meta = (BindWidget)) UButton* BrawlerButton;                                                              //collegano pulsanti
    UPROPERTY(meta = (BindWidget)) UButton* SniperButton;
    UPROPERTY(meta = (BindWidget)) UButton* InfoBrawlerButton;
    UPROPERTY(meta = (BindWidget)) UButton* InfoSniperButton;                                                           //pulsanti info collegati a render 3D
    UPROPERTY(meta = (BindWidget)) UButton* SkipTurnButton;                                                             //skip turn button
    UPROPERTY(meta = (BindWidget)) UButton* HELP;                                                                       //border che mostrerà il testo quando hoverato
    UPROPERTY(meta = (BindWidget)) UButton* INTROBUTTON;                                                                //button per regole di gioco iniziale
    UPROPERTY(meta = (BindWidget)) UTextBlock* HELPTEXT;                                                                //testo che comparirà quando si hovera il Border
    UPROPERTY(meta = (BindWidget)) UBorder* FONDO;
    UPROPERTY(meta = (BindWidget)) UBorder* INTRO;                                                                      //border che scompare iniseme al button
    UPROPERTY(meta = (BindWidget)) UImage* CLICK;
    
    //widget UI (Barre della salute)
    UPROPERTY(meta = (BindWidget)) UProgressBar* AIBRAWLERHEALTH;                                                       //come si vede da UprogressBar queste sono le progressbar relative alle vite delle pedine AI
    UPROPERTY(meta = (BindWidget)) UProgressBar* AISNIPERHEALTH;
    
    //widget UI (Bordi per la vittoria e il pareggio)
    UPROPERTY(meta = (BindWidget)) UBorder* PLAYERVICTORY;
    UPROPERTY(meta = (BindWidget)) UBorder* AIVICTORY;

    //funzioni per la gestione della vittoria e della sconfitta
    UFUNCTION() void HandleAIVictory();                                                                                 //vittoria player,ai o pareggio comparsa di immagini relative 
    UFUNCTION() void HandlePlayerVictory();

    //widget UI (Struttura ad albero dei widget)
    UPROPERTY() UWidgetTree* WidgetTreeRef;

    //variabili per la salute massima
    UPROPERTY(EditDefaultsOnly, Category = "Health") float MaxBrawlerHealth = 40.0f;                                    //ci servono a gestire la vita massima, nonche vità a cui vengono inizializzate le pedine
    UPROPERTY(EditDefaultsOnly, Category = "Health") float MaxSniperHealth = 20.0f;                                     //sia ai che non

    //variabili per la salute massima dell'IA
    UPROPERTY(EditDefaultsOnly, Category = "Health") float MaxAIBrawlerHealth = 40.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Health") float MaxAISniperHealth = 20.0f;

protected:
    //questi sono i metodi di distruzione e gestione degli effetti UI
    virtual void NativeDestruct() override;
  
    // Funzione per gestire il click del mouse
    UFUNCTION() void OnHelpButtonClicked();
private:
    bool IsMouseOverHelpBorder(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const;                   //funzione chiamata quando il mouse lascia il Border

    //riferimenti ai widget UI (Barre della salute)
    UPROPERTY(meta = (BindWidget)) UProgressBar* BrawlerHEALTH;                                                         //le progress bar mi servonono per una gestione visiva della vita delle pedine
    UPROPERTY(meta = (BindWidget)) UProgressBar* SniperHEALTH;

    //riferimenti ai testi della salute
    UPROPERTY(meta = (BindWidget)) UTextBlock* BrawlerHealthText;                                                       //il testo verrà settato in maniera coerente alla percentuale per rendere leggibile la vita attuale 
    UPROPERTY(meta = (BindWidget)) UTextBlock* SniperHealthText;                                                        //vita ex: 35/40

    //riferimenti alle immagini
    UPROPERTY(meta = (BindWidget)) UImage* HEART1;
    UPROPERTY(meta = (BindWidget)) UImage* HEART2;
    UPROPERTY(meta = (BindWidget)) UImage* SNIPERIMAGE;
    UPROPERTY(meta = (BindWidget)) UImage* BRAWLERIMAGE;                                                                //riferimenti ad immagini per pura funzionalità estetica

    //riferimenti ai bordi (Giocatore e AI)
    UPROPERTY(meta = (BindWidget)) UBorder* PlayerBorder;
    UPROPERTY(meta = (BindWidget)) UBorder* AIBorder;                                                                   //questi vengono usati per rendere visibile il turno corrente, 

    //riferimenti agli oggetti di gioco
    UPROPERTY() AMyGameMode* GameModeRef;
    UPROPERTY() ULateralPanelWidget* LateralPanelWidget;

    //rimer per la visibilità dei bordi
    FTimerHandle BorderVisibilityTimerHandle;                                                                           //definisce un timer da usare per le transizioni 

    //lista dei messaggi di gioco
    TArray<FString> MessaggiSalvati;                                                                                    //scrollbox essenziale per messaggi nella historybox

    //riferimento agli attori 3D dei personaggi
    UPROPERTY() AActor* SpawnedSniperActor;
    UPROPERTY() AActor* SpawnedBrawlerActor;

    //variabili per la gestione dell'input del mouse
    bool bIsMousePressed;
    FVector2D LastMousePosition;
    bool bIsRotating;                                                                                                   //questo booleano per far roteare il render 3D delle pedine, .obj o .fbx

    bool bIntroSliding = false;
    float IntroSlideDuration = 0.5f;
    float IntroSlideElapsed = 0.0f;
    float IntroSlideDistance = 500.0f;                                                                                  //variabili inizializzate per far scomparire dinamicamente verso il basso il border ed il button dell'intro
};