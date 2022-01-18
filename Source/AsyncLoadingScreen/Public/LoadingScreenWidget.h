#pragma once

#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

struct FALoadingScreenSettings;

/**
 * Use LoadingScreenWidget to create identicaly widget in others UserWidget created in blueprints
 */
UCLASS()
class ASYNCLOADINGSCREEN_API ULoadingScreenWidget : public UWidget
{
	GENERATED_BODY()

public:
	ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
	}

	virtual void SynchronizeProperties() override;

	static TSharedPtr<SWidget> CreateSlateWidget(const FALoadingScreenSettings& loading_settings);

	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	UPROPERTY(EditAnywhere, Category = "LoadingScreenWidget")
	FName LoadingScreenConfig;
};
