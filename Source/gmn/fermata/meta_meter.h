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

#if !defined ( __meta_meter_h__ )
#define __meta_meter_h__

/*
	Meta TMetaMeter
	and Tool functions 
	Author: Juergen Kilian
*/


#include "quantised_obj.h"
#include "timesig.h"


/// class for meter-tag  based on TQuantizedObject, numerator and denominator MUST NOT be normalised!!
class TMetaMeter : public TQuantizedObject
{
public:
    int numerator,
        denominator;
	void setMeter( int num, int denom );

	virtual void Debug( FILE *out )
	{
		TQuantizedObject::Debug(out);
		fprintf(out, "Meter\n");
	}
	TMetaMeter(TAbsTime absTime,
				  int num,
				  int denom)
	: //TMusicalObject(absTime),
     TQuantizedObject(absTime)
        {
                numerator = num;
                denominator = denom;
        };
	virtual ~TMetaMeter(void){};

	virtual TAbsTime Convert( /// gmn output file
						      ostream &gmnOut,
						      /// enpoint of pre-note
		TAbsTime preEndtime,
		/// denominator of pre-note	
		TAbsTime  prev,	
		/// control track (tempo, key, meter, ...)	
		TTRACK *Track0  
                );

        virtual TMusicalObject *GetNext(int voice);
	TTimeSignature Meter( void )
	{
            TTimeSignature temp;
            temp.initSig(numerator, denominator, 0);
            return temp;
    };
};



TMetaMeter * MetaMeter( TMusicalObject *s );

#endif
