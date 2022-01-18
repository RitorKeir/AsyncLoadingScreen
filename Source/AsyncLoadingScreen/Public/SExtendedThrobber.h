#pragma once

#include "Widgets/Images/SThrobber.h"

/**
 * A throbber widget that orients images in a spinning circle.
 */
class ASYNCLOADINGSCREEN_API SExtendedCircularThrobber
	: public SCircularThrobber
{
	static const float MinimumPeriodValue;

	SLATE_BEGIN_ARGS(SExtendedCircularThrobber)
		: _PieceImage( FCoreStyle::Get().GetBrush( "Throbber.CircleChunk" ) )
		, _NumPieces( 6 )
		, _Period( 0.75f )
		, _Radius( 16.f )
	{}

	/** What each segment of the throbber looks like */
	SLATE_ARGUMENT( const FSlateBrush*, PieceImage )
		/** How many pieces there are */
		SLATE_ARGUMENT( int32, NumPieces )
		/** The amount of time in seconds for a full circle */
		SLATE_ARGUMENT( float, Period )
		/** The radius of the circle */
		SLATE_ARGUMENT( float, Radius )
		/** Throbber color and opacity */
		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)

	SLATE_END_ARGS()

		/** Constructs the widget */
		void Construct(const FArguments& InArgs);

	/** Sets what each segment of the throbber looks like */
	void SetPieceImage(const FSlateBrush* InPieceImage);

	/** Sets how many pieces there are */
	void SetNumPieces(int32 InNumPieces);

	/** Sets the amount of time in seconds for a full circle */
	void SetPeriod(float InPeriod);

	/** Sets the radius of the circle */
	void SetRadius(float InRadius);

	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;

private:

	/** Constructs the sequence used to animate the throbber. */
	void ConstructSequence();

private:

	/** The sequence to drive the spinning animation */
	FCurveSequence Sequence;
	FCurveHandle Curve;

	/** What each segment of the throbber looks like */
	const FSlateBrush* PieceImage;

	/** How many pieces there are */
	int32 NumPieces;

	/** The amount of time in seconds for a full circle */
	float Period;

	/** The radius of the circle */
	float Radius;

	/** Color and opacity of the throbber images. */
	TAttribute<FSlateColor> ColorAndOpacity;
};
