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
|	filename    : q_funcs.CPP
|	author      : Juergen Kilian
|	date	    : 17.10.1996-98-2000-03, 2011
|   global quantize functions
|		03/05/27	revised quantisation functions
------------------------------------------------------------------*/

#include <iostream>
using namespace std;
#include <string>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <math.h>



#include "h_midi.h"
#include "portable.h"
#include "funcs.h"
#include "q_funcs.h"
#include "pattern.h"
#include "q_track.h"
#include "patternfile.h"
#include "question.h"
#include "../lib_src/ini/ini.h"
#include "q_note.h"

#include "notestatlist.h"

#include "statist.h"

#if defined (Windows)
#include <owl\owlpch.h>
#include <owl\except.h>
#endif

#undef UNUSED

// used at SetLaenge()
#define DURATION_LIMIT 0.34
//#define DURATION_LIMIT TFrac(34,100)
// if delta_after/duration < PREFER_LONG_DUR -> select longer duration
// nach
#define PREFER_LONG_DUR 0.3

#define	M_PI		3.14159265358979323846

#define MAXERROR 31

void setDefaultPattern( TPFILE *pFile )
{
	if( !pFile )
		return;
	if( pFile->name.c_str() != string("") )
	{
		FILE *out = fopen(pFile->name.c_str(),"wt");
		if( out )
		{
			fprintf(out, "{\n");
			fprintf(out, "[c/4 c c c  ],\n");
			fprintf(out, "[c/8 c c c  ],\n");
			fprintf(out, "[c/16 c c c  ]\n");
//			fprintf(out, "[ ]"\n);
			fprintf(out, "}");
			fclose(out);
			pFile->Read();
		} // if 
	} // if name
}


double durationalAccent(double IOI1, double IOI2, double IOI3)
{
#define durAccRel 1.2

	// set default vvalues for unknown iois
	if( IOI1 <= 0 )
		IOI1 = IOI2;
	if( IOI3 <= 0 )
		IOI3 = IOI2;

	// no accent if not longer/shorter than all
	if( IOI2 > IOI1 &&
		IOI2 < IOI3 )
	{
		return 0;
	}
	if( IOI2 > IOI3 &&
		IOI2 < IOI1 )
	{
		return 0;
	}
/*	
	double pPenalty;
	double sigma = 0.2;
	if( IOI1 > IOI2 )
	{
		pPenalty = -1 + GaussWindow(IOI1/IOI2,1,sigma);		
	}
	else
	{
		pPenalty = 1 - GaussWindow(IOI2/IOI1,1,sigma);		
	}
	
	double nPenalty
	if( IOI3 > IOI3 )
	{
		pPenalty = -1 + GaussWindow(IOI3/IOI2,1,sigma);		
	}
	else
	{
		pPenalty = 1 - GaussWindow(IOI2/IOI3,1,sigma);		
	}
	double res = 0.5 * pPenalty + 0.5 * nPenalty;
*/

	double avRatio = 0;
	if( IOI1 > IOI2 &&
		IOI3 > IOI2 )
	{
		avRatio = -min(IOI1/IOI2, IOI3/IOI2);
	}
	else
	{
		avRatio = min(IOI2/IOI1, IOI2/IOI3);
	}
	
	double res = 1 - GaussWindow(fabs(avRatio), 1, 0.2);
	if( avRatio < 0 )
		res *= -1;

	return res;
}


int validIOIratio( double ratio)
{
	if( ratio >= 1 || ratio < -1 )
		return 1;
	return 0;
}

//----------------------------------------------------------------------
// allowed values, will be read from maskfile
//int cDurations = 0;
//int cAttacks  = 0;
//TFrac *Durations = NULL;  // array for duration grid
//TFrac *Attacks = NULL;		// array for attackpoint grid

/*
convert MIDI-tick timing into real noteface values
*/
void	ticksToNoteface( TQTRACK *track, int ppq )
{
	TQuantizedObject *temp;
	TMusicalObject *ttemp;

	if( !track || ppq <= 0 )
		return;

	track->Debug();
	ttemp = track->FirstObject(-1 /* all voices */);
	while(ttemp)
	{
		if(ttemp)
		{
			temp = dynamic_cast<TQuantizedObject *>(ttemp);
			temp->ticksToNoteface( ppq );
			ttemp = temp->TMusicalObject::GetNext(-1/* all voices */);
		}
	} // while
//	track->Debug();
} // ticksToNoteface



