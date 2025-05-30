#ifndef __JuceDevice__
#define __JuceDevice__

/*
	GUIDO Library
	Copyright (C) 2012	Grame

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License (Version 2), 
	as published by the Free Software Foundation.
	A copy of the license can be found online at www.gnu.org/licenses.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include <stack>

#include "VGDevice.h"
#include "VGColor.h"
#include <vector>

class VGFont;
class VGSystem;
namespace juce { class Graphics; }

// --------------------------------------------------------------
// 		Juce implementation of the VGDevice class
// --------------------------------------------------------------
class JuceDevice : public VGDevice
{
	std::stack<VGColor> 	fFillColorStack;
	std::stack<VGColor> 	fPenColorStack;
	std::stack<float> 		fPenWidthStack;
	VGColor		fFillColor;
	VGColor		fPenColor;
	VGColor		fFontColor;
	VGColor     fRectColor;
	VGColor		fFontBackgroundColor;

	juce::Graphics * fGraphics;
	bool	fFreeGraphics;
	float	fXPos, fYPos;
	float	fXOrigin, fYOrigin;
	float	fXScale, fYScale;
	int		fWidth, fHeight;
	float	fLineThick;
	float	fDPI;
	unsigned int	fFontAlign;
	VRasterOpMode	fRasterOpMode;

	const VGFont*	fTextFont;
	const VGFont*	fMusicFont;
	const VGFont*	fCurrentFont;
	VGSystem*		fSystem;

	int fOfX;
	int fOfY;

	std::vector<unsigned int> fScoreTime;
	bool fScoreRead;
	int fScoreIndex;

	void	initialize();

	public:
		JuceDevice(juce::Graphics* g, VGSystem* sys = 0, int ofX = 100, int ofY = 100);
					 JuceDevice(int width, int height, VGSystem* sys=0);
		virtual		~JuceDevice();

		virtual bool			IsValid() const		{ return fGraphics != 0; }
		// - Drawing services ------------------------------------------------
		virtual bool			BeginDraw();
		virtual void			EndDraw();

		/// Invalidate a rectangle i.e. indicates the native graphic device
		/// that the corresponding rectangle needs to be refreshed.
		virtual void			InvalidateRect( float left, float top, float right, float bottom );

		// - Standard graphic primitives -------------------------
		virtual void			MoveTo( float x, float y );
		virtual void			LineTo( float x, float y );
		virtual void			Line( float x1, float y1, float x2, float y2 );
		virtual void			Frame( float left, float top, float right, float bottom );
		virtual void			Arc( float left,   float top, float right,  float bottom,
									 float startX, float startY, float endX,   float endY );
		virtual void			FrameEllipse( float x, float y, float width, float height);

		// - Filled surfaces --------------------------------------
		// The raster op mode for color filling should be specified
		// with SetRasterOpMode() before using one of these.
		virtual void			Ellipse( float x, float y, float width, float height, const VGColor& color);
		virtual	void			Triangle( float x1, float y1, float x2, float y2, float x3, float y3 );
		virtual	void			Polygon( const float * xCoords, const float * yCoords, int count );
		virtual void			Rectangle( float left,  float top, float right, float bottom );

		// - Font services ---------------------------------------------------
		virtual	void			SetMusicFont( const VGFont * font );
		virtual	const VGFont *	GetMusicFont() const;
		virtual	void			SetTextFont( const VGFont * font );
		virtual	const VGFont *	GetTextFont() const;

		// - Pen & brush services --------------------------------------------
		virtual	void			SelectPen( const VGColor & inColor, float witdh );
		virtual	void			SelectFillColor( const VGColor & c );
		virtual	void			PushPen( const VGColor & inColor, float inWidth );
		virtual	void			PopPen();
		virtual	void			PushFillColor( const VGColor & inColor );
		virtual	void			PopFillColor();

		virtual	void			SetRasterOpMode( VRasterOpMode ROpMode);
		virtual	VRasterOpMode	GetRasterOpMode() const;


		// - Bitmap services (bit-block copy methods) --------------------------
		virtual bool			CopyPixels( VGDevice* pSrcDC, float alpha = -1.0);
		virtual bool			CopyPixels( int xDest, int yDest, VGDevice* pSrcDC, int xSrc, int ySrc,
											int nSrcWidth, int nSrcHeight, float alpha = -1.0);
		virtual bool			CopyPixels( int xDest, int yDest, int dstWidth, int dstHeight,
											VGDevice* pSrcDC, float alpha = -1.0);
		virtual bool			CopyPixels( int xDest, int yDest, int dstWidth, int dstHeight,
											VGDevice* pSrcDC, int xSrc, int ySrc,
											int nSrcWidth, int nSrcHeight, float alpha = -1.0);

		// - Coordinate services ------------------------------------------------
		virtual	void			SetScale( float x, float y );
		virtual	void			SetOrigin( float x, float y );
		virtual	void			OffsetOrigin( float x, float y );

		virtual	void			LogicalToDevice( float * x, float * y ) const;
		virtual	void			DeviceToLogical( float * x, float * y ) const;

		virtual float			GetXScale() const;
		virtual	float			GetYScale() const;
		virtual	float			GetXOrigin() const;
		virtual	float			GetYOrigin() const;

		virtual	void			NotifySize( int inWidth, int inHeight );
		virtual	int				GetWidth() const;
		virtual	int				GetHeight() const;

		// - Text and music symbols services -------------------------------------
		virtual void			SetJuceDevice(juce::Graphics* g, VGSystem* sys);
		virtual bool			DrawMusicTimeRect(int scoreIndex);
		virtual bool			DrawMusicTimeRect(int scoreIndex, int x);
		virtual int				DrawMusicTimeNextRect();
		virtual int				getMusicSymbolSize();
		virtual int				getMusicSymbolPos(int scoreIndex);
		virtual void			DrawMusicSymbol(float x, float y, unsigned int inSymbolID );
		virtual	void			DrawString( float x, float y, const char * s, int inCharCount );
		virtual	void			SetFontColor( const VGColor & inColor );
		virtual	void			SetRectColor(const VGColor& inColor);
		virtual	VGColor			GetFontColor() const;
		virtual	void			SetFontBackgroundColor( const VGColor & inColor );
		virtual	VGColor			GetFontBackgroundColor() const;
		virtual	void			SetFontAlign( unsigned int inAlign );
		virtual	unsigned int	GetFontAlign() const;

		// - Printer informations services ----------------------------------------
		virtual	void			SetDPITag( float inDPI );
		virtual	float			GetDPITag() const;

		virtual void*			GetBitMapPixels()		{ return 0; }
		virtual void			ReleaseBitMapPixels()	{}

		/// temporary hack - must be removed asap
		virtual	VGSystem *		getVGSystem() const			{ return fSystem; }
		/// Returns the platform-specific device context object.
		virtual void *			GetNativeContext() const	{ return fGraphics; }


		/// Gives the current device data and returns the data associated mime type.
		virtual const char*		GetImageData(const char* & outDataPtr, int& outLength) { return 0; }
		/// Release the pointer returned by GetImageData
		virtual void			ReleaseImageData(const char *) const {}

		// - VGDevice extension --------------------------------------------
		virtual	void			SelectPenColor( const VGColor & inColor);
		virtual	void			SelectPenWidth( float witdh);
		virtual	void			PushPenColor( const VGColor & inColor);
		virtual	void			PopPenColor();		
		virtual	void			PushPenWidth( float width);
		virtual	void			PopPenWidth();
};

#endif
