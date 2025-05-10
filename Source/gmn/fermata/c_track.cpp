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
|	filename : C(LICK)TRACK.CPP
|	Autor     : Juergen Kilian
|	Datum	  : 17.10.1996-2003, , 2011
|	Aufgabe   : implementation of TCLICKTRACK
------------------------------------------------------------------*/
#include "debug.h"
#include<iostream>
using namespace std;
#include<fstream>
#include<sstream>
#include <stack>
#include <string.h>



#include "q_funcs.h"
#include "c_note.h"

#include "event.h"
#include "c_track.h"
#include <string>
#include "funcs.h"
#include "meta_tempo.h"
#include <math.h>
#include "h_midi.h"
#include "statist.h"
#include "import.h"
#include "../lib_src/ini/ini.h"


//--------------------------------------------------------------------
TCLICKTRACK::TCLICKTRACK( long offset,
		  TMIDIFILE *parent )
{
	init();
	RecTempoI = 0;
	RecPPQI   = 0;
	Offset = offset;
	mParent = parent;

} // TCLICKTRACK


TCLICKTRACK::TCLICKTRACK( int recPPQ,
						  int recTempo,
						  TQTRACK *src,
						  TMIDIFILE *parent)
{
	init();
	if( src )
		Offset = src->GetOffset();
	else
		Offset = 0;

	RecPPQI = recPPQ;
	RecTempoI = recTempo;
	mParent = parent;
}

void TCLICKTRACK::init()
{
	maxCNotes = 0;
	maxIntens = 0;
	minIntens = 128;
	OK     = 1;
	Notes  = NULL;
	Tail   = NULL;
	CurrentNote = NULL;
	ClickDuration   = -1;
	FirstClickPlay  = -1;
	LastClickPlay   = -1;
	LastDiffTick    = -1;
	FirstDiffTick   = -1;

	cInsertNotes = 0;
}

//--------------------------------------------------------------------
/*
	Delete all notes in track
*/
TCLICKTRACK::~TCLICKTRACK()
{
	Reset();
} // ~TCLICKTRACK
//--------------------------------------------------------------------
/*!
	read trackheader from MIDI-File
	return
		offset position in file after this track
		 == offset for header of next track
*/
long TCLICKTRACK::Read_Header( FILE *file )
{
	char 	temp[5];
	long 	Laenge = 0;
	int  	i;
	fpos_t 	pos;

	temp[4] = 0;
	fseek( file, Offset, 0);	// ptr to header
	fgetpos( file, &pos );

	fread( temp, 4, 1, file );  	// read 'MTrk' 
	if( strcmp( temp, "MTrk" ) != 0 )
		ErrorMsg( 20 );

	fread( temp, 4,1 , file ); 	// 4 Byte tracklength
//	Ptr = (char*)&Laenge;
	Laenge = 0;
	for( i = 0; i< 4; i++ )
	{
		Laenge *= 256;
      Laenge += temp[i];
//	   *(Ptr + i ) = temp[3-i];
	}

	return  Laenge + Offset + 8;

} // ReadHeader
//--------------------------------------------------------------------
/*!
	create a tempo profile from metronom events in this
	result:
		create tempo change events in controlTrack
	remarks:
		timing is in tick timing!
	
*/
void TCLICKTRACK::CreateTempoTags( TQTRACK *controlTrack )
{

	if( !controlTrack )
	{
		Printf("CreateTempoTrack ControlTrack==NULL!\n");
		return;
	}


	// delete existing tempo events
	controlTrack->deleteSameType(controlTrack->FirstTempo(), //from
													NULL); // to

	TCLICKNOTE *Prev = FirstNote();
	if( !Prev )
	{
		Printf("ERROR: Empty Clicktrack!\n");
		return;
	}

							
	double Tempo = 0;
	int i = 1;

	TCLICKNOTE *Now  = CLICKNOTE(Prev->GetNext(-1)); 
	TFrac PrevRealtime = 0L;	// set first tempo change at zero-time
	while( Now )		// complete list
	{

		double NewTempo = tempo(Now );
		if( ! Now->GetNext(Now->GetVoice()) )
		{
			// no next note -> no real tempo to calculate!
			NewTempo = Tempo;
		}
		if( i > 1 )	// start exponential smoothing after 2nd value
		{
			 if( fabs(NewTempo - Tempo) < 20 )
					 NewTempo = ((NewTempo * 40) +
						  (Tempo*60)) / 100;
		} // if( i > 1
// todo optimize output of tempo profile
// todo put tempo smoothing params in .ini
// todo regression -> inferring of ritardando
		if( (fabs(NewTempo - Tempo) > 10) || // store if change is > 5bpm 
				  ( i == 1) ) // always store 1st tempochange
		{
			// Create new tempo event
			TMetaTempo *Temp = new TMetaTempo(PrevRealtime,
										 NewTempo,
										 1,  // numerator
										 4); // denominator
			
			controlTrack->Insert( Temp );	// insert in list
			Tempo = NewTempo;
		} // if( i > 1 && abs (

		// goto next metronom beat
		Prev = Now;
		PrevRealtime = Prev->scoretime();
		Now  = CLICKNOTE(Now->GetNext(-1));
		i++;
	} // while
} // Create TempoTrack
//--------------------------------------------------------------------
// check if Event == valid click for typeFIlter
char TCLICKTRACK::clickEvent( TEVENT *Event,
							 /// t -1 == off; 0..127 = pitch;128..255=controller
							int typeFilter)
{
	if( !Event )
		return 0;

	if( typeFilter == -1 &&
		 Event->noteOn() )
	{
		return 1;
	}
	if( typeFilter < 128 &&  // pitch filter
		 Event->noteOn() &&
		 Event->pitch() == typeFilter )
	{
		return 1;
	}
	if( typeFilter > 127 )
		typeFilter -= 128;	// get controler nr
	if( Event->Type() == typeFilter &&
		 Event->getData(0) > 0 ) // hold-down
	{
		return 1;
	}
	return 0;
}

