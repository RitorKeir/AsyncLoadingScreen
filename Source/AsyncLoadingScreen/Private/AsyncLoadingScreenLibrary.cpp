/************************************************************************************
 *																					*
 * Copyright (C) 2020 Truong Bui.													*
 * Website:	https://github.com/truong-bui/AsyncLoadingScreen						*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/


#include "AsyncLoadingScreenLibrary.h"
#include "MoviePlayer.h"
#include "LoadingScreenSettings.h"
#include "LoadingScreenWidget.h"

//#if WITH_EDITOR
//#pragma optimize("", off)
//#endif


int32 UAsyncLoadingScreenLibrary::DisplayBackgroundIndex = -1;
int32 UAsyncLoadingScreenLibrary::DisplayTipTextIndex = -1;
int32 UAsyncLoadingScreenLibrary::DisplayMovieIndex = -1;

void UAsyncLoadingScreenLibrary::SetDisplayBackgroundIndex(int32 BackgroundIndex)
{
	UAsyncLoadingScreenLibrary::DisplayBackgroundIndex = BackgroundIndex;
}

void UAsyncLoadingScreenLibrary::SetDisplayTipTextIndex(int32 TipTextIndex)
{
	UAsyncLoadingScreenLibrary::DisplayTipTextIndex = TipTextIndex;
}

void UAsyncLoadingScreenLibrary::SetDisplayMovieIndex(int32 MovieIndex)
{
	UAsyncLoadingScreenLibrary::DisplayMovieIndex = MovieIndex;
}

const FALoadingScreenSettings* UAsyncLoadingScreenLibrary::GetLoadingScreenSettingsByName(const FName& settings_name)
{
	const ULoadingScreenSettings* settings{GetDefault<ULoadingScreenSettings>()};
	const FALoadingScreenSettings* loading_settings{&settings->StartupLoadingScreen};
	if (!settings_name.IsNone())
	{
		if (settings_name == FName(TEXT("__default")))
		{
			loading_settings = &settings->DefaultLoadingScreen;
		}
		else
		{
			loading_settings = settings->CustomLoadingScreens.Find(settings_name);
			if (loading_settings == nullptr)
			{
				loading_settings = &settings->DefaultLoadingScreen;
			}
		}
	}

	return loading_settings;
}

void UAsyncLoadingScreenLibrary::StartLoadingScreen(FName custom_settings_name)
{
	const ULoadingScreenSettings* settings{GetDefault<ULoadingScreenSettings>()};
	const FALoadingScreenSettings* loading_settings{GetLoadingScreenSettingsByName(custom_settings_name)};

	SetupLoadingScreen(*loading_settings);
}

void UAsyncLoadingScreenLibrary::SetupLoadingScreen(const FALoadingScreenSettings& loading_settings)
{
	if (loading_settings.bShowWidgetOverlay == false && loading_settings.MoviePaths.Num() == 0)
	{
		// No loading elemets to show
		return;
	}
	
	TArray<FString> movies_list = loading_settings.MoviePaths;
	// Shuffle the movies list
	if (loading_settings.bShuffle == true)
	{
		ShuffleMovies(movies_list);
	}

	if (loading_settings.bSetDisplayMovieIndexManually == true)
	{
		movies_list.Empty();

		// Show specific movie if valid otherwise show original movies list
		if (loading_settings.MoviePaths.IsValidIndex(GetDisplayMovieIndex()))
		{
			movies_list.Add(loading_settings.MoviePaths[GetDisplayMovieIndex()]);
		}
		else
		{
			movies_list = loading_settings.MoviePaths;
		}
	}

	FLoadingScreenAttributes loading_screen;
	loading_screen.MinimumLoadingScreenDisplayTime   = loading_settings.MinimumLoadingScreenDisplayTime;
	loading_screen.bAutoCompleteWhenLoadingCompletes = loading_settings.bAutoCompleteWhenLoadingCompletes;
	loading_screen.bMoviesAreSkippable               = loading_settings.bMoviesAreSkippable;
	loading_screen.bWaitForManualStop                = loading_settings.bWaitForManualStop;
	loading_screen.bAllowInEarlyStartup              = loading_settings.bAllowInEarlyStartup;
	loading_screen.bAllowEngineTick                  = loading_settings.bAllowEngineTick;
	loading_screen.MoviePaths                        = movies_list;
	loading_screen.PlaybackType                      = loading_settings.PlaybackType;

	if (loading_settings.bShowWidgetOverlay)
	{
		loading_screen.WidgetLoadingScreen = ULoadingScreenWidget::CreateSlateWidget(loading_settings);
	}

	GetMoviePlayer()->SetupLoadingScreen(loading_screen);
}

void UAsyncLoadingScreenLibrary::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void UAsyncLoadingScreenLibrary::ShuffleMovies(TArray<FString>& MoviesList)
{
	if (MoviesList.Num() > 0)
	{
		int32 LastIndex = MoviesList.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				MoviesList.Swap(i, Index);
			}
		}
	}
}


//#if WITH_EDITOR
//#pragma optimize("", on)
//#endif