//----------------------------------------------------------------------
/*
mark all qAttackpoints which are inside the searchwindow
window = played attack +|- sWindow

  result :
		char [] : where [i] == 1 if absTime - delta[i] <= sWindow
		delta[] : distances between alternatives and played attacks
		return = # of valid grid positions
*/
int possibleAttackpoints( TAbsTime   abstime,
                          TAbsTime  lsWindow /*![abstime - lsWindow, .. */,
                          TAbsTime rsWindow /*! ... , abstime+rSWindow */, 
						 TFracBinList *qAttacks,
//						 int nAttacks,
						 char *valid,
						 TFrac *delta)
{

	// allow a minimum search window
	if( lsWindow == 0L )
		lsWindow = TFrac(1,128);
	if( rsWindow == 0L )
		rsWindow = TFrac(1,128);

	int 	j;
	int cValid = 0;
	int nAttacks = qAttacks->Count();
	for( j = 0; j < nAttacks; j++)
	{
		//		ppn = (ppq * 4 ) / Notenwerte[j];   // calculate #ticks for this durations
		TFrac ppn = (*qAttacks)[j];	// face value of current grid
		if( ppn == 0L)
		{
			std::cout <<  "Error: lgDuration == 0 in default.msk!" << endl;
            delta[j] = 0L;
            valid[j] = 0;
            // skip
		}
		else
		{
			TFrac dPre = abstime % ppn;	// distance to earlier grid-position with n*Notenwerte[j]
			TFrac dPost = ppn - dPre;	// distance to later grid-position with  n*Notenwerte[j]
			
			
			// select nearest position in grid
			if( (dPre < dPost) &&
				(TFrac)dPre <=  lsWindow )
			{			// select earlier position
				delta[j] = dPre * -1;
				valid[j] = 1;
				cValid++;
			}
			else if( (TFrac)dPost <=  rsWindow ) // select later position
			{
				delta[j] = dPost;
				valid[j] = 1;
				cValid++;
			}
            else
            {
				delta[j] = dPost;
                valid[j] = 0;
            }
        } // skip
	} // for
	return cValid;			// result is bitarray of valid alternatives
} // CheckTimeWindow
//-------------------------------------------------------------------
/*
value the possible delta[i] (=attackpoints)
result:
	- probabilities for each valid attackpoint
	- doubled, equal delta i will be marked as invalid!
*/
void valueAttackpoint( TQNOTE * /* note */,
                       char *valid,
                         TFracBinList *qAttacks,
					   TFrac *delta,
					   double acurateness /*! (0...1] */,
					   double *prob	)
{
	int i,
		j,
		cPossible = 0,    // #alternatives
		MaxCDiff  = 0;	// #sameDistances
	
	int cAttacks = qAttacks->Count();
	int *CDiff;		// #sameDistances for each distance
	CDiff = new int[cAttacks];
	for( i = 0; i < cAttacks; i++) // count #sameDistances for each distance
	{
		CDiff[i] = 1; // init
	}
	
	
	for( i = 0; i < cAttacks; i++) // count #sameDistances for each distance
	{		
		if( valid[i] ) // alternative valid? ?
        {
            for( j = i+1; j < cAttacks; j++ ) // compare differenz[i] to others
            {
				if( valid[j] &&	     // valid
                    delta[i] == delta[j] )
					
                {
                    CDiff[i]++;
                    CDiff[j] = 0; // use cDiff[j] anymore
                    valid[j] = 0; // use cDiff[j] anymore
                }
            } // for( j ....
        }
	} // for( i ...
	
	
	
	// search for max #sameDistance ------------------------
	for( i = 0; i < cAttacks; i++ )
    {
        if( MaxCDiff < CDiff[i])
			MaxCDiff = CDiff[i];
        if( valid[i] )
            cPossible += CDiff[i]; // count real possible values
    }
	
	if( !cPossible )
	{
		std::cout << "WARNING: no possible valid attackpoint!" << endl;
	}

    double pi2 = 1.0/sqrt(2*M_PI);
    double sigma = max(1.0/64.0, 1.0/4.0 * (1-acurateness));
	// calc vlues   ---------------------------------------
	for( i = 0; i < cAttacks; i++ )
	{
		if( valid[i] ) // valid?
        {
            prob[i] =  (double)CDiff[i]  / cPossible;
            double d;
            d = delta[i].toDouble();
            // normalverteilung: dichte phi = 1/wurzel(2pi) * exp(-(d^2)/2)
            // N(E,v) = 1/v * phi((d-E)/v)
            // E+/- v = Wendepunkte der Kurve
            d /= sigma;
            d *= d; // == (d/v)^2
            d = /*1.0/sigma * */ pi2 *  exp( -d/2 );

            //            d = exp(-d)*0.5;
            // prob[i] =  prob[i] + (1-prob[i])*d;
            /// prefer shifting inside duration
            //             d = exp(-d * acurateness);

            // d == [1...0)
            
            double sigmaI;
            sigmaI = sigma;
            d = GaussWindow(delta[i].toDouble(), sigmaI);	
            
            prob[i] *=  d;
        }
        else
			prob[i] = 0;
    }

		
		delete [] CDiff;
} // AlternativenBewerten
//-------------------------------------------------------------------
void	getBestNSecond( TFrac *delta,
					   double *prob,
					   int &maxpos,
					   int &secpos,
					   TAbsTime time,
					   int fSize,
					   char *valid )
{
	
	int i;
	maxpos = -1;
	secpos = -1;
	
	double maxprob  = -1;	// best alue
	double secprob  = -1;	// zsecond best values
	// search for best alternative
	for( i = 0; i < fSize; i++ )
	{
		if( valid[i] )
		{
			if( prob[i] > maxprob )
			{
				secprob = maxprob;
				secpos = maxpos;
				
				maxprob = prob[i];
				maxpos = i;
			} // if( prob...
			else if( prob[i] > secprob )
			{
				secprob = prob[i];
				secpos = i;
			}
			else if (0 && prob[i] == maxprob ) // if values are the same
			{										 // select the smaller noteface
				if( (time+delta[i]).denominator() < (time+delta[maxpos]).denominator() )
				{
					maxpos = i;
					maxprob = prob[i];
				}
			} // else if
		} // if valid
	} // for( i= ....
	if( maxpos < 0 )
		maxpos = maxpos;
	  /*
	  // search for second best alternativ
	  for( i = 0; i < fSize; i++ )
	  {
	  if( prob[i] &&        		     // don't select maxprob
	  !(delta[i] == delta[maxpos] ))	// again
	  {
	  if( prob[i] > secprob  )    // search for second best
	  {
	  secprob = prob[i];
	  secpos = i;
	  } // if( prob...
	  else if ( prob[i] == secprob ) // if two values are the same
	  {					 // select the smaller distance
	  if( abs(delta[i]) <
					 abs(delta[secpos]) )
					 secpos = i;
					 } // else if
					 } // if( prob[i]
					 } // for( i= ....
					 if(secpos == -1 ) // nothing found
					 {
					 secpos = maxpos;
					 }
	*/
}
//-------------------------------------------------------------------
/*
Select the two best alternatives for the attackpoint

  result :
		return id of best class
		
		  alternatives will be stored at *note
*/

