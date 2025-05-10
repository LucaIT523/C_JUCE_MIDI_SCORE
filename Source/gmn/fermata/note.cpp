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
|	filename : NOTE.CPP
|	author     : Juergen Kilian
|	date	  : 17.10.1996-98/06-2004
|	implementation of TNOTE, TOrnament, ...
------------------------------------------------------------------*/
#include <string>
using namespace std;
#include <sstream>
#include <iostream>

#include "note.h"

#include "funcs.h"
//#include "q_note.h"
#undef UNUSED

#include "../leanguido/lgnote.h"
#include "q_chord.h"
#include "track.h" // for keyPitch etv
//----------------------------------------------------------------------
//----------------------------------------------------------------------

#include "midi.h"

// extern pkeyscale *allKeys;

TNOTE *  NOTE(TMusicalObject *s)
{
	return dynamic_cast<TNOTE *>(s);
}

const char *scale[7]={"c","d","e","f","g","a","b"};// diatonic scale

/*! compare of two TNOTE
1. criteria increasing Abstime
 2. criteria decreasing pitch
 result :
	1 : n1 >= n2
	0 : else
 */
int operator > ( TNOTE &n1,  TNOTE &n2 )

{
	// 1. criteria AbsTime
	if ( n1.GetAbsTime() > n2.GetAbsTime() )
		return (int)1;
	// 2. criteria pitch
	else if( ( n1.GetAbsTime() == n2.GetAbsTime() ) &&
				( n1.GetMIDIPitch() < n2.GetMIDIPitch() ) )
		return (int)1;
	return 0;	// n1 <= n2

}

/*! compare of two TNOTE
 1. criteria increasing Abstime
 2. criteria decreasing pitch
 result :
	1 : n1 >= n2
	0 : else
*/
int operator >= (  TNOTE &n1,  TNOTE &n2 )
{
	// 1. criteria AbsTime
	if ( n1.GetAbsTime() > n2.GetAbsTime() )
		return 1;
	// 2. criteria pitch
	else if( ( n1.GetAbsTime() == n2.GetAbsTime() ) &&
				( n1.GetMIDIPitch() <= n2.GetMIDIPitch() ) )
		return 1;
	return 0;	// n1 <= n2

} // operator >


/*!
	change chromatic pitch into diatonic pitch
 */
void TNOTE::SetPitch( int scalePitch, 
					  int accidentals, 
					  int octave )
{
	mPitch       = scalePitch;
	mAccidentals = accidentals;
	mOctave      = octave;

	if( mPitch > 6 )
	{
		mPitch -= 7;
		mOctave++;
	}
	else if( mPitch < 0  )
	{
		mPitch += 7;
		mOctave--;
	}
}

unsigned char TNOTE::GetMIDIPitch( void )
{
	unsigned char res = ChromaticPitch();
	char tOctave = mOctave + 4;
	tOctave *= 12;

	res += tOctave;
	return res;
}

/*!
	get chromatic pitch of TNOTE
*/
char TNOTE::ChromaticPitch( void )
{
//                     c d e f g a b
	int semisteps[7] = {0,2,4,5,7,9,11};
	if( mPitch < 0 || mPitch > 6 )
	{
		std::cerr << "illegal pitch " << mPitch << " in TNOTE::ChromaticPitch\n" ;
	}
	// TODO check this function!!
	char res = semisteps[mPitch%7];
	res += mAccidentals;
	return res;
}
char TNOTE::Octave( void )
{
	return mOctave;
}

/*!
	chromatic midipitch will be converted and stored as
	diatonic pitch
*/
void TNOTE::SetPitch( unsigned char midipitch )
{
			// calc pitch and octave
/*	C-Major       c  c# d e&  e  f  f# g  g#  a b&  h
					  0  0+ 1 2-  2  3  3+ 4  4+  5 6-  6
*/

		  int pitchclass    = ((midipitch & 0x7F) % 12 );
// translat into 7 classes + accidentals
		  int tMatrix[12][2] = {{0,0},   // 0  c
								  {1,-1},		// 1 d&
								  {1,0},       // 2 d
								  {2,-1},		// 3 e&
								  {2,0},			// 4 e
								  {3,0},			// 5 f
								  {3,1},			// 6 f#
								  {4,0},			// 7 g
								  {4,1},			// 8 g#
								  {5,0},			// 9 a
								  {6,-1},		// 10 b&
								  {6,0}};		// 11 b

		  mPitch = tMatrix[pitchclass][0];
		  mAccidentals = tMatrix[pitchclass][1];
		  mOctave = ((midipitch & 0x7F) / 12 );
		  mOctave = mOctave - 4;
}

