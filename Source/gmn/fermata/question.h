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

#if !defined( __question_h__ )
#define __question_h__

#include "defs.h"


#define Q_YES 1
#define Q_NO 0
#define Q_UNDECIDED -1

// global variabels for questions
// extern int TripletYesNoQuestion;
// extern int QErrorCounter;

int askForTriplet( void );
void initQuestions( void );
int CheckTriplets( TFrac *qAttacks,
					char *valid,
					int cValues,
					TFrac *delta,
					double *prob,
				    TAbsTime time);

char tripletPos( int pos );
int CheckQuantizeAgain( void );

#endif
