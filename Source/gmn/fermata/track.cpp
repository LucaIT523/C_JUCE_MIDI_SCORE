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
|	filename : TRACK.CPP
|	author     : Juergen Kilian
|	date	    : 17.10.1996-98/06, 2011
|	implementation of TTRACK
------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>

#include "portable.h"
//---------------------------------

#include <stdio.h>
#include <string.h>
#include <math.h> // for pow

#include "event.h"
#include "track.h"
#include "funcs.h"
#include "q_note.h"
#include "../lib_src/ini/ini.h"

#include "meta_tempo.h"
#include "meta_text.h"
#include "meta_key.h"
#include "meta_meter.h"

#include "ornament.h"
#include "chunklist.h"

//-----------------------------------------------------------------------
/* #Notes at one line during conversion */
#define MAXNOTEINLINE 6
#define MAX_EVENT_IN_LINE 3
//----------------------------------------------------------------------
void TTRACK::Debug( FILE *out )
{
#ifdef _DEBUG__
	char closeOut = 0;
	if( !out )
	{
		out = fopen("_ttrack.txt","wt");
		closeOut = 1;
	}
	Printf("TTRACK Debug()\n");
	fprintf(out,"Track: %d\n",TrackNumber());
	int startVoice,
		stopVoice;	
	if( cVoice() > 1 )
	{
		stopVoice = cVoice();
		startVoice = 0;
	}
	else
	{
		startVoice = -1;
		stopVoice = 0;
	}
	for( int v = startVoice; v < stopVoice; v++ )
	{
		int count = 0;
		fprintf(out, "Voice :%d-----------\n", v );
		TMusicalObject *temp = TTRACK::FirstObject(v);
		while( temp )
		{
			fprintf(out, "%d ", count++);
			temp->Debug(out);
			temp = temp->TMusicalObject::GetNext(v);
		}
	}
	if( closeOut )
	{
		fclose( out );
	}
#endif
}