int selectAttackpoint( TQNOTE      *note,
					  TFrac *delta,
					  double *prob,
						char	 *valid,		// entry is one if qDuration is inside range
                       TFracBinList *selIOIs,
					  int cAttacks)
{
    TQNOTE * prev = dynamic_cast<TQNOTE *>(note->GetPrev(note->GetVoice()));
 
 	// count possibilities
 	int cValid = 0;
    for( int i = 0; i < cAttacks; i++ )
    {
    	if( 0 && prev && valid[i] )
    	{
			// evaluate weight of possible IOI

	        TFrac plAt = note->GetAbsTime();
    	    TFrac qPAt1, qPAt2;
	        qPAt1 = prev->qAttackpoint( &qPAt2 );
		
				/// IOI to previous best
                TFrac IOI = plAt + delta[i] - qPAt1;
                double p1 = selIOIs->probability(IOI);
				/// IOI to previous second best
                IOI = plAt + delta[i];
                IOI -= qPAt2;
                double p2 = selIOIs->probability(IOI);
                double p = max( p1,p2 );
                prob[i] = prob[i] + (1-prob[i]) *p;
               
     	}            
        if( valid[i]  )
            {
				cValid++;
			}
			else
			{
				prob[i] = 0;
			}
        } // for
	if( !cValid )
		std::cout << "Error: no valid attackpoint!" << endl;

	int secpos        = -1,	// arrayposition of secbewertung
		maxpos        = -1;	// arrayposition of maxbewertung
    getBestNSecond( delta,
					prob ,
					maxpos,  // reference
					secpos,
		note->GetAbsTime(),
		cAttacks,	//reference
		valid );
	
	if( secpos == -1 )       	// all values must be the same
	{
		secpos = maxpos;		// select another attackpoint
	} // if
    note->shiftQAbstime( delta[maxpos], 
						 delta[secpos] );
    int erg = maxpos;
#ifdef _DEBUG
	if( erg < 0 )
		printf(" ");
#endif
    return erg;
    
    // store value into *note
	// smaller distance as BestAlternative
	// bigger distabce as SecondAlternative
	if( abs( delta[maxpos]) < abs( delta[secpos] ) )
	{
		note->shiftQAbstime( delta[maxpos], delta[secpos] );
		erg = maxpos;
	}
	else
	{
		note->shiftQAbstime( delta[secpos], delta[maxpos] );
		erg = secpos;
	}
	
	// calculate result
	
	if( (delta[maxpos] * delta[secpos]) > 0L )
        erg = maxpos; // + delta[secpos] ) / 2;
	return erg;
}       // Auswaehlen
//-------------------------------------------------------------------
//-------------------------------------------------------------------