/*
	create a clicktrack from events in a MIDI-FIle track
	events can be filtered by channel, pitch, or controller values
	ToDo check clicktrack filters in .ini
	
*/
char TCLICKTRACK::Read( FILE *file,
					   int  recPpq,				// resolution
					   TFrac  partsPerClick,    	// length of metronom-click
					   int  channel,				// filter: 0==off,1..16
					   int  typeFilter,			// t -1 == off; 0..127 = pitch;128..255=(controller+128)
					   FILE *log )
{
	if( !file )
	{
		Printf("ERROR: No input MIDI file available for reading a clicktrack!\n");
		return 0;
	}
	ClickDuration = partsPerClick; // duration of a metronom beat
		
	fseek(file, Offset + 8, 0 );	// goto to first event of track
	
	TEVENT *Event = new TEVENT();
	
	// read event, get running status of MIDI-File
	char Status = 0;
	Status = Event->Read( Status, file, NULL /* lgsequence */ );
	char currentChannel = 0;
	int currentTrack = 0;

	long prevAbsTime = -1;
	long AbsTime  = Event->DTime();
	
	int currentTempo = DEFAULT_TEMPO;
	currentTempo = Event->write( log, 
								AbsTime,
								recPpq,
								&currentChannel,
								0, // Track0
								currentTempo,
								currentTrack,
								1); // count
	
	TAbsTime RealTime = 0L;	  // Clicktrack starts at zero-time
	TCLICKNOTE *prevNote = NULL;	
	TAbsTime scoreTime;
	while( !feof(file) &&
		!Event->EndOfTrack() )	// untilk track end
	{
		if( //Event->NoteOn() &&
			clickEvent(Event, typeFilter) &&
			AbsTime != prevAbsTime &&  // skip multiple events at equal time
			(channel == 0 ||
			channel == Event->Channel()) )	// note start?
		{
			TCLICKNOTE *Ptr = new TCLICKNOTE(AbsTime,    // = as played
								RealTime,	// as quantized in metronome  beats
								Event->Intens(),			
								ClickDuration );

			Ptr->setScoretime(scoreTime);
			scoreTime += ClickDuration;
			Ptr->scoreDurationI = ClickDuration;
			LastClickPlay = AbsTime;
			// append to list
			if( Notes )	// append
			{
				prevNote->SetNext( Ptr, this );
			}
			else	// create list
			{                 
				Notes = Ptr;
			}
			prevNote  = Ptr;
			// copy/calc time values
			RealTime += ClickDuration;
			prevAbsTime = AbsTime;			
		} // if( Event->NotOn() )
		delete Event;
		Event = new TEVENT();
		// new running status
		Status  = Event->Read( Status, file, NULL );
		// new Abstime
		AbsTime  += Event->DTime();
	} // while
	RecPPQI = recPpq;
	delete Event;
	/// clicktrack could include wrong, double trigger clicks
	this->mergeEqual(RecPPQI, RecTempoI);
	return 1;
} // Read
//--------------------------------------------------------------------
/*
	check if this includes an upbeat
	an upbeat will be detected if some notes in track start
	before the first metronombeat.

	result
		1 : upbeat detectet
		0 : no upbeat
*/
char TCLICKTRACK::CheckForUpBeat( TQTRACK *track	)
{
	if( !track )
		return 0;

	TNOTE *Note  = track->FirstNoteObject(-1 /* all voices */);	// first note in track
	if( !Note )	// track is empty
		return 0;

	TCLICKNOTE *Click = FirstNote();				// First metronom-beat
	if( !Click )
	{
		Printf("ERROR: Empty Clicktrack!\n");
		return 0;
	}

	int Flag = 0;
	long diff = Note->GetAbsTime().toLong() - Click->Playtime();
	if( diff < 0 )	// Notes starts earlier than Click
	{
		while( Note && 
			   (diff < 0)  )
		{
			// convert in ms
			TAbsTime rtime = DTIMEtoMS( RecPPQI,
									    RecTempoI,
									    diff);
			// if rtime is very small (reaction time),
			// metronom beat will be moved forward
			if( (abs(rtime) < RTIME)	&&
				 (rtime < 0L )   &&
					  !Flag ) // this is not an upbeat
			{
				Click->SetPlaytime( Note->GetAbsTime().toLong() );
			}
			else
			{
			 	// add a new click which is earlier than the first one
			 	TCLICKNOTE *nClick = Click->getNext(1,-1);
			 	if( !nClick )
			 	{
			 		Printf("WARNING: Only single click in clicktrack!\n");
			 		return 0;
			 	}
			 	TFrac dScore = nClick->scoretime() -
			 					Click->scoretime();
			 	long dPerf = nClick->Playtime() -
			 					Click->Playtime();
				shiftScoretimes( FirstNote(),	// from first click note
								 NULL,			// until list end
								 dScore );	    // shift 
			 	
			 	TCLICKNOTE *newClick = new TCLICKNOTE( Click->Playtime() - dPerf,
			 											TFrac(0,0),
			 											Click->Intens(),
														Click->plDuration() );
				Insert( newClick );
				Click = newClick;
				Flag = 1;	// at least one note is before first metronom beat
			}
			diff = Note->GetAbsTime().toLong() - Click->Playtime();
		} // while
	} // if diff

	if( Flag )	// if an upbeat was detected all metronombeats will be moved
				// for the length of one measure
// todo compare/connect with time signature detection
	{
/*
		shiftScoretimes( FirstNote(),	// from first note
							 NULL,			// until list end
							 barlength );	// shift for barlength
*/
		return 1;
	}
	else
	{
		return 0;
	}
} // CheckForUpBeat
//--------------------------------------------------------------------
																		//	beginnt die Metronomspur erst im Takt nach diesen Noten

/*!
	Tempodetection with metronomtrack
	quantize track with metronombeats in *this

	remarks:
	this function changes AbsTime and lgDuration in all notes of track
	and can be called only once !!
 */
