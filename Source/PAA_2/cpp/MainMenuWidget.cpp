#include "../headers/MainMenuWidget.h"
#include "../headers/MyGameMode.h"
#include "../headers/CoinWidget.h" 
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::SetGameMode(AMyGameMode* GameMode)																//concettualmente impostiamo i vari widget come dei pannelli sovrapposti che andiamo a mettere in vista e rimuovere dopo che li abbiamo usato
{																														//per esempio faremo scomparire questo dopo che avremo premuto il pulsante, e dopo faremo scomparire quello della coin
	GameModeRef = GameMode;
}

void UMainMenuWidget::NativeConstruct()																					//così come per tutte le classi il nativecosnstruct è il primo elemento che viene creato, in questo caso appena si preme start compare il mainmenuwidget
{																														//è il primo pezzo del codice ad essere interpellato per l'inizializzazione/chiamata di funzioni/collegare eventi
	Super::NativeConstruct();

	PlayButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnPlayButtonHovered);										//banali funzioni per hover tipo html e css per far rimpicciolire e ingrandire il pulsante quando il mouse 
	PlayButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnPlayButtonUnhovered);									//vi passa sopra, l'ho fatto solo per aggiungere una minima dinamicità estetica della pagina
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayButtonClicked);									//questo invece verifica che playbutton (bottone omonimo in blueprint widget) sia collegato, se lo è abilita
	}																													//la funzione di click
}

void UMainMenuWidget::OnPlayButtonHovered()					
{
	if (PlayButton)
	{
		PlayButton->SetRenderScale(FVector2D(0.9f, 0.9f));														//comando per settare la scald sulle x e y di un oggetto bidimensionale ad una dimensione scelta
	}
}

void UMainMenuWidget::OnPlayButtonUnhovered()
{
	if (PlayButton)
	{
		PlayButton->SetRenderScale(FVector2D(1.0f, 1.0f));														//comando per far tornare la dimensione dell'oggetto a quella reale quando il mouse non vi è più sopra
	}
}


void UMainMenuWidget::OnPlayButtonClicked()																				//bottone collegato, ora configuriamo cosa succede se vi clicco sopra
{
	if (!CoinWidgetClass)																								//il comando è banale, questa classe è per gestire l'introduzione al gioco, una schermata iniziale, quindi
	{																													//la configuro solamente per passare alla schermata del lancio moneta quanod clicco
		UE_LOG(LogTemp, Error, TEXT("CoinWidgetClass non è configurato."));
		return;
	}																													//se non è configurato il collegamento alla schermata seguente, log di errore ed esce, altrimenti continua

	UE_LOG(LogTemp, Warning, TEXT("CoinWidgetClass è configurato correttamente."));

	CoinWidgetInstance = CreateWidget<UCoinWidget>(GetWorld(), CoinWidgetClass);
	if (!CoinWidgetInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Impossibile creare CoinWidget."));
		return;
	}
																														//creo la widgetinstance per definirne fisicamente il comporamento del widget e le sue funzionalità	
	UE_LOG(LogTemp, Warning, TEXT("CoinWidget creato con successo."));											//il widgetinstance da invece il collgamento fisico ad esso 

	CoinWidgetInstance->AddToViewport();
	UE_LOG(LogTemp, Warning, TEXT("CoinWidget aggiunto alla viewport."));										//infatti tramite l'istance quando premo pulsante creo collegamento e faccio comparire coinwidget

	CoinWidgetInstance->SetGameMode(GameModeRef);

	RemoveFromParent();																									//faccio scomparire il mainmenuwidget per rendere interagibile il coinwidget
	UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget rimosso dalla viewport."));
}