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

/*-----------------------------------------------------------------
	filename: question.cpp
	author:   Juergen Kilian
	date:     1998-2001,2011
	global question/user input functions 
	can be changed for platform dependent changes
------------------------------------------------------------------*/
#include <string>
using namespace std;
#include <string.h>

#include "question.h"
#include "import.h"
#include "q_funcs.h"
#include "funcs.h"

#define QErrorLimit 10


int TripletYesNoQuestion = Q_UNDECIDED; // undecided
int QErrorCounter = 0;

void initQuestions( void )
{
	TripletYesNoQuestion = Q_UNDECIDED; // undecided
	QErrorCounter = 0;
}
int CheckQuantizeAgain( void )
/*
	result:
		Q_YES -> Quantize again
		Q_NO  -> resume
*/
{
	const char *answer;
	if( QErrorCounter > QErrorLimit )
	{
		answer = YesNoQuestion( "Many quantize errors! Settings may be wrong. Quantize again?", "n", NULL );
		if( !strcmp(answer, JK_YES) )
			return Q_YES;
	}
	return Q_NO;
}

int askForTriplet( void )
/*
	result:
		TripletYesNoQuestion: Q_Yes, Q_NO, Q_UNDECIDED
*/
{
	const char *answer;
	answer = YesNoQuestion( "Does this piece contain triplets?", "n", NULL );
	if( !strcmp(answer,"yes") )
		TripletYesNoQuestion = Q_YES;
	else if( !strcmp(answer,"no") )
		TripletYesNoQuestion = Q_NO;
	else
		TripletYesNoQuestion = Q_UNDECIDED;
	return TripletYesNoQuestion;
}

/*
	CHeck triplet positions  for attackpoints
	result:
		0 : no changes to maske_s
		1 : maske_s has been changed
*/
int CheckTriplets( TFrac *qAttacks,
					char *valid,
					int cValues,
					TFrac *delta,
					double *prob,
				    TAbsTime time)
{
	int res = 0;
	int pos = 0;

	int tripletOn = 0;

	// Triplet Question already answered
	if( TripletYesNoQuestion == Q_YES )
		return 0;

	// find best and sec best
	for( pos = 0; pos < cValues; pos++ )
	{
		// triplet pos?
		if( qAttacks[pos].nonBinary() &&
			 valid[pos] )
			tripletOn++;
	}


	int bestPos = -1,
		 secondPos = -1;
	if( tripletOn )
	{
		if( TripletYesNoQuestion == Q_NO )
		{
			for( int i = 0; i < cValues; i ++ )
			{
				if( qAttacks[pos].nonBinary() )
				{
					valid[i] = 0;
					qAttacks[i] = 0L;
				}
			}
			res = 1;
		}
		else // ask user, TripletYesNo == Q_UNDECIDED
		{
			getBestNSecond( delta,
							 prob,
							 bestPos,
							 secondPos,
							 time,
							 cValues,
							 valid);
			if( (qAttacks[bestPos].nonBinary() ) ||
				(qAttacks[secondPos].nonBinary()) )
			{
				// ask Question
				if( askForTriplet() == Q_NO )
				{
					// remove TripletPso from maske_s
					for( int i = 0; i < cValues; i++ )
					{
						if( qAttacks[i].nonBinary() )
						{
							valid[i] = 0;
							qAttacks[i] = 0L;
						}
					}
					res = 1;
				}
			}	// if tripletPos
		}	// if TripletQuestion
	} // if triplet on
	return res;
}

