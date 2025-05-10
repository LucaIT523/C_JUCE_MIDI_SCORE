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

#ifndef __GuidoViewer__
#define __GuidoViewer__


#include "GuidoComponent.h"
#include "JuceHeader.h"

class MainAppWindow;
//==============================================================================
class GuidoViewer :
	public GuidoComponent
{

public:
	//==============================================================================
	GuidoViewer();
	virtual ~GuidoViewer() {}


	//==============================================================================
	GuidoErrCode LoadFile();
	void paint(Graphics& g);		// paint overrided for drop feedback


private:
	//==============================================================================

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuidoViewer);
};

#endif