char TCLICKTRACK::FitToClick( TQTRACK *track )
{
	//track->Debug();

	if( !track )
		return 1;

	TNOTE *Note  = track->FirstNoteObject(-1 /* all voices */);
	if( !Note )	// -> track is empty
		return 1;


	TCLICKNOTE *Click = FirstNote();	// get first metronome beat
	if( !Click )
	{
		Printf("ERROR: Empty ClickTrack!n");
		return 1;
	}
	/*
		all notes starting before first metronome beat must bew
		evaluated speratly, because timing functions work only
		between two metronome beats
	*/


	long diff = Note->GetAbsTime().toLong() - Click->Playtime();


	int Flag = 0;
	while( (diff < 0) &&
            Note != NULL )
	{
			Flag = 1;
			Note = NOTE(Note->GetNext(-1/* all voices */));
        if( Note ){
			diff = Note->GetAbsTime().toLong() - Click->Playtime();
        }
	} // while


	TNOTE *PostUpbeat = NULL;
	if( !Flag )	// no upbeat
		Note  = track->FirstNoteObject(-1 /* all voices */);	
	else
		PostUpbeat = Note;


	// start with first note after upbeat
	while( Note )	// complete track
	{

		long NoteAttack = Note->GetAbsTime().toLong();	// as recorded

		Click  = FirstNote();		// get first metronome beat
		// search until PrevClickTime <= NoteAttack < ClickTime
		while(Click && 
			  Click->GetNext(-1) &&
			  (Click->Playtime() <= NoteAttack)  )
		{
			Click = Next();
		} // while
		/*
			Now
			Click->Prev <= Note < Click
			OR
			Click <= Note < NULL
			OR
			NULL <= Note < Click
		*/

		TFrac prevScoreTime;
		long prevPerfTime = 0;
		TCLICKNOTE *pClick = Click->getNext(-1,-1);
		if( !pClick )
		{
			pClick = Click;
			Click = Click->getNext(1,-1);
		}
		if( !Click || !pClick )
		{
			Printf("ERROR: too less clicks in clicktrack!\n");
			return 1;
		}

		prevScoreTime = pClick->scoretime();
		prevPerfTime = pClick->Playtime();


		TFrac PartsPerClick = Click->scoretime() - prevScoreTime;
        PartsPerClick *= RecPPQI * 4;
		double PartsPerPerf = Click->Playtime() -  prevPerfTime; // played tick-distance between clicks

		double mult = 0;
		if( PartsPerPerf > 0 )
			mult = (PartsPerClick.toDouble() / PartsPerPerf);

		if( NoteAttack >= Click->Playtime() )
		{
			prevPerfTime = Click->Playtime();
			prevScoreTime = Click->scoretime();
		}

		diff = NoteAttack - prevPerfTime;
		// calc new value

		NoteAttack = (prevScoreTime.toDouble()*RecPPQI*4.0 + ( diff * mult ) +0.5);

		long NoteEnd = Note->GetAbsTime().toLong() + Note->GetDuration().toLong();
		while(Click && 
			  Click->GetNext(-1) &&
			  (Click->Playtime() <= NoteEnd)  )
		{
			Click = Next();
		} // while
		/*
			Now
			Click->Prev <= Note < Click
			OR
			Click <= Note < NULL
			OR
			NULL <= Note < Click
		*/

		prevScoreTime = 0L;
		prevPerfTime = 0;
		pClick = Click->getNext(-1,-1);

		prevScoreTime = pClick->scoretime();
		prevPerfTime = pClick->Playtime();


		PartsPerClick = Click->scoretime() - prevScoreTime;
        PartsPerClick *= RecPPQI * 4;
		PartsPerPerf = Click->Playtime() -  prevPerfTime; // played tick-distance between clicks

		mult = 0;
		if( PartsPerPerf > 0 )
			mult = (PartsPerClick.toDouble() / PartsPerPerf);

		if( NoteEnd >= Click->Playtime() )
		{
			prevPerfTime = Click->Playtime();
			prevScoreTime = Click->scoretime();
		}
		diff = NoteEnd - prevPerfTime;

		// calc new value
		NoteEnd = (prevScoreTime.toDouble()*RecPPQI*4.0 + ( diff * mult ) +0.5);


		Note->SetAbsTime( NoteAttack );
		Note->SetDuration( NoteEnd - NoteAttack );

		Note = NOTE(Note->GetNext(-1/* all voices */));
	} // while ( Note )
//	 track->Debug();
//	 Write();
	return 1;
} // FitToClick
//--------------------------------------------------------------------
/*
	Delete all clicks in Clicktrack
*/
void TCLICKTRACK::Reset( void )
{
	TCLICKNOTE *Ptr,
			*Next;

	Ptr = Notes;	// head of list

	while( Ptr )	// complete track
	{
		Next = CLICKNOTE(Ptr->GetNext(-1));
		delete Ptr;
		Ptr = Next;
	} // while
	Notes = NULL;
	Tail = NULL;
} // Reset
//--------------------------------------------------------------------
/*!
	insert *note sorted into tracklist
	remarks:
		- multiple notes at an equal timepoint will be merged 
		  to one single note
		- note might be deleted by merge operation!
*/
void TCLICKTRACK::Insert( TCLICKNOTE *note )
{
	TCLICKNOTE *Now,	
			*Next;		

	TAbsTime NowTime, 	// recorded timepoint
		  NextTime,		// recorded timepoint
		  NoteTime;		// recorded timepoint

	char end;

	if( !note )
		return ;


	// check statsitical values
	if( note->cNotes > maxCNotes )
		maxCNotes = note->cNotes;

	if( note->Intens() > maxIntens )
		maxIntens = note->Intens();

	if( note->Intens() < minIntens )
		minIntens = note->Intens();


	cInsertNotes++;
	NoteTime = note->Playtime();

	if( Notes )	// already elements in list
	{
		Now = Notes;
		NowTime = Now->Playtime();

		// search for position
		if( NoteTime < NowTime ) // insert as new head
		{
			note->SetNext( Now , this);
			Notes = note;	// new head of list
			note->mergeEqual( RecPPQI,	 // remove multiple notes at equal time
								RecTempoI,
								this);
		}
		else	// insert inside list
		{
			Next = CLICKNOTE(Now->GetNext(-1));
			end = 0;
			while( Next && !end ) // search in list
			{
				NowTime  = Now->Playtime();
				NextTime = Next->Playtime();
				if( NowTime == NoteTime )	// already click with equal attack?
				{
					// keep Now
					Now->copyMaxVals( note, this);
					// remove note
					delete note;
					end = 1;
				}
				else if( (NowTime < NoteTime) &&
					     (NoteTime < NextTime) )	// position found->insert
				{	
					// insert into list
					Now->SetNext( note, this );
					note->SetNext( Next, this );

					/* check if equal attacks? -> merge to one single note*/
					if( Next == Tail )
						Tail = note->mergeEqual( RecPPQI,     // compare note <->Next
							                       RecTempoI,
													this );
					else
						note->mergeEqual( RecPPQI,     // compare note <->Next
								            RecTempoI ,
											this);

					Now->mergeEqual( RecPPQI,	  // compare Now<->note/Next
									   RecTempoI,
										this );

					end = 1;
				} // if
				Now  = CLICKNOTE(Now->GetNext(-1));
				Next = CLICKNOTE(Now->GetNext(-1));
			} // while 
			if( !end ) // append at list end
			{

				Now->SetNext( note, this );
				Tail = Now->mergeEqual( RecPPQI,	// check for equal attacks
									RecTempoI,
									this );
				note = NULL;
			} // if
		 } // else
		 FirstClickPlay = Notes->Playtime();
		 if( Notes->GetNext(-1) )
				FirstDiffTick  = (CLICKNOTE(Notes->GetNext(-1))->Playtime() - FirstClickPlay);
	} // if( Head
	else // create head of list
	{
		Notes = note;
		Tail  = note;
	}
} // Insert
//--------------------------------------------------------------------
/*!
	shift all attackpoints in range [startptr, endptr) 
*/
void TCLICKTRACK::shiftScoretimes( TCLICKNOTE *startptr, // first note
				  TCLICKNOTE *endptr,	// lastnote + 1
				  TAbsTime offset )		// shift value
{
	TCLICKNOTE *Ptr;

	Ptr = startptr; 	

	if( offset == 0L )
		return; // nothing to do 
//	if( (Ptr->scoretime() + offset) >= 0L ) // negative attacks ae not valid ?
	{
		while( Ptr != endptr )	// complete list
		{
			if( Ptr->scoreDuration() > 0L )
			{
				// invalid duration, reset scoretime
				Ptr->setScoretime(Ptr->scoretime() + offset);
			}
			else
			{
				Ptr->setScoretime( Ptr->Playtime() );
			}
			Ptr = CLICKNOTE(Ptr->GetNext(-1));
		} // while
	} // if
	/*
	else
	{
		printf("\n");
	}
	*/
} // ShiftRealtimes
//--------------------------------------------------------------------
#ifdef _OLD_TEMPODETECTION
not used anymore
/*
	find threshold for velocity of emphasized notes

	remark:
		works only if notes at the 1 of a bar have been stressed 
		during recordinh
*/
int TCLICKTRACK::GetBeatIntens( void )
{
	TCLICKNOTE *Temp;
	long  sum;
	int  count,
		  value,			// intens of current note
		  Maximum,		// max velocity
		  Minimum,		// min velocity
		  average;	// average

	sum = 0;
	count = 0;
	Maximum = 0;
	Minimum = 127;
	Temp = FirstNote();
	while( Temp )	// create sum of all intens values, find min and max
	{
		count++;
		value = Temp->getIntens();
		sum += value;
		if( value > Maximum )
			Maximum = value;
		else if( value < Minimum )
			Minimum = value;

		Temp = CLICKNOTE(Temp->GetNext(-1));
	} // while
	average = BEATINTENS;	// Default value
	if( count )
	{
		average = (sum / count);	
	} // if
	// note with max intens must be emphasized
	// = 30 % over average
// todo make stress intens a option in .ini
	average += ((Maximum - average) / 3);

	return average;
} // GetBeatIntens
#endif

