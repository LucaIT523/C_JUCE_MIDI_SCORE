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

#if !defined ( __meta_text_h__ )
#define __meta_text_h__

/*!
	Meta_text
	and Tool functions 

	Author: Juergen Kilian
*/


#include "note.h"
#include "quantised_obj.h"

/// MIDI meta text class  based on TQuantizedObject
class TMetaText : public TQuantizedObject
{
	///  textstring
	char *textI;	
	/// lyrics, text, marker == TEXT_!, MARKER, LYRICS
	int typeI;		
	void init( const char *str,
				int type );

public:
	virtual TAbsTime Convert( /// gmn output file 
						ostream &gmnOut	,
				/// enpoint of pre-note
			  	TAbsTime preEndtime,
			  	/// denominator of pre-note	
			  TAbsTime  prev,	
			  /// control track (tempo, key, meter, ...)	
			  TTRACK *Track0 ); 
	virtual void Debug( FILE *out )
	{
		TQuantizedObject::Debug(out);
		fprintf(out, "Text\n");
	}
	/// create a copy
	TMetaText( TMetaText *ptr ); 
	TMetaText( TAbsTime absTime,
			  const char *str,
			  int type );
	virtual ~TMetaText(void)
	{ delete [] textI; };
/*
	virtual char *ClassName(void)
		{ return TEXT_CLASS; };
*/
	/// get next TMetaText, skip others
	virtual TMusicalObject *GetNext(int voice);
/*	virtual TMetaText *ToMetaText( void )
			{return this;};*/
}; // TMetaText




#endif
