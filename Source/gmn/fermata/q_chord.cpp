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
|	filename  : TQCHORD.CPP
|	author    : Juergen Kilian
|	date	  : 17.10.1996-2003
|	contents  : implementation TQChord
------------------------------------------------------------------*/
#include <sstream>
using namespace std;
#include <stdlib.h>
#include "track.h"
#include "q_chord.h"
#include "../lib_src/ini/ini.h"
//----------------------------------------------------------------------

#undef UNUSED


//////////////////////////////////////////////////////////////////////
// TQChord Klasse
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

TQChord::TQChord( TQNOTE *baseNote, /// root note, will be replaced by TQChord!!
				  TQNOTE *asPlayedFrom,
				  TMusicalObject *asPlayedTo,
				  TTRACK *track ) : TOrnament(baseNote, 
										asPlayedFrom,
										asPlayedTo,
										track,
										CHORD)// type
{
	// !!! base note will be deleted in TOrnament constructor and can not be used anymore!		
}

TQChord::~TQChord()
{
}


TAbsTime TQChord::SetQData(   TQNOTE *ptr, 	// previous note
							   TFracBinList *bestDurations )
{
	TFrac res = TQNOTE::SetQData( ptr, bestDurations);
	if( res > 0 )
	{
		TQNOTE *temp = asPlayed;
		while( temp )
		{
			temp->SetAbsTime(qAttackpoint());
			temp->SetDuration(qDuration());
			temp->atSelection = firstSel;
			temp->dSelection = firstSel;

			temp = dynamic_cast<TQNOTE *>(temp->GetNext(-1));
		}// while
	}
	return res;
}

/*
	gmn output for chords
	return: new default numerator
*/
TFrac TQChord::Convert( ostream &gmnOut,
				TAbsTime preEndtime,
				TFrac   prev,
				TTRACK *Track0)
{
	TAbsTime attackpoint = qAttackpoint();

	// convert control information before "this"
	if( Track0 )
	{
			preEndtime = Track0->Convert( gmnOut,
							  NULL, 				// track0
							  preEndtime,		// >= from
							  attackpoint,
							  0L /*dummy offset*/);	// < to
	} // if control track
	
	if( qDuration() <= 0 )
	{
		gmnOut << "(* Error duration <= 0! ";
		qDuration().Write( gmnOut );
		gmnOut << "*) \n";
		return TFrac(-1,8);
	}
	else
	{
	if( color > 0 )
	{
		const char *colorStr;
		colorStr = getColorString( color );
		gmnOut << " \\noteFormat<color=\" "<<colorStr <<"\">(";
	}
	
	if( getIntens() > 0 ) // don't write intens inside chords
	{
		writeIntens( gmnOut, getIntens() );
	}
	
		
		gmnOut << "{"; // chord start
		
		// Convert base note
		TFrac res = TQNOTE::Convert(gmnOut,
			preEndtime,
			prev,
			NULL); // no Control information
		
		TAbsTime newDuration = qDuration();
		TAbsTime newAttack = qAttackpoint();
		// convert as played notes
		TQNOTE *temp = asPlayed;
		int colorWritten = 0; // flag
		while(temp)
		{
			gmnOut << ",";
			
			// !!! if chord notes have different durations -> noteviewer will shift all later attacks
			temp->SetDuration(newDuration);
			// invalidate intens
			temp->setIntens(-1);
			colorWritten = 1;
			temp->SetAbsTime(newAttack);
			temp->dSelection = firstSel;
			temp->atSelection = firstSel;
			
			temp->TQNOTE::Convert(gmnOut,	// don't convert grace etc. in chord
				temp->qAttackpoint(),
				0L,
				NULL); // skip Track0 information
			if(temp->TMusicalObject::GetNext(-1))
			{
				temp = dynamic_cast<TQNOTE *>(temp->TMusicalObject::GetNext(-1));
			}
			else
			{
				temp = NULL;
			}
		} // while
		gmnOut << "} "; // chord end
		if( color > 0 )
			gmnOut << ") "; // end range of noteFormat
		return res;
	} // else
}