TTRACK::TTRACK( long offset, TMIDIFILE *parent_ )
{
	voiceNames   = NULL;
	trackName   = string();
	Offset      = offset;
	Notes       = NULL;
	CurrentNote = NULL;
	nextI 		= NULL;
	TrackNumberI = -1;
	ChannelI = 0;
	CVoice = 0;
	curTempo = NULL;
	parent = parent_;
} // TTRACK
//----------------------------------------------------------------------
TTRACK::~TTRACK( void )
{
	deleteNotes(FirstObject(-1),	// from
		NULL);  // to
	trackName = "";
	if( voiceNames )
		delete [] voiceNames;
} // ~Track
//----------------------------------------------------------------------
void TTRACK::append( TTRACK *next )
{
	TTRACK *tempTrack = Next();
	// search for last element in list
	while( tempTrack &&
		tempTrack->Next() )
	{
		tempTrack = tempTrack->Next();
	}
	
	if( tempTrack )
	{
		tempTrack->SetNext( next );
	}
	else
	{
		SetNext( next );
	}
}
//----------------------------------------------------------------------
void TTRACK::Insert( TTRACK *next )
{
	if( next )
	{
		next->SetNext( nextI );
		nextI = next;
	}
}
//----------------------------------------------------------------------
/*
Read_Header
read trackheader from file
result :
start offset for next track
*/
long TTRACK::Read_Header( FILE *file )
{
	if( !file )
		return 0;
	// preliminary dumyy offset, will be corrected during TTRACK::Read
	if( Offset < 0 )
		return 1;
		
	fseek( file, Offset, 0);	// set file ptr
	fpos_t 	pos;
	fgetpos( file, &pos );
	
	unsigned char 	temp[5];
//	char temp[5];
	temp[4] = 0;
	fread( temp, 4, 1, file );  	// read 'MTrk'
	if( strcmp( (char *)temp, "MTrk") != 0)
	{
		cout <<"Warning: No Trackstart \"MTrk\" Event at Track " << TrackNumber() <<endl;
		ErrorMsg( 20 );
		return 0L; // error
	}

	fread( temp, 4,1 , file ); 	// 4 Byte for tracklen
	long DLength  = 0;
	for( int i = 0; i< 4; i++ )
	{
		DLength *= 256;
		DLength += temp[i];
	}	
	return  DLength + Offset + 8;
} // Read_THeader
//----------------------------------------------------------------------
/*!
channel <= 0 -> Read events of all channels
channel >0 -> read events of only one channel
result :
1 : ok
0 : error
read all events and create a list of TNOTE

  -tempo change events of Track0 will be copied to all other tracks
*/
TTrackInfo TTRACK::Read( FILE *file,
						int  /* relduration */,
						int  tempoFlag,
						FILE *log,
						int ppq,
						TTRACK *Track0,
						int currentTrack,
						int channel,  //0 == off, 1-16
						TMIDIFILE *Parent,
						lgSequence *curSequence )
{
	TrackNumberI = currentTrack;
	parent = Parent;
	
	TTrackInfo res;
	res.channelCount = 0;

	if( file )
	{	
		if( Offset < 0 )
		{
			fpos_t tempOffset;
			// start at current position
			fgetpos(file, &tempOffset );
			Offset = tempOffset;
			if( !Read_Header(file) )
				return res;			
		}
		fseek(file, Offset + 8, 0 );	// ptr to first event
	}

	
	TNoteOnList *NoteOnTones = new TNoteOnList[128];
	
	// recognice textevents
	char textout = 0;	
	if( !strcmp(Parent->getInifile()->GetValChar("TEXT_OUT","OFF","[ON|OFF] output of MIDI MetaText as \\text "),"ON") )
	{
		textout = 1;
	}
	/*
	
	TMusicalObject  	*Ptr;
	TEVENT 	*Event;
	char	Status = 0;	// Runningstate of MIDI-file
	*/
	// init --------------------------------------------
	int i;
	for( i = 0; i < 16; i++ )
		res.unreadEvents[i] = 0;
	
	if( channel < 1 ) // read all channels in track
		cout << "Reading track: "<< currentTrack << ", all channels ... ";
	else			  // split channels to track
	{
		cout << "Reading track: "<< currentTrack <<", channel: "<< channel<<" ... ";
		ChannelI = channel;
	}
	// init table ------------------------------
	for ( i = 0; i<128; i++ )
	{
		// NoteOnTones[i].Time = 0; 
		NoteOnTones[i].Ptr  = NULL;
		NoteOnTones[i].overhead  = NULL;
	};
	
//	Ptr  = NULL;
	
	if( log )
		fprintf( log, "[%% Start of Track %d, channel %d\n", currentTrack, channel );
	
	// read first event
	char currentChannel = 17;
	// reset Track0
	int currentTempo = DEFAULT_TEMPO;
	TMetaTempo * curTempoEv = NULL;
	
	if( Track0 == this )	// no tempo change copies from Track0 to Track0
		Track0 = NULL;
	
	
	//! search for first SET_TEMPO of MIDIFILE
	if( Track0 ) 
	{
		TMetaTempo *firstTChange = Track0->FirstTempo();
		while( firstTChange &&
			!dynamic_cast<TMetaTempo *>(firstTChange) )
			//         strcmp(firstTChange->ClassName(), TEMPO_CLASS) )
		{
			firstTChange = MetaTempo(firstTChange->GetNext(-1 /* all voices */));
		}
		if(firstTChange)
		{
			curTempoEv = (TMetaTempo *)firstTChange;
			currentTempo = curTempoEv->GetTempo();
		}
	} // if track0
	//-------read all events-----------------------------------------------
	
	int cEv = 0;
	/// current state of sustain pedal
	char sustainState = 0;
	TIntChunkList *sustainList = new TIntChunkList();

	int eventCount = 0;
	long 	AbsTime = 0L;	// current time in file

	TMetaMeter *prevMeter = NULL;
	TEVENT *Event = new TEVENT();
	// get the next event 
	int Status  = 0;
	Status = Event->Read( Status, file, curSequence );
	
	
	while( !Event->EndOfTrack() &&
		   ( (file && !feof( file)) ||
		      curSequence) )
	{
		// Skip events if wrong channel
		
		if( Event->Channel() > 0 // ChannelMessage
			&& channel <= 0)			// no filter
		{
			channel = Event->Channel(); // setFilter
			ChannelI = channel;			// set Channel of Track
		}
		
		//---------- write event to log file ---------------
		if( channel <= 0 ||				// no filter
			Event->Channel() < 1 ||  // no channel event
			channel == Event->Channel() )
		{
			// Status < 0 -> error
			if( log && Status)
			{
				if( eventCount == MAX_EVENT_IN_LINE && log)
				{
					fprintf( log, "\n");
					eventCount = 0;
				}
				currentTempo = Event->write( log,
					AbsTime,
					ppq,
					&currentChannel,
					Track0,
					currentTempo,
					currentTrack,
					1);

			} // if log && status
			else if( !Status )
			{
				if( log )
					fprintf(log, "\nError: Unexpected end of file!\n");
				Printf("\nError: Unexpected end of file!\n");
				return res;
			}
			//-------------------------------------------------
			// new Abstime
			AbsTime = AbsTime + Event->DTime();
			eventCount++;
			
			
			// process event ---------------------------------
			if( Event->noteOn() &&	// attackpoint of new note?
				tempoFlag != ONLY_CTRL )
			{
				cEv++;
				if( sustainState == 1 )
				{
					// cut sustained notes with equal pitch
					/// we must also add a preliminary noteOff for pending events no in the sustain list
					int pi = Event->pitch();
					TNOTE *Ptr = NoteOnTones[pi].Ptr ;
					TNOTE *overhead = NoteOnTones[pi].overhead;
					if( Ptr || overhead )
					{ // cut and remove
						long playedOffset;
						if(Ptr)
							playedOffset = Ptr->sustainOffset;
						else
							playedOffset = overhead->sustainOffset;

						addNoteOff(NoteOnTones,
								   playedOffset,
								   pi,
									0,
								   Event->Channel(),
								   1,
								   log );
						if(Ptr)
							Ptr->sustainOffset = AbsTime;
						if(overhead)
							overhead->sustainOffset = AbsTime;

						// remove from sustain list
						for( int i = 0; i < sustainList->count(); i++ )
						{
							if( sustainList->get(i) == pi )
							{
								sustainList->remove(i);
								i--;
							}
						} // for
					} // if Ptr || overhead
				} // if sustainState
				/// note will be added to sustainList when reading noteOff				
				
				
				if( NoteOnTones[Event->pitch()].Ptr ) // already on
				{
					if( NoteOnTones[Event->pitch()].overhead ) // buffer is alread full
					{
						Printf("TTRACK::READ: Warning skipped double note on\n");
						if(log)
							fprintf(log,"(* double noteOn, skipped *) ");						
					}
					else
					{
						TMusicalObject *Ptr = CreateNewNote(
							AbsTime,
							1L,	// dummy lgDuration
							Event->pitch(),
							Event->Intens() );
						
						NoteOnTones[Event->pitch()].overhead = dynamic_cast<TNOTE *>(Ptr);
						Insert( Ptr );	//insert new TNOTE in list
					} // else
				}
				else
				{
					TMusicalObject *Ptr = CreateNewNote(
						AbsTime,
						0L,	// dummy lgDuration
						Event->pitch(),
						Event->Intens() );
					// write NoteOn into table
					NoteOnTones[Event->pitch()].Ptr = dynamic_cast<TNOTE *>(Ptr);
					Insert( Ptr );	//insert new TNOTE in list
				}
			} // if NoteOn
			else if( Event->noteOff() && 	// Notenend ?
				     tempoFlag != ONLY_CTRL )
			{
				cEv++;
				if( sustainState == 0 )
				{
					addNoteOff(NoteOnTones,
							   AbsTime,
							   Event->pitch(),
							   Event->Intens(),
							   Event->Channel(),
							   1,
							   log );
				} // if not sustain
				else
				{	// store pitch class in sustainList
					TNOTE *temp = NoteOnTones[Event->pitch()].Ptr;
					if( temp )
						temp->sustainOffset = AbsTime;
					temp = NoteOnTones[Event->pitch()].overhead;
					if( temp )
						temp->sustainOffset = AbsTime;
					sustainList->add(Event->pitch());
				} // else
			} // else if( Event->NoteOff() )
			else  if( Event->tempo() )	// should only occur at track0
			{
				if( tempoFlag != NO_CTRL )
				{
					cEv++;
					TMusicalObject *Ptr = new TMetaTempo (AbsTime,
						Event->GetTempo(),
						1, // numerator of tactus
						4 // denominator of tactus
						);
					//! calc distance to prev SetTempo
					// long int dTime;
					double dTimeMs,
						prevAbsTimeMs = 0;
					if( curTempoEv )
					{
						long int dTime = AbsTime - curTempoEv->GetAbsTime().toLong();
						dTimeMs = DTIMEtoMS(ppq, curTempoEv->GetTempo(), dTime);
						prevAbsTimeMs = curTempoEv->getAbsTimeMs();
					}
					else
					{
						dTimeMs = DTIMEtoMS(ppq, DEFAULT_TEMPO, AbsTime);
					}
					curTempoEv = dynamic_cast<TMetaTempo *>(Ptr);
					curTempoEv->setAbsTimeMs( dTimeMs + prevAbsTimeMs );
					currentTempo = dynamic_cast<TMetaTempo *>(Ptr)->GetTempo();
					Insert( Ptr );		// insert into list
				} // if no ctrl
			}  // else if ( Event->Tempo() )
			else if(  (Event->Type() == TEXT_1 ||
				Event->Type() == LYRICS ||
				Event->Type() ==  MARKER) &&
				tempoFlag != ONLY_CTRL )
			{
				cEv++;
				if( textout ||
					strstr( Event->GetText().c_str(), "GUIDOTAG:") )
				{
					TMusicalObject *Ptr = new TMetaText(AbsTime,
						Event->GetText().c_str(),
						Event->Type() );
					
					Insert( Ptr );
				}
			}
			else if( Event->Type() == KEY_SIGNATURE &&
				tempoFlag != NO_CTRL )
			{
				cEv++;
				int key = Event->getData(0);
				if( key > 128 )
					key = key-256;
				int minorMajor;
				if( Event->getData(1) == 0 ) // major
					minorMajor = 1;
				else
					minorMajor = 0;
				
				TMusicalObject *Ptr = new TMetaKey(AbsTime,
					key, // #accidentals
					minorMajor); // minorMajor
				Insert( Ptr );
				
			}
			else if( Event->Type() == TIME_SIGNATURE &&
				tempoFlag != NO_CTRL )
			{
				cEv++;
				int num = Event->getData(0);
				int denom = pow((double)2,(double)Event->getData(1));
				
				
				TMusicalObject *Ptr = new TMetaMeter(AbsTime,
					num, // numerator of meter sig
					denom); // denominator of meter sig
				// check for existing meter sig at same po
				if( prevMeter &&
					prevMeter->GetAbsTime() == AbsTime )
				{	
					if( prevMeter->Meter() == MetaMeter(Ptr)->Meter() )
					{
						delete Ptr;
						Ptr = NULL;
					}
					else // delete prevMeter
					{
						Printf("Warning: Different meter-signatures in MIDI-file!\n");
						prevMeter->setMeter(num, denom );
						delete Ptr; 
						Ptr = NULL;
					}
				} // if prevMeter
				
				if( Ptr )
				{
					Insert( Ptr );
					prevMeter = MetaMeter(Ptr);
				}
			}
			else if( Event->Type() == TRACKNAME ||
				Event->Type() == TRACKNAME_2 ) // = instr
			{
				cEv++;
				setTrackName( Event->GetText().c_str() );
			}
			else if(Event->Type() == CONTROL_CHANGE )
			{
				cEv++;
				if( Event->getData(0) == SUSTAIN && // sustain on
					/// data range = 0..127 
					Event->getData(1) > 10  && 
					!sustainState )
				{
					sustainState = 1; // ignore now all noteOff events
					/*
					// add all pending notes to list
					for( int i = 0; i < 128 ; i++ )
					{
						if( NoteOnTones[i].Ptr ||
							NoteOnTones[i].overhead )
						{
							sustainList->add(i);
						}
					}
					*/
				} 
				else if( Event->getData(0) == SUSTAIN && // sustain off
	  					 Event->getData(1) < 63  &&
						 sustainState )
				{
					sustainState = 0;
					// close all notes in sustainList
					int i;
					for( i = 0; i < sustainList->count(); i++ )
					{
						int pitch = sustainList->get(i);
						TNOTE *Ptr = NoteOnTones[pitch].Ptr;
						TNOTE *overhead = NoteOnTones[pitch].overhead;
						if( Ptr || overhead )
						{
							unsigned long playedOffset = NoteOnTones[pitch].Ptr->sustainOffset;
							addNoteOff(NoteOnTones, 
									playedOffset,
									pitch,
									0,Event->Channel(),
									1, log );
							if( Ptr )
								Ptr->sustainOffset = AbsTime;
							if( overhead )
								overhead->sustainOffset = AbsTime;
						}
					} // for
					// empty the sustain list
					sustainList->clear();
				} // else if
			} // if CTRL CHANGE
		} // Chennel CHeck
		else // channel error
		{
			res.channelCount++;
			if( Event->Channel() < 17 )
			{
				res.unreadEvents[Event->Channel()-1]++;
			}
			// neue Abstime
			AbsTime = AbsTime + Event->DTime();
		}
		// new runningstate
		Status  = Event->Read( Status, file, curSequence );
	} // while
	
	// check if last event == EndOfTrack
	if( !Event->EndOfTrack() ) // Error
	{
		if( log )
			fprintf( log, "\n%% !!! Error: In track %d event<\"End of track\">  is missing !!!\n", currentTrack );
		cout <<  "\n!!! Error: In track "<< currentTrack << " event<\"End of track\"> is missing !!!\n";
		return res;
	}
	else // write last event to log
	{
		currentTempo = Event->write( log,
			AbsTime,
			ppq,
			&currentChannel,
			Track0,
			currentTempo,
			currentTrack,
			1);
		eventCount++;
		fprintf(log, "\n");
	}
	// this does not work beacuse of filtering! 
	/*
	fpos_t tempPos;
	fgetpos(file, &tempPos);
	if( Next() &&
	    Next()->Offset != tempPos )
	{
		Next()->Offset = tempPos;
		Next()->Read_Header(file);
		Printf("WARNING: Corrected file offset for track %d\n", currentTrack+1);
	}
	*/
	delete Event;
	delete [] NoteOnTones;
	delete sustainList;
	cout << cEv << " events. done." <<endl;
	//this->Debug();
	return res;
} // Read
//----------------------------------------------------------------------
void TTRACK::deleteSameType( TMusicalObject *from,
							TMusicalObject *to )
{
	curTempo = NULL;
	if( !from )
		return;
	
	if( this )	// for debugging purposes
	{
		TMusicalObject *Now = from;
		while( Now &&
			   Now != to)
		{
			// !! Between Now and Next maybe skipped notes of other type !!!
			
			TMusicalObject *Prev = Now->TMusicalObject::GetPrev(-1);
			TMusicalObject *Next  = Now->GetNext(-1 /* all voices */); // get next same type
			TMusicalObject *nextObject = Now->TMusicalObject::GetNext(-1);
			
			if( Notes == Now )
				Notes = nextObject;
			
			// detach Now from list
			if( Prev )
				Prev->SetNext(nextObject);
			if(nextObject)
				nextObject->SetPrev(Prev);
			
            Now->SetPrev(NULL);
			
			delete Now;
			Now = Next; // next of same type!!
		} // while
		CurrentNote = NULL;
	} // if
}

						 /*
						 delete the list of notes
						 */
