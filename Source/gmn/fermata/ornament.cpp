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
|	filename  : NOTE.CPP
|	author    : Juergen Kilian
|	date	  : 17.10.1996-2000
|	contents  : implementation of TQNOTE, TMusicalObject, TOrnament, TQChord
------------------------------------------------------------------*/
#include <sstream>
using namespace std;
#include <string.h>

#include <stdlib.h>
#include "track.h"
#include "ornament.h"
#include "../lib_src/ini/ini.h"
//----------------------------------------------------------------------

#undef UNUSED
//----------------------------------------------------------------------
#define ORNAMENT_GRACE "grace"
#define ORNAMENT_GLISSANDO "gliss"
#define ORNAMENT_MORDENT "mord"
#define ORNAMENT_TRILL "trill"
#define ORNAMENT_TURN "turn"
#define ORNAMENT_UNDEF "undef"
#define CHORD "chord"

//----------------------------------------------------------
/*
	-create TOrnament with values of baseNote
	-delete baseNote
*/
TOrnament::TOrnament(TQNOTE *baseNote,			// first long note after ornament
				  TQNOTE *asPlayedFrom,			// first short ornamental note
				  TMusicalObject *asPlayedTo,   // first long note after ornament
				  TTRACK *track,
				  const char *type) : TQNOTE(baseNote->GetAbsTime(),
											  baseNote->GetDuration(),
											  baseNote->GetMIDIPitch(),
											  baseNote->getIntens()
														 )
{
	mId = baseNote->getId();
	typeI = type; // ornament type

	TMusicalObject *temp;

	color = baseNote->color;
	createTag = baseNote->createTag;
//	testScorePos = baseNote->testScorePos;

	// copy values from base note
	BestDiffT = baseNote->GetBestDiffAttack();
	SecDiffT  = baseNote->GetSecDiffAttack();
	BestDiffL = baseNote->GetBestDiffDuration();
	SecDiffL  = baseNote->GetSecDiffDuration();
    dSelection = baseNote->dSelection;
    atSelection = baseNote->atSelection;

	mIntens = baseNote->getIntens();
    
    SetVoice(baseNote->GetVoice());

	if(!strcmp(typeI, ORNAMENT_MORDENT) ||
	   !strcmp(typeI, ORNAMENT_TURN) ||
		!strcmp(typeI, ORNAMENT_TRILL)) // ornamental notes are played AFTER base note ornaments shorten played base note
	{
		// copy values, but they shouldn't be set at this stage
		BestDiffT = asPlayedFrom->GetBestDiffAttack();
		SecDiffT  = asPlayedFrom->GetSecDiffAttack();
		BestDiffL = asPlayedFrom->GetBestDiffDuration();
		SecDiffL  = asPlayedFrom->GetSecDiffDuration();

		//SetPitch(asPlayedFrom->GetMIDIPitch());
		mPitch  = asPlayedFrom->DiatonicPitch();
		mOctave  = asPlayedFrom->Octave();
		mAccidentals  = asPlayedFrom->accidentals();


		// detach ornamented notes form list
		asPlayed = dynamic_cast<TQNOTE *>(track->DetachedVoice(asPlayedFrom,
										asPlayedTo)); 

		// shift if first note velocity is higher!
		int vel1, vel2;
		vel1 = baseNote->getIntens();
		vel2 = asPlayedFrom->getIntens();
		if( vel2 >= vel1  ||				
			!strcmp(typeI, ORNAMENT_TRILL) )
		{
			// shift to first ornamental note
			SetDuration(baseNote->GetAbsTime() +
						baseNote->GetDuration() -
					asPlayedFrom->GetAbsTime() );
			// shift score note to beginning of ornament
			// !!! this is dangerous to clicktrack calculation!!!
			SetAbsTime( asPlayedFrom->GetAbsTime() );
		}
		else
		{
			// keep root note attackpoint
		}
		// remove baseNote from note list, will be re-inserted as TOrnament
		temp = track->DetachedVoice(baseNote,
					baseNote->TMusicalObject::GetNext(-1));


		if( baseNote )
			delete baseNote;

		if(temp && 0)
		{
			baseNote = dynamic_cast<TQNOTE *>(temp);
			delete baseNote;
		}
	}
	else // ornamental notes are played BEFORE base note keep original duration
	{
		temp = track->DetachedVoice(asPlayedFrom,asPlayedTo); // remove complete ornament from notelist
		asPlayed = QNOTE( temp );
		asPlayed = asPlayedFrom;


		// remove baseNote from note list
		if( track ) // if track -> baseNote was already inserted as QNOTE
		{
			temp = track->DetachedVoice(baseNote,
					baseNote->TMusicalObject::GetNext(-1));
			// remove baseNote from note list, will be re-inserted as TOrnament
			delete baseNote;
		}
	}

	// insert this (new ornament) into notelist
	if( track )
		track->Insert(this);
}

