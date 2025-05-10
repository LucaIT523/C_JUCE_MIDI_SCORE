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


#include "meta_key.h"
#include <string>

TMusicalObject *TMetaKey::GetNext( int voice )
{
	TMusicalObject *temp;
	temp = TMusicalObject::GetNext(voice);
	while(temp &&
       !dynamic_cast<TMetaKey *>(temp) )
//       strcmp(temp->ClassName(), KEY_CLASS) )
	{
		temp = temp->TMusicalObject::GetNext(voice);
	}
	return dynamic_cast<TMetaKey *>(temp);
}


//----------------------------------------------------------------------
const char *keyScale[15]={"c&","g&","d&","a&","e&","b&","f","c","g","d","a","e","b","f#","c#"};
const char *keyScaleMajor[15]={"C&","G&","D&","A&","E&","B&","F","C","G","D","A","E","B","F#","C#"};

TMetaKey * MetaKey( TMusicalObject *s )
{
	return dynamic_cast<TMetaKey *>(s);
}



TFrac TMetaKey::Convert(ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TFrac  defFrac,		// denominator of pre-note
			  TTRACK *Track0 ) // control track (tempo, key, meter, ...)

{

	defFrac = TQuantizedObject::Convert(gmnOut,
									  preEndtime,
									  defFrac,
									  Track0); // should bew NULL

	// GUIDO: \key<+x> \key<-x> \key<"C">
	if(!minorMajorI) // undefined type
	{
		gmnOut << "\\key<"<< nAccidentalsI << "> ";
	}
	else // defined type
	{

		const char *key;
		if(minorMajorI == 1)
			key = keyScaleMajor[(nAccidentalsI%12)+7];
		else
			key = keyScale[(nAccidentalsI%12)+7];
		gmnOut << "\\key<\""<< key << "\"> ";

	}
	return defFrac;
}


int TMetaKey::minorMajor()
{
	return minorMajorI;
}