/*

	collect succeeding notes in voice into a chord if at the same attackpoint
	create TQCHORD and insert into list;
	remarks:
		- global remove legato needs to be called before
		- !!duration of chord = earliest note off
		- changed 23.8.02:  longest duration = duration of chord!!

*/
void makeChords(TNoteInfo *noteList,
				int cAttacks,
				int /*CVoice*/,
			   TTRACK *track)
{
//#define GLOBAL_SPLITV

#ifdef GLOBAL_SPLITV
	for( int i = 0; i < cAttacks; i++ )
	{
		int pos = cAttacks-1-i; // go from right to left
		int root = noteList->rootID( pos);
		if( root > -1 &&
			noteList->isRoot( pos ) == 0)
		{
			// chord (root ... i)
			TQNOTE *baseNote = noteList[root].attack;
			int voice = noteList->rootVoice( root);
			TQNOTE *newNote;
			if( QChord(baseNote) )// already converted?
			{
				newNote = QChord(baseNote); 
			}
			else
			{
				newNote = new TQChord(baseNote,
			  					  NULL,	// asPlayedFrom
								  baseNote->GetNext(-1),		// asPlayedTo, no played notes
								  track );

				noteList[root].attack =  newNote;	// replace base note with chord
				newNote->SetVoice(voice);
			}
			// add pos to chord
			newNote->Add(noteList[pos].attack,track); // note will be removed from track
			// remove from array
			noteList[pos].attack = NULL;
		} // if
	} // for

#endif
#ifdef LOCAL_SPLITV
	int i;
	for( i = cAttacks-1; i > -1; i-- )
		// go from right to left
	{
		// look to all root notes

		int rootID = noteList->rootID(i);
		if( rootID > -1 )  // linked to left side
			 
		{
			TQNOTE *baseNote = noteList[rootID].attack;
			TQChord *newNote;
			if( !dynamic_cast<TQChord *>(baseNote)  ) // new chord
			{
				newNote = new TQChord(baseNote,
					NULL,	// asPlayedFrom
					baseNote->GetNext(-1),		// asPlayedTo, no played notes
					track );
				
				noteList[rootID].attack =  newNote;	// replace base note with chord
				newNote->SetVoice(noteList[rootID].minVoice);
				baseNote = newNote;
			}

			if( rootID != i )	// this is not the root
			{
				newNote = dynamic_cast<TQChord *>(baseNote);
				TQNOTE *temp =  noteList[i].attack;
				TAbsTime duration = temp->GetDuration();

				newNote->Add(temp,track,1); // note will be removed from track
				// remove all other notes in chord from array
				noteList[i].attack = NULL;
				// keeep longest duration as chord duration
				if( duration > baseNote->GetDuration() )
				{
					baseNote->SetDuration( duration );
				}
			} // if rootID
		} // if baseNote
	} // for i
#endif
}

/*
	add a note to chord and remove this note from track
*/
void TQChord::Add(TQNOTE *ptr, 
				  TTRACK *track,
				  char avDuration)
{
	if( this == ptr )
	{
		Printf("Warning: circular TQChord::Add!\n");
		return;
	}



	// new ptr ends before chord, try to shorten the chord
	// this works only for unquantised data! 
	/*
	chordDuration = GetDuration();
	ptrDuration = ptr->GetDuration();

	noteOffPtr = ptr->GetAbsTime() + ptrDuration;
	noteOffChord = GetAbsTime()+ chordDuration;

	if( noteOffPtr < noteOffChord )
	{
		deltaDuration = noteOffChord - noteOffPtr;
		chordDuration = chordDuration - deltaDuration;
		if(  chordDuration >= ptrDuration ) // dont make shorter than new ptr
		{
			this->SetDuration(chordDuration);
		}

	}
	*/

	// Version2 : chord duration == duration of longest note of chord
	// new ptr ends after chord, try to make the chord longer
	TAbsTime chordOffset = GetAbsTime() + GetDuration();
	TAbsTime ptrOffset = ptr->GetAbsTime() + ptr->GetDuration();


	if( avDuration )
	{
		if(  ptrOffset > chordOffset )
		{
			TAbsTime chordDuration = chordOffset - GetAbsTime();
			this->SetDuration(chordDuration);
		}
	}
	
	// detach new chord-notes from list
	// if !track -> new notes were not inserted in track
	if( track )
		track->DetachedVoice(ptr, ptr->GetNext(ptr->GetVoice()));


	ptr->SetNext(NULL);
	ptr->SetPrev(NULL);

	TQNOTE *temp = asPlayed; 

	if( dynamic_cast<TQChord *>( ptr ) ) // new note is already a chord ->split into QNOTES
	{
		TQNOTE *ptr2;
		ptr2 = new TQNOTE( *ptr );
		/*
							ptr->GetAbsTime(),
						   ptr->GetDuration(),
						   ptr->GetMIDIPitch(),
						   ptr->Intens() );
						*/

		// copy additional notes
		TQChord *tempChord;
		tempChord = dynamic_cast<TQChord *>(ptr);

		// link chain from ptr to ptr2
		ptr2->SetNext( tempChord->asPlayed );

		// set additionals in ptr2 to NULL
		tempChord->asPlayed = NULL;
		delete ptr;

		ptr = ptr2;
	}

	if( !temp ) // first additional note
		asPlayed = ptr;
	else
	{
		// search for end of playedNotes list
		while( temp->GetNext(-1) )
			temp = QNOTE(temp->GetNext(-1));
		temp->SetNext(ptr);
	}
	

}