void TTRACK::deleteNotes( TMusicalObject *from,
						 TMusicalObject *to )
{
	if( !from )
		return;
		
	TMusicalObject 	 *Prev = NULL;	
	if(from)
		Prev = from->TMusicalObject::GetPrev(-1);
	if( this )	// for debugging purposes
	{
		TMusicalObject *Now = from;
		while( Now &&
			Now != to)
		{
			TMusicalObject *NextNote  = Now->TMusicalObject::GetNext(-1 /* all voices */);
			Now->SetNext(NULL);
			Now->SetPrev(NULL);
			delete Now;
			Now = NextNote;
		} // while
		if(from == Notes) // remove head of list
			Notes   = to;
		CurrentNote = NULL;
		curTempo = NULL;
	} // if
	if( to )
		to->SetPrev( Prev );
	if(Prev)	
		Prev->SetNext(to);
} // DeleteNotes
//----------------------------------------------------------------------
/*!
insert ptr sorted into list of notes
*/
void TTRACK::Insert( TMusicalObject *ptr )
{
	if(!ptr)
	{
		Printf("TTRACK::Insert: ptr == NULL!\n");
	}
	TMusicalObject *PrevNote = NULL;	
	if( !Notes )	// this is the first object
	{
		Notes  = ptr;
	}
	else // insert sorted
	{
		TMusicalObject *NextNote = NULL;
		if( CurrentNote )
		{
			// go back in list
			while( CurrentNote &&
				   !((*ptr) > (*CurrentNote)) )
			{
				CurrentNote = CurrentNote->GetPrev(-1);
			}
			if( CurrentNote )
				PrevNote = CurrentNote->GetPrev(-1);
			NextNote = CurrentNote;
		} // if current note
		
		if( !CurrentNote )
		{
			PrevNote = NULL;
			NextNote = FirstObject(-1 /* all voices*/);
		}
		
		while( NextNote &&
			( (*ptr) > (*NextNote) ) )
		{
			PrevNote = NextNote;
			NextNote = NextNote->TMusicalObject::GetNext(-1 /* all voices */);
		} // while
		
		// store as head of list
		if( !PrevNote )
		{
			ptr->SetNext( Notes );
			Notes = ptr;       // head of list
		} // if
		else	// ptr in list
		{
			PrevNote->SetNext( ptr );
			ptr->SetNext( NextNote );
		}	// else
	} // else
	CurrentNote = ptr;
} // Insert
//-------------------------------------------------------------------
TMIDIFILE *TTRACK::Parent( void )
	{
		return parent;
	}

TNOTE *TTRACK::CreateNewNote(
							 TFrac abstime,
							 TFrac duration,
							 unsigned char pitch,
							 unsigned char intens )
{
	TNOTE *temp = new TNOTE(abstime,
				     duration,
					 pitch,
					 intens);
	return temp;
}

/*!
res = tempo at abstime
! "this" must be Track0 including tempo changes
*/
int TTRACK::GetTempo( TAbsTime abstime )
{
	if( !curTempo )
	{
		curTempo = FirstTempo(); 
	}
	
	if( !curTempo )
	{
		return DEFAULT_TEMPO;
	}
	TMetaTempo *nextTempo = curTempo;
	if( abstime >= curTempo->GetAbsTime() )
	{
		// look for next tempo change
		while( nextTempo &&
		       nextTempo->GetNext(-1) && 
			   abstime > nextTempo->GetAbsTime() )
		{
			curTempo = nextTempo;
			nextTempo = dynamic_cast<TMetaTempo *>(nextTempo->GetNext(-1));
		}
		return curTempo->GetTempo();
	}
	else
	{
		// look for next tempo change
		while( nextTempo &&
			nextTempo->GetPrev(-1) && 
			abstime < curTempo->GetAbsTime() )
		{
			curTempo = nextTempo;
			nextTempo = dynamic_cast<TMetaTempo *>(nextTempo->GetPrev(-1));
		}
		// todo : error if curTempo abstime > 0
		return curTempo->GetTempo();
	}
	
	TMetaTempo *temp;
	if( CurrentNote &&
		CurrentNote->GetAbsTime() > abstime )
		// Reset
	{
		temp = FirstTempo();
	}
	else if( CurrentNote &&
		!dynamic_cast<TMetaTempo *>(CurrentNote) )
		//          strcmp(CurrentNote->ClassName(), TEMPO_CLASS) )
	{
		temp = FirstTempo();
	}
	else
		//		temp = (TMetaTempo *)CurrentNote;
		temp = FirstTempo();
	
	if( temp )
	{
		TMetaTempo *nextNote = MetaTempo(temp->GetNext(-1 /* all voices */));
		while( nextNote &&
			nextNote->GetAbsTime() < abstime  )
		{
			temp = nextNote;
			nextNote = MetaTempo(nextNote->GetNext(-1 /* all voices */));
		}
	}
	if( temp )
	{
		return (temp->GetTempo());
	}
	else
	{
		Printf("WARNING: No SetTempo information in MIDI file!\n");
		return DEFAULT_TEMPO;
	}	
}

TMusicalObject *TTRACK::FirstObject( int voice )
{
	TMusicalObject *temp = Notes;
	if(voice >= 0 &&
		temp &&
		temp->GetVoice() != voice )
	{
		temp = temp->TMusicalObject::GetNext( voice );
	}
	this->CurrentNote = temp;
	return temp;
}