void TNOTE::WritePitch( ostream &out )
{
	/*
			// calc pitch and octave
		  pitch    = ((Pitch & 0x7F) % 12 );
		  oktave = ((Pitch & 0x7F) / 12 );
		  oktave = oktave - 4;
//		  itoa(oktave, okt, 10);
*/
//		  strcpy( str, scale[PitchI] );
		  if( mPitch < 7  &&
			  mPitch > -1 )
			out <<  scale[mPitch];
		  else
		  {
			  
			  out << "(* illegal pitch "<< mPitch << "*)c";
		  }

//		  strcat( str, okt );
		  int i;
		  for( i = 0; i < mAccidentals; i++ )
			  out << "#";
		  for( i = 0; i > mAccidentals; i-- )
			  out << "&";

		  out << (int)mOctave;
}

void TNOTE::Debug( ostream &out)
{
	
	AbsTime.Write(out);

	WritePitch( out );
	out << "*";
	mDuration.Write(out);
	if(mDuration == 0L)
	{
   	Printf("TNOTE::DEBUG Duration==0\n");
	}
	out << "sus "<< sustainOffset << ":";
	out << "V: "<< Voice <<"  ";
//	TQuantizedObject::Debug(out);
	out << "\n";
}
//----------------------------------------------------------------------
TNOTE::TNOTE( 	TAbsTime abstime,			// Attackpoint
					TAbsTime duration,
					unsigned char midipitch,
					double intens ) : TQuantizedObject(abstime)
{
//	AbsTime  = abstime;
	createTag = NULL;
	sustainOffset = 0;
	mDuration = duration;
	SetPitch( midipitch );
//	Pitch     = pitch;

	mIntens  = intens;

//	NextPtr      	 = NULL;
//	Prev    	    = NULL;

//	TempoVal     = 0;	// no Tempo spezified
	Voice     = 1;	// Default to voice 1

//   MetaType = 0;
} // TNOTE
//----------------------------------------------------------------------
/*!
 SetAll
 set all note-values
 */
void TNOTE::SetAll( TAbsTime abstime,		// Attackpoint
						  TAbsTime duration,
						  unsigned char midipitch,
						  double intens,
						  int id)
{
	mId = id;
	AbsTime   = abstime;
	mDuration  = duration;
//	Pitch      = pitch;
	SetPitch( midipitch );
	mIntens    = intens;
} // SetAll
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*!
	result : lgDuration
 */
 TAbsTime TNOTE::SetDuration( TAbsTime duration )
{
	mDuration  = duration;
/*
	if( Duration <= 0L )
	{
		Printf("WARNING: Set lgDuration <= 0!\n");
		return Duration;
	}
*/
	return mDuration;
} // SetDuration
//----------------------------------------------------------------------
TMusicalObject *TNOTE::GetNext( int voice )
{
	TMusicalObject *temp = TMusicalObject::GetNext(voice);
	TNOTE *tnote = dynamic_cast<TNOTE *>(temp);
	while(temp &&
       !tnote )
//       strcmp(temp->ClassName(), NOTE_CLASS) )
	{
		temp = temp->TMusicalObject::GetNext(voice);
		tnote = dynamic_cast<TNOTE *>(temp);
	}
	return 	tnote;
}


/*!
	 return distance between now-Attack and next attack, == IOI
	 if no next note exists -> the duration of the note will be returned instead
*/
/*
TAbsTime TNOTE::Dist2Next( void )
{
	TFrac res;
	TNOTE *temp;
	temp = NOTE(GetNext(GetVoice()));
	if( temp )
	{
		res = temp->GetAbsTime() - GetAbsTime();
	}
	else
	{
		res = GetDuration();
	}
	return res;
}
*/
TMusicalObject *TNOTE::GetPrev( int voice )
{
	TMusicalObject *temp = TMusicalObject::GetPrev(voice);
	while(temp &&
       !dynamic_cast<TNOTE *>(temp) )
	{
		temp = temp->TMusicalObject::GetPrev(voice);
	}
	return dynamic_cast<TNOTE *>(temp);
}





TFrac TNOTE::offset()
{
	return GetAbsTime() + GetDuration();
}

/*!
	 return distance between now-Attack and prev attack, == IOI
	 if no prev note exists -> -1 will be returned
*/
/*
TAbsTime TNOTE::Dist2Prev()
{

	TFrac res;
	TNOTE *temp;
	temp = NOTE(GetPrev(GetVoice()));
	if( temp )
	{
		res = GetAbsTime() - temp->GetAbsTime();
	}
	else
	{
		res = -1L;
	}
	return res;
}
*/



