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
|	filename : q_NOTE.H
|	Author     : Juergen Kilian
|	Date	    : 17.10.1996-2003-2011
|   declaration of TQNOTE
------------------------------------------------------------------*/
#ifndef __q_note_h__
#define __q_note_h__
//-----------------------------------------------------------------
#include <iostream>
#include <sstream>
using namespace std;
#include <stdlib.h>
#include "note.h"
#include "portable.h"
#include "note_info.h"
#include <stdio.h>
//------------- Bitpositionen f¸r das Flag Lock ---------------------------------------
#define	ABSTIME  1
#define  DURATION 2
//-----------------------------------------------------------------
#include "fragm.h"
#include "funcs.h"

#define TFrac TFrac


#define ORNAMENT_GRACE "grace"
#define ORNAMENT_GLISSANDO "gliss"
#define ORNAMENT_MORDENT "mord"
#define ORNAMENT_TRILL "trill"
#define ORNAMENT_TURN "turn"
#define ORNAMENT_UNDEF "undef"
#define CHORD "chord"


class TTRACK;
class TQChord;

/// macro
#define SecDiffT (diffTimes[1].DiffTime)
/// macro
#define BestDiffT (diffTimes[0].DiffTime)

/// class for quantised note == TNOTE + quantisation data
class TQNOTE : public TNOTE //, public TQuantizedObject
 {
protected:
     /// distance to best seen attackpoint
     TAbsTime BestDiffL;
     /// distance to second best attackpoint
     TAbsTime SecDiffL;	


//     TQNOTE *Next;
public :
	  /// colour id
     int color;
	 TQNOTE( lgNote *note );
	 TQNOTE( const TQNOTE &ptr );
	 // latest possible start time
	 TFrac latestStart( void );
     /// is set to 1 if played duration >= IOI
     char durationIsIOI( void );
#ifdef PENALTY_CHECK
     /// todo ? toolfunctions for penalty checker
     virtual double pPitch( void );
     virtual double pChord( void );
#endif
    /// function will be overwriten by TQChord
    virtual int closestPitch( int pitch2);
    /// function will be overwriten by TQChord
    virtual double avMIDIPitch( void );

    /// for voice separation
    /// function will be overwriten by TQChord
    virtual double overlapDistance( TAbsTime attack2,
									TAbsTime duration2,
									TMIDIFILE *theMidifile);
    /// return delta pitchclass between pitch2 and MIDIpitch
    virtual double deltaPC(int pitch2, int lookBack = 0);

    


    void removeLegato(int sVoice=-1);
	/// convert MIDI timing into score timing
    virtual void ticksToNoteface( int ppq );
     virtual void Debug( ostream &gmnOut );
    

    TQNOTE( 	TAbsTime abstime,
             TAbsTime duration,
             unsigned char pitch,
             double intens );
    virtual ~TQNOTE( void ) {};

    /// copy BestDiff, SecDiff values into abstime and duration
    virtual TAbsTime SetQData( TQNOTE *prev,  // previous note
							   TFracBinList *bestDurations);
    virtual TAbsTime SetAbsTime( TAbsTime abstime )
    {
        TNOTE::SetAbsTime( abstime );
        // reset alternative values
        BestDiffT = 0L;
        SecDiffT  = 0L;
        return AbsTime;
    }; // if

	/// write to .gmn file
    virtual TFrac Convert( ostream &gmnOut,
                           TAbsTime preAbstime,
                           TFrac   prev,
                           TTRACK *Track0);

    virtual void SetAll( TAbsTime abstime,
                         TAbsTime duration,
                         unsigned char note,
                         double intens,
						 int id)
    {
		TNOTE::SetAll( abstime,
                 duration,
                 note,
                 intens,
				 id);
        BestDiffL = 0L;
        SecDiffL  = 0L;
        BestDiffT = 0L;
        SecDiffT  = 0L;
    };

    /// todo ?
    void CheckTempo( void );
    /// check if both durtion alternatives are equal
    char LOK( void );

	// set [Best|Second]DiffT
    TAbsTime QuantizeAbsTime( TAbsTime newtime );
	// set [Best|Second]DiffL
    TAbsTime QuantizeDuration( TAbsTime duration );

    /// increment alternatives frü duration
    virtual void incQDuration( TAbsTime f, TAbsTime s)
    {
        BestDiffL += f;
        SecDiffL  += s;
		if( qDuration() <= 0L )
		{
			std::cout << "WARNING: Expand duration <= 0 at " << qAttackpoint().toDouble() << endl;
		}
    };
    void expandTo( TAbsTime f,
					char finalise )
    {
		if( !finalise )
		{
			BestDiffL = f-mDuration;
			SecDiffL  = BestDiffL;
		}
		else
		{
			SetDuration(f);
			BestDiffL = 0L;
			SecDiffL = 0L;
		}
        if( qDuration() <= 0L )
        {
            std::cout << "WARNING: expandTo duration <= 0 at " << qAttackpoint().toDouble() << endl;
        }
    };
    
    /// get alternative durations
    virtual TAbsTime qDuration( TAbsTime *secduration = NULL )            
    {					
		TQNOTE *temp = dynamic_cast<TQNOTE *>(GetNext(GetVoice()));
		
		if( durationIsIOI() && 
			temp &&
			temp->atSelection != undefSel )
		{
			TFrac IOI = temp->qAttackpoint() - qAttackpoint();
			if( IOI > 0 )
				return IOI;			
		}
		
		if( LOK() ||
			dSelection != secondSel )
		{
			if( secduration )
			    *secduration = GetDuration() + SecDiffL;
			return GetDuration() + BestDiffL;
		}
		else 
		{
			if( secduration )
			    *secduration = GetDuration() + BestDiffL;
			return GetDuration() + SecDiffL;
		}
    };
    TAbsTime GetBestDiffAttack( void )
    {	return BestDiffT; };	
        
    TAbsTime  GetBestDiffDuration( void )
    {	return BestDiffL; };	

    TAbsTime  GetSecDiffDuration( void )		
    {	return SecDiffL; };	

    void ResetDiffs( void )
    {
        BestDiffL = 0L;
        SecDiffL  = 0L;
        BestDiffT = 0L;
        SecDiffT  = 0L;
    };

    /// return 11 if overlap with next note
    int Legato( void );
    /// todo check 
    int CutOrMove( TAbsTime duration );
    virtual TAbsTime SetDuration( TAbsTime duration )
    {
        TNOTE::SetDuration( duration );
        BestDiffL = 0L;
        SecDiffL  = 0L;
        return GetDuration();
    }; // SetDuration
	 /// selected duration possibility
	 char dSelection;
 private:
	 void init( void );
	 /// selected attackpoint possibility
 }; // TQNOTE
//-----------------------------------------------------------------

/// count all attackpoints in voice?
int countAttacks( TQNOTE *now,
                  TNOTE *to);

/*
class TNoteInfo;
/// todo check ?

void makeChords(TNoteInfo *noteList,
                int cAttacks,
                int CVoice,
                TTRACK *track);
*/
TQNOTE *  QNOTE(TMusicalObject *s);

const char *getColorString( int color );
#endif