void TCLICKTRACK::Debug(void){
	stringstream ss; 
	ofstream out;
	out.open("_clicktrack.txt");
	Debug(out);
	out.close();
}

void TCLICKTRACK::Debug( ostream &out,
 						char force )
{
#ifdef _DEBUG
	force = 1;
	Printf("Debug clicktrack \n");
#endif
	if( force )
	{
	TCLICKNOTE *temp;
	temp = Notes;


	out << "ClickTrack\n";
	
	out << mParent->filename();
	out << "\nRTempo: "<< RecTempoI <<", RPPQ "<<RecPPQI <<"\n";
	out << "cInsertNotes: "<< cInsertNotes<<endl;
	out <<  endl;
	out << "format= ticktime;scoreduration;scoreattack;#;perfIOIratio;scIOIraio;pattern;\n"; 
	TFrac cDuration;
	while( temp )
	{
		temp->Debug(out, this);
		if( temp->scoreDuration() > 0 )
			out <<  "\tt=" << tempo(temp) << " ";
		if( temp->prelScDuration > 0 )
		{
			out <<  temp->prelScDuration.numerator() << "," <<
				temp->prelScDuration.denominator() ;
//			temp->prelScDuration.Write(out);
			cDuration = temp->prelScDuration;
		}
		else if( validIOIratio(temp->patScoreIOIratio) )
		{
			if( temp->patScoreIOIratio >= 1 )
				cDuration *= temp->patScoreIOIratio;
			else
				cDuration *= -1/temp->patScoreIOIratio;
		}
		else
		{
			cDuration = 0L;
		}
		if( temp->scoreDurationI > 0L )
			cDuration = temp->scoreDurationI;

		TCLICKNOTE *n;
		n = CLICKNOTE(temp ->GetNext(-1));
		if( n && cDuration > 0L)
		{
			TFrac targetDur;
			targetDur = n->testScorePos() - temp->testScorePos();
			if( targetDur != cDuration &&
				targetDur > 0L )
			{
				out <<  "***error***";
			}
		} // if n
		out <<  "\n";
		temp = CLICKNOTE(temp ->GetNext(-1));
	} // if

	} // if force
} // Write






/*
	remove double clicks with equal attackpoints
*/
void TCLICKTRACK::mergeEqual(int recPPQ, int recTempo)
{
	TCLICKNOTE *current;
	current = FirstNote();
	Tail = current;

	while( current )
	{
		current->mergeEqual(recPPQ, recTempo, this);
		Tail = current;
		current = CLICKNOTE(current->GetNext(-1));
	}
} // Check


double TCLICKTRACK::tempo(TCLICKNOTE *ptr)
{
	if( ptr )
		return ptr->tempo(RecPPQI, RecTempoI);
	else
		return -1;
}

void TCLICKTRACK::deleteLastClick( void )
{
	TCLICKNOTE *prev; 
//			   *current;


	/*
	current = FirstNote();
	// search for end of list 
	while( current->Next() )
		current = current->Next();
	*/
	if( !Tail )
		return;


	prev = CLICKNOTE(Tail->GetPrev(-1));
	if( prev ) // bugfix, delete Tail only if there is a previous click, 28.7.2011
	{
		prev->SetNext( NULL, this);
		delete Tail;
		Tail = prev;
	}
}

/*
	remove unsed click with realtime = -1
*/
void TCLICKTRACK::reorganize( void )
{
	TCLICKNOTE *temp,
			   *temp2;

	temp = Notes;


	
	
	while( temp )
	{
		if( temp->Playtime() < 0L )
		{
			temp2 = CLICKNOTE(temp->GetNext(-1));

			// unlink temp from list
			if( CLICKNOTE(temp->GetPrev(-1)) )
			{
				CLICKNOTE(temp->GetPrev(-1))->SetNext( temp2, this );
			}
			else
			{
				Notes = temp2;
				if( temp2 )
					temp2->SetPrev( NULL );
			}
			
			delete temp;
			temp = temp2;
		}
		else
		{
			TMusicalObject *a = temp->GetNext(-1);
			temp = CLICKNOTE(a);
		}
	}



}


void TCLICKTRACK::normScoreTimes( void )
{
	TAbsTime offset;


	if( !Notes )
		return;

	TFrac scoreTime;

	TCLICKNOTE *temp;
	temp = FirstNote();
	while( temp )
	{
		temp->setScoretime( scoreTime );
		scoreTime += temp->scoreDuration();
		temp = temp->getNext(1,-1);
	}
/*
	offset = Notes->scoretime() * -1;
	shiftScoretimes(Notes, NULL, offset );
*/
}


/*!
	return threshhold for weight so that only "percent"% of all beats 
	are above the returned threshold
*/
double TCLICKTRACK::getThreshold(TCLICKNOTE *from,
                                 TCLICKNOTE *to,
                                 double percent /*[0...1)*/)
{
	double threshold = 0.5, // start with 0.5
		  rate,	// #cOver/cClicks
		  dMin = 0,
		  dMax = 1;

	int cOver = 0,  // # clicks > tresh
		cClicks = 0; // #clicks
	int end = 0;

	threshold = dMin + (dMax-dMin)/2;

	while( !end )
	{
		TCLICKNOTE *Temp;
		Temp = from;
		if( !Temp ) 
		{
			end = 1;
			threshold = 0;
		}
		else // go trough list
		{
			cClicks = 0;
			cOver = 0;
			while( Temp &&	// create sum of all intens values, find min and max
                   Temp != to )
            {
				cClicks ++;
				if( Temp->weight(this) > threshold )
					cOver++;
				TMusicalObject *t;
				t = Temp->GetNext(-1);
				Temp = CLICKNOTE(t);
			} // while

			rate = (double)cOver/(double)cClicks;
			if( rate > percent )
				// threshold is to low
			{
				dMin = threshold;
				threshold = dMin + (dMax-dMin)/2;
			}
			else // threshold too high
			{
				dMax = threshold;
				threshold = dMax - (dMax-dMin)/2;
			}
			if( fabs(dMax-dMin)<0.01 )
				end = 1;
		} // else
		
	} // while
	return threshold;
}


void TCLICKTRACK::detach(TCLICKNOTE *ptr)
/*
	remove ptr from clicktrack and delete ptr
*/
{
	if( !ptr )
		return;

	if( ptr == Tail || 
		ptr == Notes )
	{
		// update list ptrs
		if( ptr == Notes )
		{
			Notes = CLICKNOTE(ptr->GetNext(-1));
			if( Notes )
				Notes->SetPrev(NULL);
		}
		if( ptr == Tail )
		{
			Tail = CLICKNOTE(ptr->GetPrev(-1));
			if( Tail )
				Tail->SetNext(NULL, this);
		}
	}
	else // inside list
	{
		// remove from list
		TCLICKNOTE *prev;
		prev = CLICKNOTE(ptr->GetPrev(-1));
		//		if( prev )
		prev->SetNext( dynamic_cast<TCLICKNOTE *>(ptr->GetNext(-1)), this);
	}
	
	delete ptr;
}

