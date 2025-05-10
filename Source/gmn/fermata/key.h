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

#if !defined(__key_h__)
#define __key_h__
/*!
   tools classes for key detection
   Juergen Kilian
*/

#include <stdio.h>
#include "note.h"
#include "h_midi.h"
//-----------------------------------------------------------

#define MAXNOTES 12
#define MAXKEYS 12
#define MAXCHORDS 12

/*

#define PRIMEWEIGHT  6
#define THIRDWEIGHT  5
#define FIFTHWEIGHT  5

#define TONICWEIGHT 5
#define SUBDOMINANTWEIGHT 6
#define DOMINANTWEIGHT 4

*/
/// return #accidenatls for key signature, key == pitchclasse 0..11 -> major, 12...23 == minor
int nAccidentals( int key ); 

/// tools class for key net based detection, not used anymore
class TKeyDetermin
{
	int PitchCount[MAXNOTES];
	int NoteCount;

	long int MajorKey[MAXKEYS];
	long int MinorKey[MAXKEYS];

	long int MajorChord[MAXCHORDS];
	long int MinorChord[MAXCHORDS];

public:
	TKeyDetermin( TInifile *inifile );
	void AddNotes( TNOTE *start, TNOTE *end );
	int Key( void );
	void Printf( FILE *out );
};

/// tools class for key net based detection, not used anymore

class TKeyDetermin2
{
	int SemiStepsUp[MAXNOTES];
	long int MajorKey[MAXKEYS];
	long int MinorKey[MAXKEYS];

public:
	TKeyDetermin2( void );
	void AddSemiSteps( TNOTE *start, TNOTE *end );
	int Key( void );
	void Printf( FILE *out );
};

/*! res = #of accidentals */
int DetectKey( THMIDIFILE *file );


#endif
