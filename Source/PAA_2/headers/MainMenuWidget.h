#pragma once                                 

#include "CoreMinimal.h"                    
#include "Blueprint/UserWidget.h"             
#include "MainMenuWidget.generated.h"          

class AMyGameMode;																										//rappresenta la modalità di gioco
class UCoinWidget;																										//rappresenta il widget per il lancio della moneta

UCLASS()
class PAA_2_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()																									//serve per creare in automatico il codice per farlo interagire ad UR

public:
	void SetGameMode(AMyGameMode* GameMode);																			//funzione per impostare il riferimento alla modalità di gioco attiva

protected:
	virtual void NativeConstruct() override;																			//come per tutte le classi

	UPROPERTY()
	AMyGameMode* GameModeRef;																							//reference alla modalità di gioco

	UPROPERTY(meta = (BindWidget))
	class UButton* PlayButton;																							//riferimento al blueprint per il pulsante di gioco, quello per farvi interagire il giocatore, è il collegamento fisico al button
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UCoinWidget> CoinWidgetClass;																		//riferimento alla classe coinwidget per far interagire mainmenu alla coinwidget, far scomparire una e comparire l'altra
	UPROPERTY() UCoinWidget* CoinWidgetInstance;																		//questo serve al collegamento fisico sul blueprint, il normale coinwidget è per la classe e le istruzioni di funzionamento
	UFUNCTION() void OnPlayButtonClicked();																				//funzione da chiamare quando si interagisce fisicamente con il button
	UFUNCTION() void OnPlayButtonHovered();																				//funzione estetica per far rimpicciolire il button quando ci passo sopra, 
	UFUNCTION() void OnPlayButtonUnhovered();																		    //con questo invece lo riportiamo a dimensioni reali quando non ci sono sopra con il mouse
};