TNOTE *TTRACK::FirstNoteObject( int voice,
								double minAttackTime,
								double maxAttackTime)
{
	TNOTE *tNote = NULL;
	TMusicalObject *temp = FirstObject(voice);
	while( temp ) 
	{
		tNote = dynamic_cast<TNOTE *>(temp);
		if( tNote && 
		     tNote->AbsTimeMS(this->parent) >= minAttackTime )
		{
			temp = NULL; // stop
		}
		else
		{
			temp = temp->TMusicalObject::GetNext(voice);
		}
	}	// while notes

	// is temp outside range?
	if( maxAttackTime > minAttackTime &&
		tNote &&
		tNote->AbsTimeMS(this->parent) >= maxAttackTime )
		return NULL;
	if( tNote &&
		tNote->AbsTimeMS(this->parent) < minAttackTime )
	{
//		int a = temp->AbsTimeMS() < minAttackTime;
		return NULL;
	}
	return (tNote);
}

TMetaKey *TTRACK::FirstKey(void)
{
	TMusicalObject *temp = FirstObject(-1 /* all voices */);
	while(temp &&
		!dynamic_cast<TMetaKey *>(temp ) )
		//       strcmp(temp->ClassName(), KEY_CLASS))
	{
		temp = temp->TMusicalObject::GetNext(-1 /* all voices */);
	}
	return dynamic_cast<TMetaKey *>(temp);
}

TMetaTempo *TTRACK::FirstTempo(void)
{
	TMusicalObject *temp = FirstObject(-1 /* all voices */);
	while(temp &&
		!dynamic_cast<TMetaTempo *>(temp) )
		//       strcmp(temp->ClassName(), TEMPO_CLASS))
	{
		temp = temp->TMusicalObject::GetNext(-1 /* all voices */);
	}
	return dynamic_cast<TMetaTempo *>(temp);
}
TMetaMeter *TTRACK::FirstMeter( void )
{
	TMusicalObject *temp = FirstObject(-1 /* all voices */);
	while(temp != NULL &&
		!dynamic_cast<TMetaMeter *>(temp) )
		//       strcmp(temp->ClassName(), METER_CLASS))
	{
		temp = temp->TMusicalObject::GetNext(-1 /* all voices */);
	}
	return dynamic_cast<TMetaMeter *>(temp);
}



/*!
remove all notes of voice in range [from,to) from notelist
*/
TMusicalObject *TTRACK::DetachedVoice(TMusicalObject *from,
                                      TMusicalObject *to)
{
	if(!from)
		return NULL;

	if( from == to )
	{
		to = from->TMusicalObject::GetNext(from->GetVoice() );
	}

	if( CurrentNote &&
	    (*CurrentNote) >= (*from) )
		CurrentNote = from->GetPrev(-1);
		
	// check for equal voices
	int voice = from->GetVoice();
	if( to &&
		to->GetVoice() != voice &&
		to != from->TMusicalObject::GetNext(-1) )
	{
		Printf("TTRACK::Warning: DetachedVoice: different voices!\n");
		
	}
	
	TMusicalObject *resTail = from;
	TMusicalObject *current = from->TMusicalObject::GetNext(-1); // == next note to be untouched
	// remove from of list
	if(from == Notes) // head of list?
	{
		Notes = current; // new head
		if( current )
			current->SetPrev(NULL);
	}
	else // remove from 
	{
		if( from->TMusicalObject::GetPrev(-1) )
			from->TMusicalObject::GetPrev(-1)->SetNext(current);
	}
	
	resTail->SetPrev(NULL);
	resTail->SetNext(NULL);
	
	// search for mor notes of voice, starting at current
	while(current &&
		current != to)
	{
		if(current->GetVoice() == voice )
		{
			// detach current
			TMusicalObject *nextNote = current->TMusicalObject::GetNext(-1);
			if(current == Notes)
			{
				Notes = nextNote;
				Notes->SetPrev(NULL);
			}
			else // detach current 
			{
				current->TMusicalObject::GetPrev(-1)->
					SetNext(nextNote);
			}
			// append current to resTail
			current->SetNext(NULL);
			resTail->SetNext(current);
			resTail = current;
			
			current = nextNote;
		}
		else
		{
			current = current->TMusicalObject::GetNext(-1);
		}
	} //while
	return from;
}


// add offset to voice of all objects
void TTRACK::shiftVoices(int offset)
{
	if(offset < 0)
	{
		cout << "shiftVoices: illegal offset " << offset << endl;
		return;
	}
	TMusicalObject *temp = FirstObject(-1); // all voices
	while(temp)
	{
		int voice = temp->GetVoice();
		if( voice >= 0) // skip unvoiced elements
		{
			temp->SetVoice(voice + offset);
		}
		temp = temp->TMusicalObject::GetNext(-1);
	}
} // shiftVoices

void TTRACK::SetTrackNumber( int i )
{ 
	if( trackName != string() )	// don't overwrite
		return; 
	TrackNumberI = i; 
	/*    
	// use tracknumber as default trackname, removed for GRAME
	
	  char *temp;
	  temp = new char[30];
	  
		sprintf(temp,"Track %i", i );
		setTrackName( temp);
		delete [] temp;
	*/
	
};


void TTRACK::Merge(TTRACK *track2)
{
	if(!track2)
	{
		Printf("TTRACK:Merge track2==NULL\n");
		return;
	}
	if( track2->cVoice() )
	{
		cout << " Merging track " <<
			TrackNumberI << ":" <<
			track2->TrackNumberI << endl;
		cout << 			track2->Channel() << "("<<
			track2->cVoice() << ")" << endl;
	}

	track2->copyUnvoicedElements();
	track2->shiftVoices(CVoice);
	
	// create new array for names of voices
	string *newNames = NULL;
	if( CVoice+track2->cVoice() ) 
	{
		newNames = new string[CVoice+track2->cVoice()];
	}
	
	// copy existing voice names
	if( !voiceNames )
	{
		for(int  i = 0; i < cVoice(); i++ )
		{
			newNames[i] = ( this->trackName );
		} // for
	}
	else
	{
		for( int i = 0; i < cVoice(); i++ )
		{
			newNames[i] = ( voiceNames[i] );
		} // for		
	}
	// add new voice names
	for(int i = 0; i < track2->cVoice(); i++ )
	{
		newNames[i+cVoice()]=( track2->voiceName(i) );
	}
	
	if( voiceNames )
		delete [] voiceNames;
	voiceNames = newNames;
	
	CVoice += track2->cVoice();
	
	TMusicalObject *temp  = FirstObject(-1);
	TMusicalObject *temp2 = track2->FirstObject(-1);
	TMusicalObject *prev = NULL;
	
	while(temp || temp2)
	{
		TMusicalObject *next;
		if( temp &&
			temp2 &&
			*temp > *temp2)
		{
			next = temp2;
			temp2 = temp2->TMusicalObject::GetNext(-1);
		}
		else if( temp &&
			temp2 )		
		{
			next = temp;
			temp = temp->TMusicalObject::GetNext(-1);
		}
		else if( temp ) // temp2 == NULL
		{
			next = temp;
			temp = NULL;
		}
		else if( temp2 ) // temp == NULL
		{
			next = temp2;
			temp2 = NULL;
		}
		else // error
		{
			Printf("TTRACK::Merge error!\n");
			return;
		}
		// insert into list
		if(prev)
		{
			prev->SetNext(next);
		}
		else
		{
			Notes = next;
			Notes->SetPrev( NULL );
		}
		prev = next;
	} // while
	
	// remove track2 from list
	track2->Notes = NULL;
	track2->CurrentNote = NULL;
	
	if(track2 &&
		track2 == nextI)
		nextI = track2->Next();
	
	CurrentNote = NULL;
}


TAbsTime TTRACK::Convert( ostream &gmnOut/*file*/,
						 //					  int  ppq,
						 TTRACK * /*Track0*/,
						 TAbsTime /* from*/ ,
						 TAbsTime /* to */ ,
						 TAbsTime /*offset */)
{
	Printf("TTRACK::Convert not implemented\n");
	exit(1);
	return TFrac(0,0);
};

/*
Pitch classes 0  1 2  3  4  5  6
c  d e  f  g  a  h

  c  c# d e&  e  f  f# g  g#  a b&  h
*/
//
keyscale C_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b
// f#
keyscale G_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b

// f#, c#
keyscale D_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{5,1},		// a#
{6,0}};	// b
// f#, c#, g#
keyscale A_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{5,1},	   // a#
{6,0}};	// b

// f#, c#, g#, d#
keyscale E_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{6,-1},	// a#
{6,0}};	// b

