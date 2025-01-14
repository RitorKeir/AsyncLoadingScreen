#include "CustomMoviePlayerThreading.h"
#include "MoviePlayer.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Framework/Application/SlateApplication.h"
#include "CustomMoviePlayer.h"
#include "HAL/PlatformApplicationMisc.h"

FThreadSafeCounter FCustomSlateLoadingSynchronizationMechanism::LoadingThreadInstanceCounter;

/**
 * The Slate thread is simply run on a worker thread.
 * Slate is run on another thread because the game thread (where Slate is usually run)
 * is blocked loading things. Slate is very modular, which makes it very easy to run on another
 * thread with no adverse effects.
 * It does not enqueue render commands, because the RHI is not thread safe. Thus, it waits to
 * enqueue render commands until the render thread tickables ticks, and then it calls them there.
 */
class FCustomSlateLoadingThreadTask : public FRunnable
{
public:
	FCustomSlateLoadingThreadTask(class FCustomSlateLoadingSynchronizationMechanism& InSyncMechanism)
		: SyncMechanism(&InSyncMechanism)
	{
	}

	/** FRunnable interface */
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
private:
	/** Hold a handle to our parent sync mechanism which handles all of our threading locks */
	class FCustomSlateLoadingSynchronizationMechanism* SyncMechanism;
};



FCustomSlateLoadingSynchronizationMechanism::FCustomSlateLoadingSynchronizationMechanism(
	TSharedPtr<FCustomMoviePlayerWidgetRenderer, ESPMode::ThreadSafe> InWidgetRenderer, 
	const TSharedPtr<IMovieStreamer, ESPMode::ThreadSafe>& InMovieStreamer)
	: WidgetRenderer(InWidgetRenderer)
	, MovieStreamer(InMovieStreamer)
{
}

FCustomSlateLoadingSynchronizationMechanism::~FCustomSlateLoadingSynchronizationMechanism()
{
	DestroySlateThread();
}

void FCustomSlateLoadingSynchronizationMechanism::Initialize()
{
	check(IsInGameThread());

	ResetSlateDrawPassEnqueued();
	SetSlateMainLoopRunning();

	bMainLoopRunning = true;

	FString ThreadName = TEXT("CustomSlateLoadingThread");
	ThreadName.AppendInt(LoadingThreadInstanceCounter.Increment());

	SlateRunnableTask = new FCustomSlateLoadingThreadTask( *this );
	SlateLoadingThread = FRunnableThread::Create(SlateRunnableTask, *ThreadName);
}

void FCustomSlateLoadingSynchronizationMechanism::DestroySlateThread()
{
	check(IsInGameThread());

	if (SlateLoadingThread)
	{
		IsRunningSlateMainLoop.Reset();

		while (bMainLoopRunning)
		{
			FPlatformApplicationMisc::PumpMessages(false);

			FPlatformProcess::Sleep(0.f);
		}

		delete SlateLoadingThread;
		delete SlateRunnableTask;
		SlateLoadingThread = nullptr;
		SlateRunnableTask = nullptr;
	}
}

bool FCustomSlateLoadingSynchronizationMechanism::IsSlateDrawPassEnqueued()
{
	return IsSlateDrawEnqueued.GetValue() != 0;
}

void FCustomSlateLoadingSynchronizationMechanism::SetSlateDrawPassEnqueued()
{
	IsSlateDrawEnqueued.Set(1);
}

void FCustomSlateLoadingSynchronizationMechanism::ResetSlateDrawPassEnqueued()
{
	IsSlateDrawEnqueued.Reset();
}

bool FCustomSlateLoadingSynchronizationMechanism::IsSlateMainLoopRunning()
{
	return IsRunningSlateMainLoop.GetValue() != 0;
}

void FCustomSlateLoadingSynchronizationMechanism::SetSlateMainLoopRunning()
{
	IsRunningSlateMainLoop.Set(1);
}

void FCustomSlateLoadingSynchronizationMechanism::ResetSlateMainLoopRunning()
{
	IsRunningSlateMainLoop.Reset();
}

void FCustomSlateLoadingSynchronizationMechanism::SlateThreadRunMainLoop()
{
	double LastTime = FPlatformTime::Seconds();

	while (IsSlateMainLoopRunning())
	{
		double CurrentTime = FPlatformTime::Seconds();
		double DeltaTime = CurrentTime - LastTime;

		// 60 fps max
		const double MaxTickRate = 1.0/60.0f;

		const double TimeToWait = MaxTickRate - DeltaTime;

		if( TimeToWait > 0 )
		{
			FPlatformProcess::Sleep(TimeToWait);
			CurrentTime = FPlatformTime::Seconds();
			DeltaTime = CurrentTime - LastTime;
		}

		if (FSlateApplication::IsInitialized() && !IsSlateDrawPassEnqueued())
		{
			// Tick engine stuff.
			if (MovieStreamer.IsValid())
			{
				MovieStreamer->TickPreEngine();
				MovieStreamer->TickPostEngine();
			}

			//FSlateRenderer* MainSlateRenderer = FSlateApplication::Get().GetRenderer();
			//FScopeLock ScopeLock(MainSlateRenderer->GetResourceCriticalSection());

			WidgetRenderer->DrawWindow(DeltaTime);

			SetSlateDrawPassEnqueued();

			// Tick after rendering.
			if (MovieStreamer.IsValid())
			{
				MovieStreamer->TickPostRender();
			}
		}

		LastTime = CurrentTime;
	}
	
	while (IsSlateDrawPassEnqueued())
	{
		FPlatformProcess::Sleep(1.f / 60.f);
	}
	
	bMainLoopRunning = false;
}


bool FCustomSlateLoadingThreadTask::Init()
{
	// First thing to do is set the slate loading thread ID
	// This guarantees all systems know that a slate thread exists
	GSlateLoadingThreadId = FPlatformTLS::GetCurrentThreadId();

	return true;
}

uint32 FCustomSlateLoadingThreadTask::Run()
{
	check( GSlateLoadingThreadId == FPlatformTLS::GetCurrentThreadId() );

	SyncMechanism->SlateThreadRunMainLoop();

	// Tear down the slate loading thread ID
	FPlatformAtomics::InterlockedExchange((int32*)&GSlateLoadingThreadId, 0);

	return 0;
}

void FCustomSlateLoadingThreadTask::Stop()
{
	SyncMechanism->ResetSlateDrawPassEnqueued();
	SyncMechanism->ResetSlateMainLoopRunning();
}