/*
search for alternatives for the duration of note
result:
-BitArray with valid alternatives
-differenz=Array with distances to alternatives
remarks:
-Alternatives for the attackpoint must be calculated before!

*/
int possibleDurations(TQNOTE *note,		// current note
					  TAbsTime duration,	// played duration
					  TAbsTime lsWindow,	// >=0 width of calculation window
					  TAbsTime rsWindow,	// >=0 width of calculation window
					  TFracBinList *qDurations,	// list of allowed durations
					  TAbsTime *delta,		// distance to each duration
					  char	 *valid		// entry is one if qDuration is inside range
//					  int      cDurations
					  )	
{
	
	TFrac minWindow(1,32);
		// allow a minimum search window
	if( lsWindow == 0L && 
		duration > minWindow )
	{
		lsWindow = minWindow;
	}

	if( rsWindow == 0L )
	{
		rsWindow = minWindow;
	}

	// calculate the two possible attackpoints for note
	TAbsTime 	Start,		// best alternative for attackpoint of note
		         Start2;		// second best alternative for attackpoint of note
	Start   = note->qAttackpoint( &Start2 );

	TFrac	StartNext2,    	// best alternative for attackpoint of next note
			StartNext;		// second best alternative for attackpoint of next note

	if( note->GetNext(note->GetVoice())) // exists a next note?
	{
		// calculate the two possible attackpoints for next note
		StartNext  = QNOTE(note->GetNext(note->GetVoice()))->qAttackpoint( &StartNext2 );
	}
	else // no next note, set duration*1.5 as max duration
	{
		StartNext  = note->GetAbsTime() + (note->GetDuration()*TFrac(3,2));
		StartNext2 = StartNext;
	}
	TAbsTime EarliestOnset,
		LatestOnsetNext;
//	EarliestOnset = min(Start ,Start2);
//	LatestOnsetNext = max(StartNext, StartNext2);
	int cDurations = qDurations->Count();

	
    	
	int cValid = 0;
	for( int j = 0; j < cDurations; j++)
	{
        valid[j] = 0;

		TFrac cur = (*qDurations)[j];		
		TFrac dShorter  = duration % cur;		// distance to shorter duration
		TFrac dLonger   = cur - dShorter;		// distance to longer duration
		

		if( dShorter == 0L ||
			( dShorter < duration/2 && // allow only half of original duration
			  duration > 1/32 ) )
		{
			if( dShorter < dLonger )
			{
				delta[j] = dShorter * -1;
				if( dShorter <= lsWindow )
				{
					valid[j] = 1;
				}
            }
			else 
			{
				delta[j] = dLonger;
				if( dLonger <= rsWindow )
				{
					valid[j] = 1;
				}
            }
        }

		if( !valid[j] )
		{
			delta[j] = dLonger;
			if( dLonger <= rsWindow )
			{
				valid[j] = 1;
			}
        }


		if( primeDenomDistance( Start.denominator(),
								cur.denominator()) >= 1 &&
			primeDenomDistance( Start2.denominator(),
								cur.denominator()) >= 1) 
		{
             valid[j] = 0;
		}
		if( valid[j] )
			cValid++;
	} // for

	return cValid;
} // CheckLength
//-------------------------------------------------------------------
/*
value the valid durations for note
result:
bewertung[] = values for the valid durations
prefer:
- longer duration
- try offset == onset(next)
- small denominator
*/
void valueDuration( TQNOTE      *note,
				   char *valid,
                    TFracBinList *qDurations,
				   TFrac *delta,
					double accurateness /*! (0...1] */,
				   double *prob )
{
	int i,
		j,
		cPossible = 0,	// # of different alternatives
		MaxCDiff      = 0;	// max # of same distances
	
	int *CDiff;		// # same distances

	int cDurations = qDurations->Count();
	CDiff = new int[cDurations];
	// count # of valid alternatives
	for( i = 0; i < cDurations; i++ )
	{
		CDiff[i] = 1;
	} // for( i= 0...
	
	// count# of distances with same size
	for( i = 0; i < cDurations; i++)
	{
		if( valid[i] )
		{
			for( j = i+1; j < cDurations; j++ ) // compare delta[i] to all others
			{
				if( delta[i] == delta[j] )
                {       
                    CDiff[i]++;
                    valid[j] = 0;
                }
            } // for( j ....
		}
	} // for( i ...
	
	// count max of same distances
	for( i = 0; i < cDurations; i++ )
    {
        if( MaxCDiff < CDiff[i])
			MaxCDiff = CDiff[i];
        if( valid[i] )
            cPossible += CDiff[i];
    }

    double pi2 = 1.0/sqrt(2*M_PI);
    double sigma = max_(1/64, 1.0/4.0 * (1-accurateness)); // varianz
    // values the durations
	for( i = 0; i < cDurations; i++ )
	{
		if( valid[i] )	// valid
		{
			prob[i] = (double)CDiff[i] / cPossible;
            double d;
            d = delta[i].toDouble();


            // normalverteilung: dichte phi = 1/wurzel(2pi) * exp(-(d^2)/2)
            // N(E,v) = 1/v * phi((d-E)/v)
            // E+/- v = Wendepunkte der Kurve
            d /= sigma;
            d *= d;
            d = /* 1.0/sigma * */ 1/pi2 * exp( -d/2 );
            //
//            d = 1-exp(-d); /// prefer longer durations!
//            d = 0.5 + 0.5 * exp(-d*acurateness);

            //d *= 0.5;
            //d += 0.5;
            //prob[i] = prob[i] + (1-prob[i])*d;
            d = GaussWindow(delta[i].toDouble(), sigma);
			prob[i] *= d;
            
            /*
			if( !i || (i % 2) ) 			// i%2 == 1 => no triplet
			bewertung[i] *= 3;	// increase bewertung[i]
			else                           // i%2 == 0 => triplet
			bewertung[i] /= 2;	// decrease bewertung[i]
			*/
			
			// negativ durations are not valid
			if( (note->GetDuration() + delta[i]) <= 0L )
				prob[i] = 0;
		} // if( pot & ...
		else
			prob[i] = 0;
	} // for
	delete [] CDiff;
} // ValueDurations
//-------------------------------------------------------------------
/*
selectDuration
select best and second best alternativ for the duration

  return :
		id best alternativ
*/
int selectDuration( TQNOTE      *note,		// current note
				   TAbsTime	 *delta,		// distances to played duration
				   double      *prob, 		// values for durations
					char	 *valid,		// entry is one if qDuration is inside range
				   TFracBinList *selDurations,
				   int cDurations )
{
	
	int
	/*		i,
	maxbewertung  = 0,
	secbewertung  = 0,*/
	maxpos        = -1,
	secpos        = -1;

	
	int cValid = 0;
	// evaluate history of durations
	TFrac plDur = note->GetDuration();
	for( int i = 0; i < cDurations; i++ )
	{
		if( valid[i] )
		{
			prob[i] = prob[i] + (1-prob[i]) * selDurations->probability(plDur+delta[i]);
			cValid++;
		}
		else
			prob[i] = 0;
	}

	
	if( !cValid )
	{
		std::cout << "no valid duration found!" << endl;
	}
	getBestNSecond( delta,
		prob,
		maxpos,
		secpos,
		note->GetDuration(),
		cDurations,
		valid);
	
	if( secpos == -1 )	// all valid alternatives are equal
		secpos = maxpos;
	
	
    note->incQDuration( delta[maxpos],
						delta[secpos] );
    return maxpos;
    
    
    TAbsTime tBest, tSecond;
	tBest = note->GetDuration() + delta[maxpos];
	tSecond = note->GetDuration() + delta[secpos];
	
	// store best and second best value in note
	// if best value is more then DURATION_LIMIT > than second best value,
	// store like calculated else:
	// store longer duration as best alternativ
	// store shorter duration as second best
	if( delta[maxpos] == 0L )  // equal to played duration
	{
		note->incQDuration( delta[maxpos],
			delta[secpos] );
		return maxpos;
	}
	else if(  delta[secpos] == 0L )  // equal to played duration
	{
		note->incQDuration( delta[secpos],
			delta[maxpos] );
		return secpos;
	}
	// prefer bigger noteface values
	else if( tSecond.denominator() < tBest.denominator() )
	{
		note->incQDuration( delta[secpos], delta[maxpos] );
		return secpos;
	}
	else if( (prob[secpos]/prob[maxpos]) < DURATION_LIMIT )
	{
		note->incQDuration( delta[maxpos], delta[secpos] );
		return maxpos;
	}
	// store longer duration as the best
	else if( delta[maxpos] > delta[secpos] )
	{
		note->incQDuration( delta[maxpos], delta[secpos] );
		return maxpos;
	}
	else
	{
		note->incQDuration( delta[secpos], delta[maxpos] );
		return secpos;
	}
}       // SetLaenge
//-------------------------------------------------------------------

