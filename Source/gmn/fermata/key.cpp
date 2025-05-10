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

/*--------------------------------------------------------------
	filename: key.cpp
	author:   Juergen Kilian
	date:     99/02/21-2001, 2011
	functions for key detection
----------------------------------------------------------------*/
#include "debug.h"

//----------------------
#define INTERACTIVE
#include "../lib_src/ini/ini.h"


#include "h_track.h"
#include "funcs.h"
#include "key.h"


int PRIMEWEIGHT;
int THIRDWEIGHT;
int FIFTHWEIGHT;

int TONICWEIGHT;
int SUBDOMINANTWEIGHT;
int DOMINANTWEIGHT;



/*
	Keydetection by net, based on paper by ____
	 init all values
	remarks:
		algorithm not used anymore
*/
TKeyDetermin::TKeyDetermin( TInifile *inifile )
{
	int i;

	TInifile *Inifile;
	Inifile = inifile;

	if( Inifile )
	{
		PRIMEWEIGHT = Inifile->GetValInt("PRIMEWEIGHT","6",NULL);
		THIRDWEIGHT = Inifile->GetValInt("THIRDWEIGHT","6",NULL);
		FIFTHWEIGHT = Inifile->GetValInt("FIFTHWEIGHT","7",NULL);

		TONICWEIGHT = Inifile->GetValInt("TONICWEIGHT","6",NULL);
		SUBDOMINANTWEIGHT = Inifile->GetValInt("SUBDOMINANTWEIGHT","6",NULL);
		DOMINANTWEIGHT = Inifile->GetValInt("DOMINANTWEIGHT","7",NULL);
	}

	if( !PRIMEWEIGHT )
		PRIMEWEIGHT = 6;
	if( !THIRDWEIGHT )
		THIRDWEIGHT = 6;
	if( !FIFTHWEIGHT )
		FIFTHWEIGHT = 6;
	if( !TONICWEIGHT )
		TONICWEIGHT = 6;
	if( !SUBDOMINANTWEIGHT )
		SUBDOMINANTWEIGHT = 6;
	if( !DOMINANTWEIGHT )
		DOMINANTWEIGHT = 6;

	NoteCount = 0;
	for( i =  0; i < MAXNOTES; i++ )
	{
		PitchCount[i] = 0;
	}
} // TKeyDetermin
//----------------------------------------
// add Pitchclasses to net
void TKeyDetermin::AddNotes( TNOTE *start,
									  TNOTE *end )
{
	int pitch;
	while( start &&
			 start != end )
	{
		pitch = start->GetMIDIPitch();
		if( pitch >= 0 ) // is this a note-event?
		{
			// normalize pitch
			pitch = pitch % MAXCHORDS;
			PitchCount[pitch]++;
		}
		start = NOTE(start->GetNext(start->GetVoice()));
	}
} // AddNotes
//----------------------------------------
/*
	res = pitchclass of key (0...11) = (c d& d e& ...
*/
int TKeyDetermin::Key( void )
{
	int i,
		 pitch = 0,
		 pitchCount;


	// init values
	for( i =  0; i < MAXKEYS; i++ )
	{
		MajorKey[i] = 0L;
		MinorKey[i] = 0L;
	}
	for( i =  0; i < MAXCHORDS; i++ )
	{
		MajorChord[i] = 0L;
		MinorChord[i] = 0L;
	}

	// calculate chord values
	for( i = 0; i < MAXNOTES; i++)
	{
		pitch = i;
		pitchCount = PitchCount[i];

		MajorChord[pitch] += (PRIMEWEIGHT*pitchCount);
		MinorChord[pitch] += (PRIMEWEIGHT*pitchCount);

		pitch = (pitch + 3) % MAXCHORDS;
		MinorChord[pitch] += (THIRDWEIGHT*pitchCount);

		pitch = (pitch + 1) % MAXCHORDS;
		MajorChord[pitch] += (THIRDWEIGHT*pitchCount);

		pitch = (pitch + 3) % MAXCHORDS;
		MinorChord[pitch] += (FIFTHWEIGHT*pitchCount);
		MajorChord[pitch] += (FIFTHWEIGHT*pitchCount);

	}
	// calculate key values
	int chord;
	long int chordCount;
	for( i = 0; i < MAXCHORDS; i++)
	{
		chord = i;
		chordCount = MajorChord[i];
		MajorKey[chord] += (TONICWEIGHT*chordCount);
		chordCount = MinorChord[i];
		MinorKey[chord] += (TONICWEIGHT*chordCount);

		chord = (chord + 5) % MAXKEYS;
		chordCount = MinorChord[i];
		MinorKey[chord] += (SUBDOMINANTWEIGHT*chordCount);
		chordCount = MajorChord[i];
		MajorKey[chord] += (SUBDOMINANTWEIGHT*chordCount);

		pitch = (pitch + 2) % MAXCHORDS;
		chordCount = MajorChord[i]; // !!! Major
		MinorKey[chord] += (DOMINANTWEIGHT*chordCount);
		chordCount = MajorChord[i];
		MajorKey[chord] += (DOMINANTWEIGHT*chordCount);
	}

	// find Maximum
	int MaxPosMajor = -1,
		 MaxPosMinor = -1;
	long int MaxKey;

	MaxKey = 0;
	for( i = 0; i < MAXKEYS; i++ )
	{
		if( MajorKey[i] > MaxKey )
		{
			MaxPosMajor = i;
			MaxKey = MajorKey[i];
			MaxPosMinor = -1;
		}
		if( MinorKey[i] > MaxKey )
		{
			MaxPosMinor = i;
			MaxKey = MinorKey[i];
			MaxPosMajor = -1;
		}
	}

	if( MaxPosMajor >= 0 )
		return MaxPosMajor;
	if( MaxPosMinor >= 0 )
		return MaxPosMinor+12;

	return -1; // error

}
//----------------------------------------
void TKeyDetermin::Printf( FILE *out )
{

	int i;
	fprintf(out,"\nWeights: prime:%d, third:%d, fifth:%d\n",
					PRIMEWEIGHT,
					THIRDWEIGHT,
					FIFTHWEIGHT );
	fprintf(out, "tonic:%d, subdom:%d, dom:%d\n",
			  TONICWEIGHT,
			  SUBDOMINANTWEIGHT,
			  DOMINANTWEIGHT );

	fprintf(out,"\nPitchClasses: ");
	for( i=0; i < MAXNOTES; i++ )
	{
		fprintf(out," %d,", PitchCount[i]);
	}
	fprintf(out,"\nMajorChords: ");
	for( i=0; i < MAXCHORDS; i++ )
	{
		fprintf(out," %ld,", MajorChord[i]);
	}
	fprintf(out,"\nMinorChords: ");
	for( i=0; i < MAXCHORDS; i++ )
	{
		fprintf(out," %ld,", MinorChord[i]);
	}
	fprintf(out,"\nMajorKeys: ");
	for( i=0; i < MAXKEYS; i++ )
	{
		fprintf(out," %ld,", MajorKey[i]);
	}
	fprintf(out,"\nMinorKeys: ");
	for( i=0; i < MAXKEYS; i++ )
	{
		fprintf(out," %ld,", MinorKey[i]);
	}
	fprintf(out, "\n");
}