/*!
search for a note with high accuracy  and heigh weight
and zero score duration (-> unprocessed)

*/
TCLICKNOTE *TCLICKTRACK::startNote( TCLICKNOTE *from,
								   TCLICKNOTE *to,
								   int direction )
{
    if( !from )
        return NULL;

    double smallestDelta = 10,
        delta,	// delta to next integer
		delta2,	// delta to 1
        curIOI;
    TCLICKNOTE *bestClick = NULL;


	double threshold;

	// get a level for the 20% best notes
	threshold = getThreshold(from,
                          to,
                          0.2);

	char strongNoteFound = 0;

	double intIOI;
	while( from &&
           from != to)
	{	
		if( from->getNext(direction,-1) )
		{
			curIOI = from->IOIratio(-1,1);
			if( curIOI > 0 )
				intIOI = (int)(curIOI + 0.5);
			else
				intIOI = (int)(curIOI - 0.5);

/*			
			if( from->scoreDuration() == 0L )
				return from;
*/
		
			// evaluate only if strong note or nothing seen until now
			if( // (  from->weight() > threshold 
//				|| !strongNoteFound) 
				from->scoreDuration() == 0L )	// unly unprocessed notes
			{
				if( from->weight(this) > threshold )
					strongNoteFound = 1;


				delta = fabs(curIOI - intIOI);
				if( delta > 0.5 )
					delta = 1 - delta;

				if( curIOI < 0 ||
					curIOI > 0 )
					delta2 = 1 - fabs(1 / curIOI);
				else
					delta2 = 1;


				delta *= 2;
				delta = delta + (1-delta)*(1-from->weight(this,0.1,0.1,0.8));
				delta = delta + (1-delta)*delta2;
/*
				if( curIOI == 0 ) // begin/end of list
					delta = 1;
*/
				if( delta < smallestDelta )
					{
						smallestDelta = delta;
						bestClick = from;
					}
			} // if 
		} // if next
		else // single note
		{
			if( from->scoreDuration() == 0L &&
				!bestClick)
				bestClick = from;
		}

        
        from = from->getNext(direction,-1);            
    }
	// start with note before bestClick
	if( bestClick )
	{
		TCLICKNOTE *prev;
		prev = bestClick->getNext(-1,-1);
		if( prev &&
			prev->scoreDuration() == 0L )
		{
			bestClick = prev;
		}
	}
    return bestClick;
}




#ifdef IMUTUS_DLL
#undef Tempo

#endif
#define MAXINTENS 127




//todo use CreateAttackTrack for CreateClickTrack
/*
		Copy all Attackpoints from file into a TCLICKTRACK
*/
TCLICKTRACK *CreateAttackTrack( THMIDIFILE *file)
{
		TQTRACK *tempTrack;
		TNOTE   *Temp;
		TCLICKNOTE *Click;

		// create new CLicktrack
		TCLICKTRACK *ClickTrack = new TCLICKTRACK( (long) 0, file );
		if( !ClickTrack )
			return 0;

		// init values
		ClickTrack->SetRecValues( file->Tempo(),
										  file->Ppq() );

		// copy all Attackpoints
		tempTrack = (TQTRACK*)(file->FirstTrack());
		while( tempTrack )
		{
				Temp  = tempTrack->FirstNoteObject(-1);
				while( Temp )
				{
//					if( Temp->GetDuration() > 0) // only real notes
					if( Temp )
					{
						Click = new TCLICKNOTE( Temp->GetAbsTime().toLong(), // play time
														0L,						  // piece time
														Temp->getIntens(),
//														Temp->GetDuration().toLong(), //weight
														Temp->GetDuration() );
						ClickTrack->Insert( Click );	// Insert into Track
					}
					Temp  = NOTE(Temp->GetNext(-1));
				}
				// For all tracks
				tempTrack = (TQTRACK *)tempTrack->Next();
		} // for
		return ClickTrack;
}

/*
	Calculate limit for recognition of intens-stressed notes
*/
double StressIntens( TCLICKTRACK *AttackTrack )
{
	int tempIntens;
	double sumIntens = 0,
			 stressIntens;

	int cNotes = 0;
	if( !AttackTrack )
		return 0;

	// Calculate average intens
	TCLICKNOTE *temp;
	temp = AttackTrack->FirstNote();
	while( temp )
	{
		tempIntens = temp->Intens();
		if( tempIntens )
		{
			sumIntens += tempIntens;
			cNotes++;
		}
		temp = CLICKNOTE(temp->GetNext(-1));
	}
	if( cNotes )
		sumIntens /= (double)cNotes;

	// calculate stress limit
	stressIntens = sumIntens + ((MAXINTENS-sumIntens)/2);


	return stressIntens;
}

//todo use FilterSTressedIOI for Anchor detection?
/*
	keep stressed Attackpoints in attacklist,
	remove unstressed notes

	stressed :
		- IOI > IOI(prev)
		- Intens > STressIntens

	remarks:
		- CLickNotes at same attackpoint must be merged before!! -> IOI == 0
*/
TCLICKTRACK *FilterStressedIOI( TCLICKTRACK *AttackTrack )
{
	TCLICKNOTE *temp,
			*tempNext,
			*prev;

	TAbsTime IOI,
				prevIOI = 0L;

	double stressIntens,
			 curIntens;

	stressIntens = StressIntens( AttackTrack );

	temp = AttackTrack->FirstNote();
	prev = NULL;
	while( temp )
	{
		tempNext = CLICKNOTE(temp->GetNext(-1));
		curIntens = temp->Intens();

		// calculate IOI
		if( tempNext )
		{
			IOI = tempNext->Playtime() -
					temp->Playtime();
		}
		else
		{
			IOI = MAXINT; // don't remove
		}

		if( (curIntens < stressIntens ) ||
			 IOI < prevIOI )
		// remove from Track
		{
			if( prev )
			{
				prev->SetNext( tempNext , AttackTrack);
				delete temp;
				temp = tempNext;
			}
			else
			{
				AttackTrack->SetFirstNote( tempNext );
			}
		}
		prev = temp;
		prevIOI = IOI;
		if( temp )
			temp = CLICKNOTE(temp->GetNext(-1));
	}

	return AttackTrack;
}

