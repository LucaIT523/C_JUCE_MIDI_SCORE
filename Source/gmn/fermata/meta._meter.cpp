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


#include "meta_meter.h"
#include <string>


TMetaMeter * MetaMeter( TMusicalObject *s )
{
	return dynamic_cast<TMetaMeter *>(s);
}

TMusicalObject *TMetaMeter::GetNext( int voice )
{
	TMusicalObject *temp;
	temp = TMusicalObject::GetNext(voice);
	while(temp &&
       !dynamic_cast<TMetaMeter *>(temp) )
//       strcmp(temp->ClassName(), METER_CLASS) )
	{
		temp = temp->TMusicalObject::GetNext(voice);
	}
	return dynamic_cast<TMetaMeter *>(temp);
}



TFrac TMetaMeter::Convert( ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TFrac  defFrac,		// denominator of pre-note
			  TTRACK *Track0 ) // control track (tempo, key, meter, ...)

{
	defFrac = TQuantizedObject::Convert(gmnOut,
									  preEndtime,
									  defFrac,
									  Track0); // should bew NULL
	// GUIDO: \meter<"4/4">
	gmnOut << "\\meter<\""<< numerator<< "/"<< denominator<<"\"> \n";


	return defFrac;
}
//----------------------------------------------------------------------


void TMetaMeter::setMeter(int num, int denom)
{
	numerator = num;
	denominator = denom;
}	