TNOTE::TNOTE(const TNOTE &ptr) : TQuantizedObject( ptr.AbsTime )
{
	mAccidentals = ptr.mAccidentals;
	mDuration = ptr.mDuration;
	mIntens = ptr.mIntens;
	mOctave = ptr.mOctave;
	mPitch = ptr.mPitch;

	diffTimes[0] = ptr.diffTimes[0];
	diffTimes[1] = ptr.diffTimes[1];
	Voice = ptr.Voice;
	atSelection = ptr.atSelection;
	
	createTag = ptr.createTag;

}
void TNOTE::fill(lgNote *note)
{
	if( !note )
		return;

	AbsTime = TFrac(note->pos().durN,
					 note->pos().durD );
	
	mAccidentals = note->accidentals();
	mDuration = TFrac( note->duration().durN, 
					  note->duration().durD );
	/// todo: read note inensity from sequence!
	mIntens = 0.5; // default
	mOctave = note->octave();
	int pitchClass = note->pitchClass();
	switch( pitchClass )
	{
	case 0 : 
		mPitch = 0;
		break;
	case 1 : // cis
		mPitch = 0;
		mAccidentals++;
		break;
	case 2 : 
		mPitch = 1;
		break;
	case 3 : // dis
		mPitch = 1;
		mAccidentals++;
		break;
	case 4 : 
		mPitch = 2;
		break;
	case 5 : 
		mPitch = 3;
		break;
	case 6 : // fis
		mPitch = 3;
		mAccidentals++;
		break;
	case 7 : 
		mPitch = 4;
		break;
	case 8 : // gis
		mPitch = 4;
		mAccidentals++;
		break;
	case 9 : 
		mPitch = 5;
		break;
	case 10 : // ais
		mPitch = 5;
		mAccidentals++;
		break;
	case 11 : 
		mPitch = 6;
		break;
	} // switch
}

TAbsTime TNOTE::getOffset()
{
	return GetAbsTime() + GetDuration();
}

double TNOTE::durationMS( TMIDIFILE *theMidifile )
{
	double res;
	if( theMidifile->tickTiming &&
	    theMidifile->RecPPQ)
	{
		res = ::DTIMEtoMS(theMidifile->RecPPQ,
					theMidifile->RecTempo,
					mDuration.toDouble() );
	
	}
	else
	{
		res = ::DTIMEtoMS(100,
					theMidifile->Tempo(&AbsTime),
					mDuration.toDouble() * 400 );
	}
	return res;	
}

void TNOTE::setIntens(double vel)
{
	mIntens = vel;
}

int TNOTE::minInterval(TNOTE *ptr)
{
	if( !ptr )
		return 0;
	int x = 0;
	TQChord  *chord = dynamic_cast<TQChord *>(ptr);
	if( chord )
	{
		// this can not be a chord if this function is called!
		x = - chord->minInterval( this );
	}
	else
	{
		x =  ptr->GetMIDIPitch() -  GetMIDIPitch();  
	}
	return x;
}


void TNOTE::pitchSpelling(TMIDIFILE *midifile)
{	
		/*
		key -7  -6 -5  -4 -3  -2 -1 0 1  2  3  4  5  6  7
		C&  G& D&  A& E&  B&  F C G  D  A  E  H  F# C#
		acc f&  c& g&  d& a&  e& b&   f# c# g# d# a# e# h#
		
		  Pitch classes 0  1 2  3  4  5  6
		  c  d e  f  g  a  h
		  
			MIDIPitch     0  1  2  3  4  5  6  7  8   9 10  11
			C-Major       c  c# d e&  e  f  f# g  g#  a b&  h
			abs=rel       0  0+ 1 2-  2  3  3+ 4  4+  5 6-  6
			
			  key = -3   -> 2,-1
			  absMIDIPitch  3  4  5  6  7  8  9  10 11  0 1   2
			  relMIDIPitch  0  1  2  3  4  5  6  7  8   9 10  11
			  E&Major       e&   e   f   g&    g    a&   a   b&   b    c   d&    d
			  rel		     0,0  0,1 1,0 2,-1  2,0  3,0  3,1 4,0  4,1  5,0 6,-1  6,0
			  abs           2,-1 2,0 3,0 4,-1  4,0  5,-1 5,0 6,-1 6,0  0,0 1,-1  1,0
			  2,0 2,0   2,0                     2,0 2,0   2,0
			  Transform:
			  abs:(pitch,acc) -> abs:MIDIPitch
			  absMIDIpitch -> rel:MIDIPitch(Key)
			  transpose
			  
	*/
	int key   = midifile->currentKey( GetAbsTime() );
//	keyscale *scale = (keyscale *) (allKeys[key + 7]);
	
	
	
	// this is midiPitch
	int midipitch = ChromaticPitch();
	int normKey = key+7;
	int pitch = keyPitch(normKey, midipitch);// (*allKeys[normKey])[midipitch][0];
	int accidental = keyAccidental(normKey, midipitch); //(*allKeys[normKey])[midipitch][1];
	
	int octave = Octave();
	if(pitch < 0)
	{
		pitch += 7;
		octave--;
	}
	else if(pitch > 6)
	{
		pitch -= 7;
		octave++;
	}	
	SetPitch(pitch, accidental, octave );			
}
