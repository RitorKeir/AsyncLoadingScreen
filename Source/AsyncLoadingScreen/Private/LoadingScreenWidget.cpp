
#include "LoadingScreenWidget.h"
#include "CoreMinimal.h"
#include "LoadingScreenSettings.h"
#include "AsyncLoadingScreenLibrary.h"
#include "SCenterLayout.h"
#include "SClassicLayout.h"
#include "SLetterboxLayout.h"
#include "SSidebarLayout.h"
#include "SDualSidebarLayout.h"

//#if WITH_EDITOR
//#pragma optimize("", off)
//#endif


void ULoadingScreenWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

TSharedPtr<SWidget> ULoadingScreenWidget::CreateSlateWidget(const FALoadingScreenSettings& loading_settings)
{
	TSharedPtr<SWidget> loading_widget;

	const ULoadingScreenSettings* settings{GetDefault<ULoadingScreenSettings>()};
	switch (loading_settings.Layout)
	{
	case EAsyncLoadingScreenLayout::ALSL_Classic:
		loading_widget = SNew(SClassicLayout, loading_settings, settings->Classic);
		break;
	case EAsyncLoadingScreenLayout::ALSL_Center:
		loading_widget = SNew(SCenterLayout, loading_settings, settings->Center);
		break;
	case EAsyncLoadingScreenLayout::ALSL_Letterbox:
		loading_widget = SNew(SLetterboxLayout, loading_settings, settings->Letterbox);
		break;
	case EAsyncLoadingScreenLayout::ALSL_Sidebar:
		loading_widget = SNew(SSidebarLayout, loading_settings, settings->Sidebar);
		break;
	case EAsyncLoadingScreenLayout::ALSL_DualSidebar:
		loading_widget = SNew(SDualSidebarLayout, loading_settings, settings->DualSidebar);
		break;
	case EAsyncLoadingScreenLayout::ALSL_CustomWidget:
		if (GEngine && loading_settings.CustomLoadingWidget.IsNull() == false)
		{
			UUserWidget* new_widget{NewObject<UUserWidget>(GEngine, loading_settings.CustomLoadingWidget.LoadSynchronous(), TEXT("LoadingScreen"), RF_Transactional)};
			new_widget->Initialize();
			//UUserWidget* new_widget{CreateWidget<UUserWidget>(GEngine->GetWorld(), loading_settings.CustomLoadingWidget.LoadSynchronous())};

			// Root widget and background
			loading_widget = SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SScaleBox)
					.Stretch(loading_settings.Background.ImageStretch)
					[
						new_widget->TakeWidget()
					]
				];

			//loading_widget = new_widget->TakeWidget();
		}
		break;
	}

	return loading_widget;
}

TSharedRef<SWidget> ULoadingScreenWidget::RebuildWidget()
{
	const FALoadingScreenSettings* loading_settings{UAsyncLoadingScreenLibrary::GetLoadingScreenSettingsByName(LoadingScreenConfig)};
	TSharedPtr<SWidget> widget{CreateSlateWidget(*loading_settings)};
	return widget.ToSharedRef();
}

//#if WITH_EDITOR
//#pragma optimize("", on)
//#endif
