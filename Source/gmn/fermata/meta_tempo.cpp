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

#include "meta_tempo.h"
#include <string>

TMetaTempo * MetaTempo( TMusicalObject *s )
{
	return dynamic_cast<TMetaTempo *>(s);
}



TMusicalObject *TMetaTempo::GetNext( int voice )
{
	TMusicalObject *temp = TMusicalObject::GetNext(voice);
	// skip all non TMetaTempo entries
	while(temp &&
          !dynamic_cast<TMetaTempo *>(temp) )
	{
		temp = temp->TMusicalObject::GetNext(voice);
	}
	return dynamic_cast<TMetaTempo *>(temp);
}



TFrac TMetaTempo::Convert(ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TFrac  defFrac,		// denominator of pre-note
			  TTRACK *Track0 ) // control track (tempo, key, meter, ...)

{
	defFrac = TQuantizedObject::Convert(gmnOut,
									  preEndtime,
									  defFrac,
									  Track0); // should bew NULL

	// GUIDO: \tempo<"Moderato","1/4=60">
	if( hidden )
	{
		gmnOut << "\n\\tempo<\"\",\"" << numerator <<"/" << denominator << "=" << tempoI << "\"> ";	
	}
	else
	{
		gmnOut << "\n\\tempo<\""<< numerator <<"/"<<denominator <<"="<<tempoI <<"\",\""<< numerator<<"/"<<denominator <<"="<<tempoI <<"\"> ";
	}
	return defFrac;
}

double TMetaTempo::getAbsTimeMs()
{
	return absTimeMs;

}

void TMetaTempo::setAbsTimeMs(double ms)
{
	absTimeMs = ms;
}
