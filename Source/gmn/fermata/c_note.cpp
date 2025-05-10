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
|	filename : C(LICK)NOTE.CPP
|	Autor     : Juergen Kilian
|	Datum	  : 17.10.1996-2004-2011
|	Aufgabe   : implementation of TCLICKNOTE
------------------------------------------------------------------*/
#include "debug.h"
#include <iostream>
using namespace std;
#include <sstream>
#include <stack>

#include "c_note.h"

#include "c_track.h"
#include "midi.h"

#include "event.h"
#include <string>
#include "funcs.h"
#include <math.h>

#include "anvoice.h"
#include "pattern.h"

#include "q_funcs.h"


double primeDenomDistance( TCLICKNOTE *cur )
{
	if(!cur)
		return 0;
	long aD = cur->scoretime().denominator();
	long dD = cur->scoreDuration().denominator();
	return primeDenomDistance(aD, dD);
}

//--------------------------------------------------------------------
/*
	Construct a TClicknote
*/
TCLICKNOTE::TCLICKNOTE(
			long  perftime,				// attacktime  in inputfile (as played)
			TAbsTime  scoretime,			// attacktime in outfile (as quantized)
			double maxIntens,	// max velocity of all notes at attacktime
			/// max duration of all notes merged to clicknote
			TFrac duration
			) : TMusicalObject(perftime) 
{
	createTag = NULL;
	cNotes = 1;
	startId = 0;
	scoretimeL  = scoretime;
	MaxIntensI = maxIntens;
	// WeightI = weight;
	durationI = duration.toLong();
	earliestOverlapI = NULL;
	SetNext( NULL, NULL );
	prelScDuration = 0L;
	
	bestPatternList = NULL;
	patternFinalID = -1;

	patScoreIOIratio = 0;
} // TCLICKNOTE
//--------------------------------------------------------------------
/*
	Merge all clicknotes with an "equal" attacktime
	Compare this <.> next
	return new Nextptr or this if next == NULL

	Todo: what happens to earliest overlap?
 */
TCLICKNOTE  *TCLICKNOTE::mergeEqual( int recPPQ,		
										int recTempo,
										TCLICKTRACK *theClickTrack )
{
    /*
	TFrac DiffTime;	// distance in ms
	 long 	 DiffClick;	// distcance in MIDI-Ticks

	TCLICKNOTE *Temp,
		       *Tail;

	TCLICKNOTE *nextPtr;
*/
	TCLICKNOTE *Tail = this;
	TCLICKNOTE *nextPtr = CLICKNOTE(GetNext(-1));
	if( nextPtr )    // end of list?
	{
		Tail = CLICKNOTE(GetNext(-1));

		// calculate distance
		long DiffClick = CLICKNOTE(nextPtr)->Playtime() - PlaytimeL();

		// convert in ms
		double DiffTime = DTIMEtoMS(recPPQ,
                                     recTempo,
                                     DiffClick);

		int comb = 0;
		double d1, d2, newPlay;

		if( DiffTime < EQUALTIME ) 	// equal attack?
		{
			comb = 1; // merge clicks
		}
		else if (DiffTime < 2*EQUALTIME ) // todo optimize TCLICKNOTE::CHeck
		{
			// check duration
			if( PlaytimeL() + durationI <= nextPtr->PlaytimeL() )
			{
				// no overlapp, don't merge
			}
			else // compare gap / duration
			{
				double gap;
				gap = (double)DiffClick;
				d1 = gap / durationI;
				d2 = gap / nextPtr->durationI;
				if( d1 < 0.125  || // 1/4 ~ 1/32
					d2 < 0.125 )
				{
					comb = 1;
				}
			} // else
		} // if 2 EQUALTIME


		/// merge "this" with this->nextPtr
		if( comb )
		{
			/// copy playtime from next???
			// SetPlaytime( nextPtr->PlaytimeL() );
			
			// calc new playtime
			d1 = weight(theClickTrack) * (double)durationI;
			d2 = nextPtr->weight(theClickTrack) * (double)nextPtr->durationI;



			newPlay = ((double)PlaytimeL()*d1 + ((double)nextPtr->PlaytimeL())*d2);			
			// newPlay /= 2;	// todo calc new playtime depending on weights
			newPlay /= d1+d2;
			SetPlaytime( (long) newPlay );


			
			// copy, calc new values
			copyMaxVals(nextPtr, theClickTrack );


			/// change all earliestOverlap == nextPtr
				TCLICKNOTE *Temp = CLICKNOTE(nextPtr->GetNext(-1));								
				long nextOffset = nextPtr->Playtime() + nextPtr->durationI;
				while( Temp &&
					   nextOffset > Temp->Playtime() )
				{
					if( Temp->earliestOverlapI == nextPtr )
						Temp->earliestOverlapI = this;
					Temp = dynamic_cast<TCLICKNOTE *>(Temp->GetNext(-1));
				} // while

			/// delete one note, reorganize list
			Temp = CLICKNOTE(nextPtr->GetNext(-1));
			delete nextPtr;
			nextPtr = Temp;
			SetNext(nextPtr,theClickTrack);
			if( nextPtr )
			{
				nextPtr->SetPrev( this );
				Tail = nextPtr;	// return new next
			}
			else
			{
				// this == end of list
				Tail = this;
			}
		} // comb
	} // if( NextPtr
	return Tail;
} // Check
//--------------------------------------------------------------------

