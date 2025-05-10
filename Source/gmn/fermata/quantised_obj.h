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
|	filename : quantised_obj.H
|	Autor     : Juergen Kilian
|	Datum	    : 2003,2011
|	classdeclaration for TQuantisedObject
------------------------------------------------------------------*/
#ifndef __quantised_obj_h__
#define __quantised_obj_h__


//-----------------------------------------------------------------
#include <sstream>
using namespace std;
#include <stdlib.h>
#include "portable.h"
#include <stdio.h>

#include "fragm.h"

#include "musical_obj.h"

#include "notestatlist.h"
/// state of attackpoint/duration selection
enum { undefSel,
		firstSel,
		secondSel,
};


/*
class TNOTE;
class TQNOTE;
class TQuantizedObject;
class TMetaTempo;
class TMetaMeter;
class TMetaKey;
class TMetaText;
class TCLICKNOTE;
*/

class TTRACK;

#define SecDiffT (diffTimes[1].DiffTime)
#define BestDiffT (diffTimes[0].DiffTime)

/// tool class for quantisation 
class TFracIndex{
public:   TFrac DiffTime;		// timeoffset
		  double prob;			// probability for this selection
			int prevIndex;		// index of alternative in prev note

		TFracIndex( void )
		{
			DiffTime = 0L;
			prevIndex = -1;
			prob = 0;
		};
}; 

/// MusicalObject with quantisation values
class TQuantizedObject :  public TMusicalObject
{

protected:
	int			diffTimeSize;
	TFracIndex *diffTimes; // Differenz zu den Alternativen
  // TAbsTime  BestDiffT,		
	//	  SecDiffT;		// den Anschlagszeitpunkt



public:
	/// selected ATselection
	 char atSelection;
virtual TAbsTime Convert( ostream &gmnOut,	// gmn output file
			  TAbsTime preEndtime,	// enpoint of pre-note
			  TAbsTime  prev,		// denominator of pre-note
			  TTRACK *Track0 ); // control track (tempo, key, meter, ...)

    /// check if both abstime alternatives are equal
    char TOK( void );

	virtual void ticksToNoteface( int ppq );
	virtual TAbsTime SetQData( TAbsTime endPrev,
								TQuantizedObject *ptr,
								TFracBinList *bestDurations);
	TQuantizedObject( TAbsTime absTime ) :
			TMusicalObject(absTime)
	{
		diffTimeSize = 2;
		diffTimes = new TFracIndex[diffTimeSize];

		diffTimes[0].DiffTime = 0L;
		diffTimes[1].DiffTime = 0L;
		atSelection = undefSel;
				/*
		BestDiffT = 0L;
		SecDiffT = 0L;
		*/
	}
	virtual ~TQuantizedObject(void)
	{
		delete [] diffTimes;
	};

	virtual TAbsTime qDuration( TAbsTime *secduration = NULL )             // liefert beste Alternative
	{
		if(secduration)
			*secduration = 0L;
		return 0L;
	}
	virtual TAbsTime qAttackpoint( TAbsTime *sectime = 0 )			// best alternatives
	{	
		if( TOK() ||
			atSelection != secondSel )
		{
			if( sectime )
				*sectime = GetAbsTime() + SecDiffT;
			return GetAbsTime() + BestDiffT;
		}
		else
		{
			if( sectime )
				*sectime = GetAbsTime() + BestDiffT;
			return GetAbsTime() + SecDiffT;
		}
	};

	
	virtual TMusicalObject *GetNext(int voice);
	void shiftQAbstime( TAbsTime f, TAbsTime s )		// Verschiebt die Alternativen
		{				// fü den Attackpoint
			diffTimes[0].DiffTime += f;
			diffTimes[1].DiffTime += s;
			/*
			BestDiffT += f;
			SecDiffT  += s;
			*/
		};
	void shiftQAbstimeTo(TAbsTime newAttack1,
					 TAbsTime newAttack2,
					 char finalise)
	{
		if( !finalise )
		{
		diffTimes[0].DiffTime = /*
			BestDiffT =  */ newAttack1 - GetAbsTime();
		diffTimes[1].DiffTime = /*
			SecDiffT  = */ newAttack2 - GetAbsTime();
		}
		else
		{
			SetAbsTime(newAttack1);
			BestDiffT = 0L;
			SecDiffT = 0L;
		}	
	}
	TAbsTime GetBestDiffAttack(void)
		{return BestDiffT;};
	TAbsTime GetSecDiffAttack(void)
		{return SecDiffT;};
};


TFrac WriteDuration( ostream &gmnOut,
						 TAbsTime duration,
//						 int ppq,
						 TFrac prev);


TQuantizedObject * QuantizedObject( TMusicalObject *s );




//-----------------------------------------------------------------
#endif
