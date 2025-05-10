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

/*
-----------------------------------------------------------------
	Functions for autocorrelation, can be used 
	for inferring the time-signature.
	Based on work of Judy Brown
	Author:   Juergen Kilian
	Date:	  1998-2001, , 2011

------------------------------------------------------------------
*/

#include "funcs.h"
#include "q_note.h"
#include "midi.h"
#include "correl.h"
#include "timesig.h"
#include "statist.h"

#include "../lib_src/ini/ini.h"
// limits for recognition of barLength
#define MIN_RELATION 0.9
#define MAX_RELATION 1.1

#ifdef _DEBUG
#define _DEBUG_AUTOCORREL
#endif


// -------------------- ms time versions ------------------------------------
 

/// meter sig auto correlation for an array of times signatures, res = array of peaks
double *autoCorrelationMS( TQNOTE *start,	// start note
						 /// -1 == all voices
						 int voice,
						  double &integrSize,
						  double &maxBarlength,
						  double &resolution,
						  int *arraySize,
						  TMIDIFILE *theMidifile)
{
	
	/// arry size of result
	if( *arraySize <= 0 )
		*arraySize = maxBarlength / resolution;
		
	double *probs = new double[*arraySize];

	/// evaluate all possible bar lengths
	double barLength = resolution;
	int i;
	for( i = 0; i < *arraySize; i++ )
	{
		// do overlapped match windows!
		double epsilon = resolution/2;
		probs[i] = autoCorrelationMS( start,
									  voice,
									  integrSize,
									  barLength,
									  epsilon,
									  theMidifile);
		barLength += resolution;
	}
	return probs;
}


/// meter sig auto correlation for a specified times signature, res = p(timeSignature)
double autoCorrelationMS( TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						  double &integrSize,
						  double &barLength,
						  double &epsilon,
						  TMIDIFILE *theMidifile )
{
	if( !start )
	{
		Printf( "NULL pointer at autoCorrelation!\n");
		return 0;
	}

	double latestAttack = start->AbsTimeMS(theMidifile) + integrSize;
	double res = 0;
	TQNOTE *now = start;
	while( now &&
		   now->AbsTimeMS(theMidifile) <= latestAttack)
	{
		// add weight of best note in close to barlength distance
		for( int i = 1; i < 4; i++ )
		{
			double eps = epsilon; //i*epsilon;
			double bl = i * barLength;
			res += autoCorrelationSingleMS( now,
										    voice,
									  		bl,
									  		eps,
									  		theMidifile);
		}	
		now = QNOTE(now->GetNext(voice));		
	}
	return res;
}


/// auto correlation for a single note and a  barlength/distance
double autoCorrelationSingleMS( TQNOTE *start,
							 /// -1 == all voices
							 int voice,
							  double barLength,
							  double &epsilon,
							  TMIDIFILE *theMidifile)
{
	if( !start )
	{
		Printf( "NULL pointer at autoCorrelation!\n");
		return 0;
	}
	double nowTime = start->AbsTimeMS(theMidifile);

	double curWeight = getWeight(start, theMidifile);
	double res = 0;
	TQNOTE *next = QNOTE(start->GetNext(voice));
	// search for "best" note in (barlength - matchWindow , barlength+matchWindow);
	while( next )
	{

		// compare if distance now-next == barLength
		double distance = next->AbsTimeMS(theMidifile) - nowTime;
        double windowEnd = barLength + epsilon;
        double windowStart = barLength - epsilon;
        if( // distance == barLength ) // match
			(distance < windowEnd) &&
			(distance > windowStart) )
		{
			double val = (curWeight * getWeight(next, theMidifile));
			// use counter for Weights
			// take best note inside search window 
			if( val > res )
				res = val;
			// don't stop loop, search for maxval in window
			next = QNOTE(next->GetNext(next->GetVoice()));
		}
		else if( distance > barLength ) // outside window go to next note
		{
				next = NULL; // stop loop
		}
		else 
			next = QNOTE(next->GetNext(voice));
	}
	return res;
}


// -------------------- score time versions ------------------------------------

// calculate weights for all barLengths[] and Weights[]
// result must be deleted!
double *autoCorrelation( TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						 int size,			// #barLengths
						 TTimeSignature *meterSigs,   // test all barLengths
						 TFrac integrSize,
						  TMIDIFILE *theMidifile)	// normalizer for weights
{
	TFrac epsilon  = TFrac(theMidifile->getInifile()->GetValChar("MATCHWINDOW","1/24","   [fraction] epsilon size for meter detection autocorrelation"));
	double *probs = new double[size];
	// retrieve a quality/probability for each time signature
	for(int i = 0; i < size; i++ )
	{
#ifdef _DEBUG_AUTOCORREL
		FILE *out = fopen("_metersigs.txt","at");
		fprintf(out,"---------- %ld/%ld -------------\n",
					meterSigs[i].numeratorI,
					meterSigs[i].denominatorI);
		fclose(out);
#endif
		probs[i] = autoCorrelation( start,
									 /// -1 == all voices
									voice,
									meterSigs[i],
									integrSize,
									epsilon,
									theMidifile);
	}
	return probs;
}

