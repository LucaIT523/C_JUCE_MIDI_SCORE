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

#include <iostream>
#include <string>

#include "JuceSystem.h"
#include "JuceDevice.h"
#include "JuceFont.h"
#include "GuidoComponent.h"

#define __DebugDevice 0
#if __DebugDevice
#include "DebugDevice.h"
#endif

using namespace std;


//==============================================================================
GuidoComponent::GuidoComponent () 
	: fARHandler(0), fGRHandler(0), fPage(0), fResizeToMusic(false), fScoreColor(0,0,0)
{
	GuidoGetDefaultLayoutSettings (&fSettings);
	setSize (600, 300);
}

GuidoComponent::~GuidoComponent()
{
	if (fGRHandler) GuidoFreeGR (fGRHandler);
	if (fARHandler) GuidoFreeAR (fARHandler);
}


//==============================================================================
GuidoErrCode GuidoComponent::GuidoInit (int width, int height, int ofX, int ofY, const char* textfont, const char* guidofont)
{
	fWidth = width;
	fHeight = height;
	fOfX = ofX;
	fOfY = ofY;	
	fCurrentIndex = -1;
	static JuceSystem gSystem(0);
	static JuceDevice gDevice(0, &gSystem, fOfX, fOfY);
	desc.graphicDevice = &gDevice;
	desc.musicFont	= guidofont;
	desc.textFont	= textfont;
	return  ::GuidoInit(&desc);
}
void GuidoComponent::setCurrentIndex(int  currentIndex)
{
	fCurrentIndex = currentIndex;
}
int GuidoComponent::getCurrentIndex()
{
	return fCurrentIndex;
}
int GuidoComponent::getNextIndex()
{
	fCurrentIndex++;
	if (fCurrentIndex > ((JuceDevice*)desc.graphicDevice)->getMusicSymbolSize()) {
		fCurrentIndex = -1;
	}
	return fCurrentIndex;
}
int GuidoComponent::getCurrentPos(int  currentIndex)
{
	return ((JuceDevice*)desc.graphicDevice)->getMusicSymbolPos(currentIndex);
}
//==============================================================================
GuidoErrCode GuidoComponent::setGMNFile (const char* file )
{
	ARHandler arh;
    GuidoErrCode err = GuidoParseFile (file, &arh);
	if (err != guidoNoErr) return err;
	
	err = setARHandler (arh);
	if (err != guidoNoErr) GuidoFreeAR (arh);
	else {
		File f(file);
		fGMNCode = f.loadFileAsString ();
		fGMNFile = file;
		setGMNCode(fGMNCode.toRawUTF8());
	}
	return err;
}

//==============================================================================
GuidoErrCode GuidoComponent::setGMNCode (const char* gmnCode )
{
	ARHandler arh;
    GuidoErrCode err = GuidoParseString (gmnCode, &arh);
	if (err != guidoNoErr) return err;
	
	err = setARHandler (arh);
	if (err != guidoNoErr) GuidoFreeAR (arh);
	else fGMNCode = gmnCode;
	return err;
}

//==============================================================================
GuidoErrCode GuidoComponent::setARHandler(ARHandler ar)
{
	GRHandler grh;
	GuidoErrCode err = GuidoAR2GR (ar, &fSettings, &grh);
	if (err == guidoNoErr) {
		if (fGRHandler) GuidoFreeGR (fGRHandler);
		fGRHandler = grh;
		fARHandler = ar;
		fPage = 1;
		if (fResizeToMusic) GuidoResizePageToMusic (fGRHandler);
		repaint();
	}
	else GuidoFreeAR (ar);
	return err;
}

//==============================================================================
void GuidoComponent::setScoreColor(const juce::Colour& color)
{
	if (fScoreColor != color) {
		fScoreColor = color;
		repaint();
	}
}

//==============================================================================
void GuidoComponent::setGuidoLayoutSettings(const GuidoLayoutSettings& layoutSettings)
{
	fSettings = layoutSettings;
	if (fGRHandler) {
		GuidoUpdateGR (fGRHandler, &fSettings);
		repaint();
	}
}

//==============================================================================
void GuidoComponent::setResizePageToMusic(bool isOn)
{
	if (fResizeToMusic != isOn) {
		fResizeToMusic = isOn;
		setARHandler (fARHandler);
	}
}

//==============================================================================
int GuidoComponent::pageCount() const
{
	return fGRHandler ? GuidoGetPageCount (fGRHandler) : 0;
}

//==============================================================================
int GuidoComponent::voiceCount() const
{
	return fARHandler ? GuidoCountVoices (fARHandler) : 0;
}

//==============================================================================
void GuidoComponent::paint (Graphics& g)
{
	if (!fGRHandler) return;

	JuceSystem sys (&g);
#if __DebugDevice
	JuceDevice* jdev = new JuceDevice(&g, &sys);
	DebugDevice dev (jdev);
#else
	//JuceDevice dev (&g, &sys, fOfX, fOfY);
	JuceDevice* dev = (JuceDevice*)desc.graphicDevice;
	dev->SetJuceDevice(&g, &sys);
#endif
	dev->NotifySize(getWidth(), getHeight());

	g.setColour (fScoreColor);
	GuidoOnDrawDesc desc;
	desc.handle = fGRHandler;
	desc.hdc = dev;
	desc.page = fPage;
	desc.scrollx = desc.scrolly = 0;
	desc.isprint = false;
	desc.updateRegion.erase = true;
	desc.sizex = fWidth;//getWidth()+2000;
	desc.sizey = getHeight();

	GuidoErrCode err = GuidoOnDraw (&desc);
	if (err != guidoNoErr)
		cerr << "error while painting: " << GuidoGetErrorString(err) << endl;

	dev->DrawMusicTimeRect(fCurrentIndex);
}