/*
	if mode > 0 -> write all successive notes
*/
void TCLICKNOTE::Debug( ostream &out, TCLICKTRACK *theClickTrack )
{
/*	if( startId == -10 || // singlenote
		startId == -11 )
		fprintf(out, "#----------------------------\n");
*/
	out << PlaytimeL()<< ";" ;
	out << 	scoreDurationI.numerator() << "/" <<
			scoreDurationI.denominator() <<"; ";
//	scoreDurationI.Write(out);
	out <<	scoretimeL.numerator()  << "/" <<
			scoretimeL.denominator() << ";" ;
//	scoretimeL.Write(out);
//	fprintf(out,"i: %d ",MaxIntensI);
//	fprintf(out,"w: %.3f ",WeightI);
//	fprintf(out,"d: %ld ",durationI);

	out << weight(theClickTrack) << ";";
	out << cNotes << ";";
	if( this->GetPrev(-1) && GetNext(-1) )
		out << IOIratio(-1,1) << ";";
	if( this->GetPrev(-1) && GetNext(-1) )
		out << relScoreIOI(-1,1) << ";";


	if( bestPatternList  )
	{
		out << "p(";
		for( int i=0; i < 3; i++ )
		{
			if( i )
				out <<";";
			if( bestPatternList[i].ptr)	
				out <<   bestPatternList[i].ptr->GetID()  << "," <<
										 bestPatternList[i].distance;
		}
		if( patternFinalID > -1 )
			out << ":" << bestPatternList[patternFinalID].ptr->GetID();
		out << ");";
		
	}
	else
		out << ";";

	if( startId != 0 )
		out << startId << ";";
	else
		out <<";";
	
	if( validIOIratio( this->patScoreIOIratio ) )
		out << patScoreIOIratio  << ";";
	else
		out << ";";

	if( createTag )
	{
		testScorePos().Write(out);
	}
	out << ";";

	if( startId == -10 || // singlenote
		startId == -12 )
		out <<  "#------------";
} // Write


void TCLICKNOTE::copyMaxVals(TCLICKNOTE *n2,
							TCLICKTRACK *theClickTrack)
{

	cNotes += n2->cNotes;
	if( theClickTrack &&
		cNotes > theClickTrack->maxCNotes )
	{
		theClickTrack->maxCNotes = cNotes;
	}


	if( n2->MaxIntensI > MaxIntensI )
		MaxIntensI = n2->MaxIntensI;

	
	// keep weight in [0..1]
	/*
	if( n2->WeightI > WeightI )
	{
		WeightI = n2->WeightI + ((1-n2->WeightI)*WeightI);
	}
	else
	{
		WeightI = WeightI + ((1-WeightI)*n2->WeightI);
	}
*/
	if( n2->durationI > durationI )
		durationI = n2->durationI;
}