/*
return a list of valid durations,
! result must be delted
*/
TDurationList  *readDurationList( const char *fname  )
{
	TFracBinList *Durations;
	// Todo: calculate alpha from something generic
	Durations = new TFracBinList(10 /* alpha */);

	Durations->readIni( fname );
	// we need at least a single duration
	if( Durations->Count() < 1 )
	{
		Durations->addIfNew(TFrac(1,2));
		Durations->addIfNew(TFrac(1,4));
		Durations->addIfNew(TFrac(1,8));
		Durations->addIfNew(TFrac(1,16));
	}
	Durations->setMinMaxRel(30.0);
	Durations->setMinweightDeltaRel(30);
	Durations->createDistribution();
	Durations->sort();
	return Durations;
} // Read Mask

TFracBinList *readAttackpointList( const char *fname )
{
	// Todo: calculate alpha from something generic
	TFracBinList *Attacks = new TFracBinList(10 /* alpha */);

	Attacks->readIni( fname );
	// we need at least a single duration
	if( Attacks->Count() < 1 )
	{
		Attacks->addIfNew(TFrac(1,2));
		Attacks->addIfNew(TFrac(1,4));
		Attacks->addIfNew(TFrac(1,8));
		Attacks->addIfNew(TFrac(1,16));
	}

	Attacks->sort();
	Attacks->setMinMaxRel(30.0 );
	Attacks->setMinweightDeltaRel(30.0);
	Attacks->createDistribution();
	return Attacks;
} // Read Mask
//-------------------------------------------------------------------
/*
count number of ',' separated fragments
return -1 if syntax error
*/
int countValues(const char *str )
{
	int state, res;
	res = 0;
	state = 0;
	while( str && *str != 0)
	{
		switch(state)
		{
		case 0: // whitespace
			if( *str == ' ' )
			{
				str++;
			}
			else if( isdigit(*str) )
			{
				state = 1; // numerator
			}
			else if( *str == ';' )
			{
				str = NULL;
			}
			else // syntax error
			{
				res = -1;
				str = NULL;
			}
			break;
		case 1 : // numerator
			if( isdigit(*str) )
			{
				str++;
			}
			else if( *str == '/' )
			{
				state = 2; // denominator
				str++;
			}
			else // syntax error
			{
				res = -1;
				str = NULL;
			}
			break;
		case 2 : // denominator
			if( isdigit(*str) )
			{
				str++;
			}
			else if( *str == ' ' )
			{
				str++; // whitespace after denominator
			}
			else if( *str == ',' || // separator
				*str == 0 )
			{
				state = 0; // whitespace
				str++;
				res++;
			}
			else if( *str == ';' )
			{
				state = 0;
				res++;
			}
			else // syntax error
			{
				res = -1;
				str = NULL;
			}
			break;
		}
	} // while 
	if( state == 2 && *str == 0 )
		res++;
	return res;
}
//-------------------------------------------------------------------

