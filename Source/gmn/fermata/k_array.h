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

#if !defined (__k_array_h__)
#define __k_array_h__

#include <stdio.h>

#include "note.h"

#define FeatureVal double

#define CFEATURES 7

class TCLICKNOTE;

/// statistic features for a single pattern implemented as a semantic array
class TFeatures
{
    /// array of floats/doubles each position has a specific semantic meaning
    FeatureVal *valI;
    /// len of array
    int cFeaturesI;
    /// create array
    void createValues( int c);
public:
    TFeatures( void );
    TFeatures( TFeatures *ptr);
    virtual ~TFeatures( void );
    
    int cFeatures(void ){return cFeaturesI;};
    FeatureVal val( int i){return valI[i];};

    void IncCNotes(void)
    { valI[0]++; };
    int CNotes(void)
    { return (int)(valI[0]);};

    void SetIOI_Rest(TFrac val)
    { valI[1] = val.toDouble();};
    void SetCDeltaIOIs( double val )
    { valI[2] = val;};
    void SetCIOIs(double val)
    { valI[3] = val;};
    void SetSSQ(double val)
    { valI[4] = val;};
    void SetAverage( double val)
    { valI[5] = val;};
    void SetdMinMaxIOI(double val)
    { valI[6] = val;};
    void Write( FILE *out );
};

/// create a feature vector for [from...to)
TFeatures *GetFeatures( TNOTE *from,
                        TNOTE *to);

/// tool struct for knn-compare function
class TK_Array
{
public:
	TK_Array( void );
	int CNotes;
	TAbsTime OnsetSum;
	double AvOnset;
	double SSQ;
	TAbsTime IOTSum;
	double AvIOT;
	double IOTSSQ;
};

/// debug
void Printf( TK_Array K_Array, FILE *out );

#endif
