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

#if !defined( __timesig_h__ )
#define __timesig_h__
#include <stdlib.h>
#include <stdio.h>
#include "fragm.h"

/// class for time-signatures, = TFrac without normalisation
class TTimeSignature{
protected:

public:
	/// generic weight/bias
	double weightI;

	/// a generic probability, used for autocorrelation;
	double prob;
	void normalize( void );
	
	
	int numeratorI;
	int denominatorI;
	
	char operator == (const TTimeSignature &f2)
	{
		return numeratorI == f2.numeratorI &&
				denominatorI == f2.denominatorI;
	};

	TTimeSignature( int numerator,
					int denominator,
					double weight);

    TTimeSignature(void);
	void initSig(  int numerator,
				int denominator,
				double weight)
	{
		numeratorI = numerator;
		denominatorI = denominator;
		weightI = weight;
	};
	TFrac toFrac( void )
	{
		return TFrac(numeratorI, denominatorI);
	}

};
/// compare function for qsort call 
 int compareTimeSig( const void *p1,
                       const void *p2 );

#endif