//-----------------------------------------------------------
/*
	key detection by counting semitone steps and
	inferring corresponding key signature
	based on ideas from discussion with Keith 1999
	 init all values
*/
TKeyDetermin2::TKeyDetermin2( void )
{
	int i;
	for( i =  0; i < MAXNOTES; i++ )
	{
		SemiStepsUp[i] = 0;
	}
} // TKeyDetermin2
//----------------------------------------
/* 
	Add semitone steps in range [start,end) to list
*/
void TKeyDetermin2::AddSemiSteps( TNOTE *start,
									  TNOTE *end )
{
	int pitch;

	TNOTE *nextNote;
	while( start &&
			 start != end )
	{
		pitch = start->GetMIDIPitch();
		nextNote = NOTE(start->GetNext(start->GetVoice()));

		// search for next real note
/*
		while( nextNote
		&&		 nextNote->GetPitch() < )
		{
			nextNote = netxNote-GetNext(1);
		}
*/
		if( nextNote &&
			  pitch > 0 ) // is this a note-event?
		{
			// check for semitone step
			if( nextNote->GetMIDIPitch() - pitch == 1 )
			// step up
			{
				SemiStepsUp[pitch%12]++;
			}
			else if( nextNote->GetMIDIPitch() - pitch == -1 )
			// step down
			{
				SemiStepsUp[(nextNote->GetMIDIPitch())%12]++;
			}
		}
		start = nextNote;
	}
} // AddNotes
//----------------------------------------
/*
	res = pitchclass of key Major:(0...11) Minor(12..23)
*/
int nAccidentals( int key ) 
{
	int res = -8;
	// check for minor key
	if( key > 11 )
	{
		key -= 9; // 12 -> c_min -> e&_maj -> 3
	}
	// convert pitchclass into # of accidentals
	switch( key )
	{
		case -1 : res = -8; break; // invalid
		case 0 : res = 0; break;
		case 1 : res = -5; break; //c#_>d&
		case 2 : res = 2;break; //d
		case 3 : res = -3;break; // e&
		case 4 : res = 4;break; // e
		case 5 : res = -1;break; // f
		case 6 : res = 6;break; // f#
		case 7 : res = 1;break; // g
		case 8 : res = -4;break; // a&
		case 9 : res = 3;break; // a
		case 10 :  res = -2;break; // b&
		case 11 : res = 5;break;  // h
	}
	return res;



}