/*
Quantize a MIDI file
followinf functions must be called before:

  SetUpbeat
  SetCalcTempo
  SetClickTrackNr
  SetRelduration
  
*/
void process( THMIDIFILE *Ptr,	// Ptr to MIDI file
			 char 		*Inifilename,
			 char      *Patternname, // file name of patternfile
			 ostream &gmnOut, // file name of gmn file
			 char 	   * /*Maskname*/,
			 long int			TimeSigNum,
			 long int			TimeSigDenom )
{
	TPFILE    *Pattern;	// Pattern file
	TAbsTime barLength;
	
	
	
	THMIDIFILE *Infile = Ptr;
	Ptr->setIniFilename(Inifilename);

	
	/*
	barLength = Frac2Duration( TimeSigNum,
	TimeSigDenom,
	Infile->Ppq());
	*/
	barLength = TFrac(TimeSigNum, TimeSigDenom);
	
	// Read pattern file
	Pattern = new TPFILE( Patternname );
	if( Pattern )
		Pattern->Read( /* Infile->Ppq() */);
	
	
	// allow all notefaces set in ReadMask;
	//	Infile->preProcess();
	// Quantize MIDI file
	Infile->Quantize( );
	
	// Convert into gmn file
	if ( !Infile->Convert( gmnOut,
						   NULL ))
		ErrorMsg( 4 );
	
	if( Pattern )
	{
		delete Pattern;
		Pattern = NULL;
	}
	
}  // Process



   /*
   Quantize AttackTImes of COntroll Events to Attackpoints of
   NoteEvents(tempTrack)
   remarks:
   -COntrol must be sorted
   -tempTrack must be sorted
*/
void quantizeToNotes(TQTRACK *ControlTrack,
					 TQTRACK *tempTrack)
{
	if(!ControlTrack ||
		!tempTrack)
		return;
	/*
	TAbsTime BestTime,
		//				distance,
		SecTime;
	
	TAbsTime relation;
	*/
	
	TQuantizedObject *currentCtrl = dynamic_cast<TQuantizedObject *>(ControlTrack->FirstObject(-1));
	
	TQNOTE	*prevNote = NULL;
	TQNOTE	*currentNote = dynamic_cast<TQNOTE *>(tempTrack->FirstNoteObject(-1));
	
	//--- process all control events
	while(currentCtrl)
	{
		
		// pre quantize ctrl time to 16th
		{
			TFrac absTime = currentCtrl->GetAbsTime();
			double fabsTime = absTime.toDouble();
			
			absTime = TFrac( (int)((fabsTime * 16)+0.5), 16);
			currentCtrl->SetAbsTime( absTime );
		}
		
		
		// search in tempTrack until preNote < currentCtrl <= currentNote
		while(currentNote &&
			currentCtrl->GetAbsTime() > currentNote->GetAbsTime() )
		{
			if(!prevNote ||
				(prevNote &&
				prevNote->GetAbsTime() < currentNote->GetAbsTime()) )
				prevNote = currentNote;
				currentNote = QNOTE(currentNote->GetNext(-1));
		}
		
		TFrac BestTime,
			  SecTime;
		// calc quantized Time
		if(prevNote &&
			currentCtrl->GetAbsTime() == prevNote->GetAbsTime() )
		{
			// shift to prev note
			BestTime = prevNote->qAttackpoint(&SecTime);
			currentCtrl->shiftQAbstimeTo(BestTime, SecTime, 0 );
		}
		else if( prevNote &&
			!currentNote &&
			currentCtrl->GetAbsTime() > 0L ) // currentCtrl > lastNote
		{
			// calculate abstime
			BestTime = prevNote->qAttackpoint(&SecTime);
			/*! Todo: shift control to end of last note ?
			relation = prevNote->GetAbsTime() /
			currentCtrl->GetAbsTime();
			if(relation > 0L )
			{
			BestTime /= relation;
			SecTime /= relation;
			}
			*/
			currentCtrl->shiftQAbstimeTo(BestTime, SecTime,0);
		}
		else if( currentNote &&
			currentCtrl->GetAbsTime() == currentNote->GetAbsTime() )
		{
			// shift to current note
			BestTime = currentNote->qAttackpoint(&SecTime);
			currentCtrl->shiftQAbstimeTo(BestTime, SecTime,0);
		}
		else if(currentNote &&
			currentCtrl->GetAbsTime() > 0L) // currentCtrl < currentNote
		{
			// calculate abstime
			BestTime = currentNote->qAttackpoint(&SecTime);
			/*
			TFrac relation = currentNote->GetAbsTime() /
								currentCtrl->GetAbsTime();
			*/
				/*!	Todo split note if needed?
				if(relation > 0L)
				{
				BestTime /= relation;
				SecTime /= relation;
				}
			*/
			currentCtrl->shiftQAbstimeTo(BestTime, SecTime,0);
		}
		currentCtrl = dynamic_cast<TQuantizedObject *>(currentCtrl->TMusicalObject::GetNext(-1));
	} // while 
	
}


