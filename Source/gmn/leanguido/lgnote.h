/*
	leanGUIDO class library
	Copyright (C) 2003  Juergen Kilian, SALIERI Project

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 *  lgnote.h
 *
 */



#ifndef __lgnote_h__
#define __lgnote_h__



#include "lgrest.h"

//! A lgEvent with additional  pitch info
class lgNote : public lgRest
{
private: 

	//! 0..11 = c,cis,...,b
    int pitchClassI;
    //! -oo .. +oo, 0 = c' 
    int octaveI;	
    //! -oo .. +oo
    int accidentalsI;	

protected:
//    virtual void writeV( FILE *out ); //! name+acc+octave

public :
	/// pitch accidentals octave * duration
	virtual string toString( lgVoice *callingSeq = NULL );
	/// -oo...+oo, 0 == c'
	int octave( void );
	/// -oo...+oo, 0 == c'
    void setOctave( int i )
    { octaveI = i; };
    
    /// 0...11 = c,cis, ... b
    int pitchClass( void );
    /// 0...11 = c,cis, ... b
    void setPitchClass( int i )
    { pitchClassI = i; };

	/// -oo..+oo
    int accidentals( void )
    { return accidentalsI; };
	/// -oo..+oo
    void setAccidentals( int i )
    { accidentalsI = i; };
    
    lgNote( //! pitchclass 0..11
    		int pc,	
    		//!octave	-oo..+oo, 0 = c'
                int oct, 
                //! # of accidentals -oo .. +oo
                int acc, 
                /// duration numerator
                long int durNum,
                /// duration denominator
                long int durDenom,
                /// # of dots
                int cdots,
                /// position numerator
                long int posNum,
                /// position denominator
                long int posDenom);
    lgNote(int pc);
    /// pitch as string "c", "c#", "gis", "g#", ...
    string pitch( void );
};



#endif
