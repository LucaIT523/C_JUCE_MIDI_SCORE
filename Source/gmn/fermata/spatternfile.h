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
|	filename : PATTERN.H
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	    : 17.10.1996-98-03
-----------------------------------------------------------*/
#ifndef __S_PATTERNFILE_H__
#define __S_PATTERNFILE_H__
   
//---------------------------------------------------------------


#include "../leanguido/lgsegment.h"
#include "spattern.h"

/// PatternFILE
class TSPFILE : public lgSegment
{ 
protected :
    /// # of Pattern
	int      CPatternP;
    /// length of current pattern
	TAbsTime	 LengthI;
    /// # of different pattern lengths in list
	int cLengthI; 
protected:
public :
    /// filename
	string name;		
	TSPFILE( const  char *name );
	virtual ~TSPFILE( void );
	/// read .gmn
	char 	 Read( void );
    /// get # of pattern or # of pattern with length == pattern length
    int  	 CPattern( TFrac *length = NULL );		
	int cLength( void )
    {
        return cLengthI;
    };

	
	TSPATTERN *firstPattern( void )			
    {
        return dynamic_cast<TSPATTERN *>(firstSequence());
    };
	TSPATTERN *nextPattern( void )			
	{
        return dynamic_cast<TSPATTERN *>(nextSequence());
	};

	/// get pattern by id 0...n
	TSPATTERN *get( int id );

	TAbsTime	 Length( void )
			{ return LengthI; };
	/// delete a pattern
	int Del( int index );
	virtual void write(FILE *out = NULL,
					   lgVoice *v = NULL);

    /// lgSegment functions
    /// overwrite this for replacing lgSequence 
//DEL     virtual lgSequence *newSequence( long int posNum,
//DEL                                      long int posDenom)
//DEL 	{
//DEL 		return factory();
//DEL 	}
    /// overwrite this for replacing lgChord
//DEL     virtual lgChord *newChord(long int posNum,
//DEL                               long int posDenom)
//DEL 	{
//DEL 		/// Patternfile can't handle chords!
//DEL 		return NULL;
//DEL 	}
    /// overwrite this function for replacing lgNote
//DEL     virtual lgNote *newNote(int pc,
//DEL                             int oct,
//DEL                             int acc,
//DEL                             long int durN,
//DEL                             long int durD,
//DEL                             int dots,
//DEL                             long int durPosN,
//DEL                             long int durPosD )
//DEL 	{
//DEL 		return new TPNOTE( pc, oct, acc, durPosN, durPosD, dots,
//DEL 							durN, durD);
//DEL 	}
    /// overwrite this function for replacing lgEmpty
//DEL     virtual lgEvent *newEmpty(long int durN,
//DEL                               long int durD,
//DEL                               int dots,
//DEL                               long int durPosN,
//DEL                               long int durPosD )
//DEL 	{
//DEL 		/// pattern can not handle empty notes
//DEL 		return NULL;
//DEL 	}
    /// overwrite this function for replacing lgRest
    /*
    virtual lgRest *newRest(long int durN,
                            long int durD,
                            int dots,
                            long int durPosN,
                            long int durPosD )
	{
		return new TPNOTE(0,0,0,durPosN, durPosD, dots,							
							durN, durD,
							0 /// rest typeID+
							);
	}
    virtual lgTag *newTag( long int no,
                           char *name );
                           */

}; //TSPFILE
//-----------------------------------------------------------------
#endif