// f#, c#, g#, d#, a#
keyscale H_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{3,2},		// f##
{4,1},		// g#
{5,0},		// a
{5,1},	// a#
{6,0}};	// b
// f#, c#, g#, d#, a#, e#
keyscale Fs_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{2,1},		// e#
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{5,1},	   // a#
{6,0}};	// b
// f#, c#, g#, d#, a#, e#, h#
keyscale Cs_maj ={{-1,1},  // h#
{0,1},    // c#
{1,0},		// d
{1,1},		// d#
{2,0},		// e
{2,1},		// e#
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{5,1},	   // a#
{6,0}};	// b

// b&
keyscale F_maj ={{0,0},  	// c
{0,1},    // c#
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{3,1},		// f#
{4,0},		// g
{4,1},		// g#
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b

// b&, e&
keyscale Bf_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b

// b&, e&, a&
keyscale Ef_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b

// b&, e&, a&, d&
keyscale Af_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{6,0}};	// b

// b&, e&, a&, d&, g&
keyscale Df_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{7,-1}};	// c&

// b&, e&, a&, d&, g&, c&
keyscale Gf_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{2,0},		// e
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{7,-1}};	// c&
// b&, e&, a&, d&, g&, c&, f&
keyscale Cf_maj ={{0,0},  	// c
{1,-1},   // d&
{1,0},		// d
{2,-1},	// e&
{3,-1},	// f&
{3,0},		// f
{4,-1},	// g&
{4,0},		// g
{5,-1},	// a&
{5,0},		// a
{6,-1},	// b&
{7,-1}};	// c&

// typedef keyscale * pkeyscale;

pkeyscale allKeys[15] ={
	&Cf_maj, &Gf_maj, &Df_maj, &Af_maj, &Ef_maj,
		&Bf_maj,&F_maj,&C_maj,&G_maj,&D_maj,
		&A_maj,&E_maj,&H_maj,&Fs_maj,&Cs_maj};
	
	
int keyPitch( int normKey, int midipitch )
{
	if( normKey < 0 ||
		normKey > 14 ||
		midipitch < 0 ||
		midipitch > 11 )
		return 0;
		
	return (*allKeys[normKey])[midipitch][0];
}

int keyAccidental(int normKey, int midipitch)
{
	if( normKey < 0 ||
		normKey > 14 ||
		midipitch < 0 ||
		midipitch > 11 )
		return 0;
		
	return (*allKeys[normKey])[midipitch][1];
}
		/*!
		convert scale into GUIDO string
		!! result must be deleted
	*/
	char *scale2Str( keyscale *scale )
	{
		char *res = new char[255];		
		strcpy( res,("["));
		
		
		for(int i = 0; i < 12; i++ )
		{
			if( i )
				strcat(res, " ");
			switch( (*scale)[i][0] )
			{
			case 0:
			case 7:
				strcat( res,"c");
				break;
			case 1:
				strcat( res,"d");
				break;
			case 2:
				strcat( res,"e");
				break;
			case 3:
				strcat( res,"f");
				break;
			case 4:
				strcat( res,"g");
				break;
			case 5:
				strcat( res,"a");
				break;
			case -1:
			case 6:
				strcat( res,"b");
				break;
			} // siwtch
			if( (*scale)[i][1]	> 0 ) // write accidentals
			{
				for(int j=0; j < (*scale)[i][1]; j++)
					strcat( res, "#" );
			}
			else if( (*scale)[i][1]	< 0 ) // write accidentals
			{
				for(int j=0; j > (*scale)[i][1]; j--)
					strcat( res, "&" );
			}
		}
		strcat( res,"]");
		return res;
	}
	
	char *skipSpace( char *buffer )
	{
		while( buffer &&
			*buffer == ' ' )
		{
			buffer++;
		}
		return buffer;
	}
	
	//! parse buffer and copy values into scale
	void setScale( char *buffer, keyscale *scale )
	{
		if( *buffer != '[' )
		{
			Printf("Syntax error in fermata.ini at scale definition!\n");
			return;
		}
		// skip "}"
		char *buffPos = strstr( buffer, " ");
		// skip space
		char *curVal = buffer;
		curVal++; // skip '['
		curVal = skipSpace(curVal); 
		int i = 0;
		while( curVal &&
			*curVal != 0 && 
			i < 12 )
		{
			if( buffPos )
			{
				*buffPos = 0; // end of val;
				buffPos++;  // start of next note
				buffPos = skipSpace( buffPos );
			}
			int val1 = 0;			
			// parse curVal;
			switch( *curVal )
			{
			case 'c' :
				val1 = 0;
				if( i > 4 )
					val1 = 7;
				break;
			case 'd' :
				val1 = 1;
				break;
			case 'e' :
				val1 = 2;
				break;
			case 'f' :
				val1 = 3;
				break;
			case 'g' :
				val1 = 4;
				break;
			case 'a' :
				val1 = 5;
				break;
			case 'h' :
			case 'b' :
				val1 = 6;
				if( i < 5 )
					val1 = -1;
				break;
			default:
				Printf("Syntax error in inifile at scale definition!\n");
				return;
			} // switch
			
			int val2 = 0;
			// read accidentals
			if( *curVal != 0 )
				curVal++;
			if( *curVal == '&' )
			{
				while( *curVal == '&' )
				{
					val2--;
					curVal++;
				}
			}
			else if ( *curVal == '#' )
			{
				while( *curVal == '#' )
				{
					val2++;
					curVal++;
				}
			}
			
			if(  *curVal != ' ' &&
				*curVal != 0 && 
				*curVal != ']')
			{
				Printf("Syntax error in inifile at scale definition!\n");
				return;
			}
			(*scale)[i][0] = val1;
			(*scale)[i][1] = val2;
			
			curVal = buffPos;
			if( buffPos &&
				*buffPos != 0 )
				buffPos = strstr( buffPos, " ");
			i++;
		} // while
		if( i != 12 )
		{
			Printf("Syntax error in inifile at scale definition!\n");
		}
} 


/*!
read scales from fermata.ini. Override defined scales
called from QMIDIFILE::ToGMN
*/
void readScales( TInifile *tempIni )
{
	const char *scaleNames[15]={"CFlat_scale",
						  "GFlat_scale",	
						  "DFlat_scale",	
						  "AFlat_scale",	
						  "EFlat_scale",	
						  "BFlat_scale",	
						  "F_scale",	
						  "C_scale",	
						  "G_scale",	
						  "D_scale",	
						  "A_scale",	
						  "E_scale",	
						  "B_scale",	
						  "FSharp_scale",	
						  "CSharp_scale"};
	
	if( tempIni )
	{
		for(int i = 0; i < 15 ; i++ )
		{
			// get default scale
			char *buffer = scale2Str( allKeys[i]);
			const char *val = tempIni->GetValChar( scaleNames[i], buffer,"accidentals for std scale, don't touch!");
			delete [] buffer;
			buffer = new char[strlen(val)+1];
			strcpy(buffer,val);
			// copy value to allKey
			setScale( buffer , allKeys[i]);
			delete [] buffer;
		} // for
	} // if
} // read Scales

int isInScale( int key,
			  int pitch, 
			  int acc)
{
	
	// c d e f g a h 
	// 0 1 2 3 4 5 6 
	int sharps[7] = {3,0,4,1,5,2,6};
	int flats[7] = {6,2,5,1,6,0,3};
	
	int *accs;
	if( key < 0 )
		accs = flats;
	else
		accs = sharps;
	
	if( key == 0 && acc == 0 )
		return 1;
	else if( abs(acc > 1 ) )
		return 0; // no double accs in scales
	else if ( key == 0  )
		return 0; // no accs in c major
	else if ( key > 0 && acc < 0 )
		return 0; // no flats in sharp scale
	else if ( key < 0 && acc > 0 )
		return 0; // no sharps in flat scale
	else
	{
		key = abs(key);
		for( int cKey = 0; cKey < key; cKey++)
		{
			if( accs[cKey] == pitch )
				return 1;
		} // for
	} // else
	// natural pitch is not flatted/sharped in key
	if(  acc == 0  )
		return 1;
	
	return 0;
}