// check max prime factor of denom1 and denom2
double primeDenomDistance( long int attackDenom,
							long int durDenom)
{
	#define pListSize 5
	long int pList[pListSize] = {2,3,5,7,11};
	int pMaxAttack = -1,
		pMaxDur = -1;
	
	if( attackDenom == durDenom ||
		attackDenom < 2 ||
		durDenom == 0 )
		return 0;


	
	// gat largest prime factor
	for( int i = 0; i < pListSize; i++ )
	{
		if( attackDenom % pList[i] == 0 )
			pMaxAttack = i;
		if( durDenom % pList[i] == 0 )
			pMaxDur = i; 
	}
	
	int pMaxExpAttack = 1;
	// gest exponent
	int p =  (int)pow(pList[pMaxAttack], (double)(pMaxExpAttack+1));
	while( !(attackDenom % p) )
	{
		pMaxExpAttack++;
		p = (int) pow(pList[pMaxAttack], (double)(pMaxExpAttack+1));
	}		
	
	if( pMaxAttack < 0 )
		return 0;
		
	p = (int)pow(pList[pMaxAttack], (double)pMaxExpAttack);
	if( attackDenom <= durDenom &&
	    !(durDenom % p)  )
	{
		return 0;
	}

	int pMaxExpDur = 1;
	// gest exponent
	p = (int)pow(pList[pMaxAttack], (double)(pMaxExpDur+1));
	while( !(durDenom % p))
	{
		pMaxExpDur++;
		p =  (int)pow(pList[pMaxAttack], (double)(pMaxExpDur+1));
	}		


	if( pMaxAttack == pMaxDur )
	{
		if( durDenom >= attackDenom ||
			durDenom == 0 ) // 1/12 :1/3,1/6,1/12 == ok
		{
			return 0;
		}
		else // 1/12 : n/24 -> error, 1/4 : n * 64
		{
			return 1.0 - (double)durDenom / (double)attackDenom;
		}
	}	
	return 1;	
}