#define PITCHDECAY 0.2

/*
	
	return pitch distance between pitch2 and closest note of chord.

	
	The version of	
	return average pitch distance of distance to highest and lowest note of chord
	did not work, because laders of big and small chords could have small distances!
	e.g. {c0, c2} {c1,d1} = 0
*/
double TQChord::deltaPC(int pitch2, int lookBack)
{


    // printf("qcDelta"); fflush(stdout);

	double av;
	int pPrev,
		count = 1;
	double 	res;
	TQNOTE *prev;



    //printf("<cp"); fflush(stdout);
	av = closestPitch( pitch2 );
    //printf("cp>"); fflush(stdout);

	// av == closest pitch in chord to pitch2 


	
	// eval lookback 
	prev = QNOTE(GetPrev(GetVoice()));
	while( lookBack && prev )
		// get average pitchdistance to previous notes
	{
		pPrev = prev->closestPitch(pitch2);
//		pPrev = prev->avMIDIPitch();
//		pDist = prev->pitchDistance(pitch2,0); 

//		if( (pDist+1) < res )
		{
			av *= PITCHDECAY;

			av += (double)pPrev * (1-PITCHDECAY);
			lookBack--;
			count++;
		}
		prev = QNOTE(prev->GetPrev(GetVoice()));

	}

	// todo : make weighted pitch 
	res = pitch2 - av;
	if( res < 0 )
		res *= -1;


	return res;
}




void TQChord::pitchSpelling(TMIDIFILE *midifile)
{
	TNOTE::pitchSpelling(midifile); // do it for the base note
	// get number of accidentals for base note
	int rootAccidentals = accidentals();
	
	// process chord notes
	TQNOTE *temp = asPlayed;
	while( temp ) // search for closest note in chord
	{
		temp->pitchSpelling(midifile);

		int pitch, acc, octave;
		if( rootAccidentals == 0 )
		{
			if( temp->accidentals() > 0)
				rootAccidentals = 1;
			else if( temp->accidentals() < 0 )
				rootAccidentals = -1;
		}
		
		if( rootAccidentals < 0 
		    && temp->accidentals() > 0)  // conflict between root and chord notes?
		{	
			// enharmonic transpose according rootAccidentals
			pitch  = temp->DiatonicPitch(); // in key c
			acc    = temp->accidentals();
			octave = temp->Octave();
			// modifiy pitch2
			enharmonicTransp( &pitch, 
							  &acc, 
						      &octave,
							  1 );

			temp->SetPitch(pitch,
							  acc,
							  octave);				
		} // if conflict
		else if( rootAccidentals > 0 
				 && temp->accidentals() < 0 )
		{
			pitch  = temp->DiatonicPitch(); // in key c
			acc    = temp->accidentals();
			octave = temp->Octave();
			// modifiy pitch2
			enharmonicTransp( &pitch, 
							  &acc, 
						      &octave,
							  -1 );

			temp->SetPitch(pitch,
							  acc,
							  octave);
		} // if conflict
		temp = QNOTE(temp->GetNext(-1));
	} // while chord notes
}

/* 
   only the root note gives an overlap distance, because the root note
   sets the chord duration
*/
double TQChord::overlapDistance(TAbsTime attack2,
								TAbsTime duration2,
								TMIDIFILE *theMidifile)
{
	double res = 0;

	// get the overlap distance for the base note and attack2
	res = TQNOTE::overlapDistance(attack2, duration2, theMidifile);
	// add played notes
	TQNOTE *temp;

	if( res > 0 )
	{
		temp = asPlayed;
		// add same penalty for each played note
		while( temp )
		{
			res = res + res - (res*res);
			temp = QNOTE(temp->GetNext(-1));
		}
	}
#ifdef _DEBUG
	if( res < 0 || res > 1 )
		Printf("overlapDistance out of range!\n");
#endif

	return res;
}


