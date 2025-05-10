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
|	Datum	    : 17.10.1996-03
|	Aufgabe   :  classdefinitios  TCLICKNOTE
------------------------------------------------------------------*/

#ifndef __c_note_h__
#define __c_note_h__

//-----------------------------------------------------------------
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "note.h"
#include "portable.h"
#include "patternfile.h"
#include "fragm.h"


using namespace std;


class TCLICKTRACK;
class TMIDIFILE;
//-----------------------------------------------------------------
/// click note class, used for click track based on TMusicalObject
class TCLICKNOTE : public TMusicalObject
{	
private :
    /// attacked time of midifile in ticks, == performance time
    long PlaytimeL( void )		
    {
    	return GetAbsTime().toLong(); 
    };
	
    /// attacktime in score, must be inferred
    TAbsTime scoretimeL;		
	
	/// weight depends on 
    // double weightI;
	
    /// longest duration starting at this attackpoint !!! in [ms]
	long durationI;

	
    /// max intensity played at this attackpoint
    double MaxIntensI;	
	
public :
	/// score pos manually given from input data (Cemgil testfiles)
	TFrac testScorePos( void )
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
	
	};
	lgTag *createTag;
	/// return accentuation in range between -1..0..1
	double durationalAccent( void );
	/// preliminary score IOIratio set by pattern matching
	double patScoreIOIratio;
	/// final pattern selected
	int patternFinalID;
	/// best matching pattern
	TPatternDistance *bestPatternList;
	/// dTempo/dTime 
	double dTempo_dTime( TFrac &newDur, 
						int direction,
						  TCLICKTRACK *ClickTrack,
						  TMIDIFILE *midifile );
						  
	double IOIratioError( void );
	/// performance duration
	long plDuration( void );
	/// performance offset position
	long plOffset( void );
	TFrac prelScDuration;
	/// retursn prevRIOI * perfIOI
	double plRIOI( int direction, double prevQRIOI );
	/// # notes merged to clicknote
	int cNotes;
	/// start id during tempo detection, > 0 == start, < 0 == stop
	int startId;
	TAbsTime scoreDuration( void );

    virtual void  SetNext( TCLICKNOTE * ptr, TCLICKTRACK *theClickTrack );
	double relScoreIOI(int prevPos, int nextPos);

	TAbsTime scoreIOI( int dir);
	TCLICKNOTE * getNext( int direction, int voice  );
	/// return weight depending on longest plDuration, maxIntens and #of notes
	double weight( TCLICKTRACK *theClickTrack,
			   	   double durAlpha = 0.5 ,
				   double intensAlpha = 0.2,
				   double cNotesAlpha = 0.3);
    //! normalise weight of notes
    // void normWeight( double norm); 
	double tempo( int RecPPQI, int RecTempoI );
	
	/// scoreduration for distance this->next
	TFrac scoreDurationI;		  
	
	/// copy max intens, longest duration ,...
	void copyMaxVals( TCLICKNOTE *n2,
			TCLICKTRACK *theClickTrack);
	
	
	
	TCLICKNOTE( long perfAttack,
				TAbsTime scoreAttack,
				double maxIntens,
				TFrac perfDuration);
	virtual ~TCLICKNOTE ( void )
	{
		if( bestPatternList )
			delete [] bestPatternList;

	}
	
	void SetPlaytime( long playtime )
	{ AbsTime = playtime; };
	
	void setScoretime( TAbsTime scoretime );
	
	long Playtime( void )
	{ return PlaytimeL(); };
	
	TAbsTime scoretime( void )
	{ return scoretimeL; };
	
	/// merge succeeding clicknotes at with a nearly equal attack position
	TCLICKNOTE *mergeEqual( int recPPQ,  
							int recTempo,
							TCLICKTRACK *theClickTrack );
	
	unsigned char Intens( void )
	{ return (unsigned char)MaxIntensI; };
	
	void SetIntens( int intens )
	{ MaxIntensI = intens; };
	
	virtual void Debug( ostream &out,
			TCLICKTRACK *theClickTrack );
	
	virtual TCLICKNOTE *ToCLICKNOTE( void );
	TFrac testScoreIOI(void);
	
protected:
	/// ptr to the earliest overlapping note 
	/// the length of an overlap is retrieved from durationI
	TCLICKNOTE * earliestOverlapI;
}; //TCLICKNOTE


//! conversion function
TCLICKNOTE *CLICKNOTE( TMusicalObject *s );
//-----------------------------------------------------------------

/// calculate the relation of denominator complexity, for attackpoint/duration relation
double denomComplexity(TCLICKNOTE *from,
					   TCLICKNOTE * to,
					   TFrac *mult = NULL );

/// expand all note durations and shift attackpoints, return deltaShift after last note
TFrac expandDurations(TCLICKNOTE *from,
					   TCLICKNOTE *to,
					   TFrac *mult );

double primeDenomDistance( TCLICKNOTE *cur );
typedef TCLICKNOTE * PTCLICKNOTE;

#endif