void TOrnament::Debug( ostream &out )
{
	out << string(typeI) << "{ ";
	TQNOTE::Debug(out);
	out << "( \n";
	TNOTE *temp = NOTE(asPlayed);
	while( temp )
	{
		out << "  -";
		temp->TNOTE::Debug(out);
		temp = NOTE(temp->GetNext(-1));
	}
	out <<  ")}\n";
}
TFrac TOrnament::Convert( ostream &gmnOut,
				TAbsTime preEndtime,
				TFrac   prev,
				TTRACK *Track0)
{
	TFrac res;
	TAbsTime attackpoint;  // attackpoint

	attackpoint = qAttackpoint();


	// convert control information until now
	if( Track0 )
	{
			preEndtime = Track0->Convert( gmnOut,
							  NULL, 				// track0
							  preEndtime,		// >= from
							  attackpoint,
							  0L /*offset*/);	// < to

	}

/* final version
	fprintf(file,"\\%s({",typeI);
	// Convert base note
	res = TQNOTE::Convert(file,
						 preEndtime,
						 prev,
						 NULL); // no Control information

*/

	TInifile *tempIni = Track0->Parent()->getInifile();
	// write default
	tempIni->GetValChar ("ORNAMENT","OFF", "ornament detection [OFF|DETECT]");
	const char *valOut = tempIni->GetValChar ("ORNAMENT_OUT","ON","[ON|OFF] Output of ornament tags \\grace, \\trill, ...");
	if( strcmp( valOut, "ON") == 0	)
	{
	
		if(!strcmp(typeI, ORNAMENT_MORDENT) ) // mordent
		{
			gmnOut <<  "\n\\" << typeI << "(";

			// ignore as played notes
		}
		else if(strcmp(typeI, ORNAMENT_GRACE) ) // no grace
		{
			// debug version
			gmnOut << "\n\\text<\""<< typeI << " {";		

			// convert as played notes
			TQNOTE *temp;
			temp = asPlayed;
			while(temp)
			{
					temp->dSelection = secondSel;
					temp->atSelection = firstSel;
					// don't write colors into string!
					temp->color = 0;
					// supress intens writing
					temp->setIntens(0);
					temp->Convert(gmnOut,
					 temp->qAttackpoint(),
					 0L,
					 NULL); // skip Track0 information
					if(temp->TMusicalObject::GetNext(-1))
					{
						temp = dynamic_cast<TQNOTE *>(temp->TMusicalObject::GetNext(-1));
						gmnOut << ","; // separate played notes
					}
					else
						temp = NULL;

			} // while
			gmnOut << "}\">("; // close tag
		}
		else // grace note convert
		{
			// debug version
			gmnOut << "\\slur( \\"<< typeI << "(";		

			// convert as played notes
			TQNOTE *temp;
			temp = asPlayed;
			while(temp)
			{
					temp->SetDuration(TAbsTime(1,16));
					temp->dSelection = firstSel;
					temp->atSelection = firstSel;

					// supress intens tag output
					temp->setIntens(0);
					// as played notes shouldn_t be chords
					temp->TQNOTE::Convert(gmnOut,
						 temp->qAttackpoint(),
						0L,
						NULL); // skip Track0 information
					if(temp->TMusicalObject::GetNext(-1))
					{
						temp = dynamic_cast<TQNOTE *>(temp->TMusicalObject::GetNext(-1));
						gmnOut << " "; // separate played notes
					}
					else
						temp = NULL;

			} // while
			gmnOut << ") ";
		}
	} // if output on
	// Convert base note
	res = TQNOTE::Convert(gmnOut,
						 preEndtime,
						 prev,
						 NULL); // no Control information
	
	if( strcmp( valOut, "ON") == 0
		)
	{
	// close range of tag, or slur of ornament
//	fprintf(file,"}) ");  
			gmnOut << ") ";  
	}
	return res;
}

TOrnament::~TOrnament( void )
{
	TMusicalObject *temp;

	// delete ornament list
	temp = asPlayed;
	while(temp)
	{
		temp = temp->TMusicalObject::GetNext(-1);
		delete asPlayed;
		asPlayed = QNOTE( temp );
	}

}

void TOrnament::ticksToNoteface( int ppq )
{
	TQNOTE::ticksToNoteface(ppq);

	TQNOTE *temp;

	temp = asPlayed;
	while(temp)
	{
		temp->ticksToNoteface(ppq);
		if(temp->GetNext(-1))
		{
			temp = dynamic_cast<TQNOTE *>(temp->GetNext(-1));
		}
		else
			temp = NULL;
	}
}


void TOrnament::pitchSpelling(TMIDIFILE *midifile)
{
	TNOTE::pitchSpelling(midifile);
	TNOTE *temp = asPlayed;	
	while(temp)
	{
		temp->pitchSpelling(midifile);
		temp = NOTE(temp->GetNext(-1));
	}
}
