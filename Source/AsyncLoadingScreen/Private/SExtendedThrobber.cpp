#include "SExtendedThrobber.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"


// SExtendedCircularThrobber
const float SExtendedCircularThrobber::MinimumPeriodValue = SMALL_NUMBER;

void SExtendedCircularThrobber::Construct(const FArguments& InArgs)
{
	PieceImage = InArgs._PieceImage;
	NumPieces = InArgs._NumPieces;
	Period = InArgs._Period;
	Radius = InArgs._Radius;
	ColorAndOpacity = InArgs._ColorAndOpacity;

	ConstructSequence();
}

void SExtendedCircularThrobber::SetPieceImage(const FSlateBrush* InPieceImage)
{
	PieceImage = InPieceImage;
}

void SExtendedCircularThrobber::SetNumPieces(const int32 InNumPieces)
{
	NumPieces = InNumPieces;
}

void SExtendedCircularThrobber::SetPeriod(const float InPeriod)
{
	Period = InPeriod;
	ConstructSequence();
}

void SExtendedCircularThrobber::SetRadius(const float InRadius)
{
	Radius = InRadius;
}

void SExtendedCircularThrobber::ConstructSequence()
{
	Sequence = FCurveSequence();
	Curve = Sequence.AddCurve(0.f, FMath::Max(FMath::Abs(Period), MinimumPeriodValue));
	Sequence.Play(this->AsShared(), true);
}

int32 SExtendedCircularThrobber::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FLinearColor FinalColorAndOpacity;
	if (ColorAndOpacity.IsSet())
	{
		FinalColorAndOpacity = ColorAndOpacity.Get().GetColor(InWidgetStyle);
	}
	else
	{
		FinalColorAndOpacity = InWidgetStyle.GetColorAndOpacityTint() * PieceImage->GetTint(InWidgetStyle);
	}

	const FVector2D LocalOffset = (AllottedGeometry.GetLocalSize() - PieceImage->ImageSize) * 0.5f;
	const float Phase = Period < 0.0 ? PI - Curve.GetLerp() * 2 * PI : Curve.GetLerp() * 2 * PI;

	if (Radius > 0.0f)
	{
		const float DeltaAngle = NumPieces > 0 ? 2 * PI / NumPieces : 0;

		for (int32 PieceIdx = 0; PieceIdx < NumPieces; ++PieceIdx)
		{
			const float Angle = DeltaAngle * PieceIdx + Phase;
			// scale each piece linearly until the last piece is full size
			FSlateLayoutTransform PieceLocalTransform(
				(PieceIdx + 1) / (float)NumPieces,
				LocalOffset + LocalOffset * FVector2D(FMath::Sin(Angle), FMath::Cos(Angle)));
			FPaintGeometry PaintGeom = AllottedGeometry.ToPaintGeometry(PieceImage->ImageSize, PieceLocalTransform);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeom, PieceImage, ESlateDrawEffect::None, FinalColorAndOpacity);
		}
	}
	else
	{
		// scale each piece linearly until the last piece is full size
		FSlateLayoutTransform PieceLocalTransform(1.0f, LocalOffset);
		FPaintGeometry PaintGeom = AllottedGeometry.ToPaintGeometry(PieceImage->ImageSize, PieceLocalTransform, FSlateRenderTransform(FQuat2D(Phase)));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeom, PieceImage, ESlateDrawEffect::None, FinalColorAndOpacity);
	}
	
	return LayerId;
}

FVector2D SExtendedCircularThrobber::ComputeDesiredSize(float) const
{
	if (Radius > 0)
	{
		return FVector2D(Radius, Radius) * 2;
	}

	return PieceImage->ImageSize;
}