/// used by TQNOTE::deltyPC if lookback==1
double TQChord::avMIDIPitch()
{
	double res;
	int count = 1;

	res = GetMIDIPitch();

	TQNOTE *temp;
	temp = asPlayed;
	while( temp ) // search for closest note in chord
	{
		res += asPlayed->GetMIDIPitch();
		count++;
		temp = QNOTE(temp->GetNext(-1));
	}
	res /= count;

	return res;
}

int TQChord::closestPitch(int pitch2)
{
	int p,
		p2,
		delta,
		deltaMin;

	TQNOTE *temp;

	// find closest pitch in chord 
	p = GetMIDIPitch(); // base note
	delta = abs( p - pitch2);
	deltaMin = delta;
	p2 = p;

	temp = asPlayed;
	while( temp ) // linked chord notes
	{
		p = temp->GetMIDIPitch();
		delta = abs(p - pitch2);
		if( delta < deltaMin )
		{
			deltaMin = delta;
			p2 = p;
		}

		temp = QNOTE(temp->GetNext(-1));
	}
	return p2;

}



// ------------------------------------
#ifdef PENALTY_CHECK 
	// toolfunctions for penalty checker
#include "voiceinfo.h"

double TQChord::pPitch( void )
{
	double res;
	// eval pitch distance of all chord notes
	double interval;

	TQNOTE *prev;
	prev = QNOTE(GetPrev( GetVoice() ));

	// first eval basenote
	if( prev )
	{
		interval = prev->pitchDistance(GetMIDIPitch());
	}
	else
	{
		TInifile *tempIni;
		tempIni = GetInifile();
		if(tempIni )
		{
			interval = tempIni->GetValInt("EMPTYVOICEIV","11; start interval in empty voice");
		}
		else
		{
			interval = 11;
		}
	}
	res = pitchPenalty( interval );
	// now do asPlayedNotes;
	TQNOTE *temp;
	temp = asPlayed;
	double cur;
	while( temp && prev ) 
	{
		interval = prev->pitchDistance(temp->GetMIDIPitch());
		cur = pitchPenalty( interval );
		res = res + cur - (res*cur);
		temp = QNOTE(temp ->GetNext(-1));
	}



	return res;
}
double TQChord::pChord( void )
{
	double res;
	// get ambitus
	int minPitch,
		maxPitch;

	minPitch = GetMIDIPitch();
	maxPitch = minPitch;
	TQNOTE *temp;
	temp = asPlayed;
	while( temp ) 
	{
		if( temp->GetMIDIPitch() < minPitch )
			minPitch = temp->GetMIDIPitch();
		else if( temp->GetMIDIPitch() > maxPitch )
			maxPitch = temp->GetMIDIPitch();
		temp = QNOTE(temp ->GetNext(-1));
	}
	int ambitus;
	ambitus = maxPitch - minPitch;
	res = chordPenalty( ambitus );
	
	return res;
}
#endif




int TQChord::cNotes()
{
	int res = 1;
	TMusicalObject *temp;
	temp = asPlayed;
	while( temp )
	{
		res++;
		temp = temp->GetNext(-1);
	}
	return res;
}

int TQChord::minInterval(TNOTE *ptr)
{
	int x = 0;
	if( !ptr )
		return 0;
	TQChord  *chord = dynamic_cast<TQChord *>(ptr);
	if( chord )
	{
		// test all single notes against complete chord
		x =  ptr->GetMIDIPitch() -  GetMIDIPitch(); 
		TNOTE * curNote = this->asPlayed;
		while( curNote && x != 0 )
		{
			int curIv = -chord->minInterval( curNote );
			if( abs(curIv) < abs(x) )
			{
				x = curIv;
			}
			curNote = NOTE(curNote->GetNext(-1));	
		} // while
	}
	else
	{
		// test the single note against all chordal notes
		x =  ptr->GetMIDIPitch() -  GetMIDIPitch(); 
		TNOTE * curNote = this->asPlayed;
		while( curNote && x != 0)
		{
			int curIv = TNOTE::minInterval( curNote );
			if( abs(curIv) < abs(x) )
				x = curIv;
			curNote = NOTE(curNote->GetNext(-1));	
		} // while
	}
	
	return x;
}