double TCLICKNOTE::tempo( int RecPPQ, int RecTempo )
{
	
    // search for neighbour with specified score duration
    if( scoreDurationI == 0L )
	{
		// Printf("Warning: duration of clicknote == 0! Can't calc tempo!\n");
		// search neighbours for possible tempo
		TCLICKNOTE *tempL = dynamic_cast<TCLICKNOTE *>(GetPrev(-1));
		TCLICKNOTE *tempR = getNext(1,-1);

		TFrac zero;
        while( tempL &&
               tempL->scoreDuration() == zero)
        {
            tempL = dynamic_cast<TCLICKNOTE *>(tempL->GetPrev(-1));
        }

        while( tempR &&
               tempR ->scoreDuration() == zero)
        {
            tempR = tempR->getNext(1,-1);
        }

        double tempoL, tempoR;
		if( tempL )
			tempoL =  tempL->tempo( RecPPQ, RecTempo );
		if( tempR )
			tempoR =  tempR->tempo( RecPPQ, RecTempo );
        if( tempL && tempR )
        {
            return (tempoL + tempoR) / 2;
        }
        else if( tempR )
            return tempoR;
        else if( tempL )
            return tempoL;
        else // no neighbours
			std::cout << "WARNING: TCLICKNOTE scoreduration == 0, can't calc tempo!." << endl;
		return -1;

	} // if zero score duration



    // this has a score duration and a score position -> calculate the tempo
    // search for next valid note
	TCLICKNOTE *tempNext  = CLICKNOTE(GetNext(-1));
	while( tempNext &&
		   tempNext->Playtime() < 0 )
	{
		// skip invalid
  // todo: check during tempo detection!
        Printf("Check tempo calculation!\n");
        tempNext = CLICKNOTE(tempNext->GetNext(-1));
	}

	// delta in float ticks
	double difftickF;
	if( tempNext ) // calculate from attack distance
	{
		difftickF = (double)(tempNext->Playtime() - Playtime());
	}
	else // calc from duration
	{
		difftickF = -1;
		if( durationI > 0 )
		{
			difftickF =  (double)durationI;
		}
		else
		{
			tempNext  = CLICKNOTE(GetPrev(-1));
			if( tempNext )
				difftickF = tempNext->ioi(1,-1).toDouble();
		}			
	} // else, no next

// 	TFrac zero;

    TFrac rel = TFrac(1,4) / scoreDurationI;
    difftickF *= (double)rel.toDouble();

    long difftick = (long)(difftickF + 0.5); /// convert into long
    double ms_p_quarter = DTIMEtoMS( RecPPQ, RecTempo, difftick);

    double res = quarterMSToBPM( ms_p_quarter );
    return res;

}

/*
void TCLICKNOTE::normWeight(double norm)
{
	if( norm > 0 )
		WeightI /= norm;
}
*/

double TCLICKNOTE::weight(
				TCLICKTRACK *theClickTrack,
		   	   	   double durAlpha,
				   double intensAlpha,
				   double cNotesAlpha)
{


	/// 0.5s -> weight = 0.5
	double durWeight = 1 - exp(-0.69315 * (double)durationI*2 / theClickTrack->getParent()->quarterLengthMS());
	double intensWeight = 0,
		   notesWeight = 0;
	if( theClickTrack )
	{
		/// we need +1 to avoid 0/0!
		intensWeight = (MaxIntensI-theClickTrack->minIntens+1) /
						(theClickTrack->maxIntens - theClickTrack->minIntens + 1);
		notesWeight = cNotes / theClickTrack->maxCNotes;
	}
	else
	{
		intensWeight = MaxIntensI /128;
		notesWeight = 1 - exp(-cNotes * 0.16094);
	}

	/// 10 notes -> weight = 0.8 

#ifdef _DEBUG
	if( durAlpha + intensAlpha + cNotesAlpha != 1 )
	{
		Printf("WARNING: CLICKTNOTE::weight alpha values != 1\n");

	}
#endif

	double res = durAlpha * durWeight + intensAlpha*intensWeight + cNotesAlpha* notesWeight;
	return res;
}

TCLICKNOTE *TCLICKNOTE::ToCLICKNOTE( void )
{
	return this;
}

TFrac TCLICKNOTE::testScoreIOI(void)
{
	TFrac x;
	TCLICKNOTE *n = CLICKNOTE(GetNext(1));
	if( n )
	{
		x = n->testScorePos() - testScorePos();
	}
	return x;
}

void TCLICKNOTE::setScoretime( TAbsTime scoret )
{ 
	scoretimeL =  scoret; 
	/*
	TCLICKNOTE *p;
	if( GetPrev(-1) )
	{
		p = CLICKNOTE(GetPrev(-1));
		if( p->scoreDuration() > 0L )
		{
			p->scoreDurationI = scoretime() - p->scoretime();
		}
	}
	*/
};


