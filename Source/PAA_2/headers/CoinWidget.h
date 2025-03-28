#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h" 
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Styling/SlateTypes.h"
#include "CoinWidget.generated.h"

class AMyGameMode;
class ABP_Obstacles;
class UButton;
class UBorder;
class USlider; 

UCLASS()
class PAA_2_API UCoinWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetGameMode(AMyGameMode* GameMode);                                                                            //idem per altre classi precedenti

    UFUNCTION(BlueprintCallable, Category = "Coin Widget")                              
    void OnEasyModeClicked();                                                                                           //funzionmi per la gestione della easy mode o hard mode

    UFUNCTION(BlueprintCallable, Category = "Coin Widget")
    void OnHardModeClicked();

    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;                

    UFUNCTION()
    void OnObstacleProbabilitySliderChanged(float Value);                                                               //funzione per una feature aggiuntiva, lo slider verticale per permettere al giocatore di scegliere autonomamente la percentuale di spawn degli ostacoli
    UFUNCTION()
    void OnMonetaButtonClicked();                                                                                       //altro pulsante per l'interazione con il widget, se lo premo estraggo un valore random booleano, se true = testa, se false = croce
    UFUNCTION()
    void OnStartGameButtonClicked();                                                                                    //pulsante per startare il gioco
    UFUNCTION()
    void OnInformazioniButtonClicked();                                                                                 //pulsante per far comparire o scomparire istruzioni di gioco
    FTimerHandle UpdateGridTimer;
    UFUNCTION()
    void HandleEasyModeButtonClicked();                                                                                 //bottoni corrispondenti
    UFUNCTION()
    void HandleHardModeButtonClicked();

private:

    void SetObstacleProbability(float Value);                                                                           //funzione per gestire il cambiamento della probabilità in contatto anche con la classe ostacoli e griglia            

    AMyGameMode* GameModeRef;

    UPROPERTY(meta = (BindWidget))
    UButton* MonetaButton;                                                                                              //creazione finisca collegamento blueprint widget button per la moneta

    UPROPERTY(meta = (BindWidget))
    UButton* StartGameButton;                                                                                           //idem per il moneta

    UPROPERTY(meta = (BindWidget))
    UButton* InformazioniButton;                                                                                        //idem per informazioni

    UPROPERTY(meta = (BindWidget))
    class UButton* EasyModeButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* HardModeButton;

    UPROPERTY(meta = (BindWidget))
    UBorder* BorderTesta;                                                                                               //questi due, testa o croce, sono due border che faccio comparire per rendere evidente al player quale sia il valore estratto casualmente

    UPROPERTY(meta = (BindWidget))
    UBorder* BorderCroce;                                                                                               //idem

    UPROPERTY(meta = (BindWidget))
    UBorder* MenuTendina;                                                                                               //border per far vedere le regole di gioco, scomparirà o apparirà alla pressione sul button 

    UPROPERTY(meta = (BindWidget))
    USlider* ObstacleProbabilitySlider;                                                                                 //collegamento fisico allo slider per configurare la percentuale di comparsa degli ostacoli

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ObstacleProbabilityLabel;                                                                               //testo direttamente collegato allo slider per far visualizzare quale percentuale stia effettivamente scegliendo
    
    bool bMenuAperto;
    bool bMonetaLanciata;
    ABP_Obstacles* GetObstacleActor();                                                                                  //è un riferimento alla classe BP_obstalces perchè già dal coinwdiget possiamo definire la percentuale degli ostacoli 
    bool Coinflip();                                                                                                    //funzione per lanciare la moneta frand
    void ApplySliderStyle();                                                                                            //ho modificato lo stile in c++ per lo slider, non riesco a farlo come voglio in blueprint
    void ApplyObstacleProbability();                                                                                    //questo per effettivamente far variare la percentuale di generazione in base alla scelta sullo slider
    void UpdateModeButtonColors(bool bEasySelected);
};