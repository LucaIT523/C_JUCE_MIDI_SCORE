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
|	Filename : q_chord.h
|	Author     : Juergen Kilian
|	Date	    : 2003
|	declaration of TQChord
------------------------------------------------------------------*/
#ifndef __q_chord_h__
#define __q_chord_h__
//-----------------------------------------------------------------
#include <sstream>
using namespace std;
#include <stdlib.h>
#include "ornament.h"
#include "note.h"

#include <stdio.h>

/// Quantised Chord
class TQChord : public TOrnament  
{
public:

#ifdef PENALTY_CHECK
	/// toolfunctions for penalty checker
	virtual double _pPitch( void );
	/// toolfunctions for penalty checker
	virtual double _pChord( void );
#endif

	/// return pitch of chord which is closest to pitch2
	virtual int closestPitch( int pitch2 );
    /// return average MIDI pitch of all chord notes
    /// used by TQNOTE::deltyPC if lookbakc > 0
    virtual double avMIDIPitch( void );
    /// calculate overlap penalty?
    virtual double overlapDistance( TAbsTime attack2,
									TAbsTime duration2,
									TMIDIFILE *theMidifile);
    /// re-write accidentals depending on current key-sig
    virtual void pitchSpelling( TMIDIFILE *midifile);


//	int color; // 0=none, 1 = red
    /// return delta pitch class between pitch2 and closest pitch in chord
    virtual double deltaPC( int pitch2, int lookback = 0);
    /// add ptr to chord and remove ptr from track note list
    void Add(TQNOTE *ptr, 
			 TTRACK *track, 
			 char avDuration);
	TQChord( TQNOTE *rootNote /*! root note, will be replaced by TQChord!!*/,
			  TQNOTE *asPlayedFrom 	/*! might be NULL if notes get added manually */,
			  TMusicalObject *asPlayedTo /*! might be NULL if notes get added manually */,
			  TTRACK *track /*!  ptr to parent track */ );
	virtual ~TQChord( void );
    /// write to .gmn file, return duration of chord
    virtual TFrac Convert( ostream &gmnOut /*! .gmn file*/, 
				TAbsTime preAbstime /*! attacktime of previous note */,
				TFrac   prev /*! duration of prev note */,
				TTRACK *Track0 /*! control track */ );
    virtual TAbsTime SetQData( TQNOTE *prev,	// previous note
							   TFracBinList *bestDurations );
	/// return # of notes inside chord
	int cNotes( void );
    virtual void incQDuration( TAbsTime f, TAbsTime s)
    {
		TQNOTE::incQDuration(f,s);
	}
    virtual int minInterval(TNOTE *ptr);
	

};

//-----------------------------------------------------------------

#endif