/*!
	return autocorrelation value for *start
		   search inside a window
*/
double autoCorrelationSingle( TQNOTE *start,
							 /// -1 == all voices
							 int voice,
							 TFrac barLength,
							 TFrac &epsilon,
							 int nTimes,
							 TMIDIFILE *theMidifile )
{
#ifdef _DEBUG_AUTOCORREL
	FILE *out = fopen("_metersigs.txt","at");
#endif
	if( !start )
	{
		Printf( "NULL pointer at autoCorrelation!\n");
		return 0;
	}
	TFrac nowTime = start->GetAbsTime();

	TQNOTE *now = start;
	double curWeight = getWeight(now,theMidifile);
	double res = 0;
	TQNOTE *frameStart = QNOTE(now->GetNext(voice));
	int i = 1;
	int countHits = 0;
#ifdef _DEBUG_AUTOCORREL
	fprintf(out,"--- ");
	stringstream sOut;
	start->WritePitch(sOut);
	fprintf(out, sOut.str().c_str());
	fprintf(out," %ld/%ld ", nowTime.numerator(), nowTime.denominator() );
	fprintf(out, "%f\n", curWeight);

#endif
	while( nTimes )
	{
	    TFrac windowEnd = (barLength * i) + epsilon;
	    TFrac windowStart = (barLength * i) - epsilon;
	    double curBest = 0;
	    char end = 0;
	    TQNOTE *next = frameStart;
	    frameStart = NULL;
		while( next  && !end)
		{
			// compare if distance now-next == barLength
			TFrac distance = next->GetAbsTime() - nowTime;
	        if( // distance == barLength ) // match
				(distance < windowEnd) &&
				(distance > windowStart) )
			{
				// next is inside match window
				double val = getWeight(next, theMidifile);
				// use counter for Weights
				// get best weight in woindow
				if( val > curBest )
				{
					curBest = val;
					countHits++;
				}
				// start search for next frame at begnning of current frame
				// some notes inside searchwindow might belong to 
				// several frames.
				if( !frameStart )
					frameStart = next;
				// don't stop loop, search for maxval in window
				next = QNOTE(next->GetNext(voice));
			}
			else if( distance > windowEnd ) // outside window go to next note
			{
				end = 1;
			}
			else
			{ 
				next = QNOTE(next->GetNext(voice));
			}
		} // while next
#ifdef _DEBUG_AUTOCORREL
			if( curBest > 0 )
			{
				fprintf(out, " %f\n", curBest);
			}
#endif
			res += curBest * curWeight;

		nTimes--;
		i++;
	} // while nTimes
	// for autocorrelation this is not correct
	/*
	if( countHits )
		res /= (double)countHits;
	*/
#ifdef _DEBUG_AUTOCORREL
	fprintf(out, "%f-----\n", res);
	fclose(out);
#endif
	return res;
}


// calculate weight for one barLength
double autoCorrelation( TQNOTE *start,
						 /// -1 == all voices
						 int voice,
						/// current time signature
						TTimeSignature meterSig,
						TFrac integrSize, // position of latest start note
						TFrac &epsilon,
						  TMIDIFILE *theMidifile)	// normalize weights
{
	TFrac barLength(meterSig.numeratorI,
					meterSig.denominatorI);

	if( !start )
	{
		Printf( "NULL pointer at autoCorrelation!\n");
		return 0;
	}

	TFrac latestAttack = start->GetAbsTime() + integrSize;
	double res = 0;
	TQNOTE *now = start;
	while( now &&
		   now->GetAbsTime() <= latestAttack)
	{
		/// get best match in barlength+/-epsilon
		res += autoCorrelationSingle( now,
									  voice,	
									  barLength,
									  epsilon,
									  1, /*single note processing*/
									  theMidifile);
		now = QNOTE(now->GetNext(voice));		
	}
	return res;
}

/// calculate a weight for note, used for autocorrelation
double getWeight( TQNOTE *note, TMIDIFILE *theMidifile )
{
	double res = 0;
	if( note ) // use only duration
	{
		TNOTE *next = NOTE(note->GetNext(note->GetVoice()));
		double durMS = note->durationMS(theMidifile);
		if( next )
		{
			double ioiMS = next->AbsTimeMS(theMidifile) - note->AbsTimeMS(theMidifile);
			/// for clicknotes duration > ioi!
			if( ioiMS > durMS )
				durMS = (ioiMS + durMS) / 2.0;
		}
		res =  1 - GaussWindow(durMS, 
							   500); 							   
	}
	else
	{
		Printf( "NULL pointer at getWeight!\n");
	}
	return res;
}




/*
	return beatposition with max autocorrelation weight for barlength
*/
TNOTE *phase( TNOTE *start,
			  int voice,
			 TFrac barLength,
			 /// result
			 TFrac &phase,
			 TFrac &epsilon,
		     TMIDIFILE *theMidifile)

{
	if(!start )
		return NULL;

	TNOTE *from = start;
	TNOTE *next = QNOTE(from->GetNext(voice));

	TFrac nowTime = from->GetAbsTime();
	TFrac endTime = nowTime + barLength;
	double maxWeight = -1;
    TFrac pos(0,1);
	while( from &&
		   nowTime < endTime)
	{
		nowTime = from->GetAbsTime();
		double weight = autoCorrelationSingle(QNOTE(from),
											   voice,
									  		  barLength,
									  			epsilon,
									  			16,
									  			theMidifile
									  			);

		if( weight > maxWeight )
		{
			maxWeight = weight;
			pos = from->qAttackpoint() - start->qAttackpoint();
		}
		from = QNOTE(from->GetNext(voice));
	} // while
	phase = pos;
	return next;
}
