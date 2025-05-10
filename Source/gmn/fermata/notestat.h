/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifndef __notestat_h__
#define __notestat_h__

#include "liste.h"
#include <stdio.h>

//! statistical values for duration class of binStatistics modul
class TFracBinClass : public TDoubleBinClass  
{
    /// norm value of class
    TFrac durationI;
public:
	virtual string toString( void );
	virtual void debug( FILE *out );
    /// increment class counter
    virtual void addValue(double val, double norm);
    TFrac duration( void );
    /// distance between norm value and duration
    virtual double distance( double *val, 
							 TFrac *duration, 
							 double maxWeight,
							double lAlpha,
							double rAlpha,
							int cClasses);
    /// return norm value
    virtual  double FirstV( void );
    /// return current av value
    virtual double Value( void );
    TFracBinClass(TFrac durationI,
              int id,
              /// bias
              double b);
    virtual ~TFracBinClass();
    // double typicalIOI;
};


#endif