/*
int TCLICKTRACK::accuracy(double &attack, double &duration)
{
		TDoubleBinList *statList;
		
		TDoubleBinClass *equalClass, 
				*doubleClass,
				*halfClass;
		
		statList = new TDoubleBinList(95,105, 20);
		
		// fill list with search values (rIOI - 1 !!!)
		equalClass = statList->addClass(0);      // == equal IOI	
		doubleClass = statList->addClass(1);	    // 1/4-1/8
		//	statList->addClass(1.3333);	// 1/12 - 1/16 
		statList->addClass(1.5);		// 1/8 - 1/12, 3/16-1/8
		statList->addClass(2);		// 3/16-1/16
		statList->addClass(2.5);		// 3/16-1/16
		statList->addClass(3);   	// 1/1-1/4
		statList->addClass(3.5);		// 3/16-1/16
		statList->addClass(4);		// 5/8-1/8
		statList->addClass(5);		// 6/8-1/8
		statList->addClass(5.333);	// 1/1-1
		statList->addClass(6);
		statList->addClass(7);
		statList->addClass(8);
		statList->addClass(11);
		
		//	statList->addClass(-0.3333);	// 3/16 - 1/16
		halfClass = statList->addClass(-1);
		//	statList->addClass(-1.3333);		// 1/12 - 1/16, 
		statList->addClass(-1.5);		// 1/8 - 1/12  
		statList->addClass(-2);
		statList->addClass(-2.5);		// 3/16-1/16
		statList->addClass(-3);
		statList->addClass(-3.5);		// 3/16-1/16
		statList->addClass(-4);
		statList->addClass(-5);
		statList->addClass(-5.333);
		statList->addClass(-6);
		statList->addClass(-7);
		statList->addClass(-8); // for invent13b
		
		
		statList->resetMinWeight(0.01);		  
		statList->resetMaxWeight(1);		  
		statList->setDelta( 0.001 );

		TCLICKNOTE *cur;
		cur = FirstNote();
		double plIOIRatio;
		int cEvents = 0;
		while( cur )
		{
			plIOIRatio = cur->IOIratio(-1,1);
			if( plIOIRatio > 0 )
			{		
				cEvents++;			
				statList->addValue(plIOIRatio-1);
			}
			else if(plIOIRatio < 0)
			{		
				cEvents++;
				statList->addValue(plIOIRatio+1);
			}
			
			 cur = cur->getNext(1,-1);
		}
	
			
//			statList->write( stdout );

			char isMachinePlayed = 3;
			if( equalClass->Count() &&
				(equalClass->lVariance() != 0 ||
				equalClass->rVariance() != 0) )
			{
				isMachinePlayed--;
				cEvents += equalClass->Count();
			}
			if( halfClass->Count() &&
				(halfClass->lVariance() != 0 ||
				halfClass->rVariance() != 0) )
			{
				isMachinePlayed--;
				cEvents += halfClass->Count();
			}
			else if( !halfClass->Count() )
			{
				isMachinePlayed--;
			}
			
			if( doubleClass->Count() &&
				(doubleClass->lVariance() != 0 ||
				doubleClass->rVariance() != 0) )
			{
				isMachinePlayed--;
				cEvents += doubleClass->Count();
			}			
			else if( !doubleClass->Count() )
			{
				isMachinePlayed--;
			}
		if( isMachinePlayed )
		{
			attack = 1;
			duration = 1;
		}
		else
		{
			// Todo: estimate usefull values
		}
	return cEvents;
}	
*/

/// remove all insiginficant click notes
void TCLICKTRACK::filterClicks(double )
{
	// estimate distribution
	int count = 0;
	double av = 0;
	TCLICKNOTE *current = FirstNote();
	// calculate mean
	while( current )
	{
		av += current->weight(this);
		count++;
		current = current->getNext(1,-1);
	}
	// can't do anything
	if( count < 2 )
		return;

	av /= count;
	// calculate std dev
	double var = 0;
	current = FirstNote();
	while( current )
	{
		var += pow(current->weight(this) - av,2);
		current = current->getNext(1,-1);
	}
	var /= count-1;

	double std = pow( var, 0.5 );



	// count filtered clicks
	int cFilter = 0;
	current = FirstNote();
	// don't filter first click!
	if( current )
		current = current->getNext(1,-1);
	TCLICKNOTE *ptr = NULL;

	while( current )
	{

		double dAcc = current->durationalAccent();
		// filter short, unaccented notes
		if( dAcc <= -0.8 )
		{
//			dAcc = current->durationalAccent();
			long int cIOI =  current->ioi(1,-1).toLong();
			double ioiMS = DTIMEtoMS( mParent->RecPPQ,
								mParent->RecTempo,
							   cIOI );
			if( ioiMS >= 0 &&
				ioiMS < 250 ) // filter short notes
			{
				cFilter++;
				TCLICKNOTE *nClick = current->getNext(1,-1);
				if( nClick )
				{
					// filter note with smaller wieght
					if( nClick->weight(this) < current->weight(this) )
					{
						ptr = nClick;
					}
					else
					{
						ptr = current;
					}

					#ifdef _DEBUG
					double IOIratio = ptr->plRIOI(-1,0);
					IOIratio = ptr->plRIOI(1,0);
					current->durationalAccent();
					#endif
				}
			}
		} // if durational accent

		if( ptr == current )
		{
			// filter current note
			current = current->getNext(1,-1);
		}
		else if( ptr )
		{
			// filter nextNote and stay at current
		}
		else
		{
			current = current->getNext(1,-1);
		}
		if( ptr )
			detach( ptr );
		ptr = NULL;
	}

return; //-----------------------------------

	while( current &&
		   current->getNext(1,-1) )	// don't remove last click
	{
		/* double p = normal_pdf( current->weight(),
						av,
						var ); */
		// fprintf(out, "%f,%f\n", current->weight(),p);
		/*
		if( current->weight() < av  &&
			p < alpha )
		*/
		if( current->weight(this) < av - std )
		{
			cFilter++;
			ptr = current;
		}
		current = current->getNext(1,-1);
		if( ptr )
			detach( ptr );
		ptr = NULL;
	}
	// now look for very small rIOI's
	/// try to filter single, clicknotes with very small IOIratio's ans low weights
	current = FirstNote();
	TCLICKNOTE *prev = NULL,
		       *next;
	int direction;
	for( int i = 0; i < 1; i++ )
	{
		if( i == 0 )
			direction = dirforward;
		else
			direction = backward;
		int nSkip = 0;	
		while( current )
		{				
			if( (direction == dirforward && current->IOIratio(-1,1 ) < -9) ||
			     (direction == backward && current->IOIratio(1,2 ) < -9) )
			{
				// prev note-duretion >> current->duration
				next = current->getNext(direction,-1);
				if( next )
				{
					double nIOI;
					if( direction == dirforward )
						nIOI = next->IOIratio(-1,1);
					else
						nIOI = next->IOIratio(1,2);
						
					if(	nIOI > 2 &&
						next->weight(this) > current->weight(this) )
					{
						// current is a single short note
						detach(current);
						current = prev;
						nSkip++;
						cFilter++;
					}
					else if( nSkip < 2 && 0)
					{
						// don't skip more than 1-2 notes!
						// try to skip notes after current
						detach(next);
						current = prev;
						nSkip++;
						cFilter++;
					}
				}
				else // > -9
				{
					// current is last note in list, keep
				}
			}
			else
			{
				nSkip = 0;
			}
			prev = current;
			current = current->getNext(direction,-1);
		}// while
	} // for


//	fclose(out);
//	Write();


}


double hit( TAbsTime &scorePos,
             TAbsTime &grid )
{
    TAbsTime delta;
    delta = scorePos % grid;
    if( delta == 0L )
        return 1;
    else
        return 0;
}

