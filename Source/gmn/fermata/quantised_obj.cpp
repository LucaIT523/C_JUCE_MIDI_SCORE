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

/*------------------------------------------------------------------
|	filename : quantised_obj.CPP
|	author     : Juergen Kilian
|	date	  : 2003, 2011
|	implementation of TQuantisedObject
------------------------------------------------------------------*/
#include "quantised_obj.h"
#include "funcs.h"
#include <string>
//----------------------------------------------------------------------


TQuantizedObject * QuantizedObject( TMusicalObject *s )
{
	return dynamic_cast<TQuantizedObject *>(s);
}

TAbsTime TQuantizedObject::SetQData( TAbsTime /*endPrevious*/ ,	// end point of pre-note
									TQuantizedObject * /*ptr*/ ,	// previous note
									TFracBinList *)
{
// ToDo needs to be debugged
	AbsTime = AbsTime + BestDiffT;
	BestDiffT = 0L;
	SecDiffT = 0L;
	return AbsTime;
}

char TQuantizedObject::TOK( void )
{
	if( BestDiffT == SecDiffT )
		return 1;
	else
		return 0;
} // TOK


#ifdef kasshkjhdf
void TQuantizedObject::QuantizeToN( long ppn )
/*
	Quantize attackpoint to n*ppn
	can be used to quantize tempoTracks to 1/4
*/
{
	long 	temp;
	long 	mal;

	temp = AbsTime.toLong();
	mal = ( temp + ( ppn/2 ) ) / ppn ;

	// quantize to n*ppn
	BestDiffT = AbsTime.toLong() - (ppn * mal);
	SecDiffT  = BestDiffT;
} // QuantizeToN
#endif


TFrac TQuantizedObject::Convert( ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TFrac  defFrac,		// denominator of pre-note
			  TTRACK * /*Track0*/ ) // control track (tempo, key, meter, ...)

{
	TAbsTime attackpoint = qAttackpoint();
	if(attackpoint < preEndtime )
	{
		Printf("TQuantizedObject QError: ");
		Printf(attackpoint);
		Printf(" < ");
		Printf(preEndtime);
		Printf("\n");

		gmnOut << "(*qerror*)";
	}
	else if(attackpoint > preEndtime )
	{
		gmnOut << "_";	// write rest
		defFrac = WriteDuration(gmnOut,
							attackpoint - preEndtime,
							defFrac);
	}

/*
	TAbsTime attackpoint;  // attackpoint

	attackpoint = qAttackpoint();
*/
	return defFrac;
}
//----------------------------------------------------------------------


void TQuantizedObject::ticksToNoteface( int ppq )
/*
	Convert all tick-timing information into noteface information
*/
{
	TMusicalObject::ticksToNoteface(ppq);

	TFrac resolution = TFrac( 1, ppq*4);
/*
	ppq == 192
	192/1 -> 1/4
	96/1  -> 1/8
	n/1   -> n/(ppq*4)
*/
	BestDiffT *= resolution;
	SecDiffT *= resolution;
}



TMusicalObject *TQuantizedObject::GetNext( int voice )
{
	TMusicalObject *Temp = TMusicalObject::GetNext(voice);
	while(Temp &&
       !dynamic_cast<TQuantizedObject *>(Temp) )
//       !strcmp(Temp->ClassName(), MUSICAL_CLASS) )
	{
		Temp = Temp->TMusicalObject::GetNext(voice);
	}
	return dynamic_cast<TQuantizedObject *>(Temp);
}

//DEL TQuantizedObject::TQuantizedObject(const TQuantizedObject &ptr) : TMusicalObject( ptr )
//DEL {
//DEL 	atSelection = ptr.atSelection;
//DEL 	diffTimes = ptr.diffTimes;
//DEL 	diffTimeSize = ptr.diffTimeSize;
//DEL }
