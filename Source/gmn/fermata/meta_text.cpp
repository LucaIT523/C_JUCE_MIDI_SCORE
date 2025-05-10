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

#include "meta_text.h"

#include <string>
using namespace std;
#include <string.h>

#include "event.h"


TAbsTime TMetaText::Convert( ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TAbsTime  defFrac,		// denominator of pre-note
			  TTRACK *Track0 ) // control track (tempo, key, meter, ...)
{

	// write rest duration
	/*
	defFrac = TQuantizedObject::Convert(file,
									  preEndtime,
									  defFrac,
									  Track0); // should bew NULL
	*/
	// parse textI GUIDOTag
	if( strstr(textI, "GUIDOTAG:") )
	{
		string tmp = string(textI+strlen("GUIDOTAG:"));
		gmnOut << tmp;
		
	}
	else 
	{
		
		
		switch( typeI )
		{
		case TEXT_1 :
		case LYRICS :
		case MARKER :
			gmnOut << "\n\\text<\""<< textI <<" \">";
			break;
			
		}
	}
	return defFrac;
}


void TMetaText::init( const char *str,
				int type )
{

	if( str )
	{
		textI = new char [strlen(str)+1];
		strcpy(textI, str);
	}
	else // error
	{
		textI = new char[3];
		strcpy(textI, " ");
	}
	typeI = type;
}

TMetaText::TMetaText( TMetaText *ptr ) : // copy
			TQuantizedObject( ptr->GetAbsTime() ) /*,
				TMusicalObject( ptr->GetAbsTime() )*/
{
	init( ptr->textI, ptr->typeI );
	SetVoice( ptr->GetVoice() );
}

TMetaText::TMetaText( TAbsTime absTime,
			  const char *str,
			  int type ) : TQuantizedObject(absTime)/*,
							 TMusicalObject(absTime)*/
{
	init(str, type);
} // TMetaText


TMusicalObject *TMetaText::GetNext( int voice )
{
	TMusicalObject *temp;
	temp = TMusicalObject::GetNext(voice);
	while(temp &&
       !dynamic_cast<TMetaText *>(temp) )
//       strcmp(temp->ClassName(), TEXT_CLASS) )
	{
		temp = temp->TMusicalObject::GetNext(voice);
	}
	if(temp)
   	return temp;
	return NULL;
}
