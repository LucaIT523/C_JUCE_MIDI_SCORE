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

/*--------------------------------------------------------------
	Implementation of TTimeSignature
	Author: Juergen Kilian
	Date:   1998-2002
----------------------------------------------------------------*/
#include "timesig.h"
#include <stdlib.h>
#include <string>


 int compareTimeSig( const void *p1,
                       const void *p2 )
 {
	 
	double val1 = ((TTimeSignature *)p1)->prob  *
				  ((TTimeSignature *)p1)->weightI;
	double val2 = ((TTimeSignature *)p2)->prob  *
				  ((TTimeSignature *)p2)->weightI;
	/*
	double val1 = ((TTimeSignature *)p1)->prob;
	double val2 = ((TTimeSignature *)p2)->prob;
	*/
     if( val1 > val2 )
         return 1;
     else if ( val1 < val2 )
         return -1;
     return 0;
 }

TTimeSignature::TTimeSignature( void ) 
{
    initSig(0,0,0);
};

TTimeSignature::TTimeSignature( int numerator,
					int denominator,
					double weight)
	{
		initSig( numerator, denominator, weight );
	};


void TTimeSignature::normalize()
/*
	try tu normalize signature to usefull fraction

*/
{

	// eliminate non ^2 signatures
	while( denominatorI > 4 &&
		   numeratorI > 3 &&
		   !(numeratorI % 3) &&
		   !(denominatorI % 3) )
	{
		denominatorI /= 3;
		numeratorI /= 3;
	}

	 // eliminate > 10, mod 2 denominators
	while( numeratorI > 10 &&
		   !(numeratorI % 2) )
	{
		numeratorI /= 2;
	}

	
	while( denominatorI > 8 &&
		   numeratorI > 2 &&
		   !(numeratorI % 2) &&
		   !(denominatorI % 2) )
	{
		denominatorI /= 2;
		numeratorI /= 2;
	}
	// don't use 2/x 
	if( numeratorI == 2 && 
		denominatorI > 2 )
		numeratorI = 4;
}