TCLICKNOTE * TCLICKNOTE::getNext(int nSkip, int voice)
{
	TCLICKNOTE *res = NULL;
	if( nSkip > 0 )
	{
		res = CLICKNOTE(GetNext( voice ));
		nSkip--;
		while( nSkip > 0 &&
			   res)
		{
			res = CLICKNOTE(res->GetNext( voice ));
			nSkip--;
		}
	}
	else if( nSkip < 0 )
	{
		res =  CLICKNOTE(GetPrev( voice ));
		nSkip++;
		while( nSkip < 0 &&
			res)
		{
			res =   CLICKNOTE(res->GetPrev( voice ));
			nSkip++;
		}
	}	
	return res;
}

TAbsTime TCLICKNOTE::scoreIOI(int nSkip)
{
	TAbsTime res = 0l;
	TCLICKNOTE *temp = getNext(nSkip, -1);
	// caluclate distance
	if( temp )
	{
		if( nSkip == 1 &&
				scoreDuration() > 0L )
			res = scoreDuration();
		else if( nSkip == -1 &&
				 temp->scoreDuration() > 0L )
			res = temp->scoreDuration();
		else if( temp->scoreDuration() > 0L && // valid
			scoreDuration() > 0L )				// valid
			res = abs( temp->scoretime() - scoretime() );

	}
	return res;
}

double TCLICKNOTE::relScoreIOI(int prevPos, int nextPos)
{
	double res;
	double ioiPC = scoreIOI(prevPos).toDouble();
	double ioiCN = scoreIOI(nextPos).toDouble();
	if( ioiPC != 0 &&
		ioiCN != 0 )
	{
		res = ::IOIratio( ioiPC, ioiCN);
	}
	else
		res = 0;
	return res;	
}

void TCLICKNOTE::SetNext(TCLICKNOTE *ptr, TCLICKTRACK *theClickTrack)
{
	TMusicalObject::SetNext(ptr );
	
	if( !ptr )
		return;
		
	if( theClickTrack )
	{
		if( ptr->cNotes > theClickTrack->maxCNotes )
			theClickTrack->maxCNotes = ptr->cNotes;
		
		if( ptr->Intens() > theClickTrack->maxIntens )
			theClickTrack->maxIntens = ptr->Intens();
		
		if( ptr->Intens() < theClickTrack->minIntens )
			theClickTrack->minIntens = ptr->Intens();
	}
	/// is "this" already overlapped?
	if( earliestOverlapI )
	{
		if( earliestOverlapI->Playtime() + 
			earliestOverlapI->durationI >
			ptr->Playtime() )
		{
			ptr->earliestOverlapI = earliestOverlapI;
		}
	}
	else if( Playtime() + durationI > ptr->Playtime() )
	{
		ptr->earliestOverlapI = this;
	}
}



/// return velocityRatio for TNOTE and TCLICKNOTE
/*! result 
	error : < 0 
	else ptr->Intens/next->Intens = (0..127]
*/
double velocityRatio( TMusicalObject *ptr)
{
	double v1 = -1,
			v2 = -1;

	// get ptr and ptr->next intnsity/velocity
	TCLICKNOTE *clickPtr = dynamic_cast<TCLICKNOTE *>(ptr);
	if( clickPtr )
	{
		v1 = clickPtr->Intens();
		clickPtr = dynamic_cast<TCLICKNOTE *>(ptr->GetNext(-1));
		if( clickPtr )
		{
			v2 = clickPtr->Intens();
		}
	}
	else
	{
		TNOTE *notePtr = dynamic_cast<TNOTE *>(ptr);
		if( notePtr )
		{
			v1 = notePtr->getIntens();
			notePtr = dynamic_cast<TNOTE *>(ptr->GetNext(-1));
			if( notePtr )
			{
				v2 = notePtr->getIntens();
			}			
		}
	} // else, no clickPtr
	// calc velocityRatio
	double res = -1;	
	if( v1 > 0 && 
		v2 > 0 )
	{
		res = v1/v2;
	}
	else if( v1 < 0 ||
			 v2 < 0 )
			 // one value does not exist
	{
		res = -1;
	}
	else // one value == 0
	{
		res = 0;
	}
	return res;
}



TCLICKNOTE *CLICKNOTE( TMusicalObject *s )
{
	if( s != NULL )
		return dynamic_cast<TCLICKNOTE *>(s);
	else
		return NULL;
}

