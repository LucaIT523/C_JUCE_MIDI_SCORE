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

#if !defined ( __meta_key_h__ )
#define __meta_key_h__

/*
		TMetaKey
	and Tool functions 

	Author: Juergen Kilian
*/

#include <sstream>
using namespace std;
#include "note.h"
#include "quantised_obj.h"

/// class for key-tag based on TQuantizedObject
class TMetaKey : public TQuantizedObject
{
    /// & =< 0, # => 0
    int nAccidentalsI;
    /// -1 == minor, 1 == major, 0 == undef(MIDIFILE)
    int minorMajorI; 
public:
	int minorMajor( void );
    /// write to .gmn
    virtual TAbsTime Convert( /// gmn output file
    						ostream &gmnOut,	
    							/// enpoint of pre-note
			  TAbsTime preEndtime,
			  /// denominator of pre-note
			  TAbsTime  prev,
			  /// control track (tempo, key, meter, ...)	
			  TTRACK *Track0  
                           );
        

    virtual void Debug( FILE *out )
	{
		TQuantizedObject::Debug(out);
		fprintf(out, "Key\n");
	}
	TMetaKey( TAbsTime absTime,
			int nAcci,
			 /// -1 == minor, 1 == major, 0 == undef
			int type 
           )
           : TQuantizedObject(absTime)
        {
                nAccidentalsI = nAcci;
                minorMajorI = type;
        };
	virtual ~TMetaKey(void){};
	virtual TMusicalObject *GetNext(int voice);
	int nAccidentals(void)
	{ return  nAccidentalsI; };
};
TMetaKey * MetaKey( TMusicalObject *s );

#endif
