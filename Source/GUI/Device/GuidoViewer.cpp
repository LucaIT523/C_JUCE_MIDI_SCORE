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

#include "GuidoViewer.h"

using namespace std;
using namespace juce;

//==============================================================================
GuidoViewer::GuidoViewer()
{
	setResizePageToMusic(true);
}
//-------------------------------------------------------------------------------
GuidoErrCode GuidoViewer::LoadFile()
{
	return setGMNFile("D:\\sample.gmn");
}

//-------------------------------------------------------------------------------
void GuidoViewer::paint(Graphics& g)
{
	GuidoComponent::paint(g);
}