void enharmonicTransp( int *pitch,
					  int *acc,
					  int *octave,
					  int deltaPitch )
{
	
/*
Up
0,-1 ->1,-3    c -> d
0,0 -> 1,-2
0,1 -> 1,-1
0,2 -> 1,0
0,3 -> 1,1

		1,-1 -> 2,-3   d -> e
		1, 0 -> 2,-2
		1, 1 -> 2,-1
		1, 2 -> 2, 0	
		
		  2,0  -> 3,-1   e -> f
		  3,0  -> 4,-2   f -> g
		  4,0 ->  5,-2   g -> a
		  5,0 ->  6,-2   a->b
		  6,0 ->  0/7,-1 b-> c
		  
			
			  Down		  
			  0,-2 -> 6,-1 c -> h
			  0,-1 -> 6,0
			  
				0,0  -> 6/-1,1 
				
				  0,1  -> 6,2
				  0,2  -> !!
				  
					1,-2 -> 0,0  
					1,-1 -> 0,1
					1,0  -> 0,2 d -> c
					1,1  -> !!
					
					  2,0 -> 1,2 e-> d
					  3,0 -> 2,1 f->e
					  4,0 -> 3,2 g->f
					  5,0 -> 4,2 a->g
					  6,0 -> 5,2 b->a
					  
						
						  
	*/
	if( deltaPitch < 0 )
	{
		if( *pitch == 0 ||
			*pitch == 3 )
			*acc += 1;
		else
			*acc += 2;
		
		*pitch -= 1;
		if( *pitch < 0 )
		{
			*pitch += 7;
			*octave -= 1;
		}
	}
	else
	{
		if( *pitch == 2 ||
			*pitch == 6 )
			*acc -= 1;
		else
			*acc -= 2;
		*pitch += 1;
		if( *pitch > 6 )
		{
			*pitch -= 7;
			*octave +=1 ;
		} // if
	} // else
}
/*! 
only 2 pitches should appear, more than 2 notes should be there
one pitch == already in scale
second pitch == #/& of next in scale
*/
void 	markChromaticTrill(TNOTE *cScaleStart,
						   TNOTE *cScaleEnd,
						   int key)
{
	TNOTE *current = cScaleStart;
	if( !current )
		return;
	
	// calc pitches
	int mPitch1 = current->GetMIDIPitch();
	int pitch1  = current->DiatonicPitch(); // in key c
	int acc1    = current->accidentals();
	int octave1 = current->Octave();
	
	current = NOTE(current->GetNext(current->GetVoice()));
	if( !current )
		return ;
	
	int mPitch2;
	int pitch2;
	int acc2;
	int octave2;
	do // search for seconf pitch
	{
		mPitch2 = current->GetMIDIPitch();
		pitch2  = current->DiatonicPitch(); // in key c
		acc2    = current->accidentals();
		octave2 = current->Octave();
		
		current = NOTE(current->GetNext(current->GetVoice()));
	}
	while( current && 
		   mPitch1 == mPitch2);
	
	if( mPitch1 == mPitch2 )
	{
		Printf("\nWarning: Wrong chromatic trill detected!\n");
		return;
	}
	int deltaPitch = mPitch2 - mPitch1; // 1 || -1
	
	if( pitch1 != pitch2 ) // do nothing if already different notes
		return;

	int keepPitch,
		changePitch,
		changeAcc,
		changeOctave;
	if( isInScale( key, pitch1, acc1) )
	{
		deltaPitch = mPitch2 - mPitch1;
		// modifiy pitch2
		enharmonicTransp( &pitch2, 
			&acc2, 
			&octave2,
			deltaPitch );
		keepPitch = mPitch1;
		changePitch = pitch2;
		changeAcc = acc2;
		changeOctave = octave2;
	}
	else // modify pitch1
	{
		deltaPitch = mPitch1 - mPitch2;
		enharmonicTransp( &pitch1, 
			&acc1, 
			&octave1,
			deltaPitch );
		keepPitch = mPitch2;
		changePitch = pitch1;
		changeAcc = acc1;
		changeOctave = octave1;
	}
	
	// process complete line
	current = cScaleStart;
	while( current && 
		current != cScaleEnd )
	{
		if( current->GetMIDIPitch() != keepPitch )
		{
			current->SetPitch(changePitch,
				changeAcc,
				changeOctave);
		}
		current = NOTE(current->GetNext(current->GetVoice()));
	} // while
};

/*!
keep scale notes, enharmonic transp non scale notes
downwards -> natural or &
upwards -> natural or #
*/
void	markCScale(TNOTE *cScaleStart,
				   TNOTE *cScaleEnd,
				   int key,
				   int deltaPitch)
{
	TNOTE *current = cScaleStart;
	if( !current )
		return;
	
	// invert delta pitch for transpose
	while( current && 
		current != cScaleEnd )
	{
		// calc pitche
		int pitch  = current->DiatonicPitch(); // in key c
		int acc    = current->accidentals();
		int octave = current->Octave();
		if( !isInScale( key, pitch, acc) )
		{
			if( deltaPitch > 0 ) // upward scale
			{
				if( acc < 0 )		   // transpose
				{
					// modifiy pitch2
					enharmonicTransp( &pitch, 
						&acc, 
						&octave,
						-1 );
					
					current->SetPitch(pitch,
						acc,
						octave);
				} // if flats				
			}
			else // downward scale
			{
				if( acc > 0 )
				{
					// modifiy pitch
					enharmonicTransp( &pitch, 
						&acc, 
						&octave,
						1 );
					
					current->SetPitch(pitch,
						acc,
						octave);
				} // if sharps
			} // if
			// other not in key notes must be already correct!!
		} // ! in scale
		current = NOTE(current->GetNext(current->GetVoice()));
	} // while notes
};

//! set correct accidentals in chromatic scales
//! pitchSpelling must be called before!!
void TTRACK::checkChromaticScales(TMIDIFILE *midifile)
{
	for( int voice = 0; voice < CVoice; voice ++ )
	{
		TNOTE *cScaleStart = NULL;
		TNOTE *cScaleEnd = NULL;
				
		TNOTE *current = FirstNoteObject( voice );
		if( current )
		{
			int currentKey = midifile->currentKey( current->GetAbsTime() );
			TNOTE *next = NOTE(current->GetNext(current->GetVoice()));
			int nNotes = 0,		// #notes in scale
				nPitches = 0,	// #pitchclasses in scale
				prevDeltaPitch = -100;
			while( next || cScaleStart )
				// search complete voice
			{
				int nextKey;
				if( next )
					nextKey = midifile->currentKey( next->GetAbsTime() );
				else
					nextKey = currentKey;
				
				if( next && 
					nextKey != currentKey ) 
					// key change 
				{
					
				}
				// calculate deltaPitch
				int deltaPitch;
				if( next )
					deltaPitch =  next->GetMIDIPitch()
								- current->GetMIDIPitch();
				else
					deltaPitch = 0; // -> end of scale
				
				if( !cScaleStart ) // chromatic scale hasn't started
				{
					if( deltaPitch == 1 ||
						deltaPitch == -1 )
					{
						cScaleStart = current;
						prevDeltaPitch = deltaPitch;
						nNotes = 2;
						nPitches = 2;
					}
					else if( deltaPitch == 0 ) // check equal pitches
					{
						cScaleStart = current;
						prevDeltaPitch = deltaPitch;
						nNotes = 2;
						nPitches = 1;
					}
				}
				else // scale in progress
				{
					// check for end of scale
					if( deltaPitch != 1 &&
						deltaPitch != -1 &&
						deltaPitch != 0)
					{
						cScaleEnd = next;
					}
					else if ( deltaPitch == 0 )
					{
						nNotes++;
					}
					else if( deltaPitch == prevDeltaPitch )
						// +1 || -1 scale keeps going
					{
						nNotes++;
						nPitches++;
					}
					else // alternating scale +1,[0],-1,[0]*
					{
						if( nPitches <= 2 ) // chromatic trill
							// go ahead
						{
							nNotes++;
							prevDeltaPitch = deltaPitch;
							nPitches = 2;
						}
						else // stop
						{
							cScaleEnd = next;
						}
					} // else					
				} // else in progress
				if( (next && cScaleEnd) ||
					(!next && cScaleStart) ) // open end
				{					
					if( nNotes > 2 )
					{
						if( nPitches == 2 )
							markChromaticTrill(cScaleStart,
							cScaleEnd,
							currentKey);
						else if( nPitches > 2 )
							markCScale(cScaleStart,
							cScaleEnd,
							currentKey,
							prevDeltaPitch);
					} // if nNotes
					
					cScaleStart = NULL;
					cScaleEnd = NULL;
					nNotes = 0;
					nPitches = 0;
					deltaPitch = 0;
					prevDeltaPitch = 0;
				}
				current = next;
				currentKey = nextKey;
				
				if( current ) // == next
					next = NOTE(current->GetNext(current->GetVoice()));
			} // while
		} // if
	} // for;	
}