TAbsTime TCLICKNOTE::scoreDuration()
{
	return scoreDurationI;
}

double TCLICKNOTE::plRIOI(int direction, double prevError)
{
	double std; 
	if( direction == dirforward )
		std =  IOIratio(-1,1); 
	else
	{
		std =  IOIratio(1,2); 
		std *= -1;
	}

	if( prevError != 0 )
	{
		if( std > 1 )
			std = std / prevError;
		else
			std = std * prevError;
	}
	if( std > 0  &&
		std < 1 )
		std = -1 / std;
	else if( std < 0 &&
			 std > -1 )
		std = -1 / std;

	return std;
}

long TCLICKNOTE::plOffset()
{
	return this->Playtime()+this->durationI;
}

long TCLICKNOTE::plDuration()
{
	return durationI;
}

double TCLICKNOTE::IOIratioError()
{
	// todo: compare also  weight of notes
	double i1 = IOIratio(-1,1);
	double i2 = relScoreIOI(-1,1);
		
	// normalize
	if( i1 > 0 ) 
		i1 -= 1;
	else
		i1 += 1;
	
	// normalize
	if( i2 > 0 ) 
		i2 -= 1;
	else
		i2 += 1;
	
	double er = 1 - exp( - 2 * pow(i1-i2,2) ); 	
	return er;
}


TFrac expandDurations(TCLICKNOTE *from,
					   TCLICKNOTE *to,
					   TFrac *mult )
{
	TFrac offset;
	if( from )
		offset = from->scoretime();

	TFrac res;
	TCLICKNOTE *cur= from;
	while( cur &&
		   cur != to )
	{
		TFrac dur = cur->scoreDuration();
		if( mult )
			dur *= *mult;
		

		TFrac attack = cur->scoretime();
		if( mult )
		{
			attack -= offset;
			attack *= *mult;
			attack += offset;
		}

		res = attack;
		res -= cur->scoretime();
		res += dur;
		res -= cur->scoreDuration();

		cur->setScoretime( attack );
		cur->scoreDurationI = dur;

		cur = cur->getNext(1,-1);
	}
	return res;

}


double denomComplexity(TCLICKNOTE *from,
					   TCLICKNOTE *to,
					   TFrac *mult )
{
	double atDenomSum = 0;
	double durDenomSum = 0;

	TFrac offset;
	if( from )
		offset = from->scoretime();

	TCLICKNOTE *cur = from;
	while( cur &&
		   cur != to )
	{
		TFrac dur = cur->scoreDuration();
		if( mult )
			dur *= *mult;
		durDenomSum += dur.denominator();

		TFrac attack = cur->scoretime();
		if( mult )
		{
			attack -= offset;
			attack *= *mult;
			attack += offset;
		}

		atDenomSum += attack.denominator();
		cur = cur->getNext(1,-1);
	} // while notes
	double res = 0;
	if( durDenomSum > 0 )
		res = atDenomSum / durDenomSum;
	return res;
}
/* debug before usage!!!
double TCLICKNOTE::dTempo_dTime(TFrac &newDur, 
								int direction,
								TCLICKTRACK *ClickTrack,
								TMIDIFILE *midifile)
{
	double curTempo = ClickTrack->tempo(this);
	TFrac oldScoreDur = scoreDurationI;
	scoreDurationI = newDur;
	double newTempo = ClickTrack->tempo(this);
	scoreDurationI = oldScoreDur;
	
	double res = 0;

	double dTempo = newTempo - curTempo;
	if( getNext(direction,-1) )
	{
		long delta = (getNext(direction,-1)->GetAbsTime() - GetAbsTime()).toLong();
		double dTime = DTIMEtoMS(delta, midifile->RecTempo, midifile->RecPPQ );
			
		if( dTime < 0 )
			dTime *= -1;
		res = dTempo / dTime;
	}
	return res;
}
*/
double TCLICKNOTE::durationalAccent()
{
	double  prevIOI = 0;
	TCLICKNOTE *p = getNext(-1,-1);
	if( p )
		prevIOI = p->ioi(1,-1).toDouble();
		
	double nextIOI = 0;
	TCLICKNOTE *n = getNext(1,-1);
	if( n ) 
		nextIOI = n->ioi(1,-1).toDouble();
		
	double curIOI = ioi(1,-1).toDouble();
	return ::durationalAccent(prevIOI, curIOI, nextIOI);	
}