/*
	res = pitchclass of key Major:(0...11) Minor(12..23)
*/
int TKeyDetermin2::Key( void )
{


	int j;

	// init values
	for( j =  0; j < MAXKEYS; j++ )
	{
		MajorKey[j] = 0L;
		MinorKey[j] = 0L;
	}

	// count semitones of for key[j]
	for( j = 0; j < MAXNOTES; j++ )
	{
		MajorKey[j] = SemiStepsUp[(j+11)%12]+(SemiStepsUp[(j+4)%12]);
		MinorKey[j] = SemiStepsUp[(j+2)%12]+(SemiStepsUp[(j+7)%12]);
	}


	// find Maximum
	int MaxPosMajor = -1,
		 MaxPosMinor = -1;

	long int MaxKey;

	MaxKey = 0;
	int i;
	for( i = 0; i < MAXKEYS; i++ )
	{
		if( MajorKey[i] > MaxKey ||
			(MajorKey[i] == MaxKey &&		// same value
			 abs(nAccidentals(i)) < abs(nAccidentals(MaxPosMajor)) )) // less accidentals
		{
			MaxPosMajor = i;
			MaxKey = MajorKey[i];
			MaxPosMinor = -1;
		}
		if( MinorKey[i] > MaxKey )
		{
			MaxPosMinor = i;
			MaxKey = MinorKey[i];
			MaxPosMajor = -1;
		}
	}

	if( MaxPosMajor >= 0 )
		return MaxPosMajor;
	if( MaxPosMinor >= 0 )
		return MaxPosMinor+12;

	return -1; // error

}
//----------------------------------------
void TKeyDetermin2::Printf( FILE *out )
{

	int i;

	if( !out )
   	return;
	fprintf(out,"\nSemiStepsUp: ");
	for( i=0; i < MAXNOTES; i++ )
	{
		fprintf(out," %d,", SemiStepsUp[i]);
	}
	fprintf(out,"\nMajorKeys: ");
	for( i=0; i < MAXKEYS; i++ )
	{
		fprintf(out," %ld,", MajorKey[i]);
	}
	fprintf(out,"\nMinorKeys: ");
	for( i=0; i < MAXKEYS; i++ )
	{
		fprintf(out," %ld,", MinorKey[i]);
	}
	fprintf(out, "\n");
}


/*
	result = # of accidentals (-7...+7)
*/
int DetectKey( THMIDIFILE *file )
{
//todo implement segmented DetectKey, infer key changes
	if( !file )
		return 0;

	int res;
	/*
	TKeyDetermin *KeyDetermin;
	KeyDetermin = new TKeyDetermin();
	 */
	TKeyDetermin2 *KeyDetermin;
	KeyDetermin = new TKeyDetermin2();


	THTRACK *temp;
	Printf(" Keydetection ... ");
	temp = (THTRACK*)file->FirstTrack();
	int i;
	while( temp )
	{

		for(i = 0; i < temp->GetCVoice();i++)
		{
			KeyDetermin->AddSemiSteps( temp->FirstNoteObject(i), NULL );
		}
		temp = (THTRACK*)temp->Next();
	}
	Printf("done ");
	res = KeyDetermin->Key();
	printf("key=%d ",res); 
	res = nAccidentals( res );
	printf("accs=%d\n",res); 
#ifdef _DEBUG
	FILE *out;
	out = fopen( "_keys.txt","wt");
	KeyDetermin->Printf(out);
	fclose(out);
//	Printf("end<keydetection>\n");
#endif
	delete KeyDetermin;

	/*
	// check for minor key
	if( res > 11 )
	{
		res -= 9; // 12 -> c_min -> e&_maj -> 3
	}
	// convert pitchclass into # of accidentals
	switch(res )
	{
		case -1 : res = -8; break; // invalid
		case 0 : res = 0; break;
		case 1 : res = -5; break; //c#_>d&
		case 2 : res = 2;break; //d
		case 3 : res = -3;break; // e&
		case 4 : res = 4;break; // e
		case 5 : res = -1;break; // f
		case 6 : res = 6;break; // f#
		case 7 : res = 1;break; // g
		case 8 : res = -4;break; // a&
		case 9 : res = 3;break; // a
		case 10 :  res = -2;break; // b&
		case 11 : res = 5;break;  // h
	}
	*/
	return res;
}