/// get optimised offset -> optimse weights of notes at siginificant click positions, works only to right
/// -> try to minimise the attackpoints denominator sum!
/// -> try to make all attackpoint.denoms <= duration denoms
/// try to shift so that 
/*
	dur_denom % attack_denom == 0 -> ok
	dur_denom % 3 != 0 && dur_denom % 2 == 0 -> attack_denom % 2 == 0 -> ok
	
	max_Prime | dur_denom % max_Prime == 0 -> attack % maxPrime == 0 -> ok
*/
TAbsTime TCLICKTRACK::optimumOffset( TCLICKNOTE *from,
                        TCLICKNOTE *to,
                        /// significance = n*bestPos
                        TAbsTime &bestPos,
                        /// size of testing grid
                        TAbsTime &gridResolution,
						/// current quality
						double &oldQuality,
						/// quality after shift
						double &newQuality )
{


    TAbsTime bestOffset;

	if( !from )
		return bestOffset;

	newQuality = 1;
	oldQuality = 1;


	TCLICKNOTE *prev = from->getNext(-1,-1);
	   
	// don't allow large step shifts
	if( gridResolution.toDouble() >= 0.25 )
		return bestOffset;



	if( gridResolution <= 0L )
		gridResolution = bestPos / 2;

	double perfIOIratioFrom = from->IOIratio(-1,1);
	double perfIOIratioPrev = 0;
    TFrac curOffset;
	if( prev )
	{
		perfIOIratioPrev = prev->IOIratio(-1,1);

		/* scoreDuration might != n*gridResolution */
		// start at 
		TFrac minCurOffset = prev->scoreDuration() * TFrac(-1,2);
		curOffset = 0L;
		while( curOffset - gridResolution >= minCurOffset )
		{
			curOffset -= gridResolution;
		}
	}// if prev
	
	if( validIOIratio(perfIOIratioPrev) &&
		validIOIratio(perfIOIratioFrom) )
	{
		perfIOIratioPrev = normIOIratio(perfIOIratioPrev);
		perfIOIratioFrom = normIOIratio(perfIOIratioFrom);	
	}
	
	
	
    double bestQuality = 0;
    double bestPIOIratio = 0;
	// double bestDenom = bestPos.denominator();
	// try quality of different shift positions
	// loop through offset positions
    while( curOffset <= bestPos )
    {
		// loop trough segment
        TCLICKNOTE *temp = from;
		double primeDistSum = 0;
		// # of clicknotes
        int count = 0;
        // # of perf notes attached to clicknotes
        int cn = 0;
        while( temp &&
               temp != to )
        {
            count++; 
            cn += temp->cNotes;
            TAbsTime offsetPos = temp->scoretime()+curOffset;
            TFrac scDur = temp->scoreDuration();
            
            // evaluate weight of current note?
            /*
            	maybe not: a syncopation on a main note is ok, if
            	all minor notes are on good positions
            */
            primeDistSum += primeDenomDistance(offsetPos.denominator(),
            								scDur.denominator() );
            
            temp = temp->getNext(1,-1);
        } // while
		if( count )
		{
			primeDistSum /= count;
		}

		// check changes in prev and in first note
		// IOIratio qualtiy 1 == good
		double pIOIratio = 1;
		if( prev )
		{

			TFrac oldScoreDuration = prev->scoreDuration();
			prev->scoreDurationI = oldScoreDuration + curOffset;
			double scIOIratioPrev = prev->relScoreIOI(-1,1);
			double scIOIratioFrom = from->relScoreIOI(-1,1);
			// reset prev
			prev->scoreDurationI = oldScoreDuration;
						
			if( validIOIratio(scIOIratioPrev) &&
			    validIOIratio(scIOIratioFrom) )
			{
				scIOIratioPrev = normIOIratio(scIOIratioPrev);
				scIOIratioFrom = normIOIratio(scIOIratioFrom);
				
				// get distance
				double pIOIratioPrev = GaussWindow(perfIOIratioPrev, 
								    			   scIOIratioPrev,
													0.3);
				double pIOIratioFrom = GaussWindow(perfIOIratioFrom, 
								    			   scIOIratioFrom,
													0.3);
				pIOIratio = pIOIratioPrev * pIOIratioFrom;
			} // if valid				
		} // if prev

		double curQuality = (1-primeDistSum)*0.8 + pIOIratio*0.2;
		if( curQuality > bestQuality ||
		    (curQuality == bestQuality &&
				 pIOIratio > bestPIOIratio)
			)
		{
			bestQuality = curQuality;
			bestOffset = curOffset;
			bestPIOIratio = pIOIratio;
		}
        curOffset += gridResolution;
    } // while
	newQuality = bestQuality;
    return bestOffset;
}

void TCLICKTRACK::markStartStop(TMusObjList *startList, TMusObjList *stopList)
{

	TCLICKNOTE *temp = FirstNote();
	while( temp )
	{
		// check if in startlist or in stop list
		for( int i= 0; i < startList->count(); i++ )
		{
			if( temp == startList->get(i) &&
				temp == stopList->get(i) )
			{
				temp->startId = -10;	// single note
				i = startList->count(); // stop loop
			}
			else if( temp == startList->get(i) )
			{
				temp->startId = -11; // start
			}
			else if( temp == stopList->get(i) )
			{
				temp->startId = -12; // stop
			}

		}
		temp = temp->getNext(1,-1);
	}
}

double TCLICKTRACK::avTempo(TCLICKNOTE *from, TCLICKNOTE *to)
{
	double res = 0;
	int count = 0;
	while( from &&
		   from != to )
	{
		count++;
		res += tempo(from);
		from = from->getNext(1,-1);
	}
	if( count )
		res /= count;

	return res;
}

/// analysis function for cemgil files
double TCLICKTRACK::analyseQuality(const string &fname)
{


	double sim1 = 0; // IOI sim
	double sim2 = 0; /// pos sim
	TCLICKNOTE *current;
	current = FirstNote();

	int count = 0;
	int count2 = 0;
	double correctIOI= 0;
	double falseIOI=0;
	while( current )
	{
		double estIOI,
			   origIOI;

		double estPos,
			   origPos;
		estIOI = current->scoreIOI(1).toDouble();
		origIOI = current->testScorePos().toDouble();

		estPos = current->scoretime().toDouble();
		origPos = current->testScorePos().toDouble();
		double posDelta;
		/// calculate abs position similarity
		posDelta = estPos - origPos;
		posDelta = exp(-pow(posDelta,2)/(2*acos(-1.0)*pow(0.0625,2)) );
		sim2 += posDelta;
		count2++;

		/// calc ioi similarity
		current = current->getNext(1,-1);
		if( current )
		{
			count++;
			origIOI = current->testScorePos().toDouble() - origIOI;
			double delta;

			delta = origIOI - estIOI;
			if( delta < 0 ||
				delta > 0 )
			{
				falseIOI++;
			}
			else
			{
				correctIOI++;
			}
			delta = exp(-pow(delta,2)/(2*acos(-1.0)*pow(0.0625,2)) );
			sim1 += delta;
		}
	}
	if( count )
		sim1 /= count;
	if( count2 )
		sim2 /= count2;
	cout << "   Clicktrack similarity: " << sim1 << "/" <<  sim2 << endl;
	// write to analysis file
	FILE *out;
	out = fopen("_ctrackAnalyse.csv","at");
	if( out )
	{

		fprintf(out,"\"%s\" ",fname.c_str());
		fprintf(out,"%f ",sim1);
		fprintf(out,"%f ",sim2);
		fprintf(out,"%f\n", correctIOI /(falseIOI+correctIOI));
		fclose( out );
		if( sim2 < 0.5 )
		{
			stringstream dummy;
			Debug(dummy);
		}
	}
	else
	{
		Printf("WARNING: can't open clicktrack analysis file!\n");
	}
	return sim1;
}



