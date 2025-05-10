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

#if !defined( __C_TRACK_H__ )
#define __C_TRACK_H__

#include<string>
using namespace std;
#include "event.h"
#include "portable.h"
#include "defs.h"
#include "c_note.h"
#include "q_track.h"

#define ONLY_TEMPO 1
#define NO_TEMPO   2

//-----------------------------------------------------------------
/*! clicktrack, includes a list of TCLICKNOTE and is used for
creation of tempo profile can be derived from a TTRACK or by analysis of played events
or by direct read of a midifile
*/
/// metronome track including a list of TCLICKNOTE 
class TCLICKTRACK {
private:
    /// init score values of all click notes
    void init( void );
    /// error flag will be set to 0 if an error during reading occurs
    char   OK;
    // byte offset in midifile
    long   Offset;
    TCLICKNOTE  *CurrentNote;
    /// ptr to head of ClickNote list
    TCLICKNOTE  *Notes;	      
    TCLICKNOTE  *Tail;
    /// score duration of a clicknoote
    TFrac ClickDuration;

    /// recording tempo of midifile
    int    RecTempoI;
    /// ppq during recording of midifile
    int    RecPPQI;	

	
    /// test function if Event is a click event
    char clickEvent( TEVENT *Event,
                     int typeFilter); // t -1 == off; 0..127 = pitch;128..255=controller


    /// Todo:  only for debug and test, should be removed in final releas
    long   FirstClickPlay;	// Zeitpunkt des ersten Click nach dem Einz„hlen
    long   LastClickPlay;	// Zeitpunkt des letzten gelesenen Click
    long   FirstDiffTick;	//	Anzahl Ticks zwischen den beiden ersten Schlägen
    long   LastDiffTick;	// Anzahl Ticks zwischen den beiden letzten Schlägen

    TMIDIFILE *mParent;
public:
    TMIDIFILE *getParent(){
    	return mParent;
    }
	double analyseQuality(const  string  &filename);
	double avTempo( TCLICKNOTE *from, TCLICKNOTE * to );
	// mark all clicknotes from start and stop List
	void markStartStop( TMusObjList *startList, TMusObjList *stopList);
	double maxCNotes;
	double minIntens;
	double maxIntens;

    /// get optimised offset -> optimse weight at siginificant click positions, works to right direction!
    TAbsTime optimumOffset( TCLICKNOTE *from,
                            TCLICKNOTE *to,
                            /// significance = n*bestPos
                            TAbsTime &bestPos,
                            /// size of testing grid
                            TAbsTime &gridResolution,
							/// current quality
							double &oldQuality,
							/// quality after shift
							double &newQuality);
    
    /// remove all insignificant click notes 
	void filterClicks(  /// phi value (normal_pdf)
						double alphaLimit );

	/* int accuracy(double &attack, double &duration); */
    /// remove ptr from list
    void detach( TCLICKNOTE *ptr);
    /// insert ptr into list
    void Insert( TCLICKNOTE *ptr );
    
    int cInsertNotes;

    /// return threshhold for weight so that only "percent"% of all beats 
	/// are above the returned threshold
    double getThreshold( TCLICKNOTE *from,
                         TCLICKNOTE *to,
                         double percent = 0.8 /*[0...1)*/ );
	/// set scoreTime(i) = scoreTime(i-1)+scoreDuration(i-1)
    void normScoreTimes( void );
	/// remove unused clicks (duration < 0)
    void reorganize( void );
    void deleteLastClick( void );
    double tempo( TCLICKNOTE *ptr );
    void mergeEqual(int recPPQ, int recTempo);
    TCLICKTRACK( int recPPQ,
                 int recTempo,
                 TQTRACK *src,
                 TMIDIFILE *parent);
    TCLICKTRACK( long offset, TMIDIFILE *parent );
    virtual ~TCLICKTRACK( void );
    /// read track header from Midifile, Offset must be set!
    long Read_Header( FILE *file );
    /// read track info from midifile, readHeader must be called
    /// and Offset set before
    char Read( FILE *file,
               int  recPpq,
               TFrac  partsPerClick,
               int  channel,	/// filter: 0 == all, 1,..,16
               int  type,	/// typeFilter: -1 == off; 0..127 = pitch;128..255=controller
               FILE *log );
    
    void Reset( void );

	/// returns byte offset in midi file
    long GetOffset( void )		
    { return Offset; };
    /// set byte offset position
    void SetOffset( long offset )
    { Offset = offset; };
    /// returns first note and sets CurrentNote
    TCLICKNOTE *FirstNote( void )
    {
        CurrentNote = Notes;
        return CurrentNote;
    };
    void SetFirstNote( TCLICKNOTE *temp )
    {
        Notes = temp;
        CurrentNote = Notes;
    }
    /// return next clicknote, update current note
    TCLICKNOTE *Next( void )
    {
        CurrentNote = CLICKNOTE(CurrentNote->GetNext(-1));
        return CurrentNote;
    };
    /// re-calculate all timing info of Track according to score-times of clicknotes
	/*!
		re-calculate all timing info of Track according to score-times of clicknotes
		The score-times will be still in click timing(!) otherwise we run
		into troubles with large denominators during quantisation!
	*/
    char FitToClick( TQTRACK *Track );
	
	/// pickup, upbeat detection. a) if notes played berofre first click, b) optimise strong positions to barlength
    char CheckForUpBeat(  TQTRACK *track /*,
                          TAbsTime    barlength */ );
    /// create  \tempo tags in controlTrack from events in this 
    void CreateTempoTags( TQTRACK *controlTrack );

    void shiftScoretimes( TCLICKNOTE *startptr,
                          TCLICKNOTE *endptr,
                          TAbsTime      offset );
    TAbsTime GetFirstClickPlay( void )
    { return FirstClickPlay; };
    TAbsTime GetLastClickPlay( void )
    { return LastClickPlay; };
    TAbsTime GetLastDiffTick( void )
    { return LastDiffTick; };
    TAbsTime GetFirstDiffTick( void )
    { return FirstDiffTick; };

    int GetBeatIntens( void );
    /// retrieve a probably accurate start note for  the tempo detection
    // todo:: take care of note weight
    TCLICKNOTE *startNote( TCLICKNOTE *from,
                            TCLICKNOTE *to,
                            int direction );
    void SetRecValues( int recTempo,
                       int recPPQ )
    {
        RecTempoI = recTempo;
        RecPPQI   = recPPQ;
    };
    void Debug(void);
    void Debug( ostream &out,
    			// if 1 force debug 
    			char force = 0);
    int fixPrimeDistanceErrors(TCLICKNOTE *from, TCLICKNOTE *to);
    void Debug(string fname);
}; // TCLICKTRACK




///todo use FilterSTressedIOI for Anchor detection?
/*
	keep stressed Attackpoints in attacklist,
	remove unstressed notes

	stressed :
		- IOI > IOI(prev)
		- Intens > STressIntens

	remarks:
		- CLickNotes at same attackpoint must be merged before!! -> IOI == 0
*/
TCLICKTRACK *FilterStressedIOI( TCLICKTRACK *AttackTrack );
/*!
	Calculate limit for recognition of intens-stressed notes
*/
double StressIntens( TCLICKTRACK *AttackTrack );
///todo use CreateAttackTrack for CreateClickTrack
/*!
		Copy all Attackpoints from file into a TCLICKTRACK
*/
TCLICKTRACK *CreateAttackTrack( THMIDIFILE *file);


#endif