//! calculate the correct pitch information for all notes of track
void TTRACK::pitchSpelling(	 TMIDIFILE *midifile )
{
	for( int voice = 0; voice < CVoice; voice ++ )
	{
		TNOTE *current = FirstNoteObject( voice );
		while( current )
		{
			current->pitchSpelling( midifile );
			current = NOTE(current->GetNext(current->GetVoice()));
		} // while
	} // for;	
} // calcPitchClasses



TNOTE * TTRACK::noteAt(TAbsTime evTime)
{
	TNOTE *temp = FirstNoteObject(-1);
	while( temp &&
		temp->GetAbsTime() < evTime )	
	{
		temp = NOTE(temp->GetNext(-1));
	}
#ifdef _DEBUG
	if( temp->GetAbsTime() != evTime )
		printf("Warning TTRACK::noteAt temp->GetAbsTime != evTime");
#endif
	return temp;
}

void TTRACK::shiftAttacks(TFrac offset)
{
	TMusicalObject *obj = FirstObject(-1);
	while( obj )
	{
		obj->SetAbsTime( obj->GetAbsTime() + offset );
		obj = obj->TMusicalObject::GetNext(-1);
	} // for all objectsbj
}


unsigned long TTRACK::addNoteOn(TNoteOnList *NoteOnTones, 
								unsigned long absTime, 
								int pitch, 
								int vel, 
								int chan,
								FILE *log)
{	
	if( vel == 0 ) // -> noteOff
		return addNoteOff( NoteOnTones,
		absTime, 
		pitch, 
		vel, 
		chan,
		1,
		log );
	
	if( NoteOnTones[pitch].Ptr ) // already on	
	{
		if( NoteOnTones[pitch].overhead ) // buffer is alread full
		{
			Printf("TTRACK::READ: Warning skipped double note on\n");
			if(log)
				fprintf(log,"(* double noteOn, skipped *) ");			
		}
		else
		{
			TMusicalObject *Ptr = CreateNewNote(
				absTime,
				1L,	// dummy lgDuration
				pitch,
				vel );
			
			NoteOnTones[pitch].overhead = dynamic_cast<TNOTE *>(Ptr);
			Insert( Ptr );	//insert new TNOTE in list
		} // else
	}
	else
	{
		TMusicalObject *Ptr = CreateNewNote(
			absTime,
			1L,	// dummy lgDuration
			pitch,
			vel );
		// write NoteOn into table
		NoteOnTones[pitch].Ptr = dynamic_cast<TNOTE *>(Ptr);
		Insert( Ptr );	//insert new TNOTE in list
	}
	return absTime;
}


unsigned long TTRACK::addNoteOff( TNoteOnList *NoteOnTones, 
								 unsigned long AbsTime, 
								 int pitch, 
								 int /* vel*/, 
								 int /* chan */,
								 int relduration,
								 FILE *log)
{
	TNOTE *Ptr = NoteOnTones[pitch].Ptr; // fifo
	if( !Ptr )
		Ptr = NoteOnTones[pitch].overhead;
				
	if( Ptr )
	{
		// calculate duration
		long dur = AbsTime - Ptr->GetAbsTime().toLong(); // click timing
		if( relduration != 1 )
			dur = (dur * 100 ) / relduration;
		/*
		else
			dur = dur + 1;
		*/
		
		if(dur <= 0)
		{
			Printf("TTRACK::READ: dur == 0!\n");
			if(log)
				fprintf(log,"(* zero duration *)");
		}
		else
		{
			// store duration in existing note
			dynamic_cast<TNOTE *>(Ptr)->SetDuration(dur);
		}
		Ptr->sustainOffset = Ptr->offset().toLong();
		// reset table
		NoteOnTones[pitch].Ptr = NoteOnTones[pitch].overhead; // fifo
		NoteOnTones[pitch].overhead = NULL;
	} // if NoteOnTones[]
	else
	{
		Printf("TTRACK::READ: Warning NoteOn is missing!\n");
		if(log)
			fprintf(log,"(* note on is missing, skipped *)");
	}				
	return AbsTime;
}

unsigned long TTRACK::addTimeSig( TNoteOnList * /*NoteOnOff*/, 
								 unsigned long absTime, 
								 int num, 
								 int denom, 
								 FILE * /*log*/)
{
	TMusicalObject *Ptr = new TMetaMeter(absTime,
						num,denom);
	Insert( Ptr );	
	return absTime;
}

unsigned long TTRACK::addKeySig( TNoteOnList * /*NoteOnOff*/, 
								unsigned long absTime, 
								int key, 
								int minorMajor,
								FILE * /*log*/)
{
	TMusicalObject *Ptr = new TMetaKey(absTime,
						key, // #accidentals
						minorMajor); // minorMajor
	Insert( Ptr );	
	return absTime;
}

unsigned long TTRACK::addSetTempo( TNoteOnList * /*NoteOnOff*/, 
                                    unsigned long absTime, 
                                    double bpm, 
                                   FILE * /* log */)
{
	TMusicalObject *Ptr = new TMetaTempo (absTime,
		(int)bpm,
		1, // numerator of tactus
		4 // denominator of tactus
		);
	Insert( Ptr );		// insert into list	
	return absTime;
}

unsigned long TTRACK::addText( TNoteOnList * /*NoteOnOff*/, 
								unsigned long  absTime, 
								char *, //str, 
								int, // chan, 
								FILE * /* log*/ )
{
	return absTime;
}


void TTRACK::applyRelDuration(int /* relduration */ )
{
	
}

void TTRACK::setTrackName(const char *ptr)
{
	if( !ptr )
		return;
	
	trackName = string( ptr );
}

/*
TVoiceName::TVoiceName( const char *ptr )
{
	name = NULL;
	setName( ptr );
}

TVoiceName::~TVoiceName( void )
{
	if( name )
		delete [] name;
	name = NULL;
}

void TVoiceName::setName( const char *ptr )
{
	if( !ptr )
		return;
	if( name )
		delete [] name;
	name = new char[strlen(ptr)+1];
	strcpy( name, ptr );
}
*/
string TTRACK::voiceName(int i)
{
	if( i < 0 ||
		i >= cVoice() ||
		!voiceNames )
		return trackName;
	
	return voiceNames[i];
	
}


/*!
copy all unvoiced elemnts of track into all voices
*/
void TTRACK::copyUnvoicedElements()
{
	for(int i = 0; i < cVoice(); i++ )
	{
		if( FirstNoteObject(i) ) // notes in this voice
		{
			TMusicalObject *temp = Notes;
			while( temp )
			{
				if( temp->GetVoice() == -1 ) // unvoice
				{
					// Todo: make copies of temp! and put them into the voices
					if( dynamic_cast<TMetaText *>(temp))
						//!strcmp(temp->ClassName(), TEXT_CLASS ) )
					{
						TMusicalObject *newNote = new TMetaText(  dynamic_cast<TMetaText *>(temp) );
						newNote->SetNext( temp->TMusicalObject::GetNext(-1) );
						temp->SetNext( newNote );
						newNote->SetVoice(i);
					}
					else
						temp->SetVoice( i );
				}
				temp = temp->TMusicalObject::GetNext(-1);
			} // while
		} // if
	} // for
	
	// delete now all remaining unvoiced elements
	TMusicalObject *temp = Notes;
	while( temp )
	{
		if( temp->GetVoice() == -1 ) // unvoice
		{
			TMusicalObject *delNote = temp;
			temp = temp->TMusicalObject::GetNext(-1);
			
			deleteNotes( delNote, temp );
		} // while
		else
			temp = temp->TMusicalObject::GetNext(-1);
	} // while notes	
}	

//DEL int TTRACK::cEvents()
//DEL {
//DEL 	
//DEL }