int TCLICKTRACK::fixPrimeDistanceErrors(TCLICKNOTE *from, 
										TCLICKNOTE *to)
{
	char mode = 0; // verbose
	if( strcmp(mParent->getInifile()->GetValChar("MODE",
										"INTERACTIVE",
										"[INTERACTIVE|SILENT] use SILENT for batch processing "),
						"INTERACTIVE") )
	{
		// mode = 1; // silent -> autofix
		return 0;
	}
		
	
	int cFixError = 0;
	double syncopationSum = 0;
	TCLICKNOTE *temp = from;
	while( temp &&
			temp != to )
	{
		double cDistance =  primeDenomDistance(temp->scoretime().denominator(),
											   temp->scoreDuration().denominator());

		char askUser = 0;
		if( cDistance >= 1 ) // really impossible position 
		{
			syncopationSum = 0;			
		}
		else if( cDistance > 0 )
		{
			syncopationSum = syncopationSum + cDistance - syncopationSum*cDistance;
		}
		else // cDistance == 0
		{
			syncopationSum = 0;
		}
		if( syncopationSum > 0.85 )
		{
			askUser = 1;
		}

		if( askUser )
		{
			askUser = 0;

			// now we can change duration of temp or duration of prev
			syncopationSum = 0;			
			cFixError++;

			TFrac curScDuration = TFrac(0,0);
			long curScAttack = 0L;			
			if( !mode ) // interactive
			{
				// fix error for current note
				ostringstream  prompt;
				prompt << "Enter correction (pos:dur) for durPos error at ";
				prompt << temp->Playtime();
				prompt << "\n";
			
				ostringstream def;
				def << temp->Playtime();
				def << ":";
				def << temp->scoreDurationI.toString();
				ofstream ssOut;
				ssOut.open("_clicktrack.txt");
				Debug(ssOut, 1 /* force */);
				ssOut.close();

				string answ = InputQuestion(prompt.str().c_str(),
							  def.str().c_str(),
							  NULL	);
			
				char *answString = new char[strlen(answ.c_str())+1];
				strcpy( answString, answ.c_str() );
			
				char *durStr = strstr(answString,":");
				if( durStr )
					durStr++;			
				curScDuration = TFrac(durStr);
				curScAttack = saveAtol(answString);
				
				delete [] answString;
			}
			else // try auto fix
			{
				// if some of the following notes show also
				// primeDenomerrors -> change duration of prev note
				// else do nothing
				TCLICKNOTE *nTest = temp->getNext(1,-1);
				int c = 0;
				while( nTest &&
					   primeDenomDistance(nTest->scoretime().denominator(),
					   				   nTest->scoreDuration().denominator() > 0 ) )
				{
					nTest = nTest->getNext(1,-1);
					c++;
				}
				if( !nTest || c > 4 ) // try to fix
				{
					
					TCLICKNOTE *pNote = temp->getNext(-1,-1);
					curScAttack = pNote->Playtime();
					
					TFrac eigth(1,8),
					      eTriplet(1,12);
					if( temp->scoreDuration().denominator() % 3 == 0 )
					{
						if( pNote &&
							pNote->scoreDuration() == eigth )
						{
							curScDuration = eTriplet;
						}
					}
					else if( temp->scoreDuration().denominator() % 3 != 0)
					{
						if( pNote &&
							pNote->scoreDuration() == eTriplet )
						{
							curScDuration = eigth;
						}
					} 
					if( curScDuration > 0L )
					{
						if( primeDenomDistance(pNote->scoretime().denominator(),
											   curScDuration.denominator() ) > 0)
						{
							curScDuration = 0L; // don't use
						}
						else 
						{
							TFrac nScoretime = pNote->scoretime() + curScDuration;
							if( primeDenomDistance(nScoretime.denominator(),
											       temp->scoreDuration().denominator() ) > 0)
							{
								curScDuration = 0L;
							}
						} 
						if( curScDuration > 0L )
						{
							cout << " auto fixing prime denom error at " <<
								temp->GetAbsTime().numerator() << "/" <<
								temp->GetAbsTime().denominator() << endl;
						}
					}
				} // if nTest				
			}// auto fix 
			// input is valid 
			if( curScDuration > 0L )
			{
				// search for temp2
				TCLICKNOTE *temp2 = temp;
				if( temp2->Playtime() > curScAttack )
				{
					do{
						temp2 = temp2->getNext(-1,-1);
					}
					while( temp2 &&
							temp2->Playtime() > curScAttack );
				}
				else if( temp2->Playtime() < curScAttack )
				{
					do{
						temp2 = temp2->getNext(1,-1);
					}
					while( temp2 &&
							temp2->Playtime() < curScAttack );
				}// else if
				if( (!temp2 && curScAttack > 0L) ||
					(temp2 && temp2->Playtime() != curScAttack) )
				{
					Printf("Error: position not found!");
				}
				else // fix everything after temp2
				{
					// restart
					if( !temp2 )					
					{
						temp = FirstNote();						
					}
					else
					{
						temp = temp2->getNext(1,-1);
					}
						
				 do{
				 	
					 TCLICKNOTE *nClick;
					 if( temp2)
					 	nClick = temp2->getNext(1,-1); 
					 else
					 	nClick = FirstNote();
					 	
					 double nIOIratio = 0;					
					 if( nClick )
					 {
						nIOIratio = nClick->relScoreIOI(-1,1);
					 }
					 TFrac delta;
					 // shift position of first note
					 if( temp2 )
					 	delta = curScDuration - temp2->scoreDurationI;
					 else
					 	delta = curScDuration - nClick->scoretime();
					
					 if( delta != 0L )
					 {					
					 	// change temp2 duration
					 	if( temp2 )
						 	temp2->scoreDurationI = curScDuration;
						 // update scoreDuration
						 if( nIOIratio > 0 )
					 		curScDuration *= TFrac((long)(nIOIratio*960+0.5),960L);
						 else if( nIOIratio < 0 )
							 curScDuration /= TFrac((long)(-nIOIratio*960+0.5),960L);				
						 	temp2 = nClick;
							 shiftScoretimes( temp2,
											 NULL,
											 delta);
					}
					else
					{
						temp2 = NULL;
					}
					}while(temp2 &&
							temp2->patternFinalID < 0 ); 				
				} // else
			}// if scoreDUration was entered			
		} // if error
		temp = CLICKNOTE(temp->GetNext(-1));
	} // while
	
	return cFixError;
}

void TCLICKTRACK::Debug(string fname)
{
	FILE *out = fopen(fname.c_str(),"wt");
	if( out )
	{
		stringstream ss;
		Debug(ss,1);
		fprintf(out, "%s", ss.str().c_str());
		fclose(out);
	}
}
