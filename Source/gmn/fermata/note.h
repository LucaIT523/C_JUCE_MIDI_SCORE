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
|	filename : NOTES.H
|	Autor     : Juergen Kilian
|	Datum	    : 17.10.1996-2003
|	classdeclaration for TNOTE
------------------------------------------------------------------*/
#ifndef __note_h__
#define __note_h__

#include "debug.h"
#include <iostream>
using namespace std;

#include "quantised_obj.h"
/*
class TMetaTempo;
class TMetaMeter;
class TMetaKey;
class TMetaText;
class TCLICKNOTE;
class TTRACK;
*/

 class TMIDIFILE;
#include "../leanguido/lgnote.h"
#include "../leanguido/lgtag.h"

//#include "note.h"


typedef char keyscale[12][2];
typedef keyscale * pkeyscale;


/// a basic note == a GUIDO event, based on TQuantizedObject
class TNOTE  : public TQuantizedObject
{

private:

protected:
    TAbsTime mDuration;		// original duration

    /// picthclass 0...7
    char mPitch;
    /// # of accidentals -oo..0..+oo
    char mAccidentals;
    // acotvae info -oo..0..+oo
    char mOctave;
    /// MIDI velocity 0...127
    double mIntens;

public :
	lgTag *createTag;


	/// score pos manually given from input data (Cemgil testfiles)
	TFrac testScorePos(void)
	{
		
		if( createTag )
		{
			double st = 0;
			string unit;
			if( createTag->getParamFloat("st",-1,st, unit  ) )
			{
				TFrac scPos( (long)(st * 1000 + 0.5), 96*1000L );
				return scPos;
			}
		}
		return TFrac(-1,1);
	}
	/// time position of offset point by sustian pedal [midiTicks]
	long sustainOffset;
	TAbsTime getOffset( void );
	TNOTE( const TNOTE &ptr );

	/// fill with note data
	 void fill( lgNote *note );

        /* use ioi
        /// distance between prev->offset  and this-<abstime
    TAbsTime Dist2Prev( void );
    /// next->absTime - this->offset
    TAbsTime Dist2Next( void );
    /// abstime + duration
    */
    virtual TFrac offset( void );
    int accidentals( void ){return mAccidentals; };
    /// respell pitch according to key-info of midifile
    virtual void pitchSpelling( TMIDIFILE *midifile);

     virtual void Debug( ostream &out );

    /// write pitch in GUIDO syntax
    void WritePitch( ostream &gmnOut );

    TNOTE( TAbsTime abstime,
            TAbsTime duration,
            unsigned char pitch,	/// MIDI picth	
            double intens );	
    virtual ~TNOTE( void ) {};
	
    virtual TMusicalObject *GetNext(int voice);
    virtual TMusicalObject *GetPrev(int voice);
	
    /// change note data
    virtual void SetAll( TAbsTime abstime,
                         TAbsTime duration,
                         unsigned char note,
                         double intens,
						 int id);


    TAbsTime GetDuration( void )
    {
        return mDuration;
    };
    // !! a call to SetDuration will reset Best|SecDiff's !!
    virtual TAbsTime SetDuration( TAbsTime duration );

	double durationMS( TMIDIFILE *theMidifile );

    /// semitone picth 0...127
    unsigned char GetMIDIPitch( void ); 
    /// pitchclass 0...11
    char ChromaticPitch( void );
    /// diatonic pitch 0...7
    char DiatonicPitch( void )
    { return mPitch; };
		
    char Octave( void );
    void SetPitch( unsigned char midipitch );
    void SetPitch( int pitch, int accidental, int octave );

    double getIntens( void )
    { return mIntens; };
    virtual void setIntens(double vel);
    /// return min interval in MIDI Pitch distance, TQChord has own implementation
    virtual int minInterval(TNOTE *ptr);
}; // TNOTE

/// sort: 1. abstiome 2. pitch
int operator > ( TNOTE &n1,  TNOTE &n2 );
/// sort: 1. abstiome 2. pitch
int operator >= ( TNOTE &n1,  TNOTE &n2 );

TNOTE *  NOTE(TMusicalObject *s);

typedef TNOTE *PTNOTE;

//-----------------------------------------------------------------
#endif