// estimate accuracy of track
// return the number of evaluated events
int TTRACK::accuracy(double &attack, double &duration)
{
	// find voice with max events
	int maxVoice = -1,
		/// voice with max events
		maxEv = 0;	
	for( int i = 0; i < cVoice(); i++ )
	{
		int ev = cEvents( i );
		if( ev > maxEv )
		{
			maxEv = ev;
			maxVoice = i;
		}
	}
	// Todo use better defaults for accuracy
	attack = 0.7;
	duration = 0.4;
	
	// evaluate accuracy in maxVoice
	if( maxVoice > -1 )
	{
		TDoubleBinList *statList = new TDoubleBinList(98,102, 10);
		
		// fill list with search values (rIOI - 1 !!!)
		TDoubleBinClass *equalClass = statList->addClass(0);      // == equal IOI	
		TDoubleBinClass *doubleClass = statList->addClass(1);	    // 1/4-1/8
		
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
		
		TDoubleBinClass *halfClass = statList->addClass(-1);
		
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
		
		
		statList->setMinMaxRel(30.0);		  
		statList->setMinweightDeltaRel( 30.0 );
		statList->createDistribution();
		
		
		int cEvents = 0,
			cExact = 0,
			cExact1 = 0,
			cExact2 = 0,
			cExactm2 = 0;
			
		double avIOIDurRatio = 0;
		double varIOIDurRatio = 0;

		TNOTE *cur = FirstNoteObject(maxVoice);
		if( cur )
			cur =  dynamic_cast<TNOTE *>(cur->GetNext(maxVoice));
		double plIOIRatio;
		while( cur &&
			  cur->GetNext(maxVoice) )
		{
			cEvents++;


			/// compare/write plIOI to list
			double ioi = cur->ioi(1,maxVoice).toDouble();
			double dur = cur->GetDuration().toDouble();
			double ioiDurRatio = dur/ioi;
			avIOIDurRatio += ioiDurRatio;
			if( cEvents > 1 )
			{
				varIOIDurRatio += pow(avIOIDurRatio/cEvents -  ioiDurRatio,2);
			}


			plIOIRatio = cur->IOIratio(-1,1);
			if( plIOIRatio > 0 )
			{		
//				ioi = plIOIRatio;
				plIOIRatio -= 1;
			}
			else if(plIOIRatio < 0)
			{		
//				ioi = -1 / plIOIRatio;
				plIOIRatio += 1;
			}

			TDoubleBinClass *tempClass;
			TDistanceStruct ds = statList->closestClass( &plIOIRatio);
			if( ds.distance < 0.5 )
				tempClass = statList->addValue(plIOIRatio);
			else
				tempClass = NULL;
			if( ds.distance == 0 )
			{
				cExact++;
			}	


			if( fabs(plIOIRatio) < 0.0001 )
				cExact1++;
			else if( fabs(plIOIRatio - 1) < 0.0001)
				cExact2++;
			else if( fabs(plIOIRatio + 1 ) < 0.0001) 
				cExactm2++;

			cur = dynamic_cast<TNOTE *>(cur->GetNext(maxVoice));
		} // while
		avIOIDurRatio /= cEvents;
		varIOIDurRatio /= cEvents;

#ifdef _DEBUG
		// statList->write( stdout );
#endif
		char isMachinePlayed = 3;
		if( varIOIDurRatio < 0.01 &&
		    avIOIDurRatio > 0.3 ) // do not evaluate percussive stuff!
		{
			isMachinePlayed = 1;
			cout << "constant duration/IOI ratio ("<< avIOIDurRatio <<"), file seems to be machine played!\n";
		}
		else
		{
			double exactRate = 0;
			if( cEvents )
				exactRate = (double)cExact / (double)cEvents;
			if( exactRate > 0.9 ) 
			{
				isMachinePlayed = 1;
				cout << exactRate << "%% exact hits, file seems to be machine played!\n";
			}
			else
			{
				isMachinePlayed = 0;
			}
		} // else
		/*	
		int cEvalEvents = 0;
		if( equalClass->Count() &&
			(equalClass->lVariance() != 0 ||
			equalClass->rVariance() != 0) )
		{
			isMachinePlayed--;
			cEvalEvents += equalClass->Count();
		}
		if( halfClass->Count() &&
			(halfClass->lVariance() != 0 ||
			halfClass->rVariance() != 0) )
		{
			isMachinePlayed--;
			cEvalEvents += halfClass->Count();
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
			cEvalEvents += doubleClass->Count();
		}			
		else if( !doubleClass->Count() )
		{
			isMachinePlayed--;
		}
		*/
		if( Parent()->getInifile()->GetValInt("USE_MACHINE_FEATURE","0","",0) <= 0 )
		{
			isMachinePlayed = 0;
			Printf("Analysis for machine-played detection is turned off\n");
		}
		if( isMachinePlayed)
		{
			attack = 1;
            // allow always durational inaccuracies
            duration = 0.8;
		}
		else // calculate acuracy
		{
			double avSigma = 0;
			int c;
			/// use largest of the three classes
			if( equalClass->Count() > doubleClass->Count() )
			{
				avSigma = equalClass->sigma();
				c = equalClass->Count();
			}
			else
			{
				avSigma = doubleClass->sigma();
				c = doubleClass->Count();
			}
			if( halfClass->Count() > c )
			{
				avSigma = halfClass->sigma();
			}
				
			/*
				alphha value generated from noisy files 5%,10%,50%
			*/
			attack = 50 * exp(-2.8217 * avSigma) + 50;
			duration = attack * 0.8;
			// Todo: estimate usefull values
			// calculate normal distribution from av precise!
			attack /= 100;
			duration /= 100;
			
		}
		delete statList;
	} // if maxVoice > -1

	/// return #of events in selected voice
	return maxEv;
}
	
int TTRACK::cEvents(int voice)
{
	int res = 0;
	TMusicalObject *temp = FirstNoteObject(voice);
	while( temp )
	{
		res++;
		temp = temp->GetNext( voice );
	}
	return res;
}



/// fill with sequence content
int TTRACK::fill( lgSequence *sequence,
				TTRACK * /*ControlTrack*/,
				int &recPPQ,
				int &recTempo)
{
	int cnotes = 0;
	// copy all notes
	lgEvent *temp = sequence->firstEvent();
	while( temp )
	{
		cnotes++;
		append( temp );
		// todo: what happens to chords?
		temp = sequence->nextEvent(temp);
	}
	// add all known tags
	int cTagNotes = 0;
	int maxVoice = 0;
	for( int i = 0; i <= maxVoice; i++ )
	{
		TNOTE *prevNote = NULL;
		lgTag *tag = sequence->firstTag();
		while( tag )
		{
			TNOTE *curNote = append( tag,
								i,
								maxVoice );
			if( curNote )
			{
				cTagNotes++;
				if( prevNote &&
				    prevNote->GetAbsTime() < curNote->GetAbsTime() )
				{
					if( prevNote->sustainOffset >
					    curNote->GetAbsTime().toLong() )
					 {
					 	prevNote->sustainOffset = 0;
					 	prevNote->SetDuration( curNote->GetAbsTime() - prevNote->GetAbsTime() );
					 }
				}
				prevNote = curNote;
			}
			// todo: what happens to chords?
			tag = sequence->nextTag(tag);
		} // while
	} // for
	if( cnotes > 0 &&
		cTagNotes > 0 )
	{
		Printf("WARNING: infile contains score-timing AND real-time information!\n");
	}
	if( cTagNotes )
	{
		recPPQ = 960;
		recTempo = 60;
	}
	else // no recording info available
	{
		recPPQ = 0;
		recTempo = 0;
	}
//	this->Debug();
	// copy all tags
	return 1;
}

void TTRACK::append(lgEvent * /*event*/)
{
	// do nothing here -> do it in TQTRACK!!

}

/*
	should be called before voice separation
*/
void TTRACK::resolveSustain()
{
//	Debug();
	TNOTE *cur = FirstNoteObject(-1);
	while( cur )
	{
		int minPitch = cur->GetMIDIPitch() - 12;
		int maxPitch = cur->GetMIDIPitch() + 12;
		// cut this to first note in [minPitch,maxPitch] and [offset,sustainOffset]
		TNOTE *temp = dynamic_cast<TNOTE *>(cur->GetNext(-1) );
		while( temp )
		{
			if( cur->sustainOffset <= 0 )
			{
				temp = NULL;
			}
			else if( temp->GetAbsTime() > cur->sustainOffset )
			{
				// no cut -> expand to sustain
				cur->SetDuration( TFrac(cur->sustainOffset,1L) - cur->GetAbsTime() );
				temp = NULL; // stop
			}
			else if( temp->GetAbsTime() >= cur->offset() )
			{
				if( temp->GetMIDIPitch() >= minPitch &&
					temp->GetMIDIPitch() <= maxPitch )
				{
					cur->SetDuration( temp->GetAbsTime() - cur->GetAbsTime() );
					temp = NULL; // stop
				}
			}
			if( temp )
				temp = dynamic_cast<TNOTE *>(temp->GetNext(-1) );
		} // while
		cur = dynamic_cast<TNOTE *>(cur->GetNext(-1) );
	} // while
//	Debug();
}

TNOTE * TTRACK::append(lgTag * /*tag*/,
						int /*voice*/,
						int & /*maxVoice*/)
{
	// do nothing here but in TQTRACK!!
	return NULL;
}
