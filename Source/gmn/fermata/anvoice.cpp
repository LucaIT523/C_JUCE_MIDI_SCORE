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

/*-----------------------------------------------------------------------
	Analyse voice classes
	-determine #voice profile of track
	(c) by Juergen Kilian 1999-2001-03, 2011
	03/05/27	code cleaning
-----------------------------------------------------------------------*/
#include <sstream>
#include <string>
using namespace std;
#include <stack>

#include "anvoice.h"
#include "h_midi.h"
#include "k_array.h"
#include "q_track.h"
#include "funcs.h"
#include <math.h> 
#include "statist.h"
#include "c_track.h"
#include "musical_obj.h"
#include "q_chord.h"

#define VOICE_LOG "voices.log"

double IOIratio( lgNote *);

//-----------------------------------------------------------------------
// values will be read from .ini at TQMIDIFILE
// extern int EQUAL_TIME;
// extern int LEGATO_TIME;
// extern int STACCATO_TIME;
// extern int ORNAMENT_TIME; 
//-----------------------------------------------------------------------
void Debug( TVoiceCountEntry *start,
			FILE *out )
{
	while(start)
	{
		start->Write(out);
		start = start->Next();
	}
}

/*
	Implements a linked list of VoiceCounting elements
	count nr of voices at each attack
*/
TVoiceCountEntry::TVoiceCountEntry( int  voices,
						 TNOTE *note )
{
	voicesI = voices;
//	timeI  = time;
	nextI  = NULL;
	notesI = 1;
	noteI = note;
#ifdef _DEBUG
	if( !note )
		printf("Error TVoiceCountEntry: noteI = NULL! ");
#endif
}

/*
	append next at end of list
*/
int TVoiceCountEntry::append( TVoiceCountEntry *next )
{
	int res = 1;

	if( !next )	// guard
		return 1;

	// search list for insert position
	TVoiceCountEntry *temp,
					*prev;
	temp = nextI;
	prev = this;

	// search for end of list
	while( temp ) 
	{
		prev = temp;
		temp = temp->Next();
	}
	prev->nextI = next;

	return 1;
	return res;
}

TVoiceCountEntry *TVoiceCountEntry::Next( void )
{
	return nextI;
}

TAbsTime TVoiceCountEntry::Time( void )
{
	if( noteI )
		return noteI->GetAbsTime();

	return -1L;
}

int TVoiceCountEntry::Voices( void )
{
	return voicesI;
}

void TVoiceCountEntry::Write( FILE *out )
{
	if( !out )
		return;

	if( Voices() < 0 )
	{
 	 	fprintf(out,"error");
	}
	double percent = 0;
	if( Voices() )
		percent = ((double)Notes())/((double)Voices());
	fprintf(out,"At: %ld/%ld,#Notes %d, Voices: %d, Percent: %f\n",Time().numerator() ,
												Time().denominator()
												,Notes(),Voices(),percent);
}

void Delete( TVoiceCountEntry *cur)
{
	TVoiceCountEntry *temp;
	while( cur )
	{
		temp = cur->Next();
		delete cur;
		cur = temp;
	}
}

/*
	returns max number of needed voices
*/
int TVoiceCountEntry::maxVoice()
{
	int res = 0;
	TVoiceCountEntry *temp;
	temp = this;
	while(temp)
	{
		if(temp->Voices() > res )
			res = temp->Voices();
		temp = temp->Next();
	}
	return res;
}

/*
	Count number of voice events with voiceI == num
	if num == -1, count complete list
*/
int TVoiceCountEntry::cVoiceEvents(int num)
{
	TVoiceCountEntry *temp;
	temp = this;
	int res = 0;
	while(temp)
	{
		if( num == -1 ||
			temp->Voices() == num )
		{
			res += temp->Notes();
		}
		temp = temp->Next();
	}
	return res;
}

// returns #of voices at time position
int TVoiceCountEntry::cVoicesAt(TNOTE *note)
{
	TVoiceCountEntry *temp;
	temp = this;
	int res = -1;
	while( temp &&
		   temp->notePtr() != note)
	{
		temp = temp->Next();
	}
	if( temp )
		res = temp->Voices();
	return res;
}

//--------------------------------------------------------------


/*
	Implemnts a linked list of TPitchTime elements
*/
TPitchTime::TPitchTime( TNOTE *ptr,
						TAbsTime time )
{
	ptrI = ptr;
	timeI  = time;
	nextI  = NULL;
}

/*
	Sorted insert of next into list
*/
void TPitchTime::SetNext( TPitchTime *next )
{
	if( !next )	// guard
		return;

	// search list for insert position
	TPitchTime *prev,
				  *temp;
	prev = this;
	temp = nextI;
	while( temp &&
			 ( next->Time() > temp->Time()
			 || (temp->Time() == next->Time()
				  &&  next->pitch() > temp->pitch())
				  ) )
	{
		prev = temp;
		temp = temp->Next();
	}
	if( prev != this ) // guard
	{
		//insert: prev->nextI >...> prev->next->nextI
		prev->SetNext( next );
		return;
	}

	// insert sorted
	if( next->Time() > Time() ||	// key1 == Time
		( next->Time() == Time() &&	// key2 == Pitch
		  next->pitch() >= pitch()
		)
	  )
	{
		next->SetNext( Next() );
		nextI = next;
	}
	else // insert before this
	{
		TNOTE *tempPtr = next->notePtr();
		TAbsTime tempAbsTime = next->Time();


		// swap values this<->next
		next->timeI  = Time();
		next->ptrI = notePtr();

		timeI = tempAbsTime;
		ptrI = tempPtr;

		// insert again
		SetNext( next );
	}
}

TPitchTime *TPitchTime::Next( void )
{
	return nextI;
}

TAbsTime TPitchTime::Time( void )
{
	return timeI;
}

TPitch TPitchTime::pitch( void )
{
	if( ptrI )
		return ptrI->GetMIDIPitch();
	
	std::cout << "Error TPitchTime: ptrI == NULL!" << endl;
	return -1;
}
//-------------------------------------------------------------
TVoiceCountList *GetVoiceProfile( TQTRACK *track )
{
	TNOTE *from;
	from = track->FirstNoteObject(-1 /* all voices */);
	return GetVoiceProfile(from, NULL);
}

/*
	Determine voiceCount profile
	- !ornament notes needs to be filtered before
	- remove legato needs to be called before
*/
TVoiceCountList *GetVoiceProfile( TNOTE *from,
							  TNOTE *to)
{
	TNOTE *current;
	TAbsTime curTime,			// attackpoint of current
				evTime = 0L,
				curEndTime;		// endpoint of current


	current = from;

	if( !current )
		return NULL;

	TVoiceCountList *VoiceCount = NULL;
	TVoiceEvent VoiceEvent;
	TPitchTime *tempPitchTime;

	TPitchTime *tempOffset,
			   *offsetList = NULL;

	TNOTE *prev = NULL;
	int soundingVoices = 0; // currently sounding voices
	while( current &&
		   current != to )
	{
		if( prev != current )
			// calc new values if current has changed
		{
			curTime = current->GetAbsTime();
			curEndTime = curTime + current->GetDuration();
			// insert sorted into offsetList
			tempOffset  = new TPitchTime( current,
										  curEndTime);
			 if( offsetList )
					 offsetList->SetNext( tempOffset );
			 else
					offsetList = tempOffset;
			prev = current;
		}
		// get next noteOn | noteOff event
		VoiceEvent = GetNextEvent( current,
								   offsetList );
		if( VoiceEvent.absTime >= 0L ) // Check for errors
		{
			if( VoiceEvent.deltaVoice >= 0 ) // note on event
			{
				// calc delta Voice
				if( VoiceEvent.deltaVoice > 0)
				// VoiceEvent == noteOn 
				{
					soundingVoices += VoiceEvent.deltaVoice;
				}
				else // deltaVoice == 0 <- noteOn.time == noteOff.time
				// VoiceEvent == noteOff
				{
					// pop first element of offset list
					tempPitchTime = offsetList->Next();
					delete offsetList;
					offsetList = tempPitchTime;
				}

				// create new entry in result
				if( VoiceCount )
				{
					TVoiceCountEntry *temp;
					temp = new TVoiceCountEntry( soundingVoices,
										current);
					VoiceCount->append(temp);
				}
				else
				{
					VoiceCount = new TVoiceCountList( soundingVoices,
										current);
				}
				
				// process noteOn and noteOff of next note
				current = NOTE(current->GetNext(-1 /* all Voices*/));
			}
			else // VoiceEvent.deltaVoice < 0 
			// VoiceEvent == noteOff
			{
				// process noteOff's
				evTime = VoiceEvent.absTime;
				soundingVoices += VoiceEvent.deltaVoice;
				// pop first element of offset list
				TPitchTime *tempPitchTime;
				tempPitchTime = offsetList->Next();
				delete offsetList;
				offsetList = tempPitchTime;
			} // if abstime >= 0				
		} // if abstime >= 0
	} // while current
	while( offsetList )
	{
		tempPitchTime = offsetList->Next();
		delete offsetList;
		offsetList = tempPitchTime;
	}
	// set voices to max voices during duration
	VoiceCount->recalcEntries();
return VoiceCount;

}

/* average analysis of voice profile not used anymore,   */
/*
void analyseVoices( TQTRACK *track,
						  FILE *log )
{
	FILE *out = log;
// ----analyse voice profile---------------------------------------
	TVoiceCountEntry *temp, *temp2, *VoiceCount;
	VoiceCount = GetVoiceProfile( track );

	TDecayAverage *flAverage;
	TDecayStruct decayStruct[6];

// check values for voice count
	decayStruct[0].decay = 0.9;
	decayStruct[0].range = 1;
	decayStruct[1].decay = 0.89;
	decayStruct[1].range = 2;
	decayStruct[2].decay = 0.85;
	decayStruct[2].range = 4;

	decayStruct[3].decay = 0.8;
	decayStruct[3].range = 8;
	decayStruct[4].decay = 0.7;
	decayStruct[4].range = 16;
	decayStruct[5].decay = 0.6;
	decayStruct[5].range = 32;

	flAverage = new TDecayAverage(6,		   // # of classes
							      decayStruct, // {decay,range}
								  1.4);		   // pBreakLimit

	out = fopen("average.txt","wt");
	temp = VoiceCount;
	while( temp )
	{
		flAverage->addValue(temp->Voices());
		temp = temp->Next();
	}
	flAverage->writeValues(out);

	flAverage->iterate(0); // reset
	do
	{
		flAverage->writeAvs(out);
	}
	while( flAverage->iterate(1) );


	fclose(out);

	delete flAverage;


	temp = VoiceCount;
	while( temp )
	{
		temp->Write(out);
		temp2 = temp;
		temp = temp->Next();
		delete temp2;
	}
}
-----------------------------------------------------*/

/*
	Get AbsTime of next Event. note -> noteOn Event
										onsetList -> noteOff Event
	result =
		TVoiceEvent
	error
		absTime == -1
*/
TVoiceEvent GetNextEvent( TNOTE *note,
						  TPitchTime *offsetList)
{
	TVoiceEvent res;
	TAbsTime onTime,
				offTime;

	if( note && offsetList ) // compare, return earliest event
	{
		onTime = note->GetAbsTime();
		offTime = offsetList->Time();
		if( onTime > offTime )
		{
			res.absTime = offTime;
			res.deltaVoice = -1;
			res.ptr = offsetList->notePtr(); 
		}
		else if( onTime < offTime )
		{
			res.absTime = onTime;
			res.deltaVoice = 1;
			res.ptr = note;
		}
		else // ==
		{
			res.absTime = offTime; // == onTime
			res.deltaVoice = 0;
			res.ptr = note;
		}
	}
	else if( offsetList ) // no more noteOn, note == NULL
	{
			res.absTime = offsetList->Time(); // == onTime
			res.deltaVoice = -1;
			res.ptr = offsetList->notePtr();
	}
	else if( note ) // offsetList == NULL
	{
			res.absTime = note->GetAbsTime(); // == onTime
			res.deltaVoice = 1;
			res.ptr = note;
	}
	else // guard
	{
			res.absTime = -1; // error
			res.deltaVoice = 0;
			res.ptr = NULL;
#ifdef _DEBUG
			Printf("Error GetNextEvent all params == NULL!");
#endif
	}

	return res;
}

/*
	remove noteOff Counters from TVoiceCountEntry-list
*/
void ReCalc( TVoiceCountEntry *start )
{
	TVoiceCountEntry *next;
	int deltaVoice;
	while( start )
	{
		next = start->Next();
		if( next )
		{
			// Check if noteOffs have been counted
			if( start->Voices() > next->Voices() )
			{
				deltaVoice = start->Voices() - next->Voices();
				// remove NoteOff Notes
				next->SetNotes( next->Notes() - deltaVoice );
			}

		}
		// process next element of list
		start = next;
	}
}

/*
	search for notes that can start at the same time as *start
	remarks
	-if glissando, arpeggio and grace notes are detected,
	  res = -1
	return:
		-ptr to first that doesn't start at same time as start
		-res = average between first and last detected attackpoint

  Seems to be not used anymore!!!
*/
TNOTE *CheckEqualWindow( TNOTE *start,
						int recTempo,
						TAbsTime &res,
						int ORNAMENT_TIME,
						int EQUAL_TIME)
{
	TNOTE *next;
	char end = 0;
	res = -1;
	TAbsTime DiffTime,
				DiffTicks;

	TAbsTime startAttack,
				prevAttack,
				prevEnd,
				nextAttack;

	if( !start )	// guard
		return 0;

	startAttack = start->GetAbsTime();
	prevAttack = startAttack;
	nextAttack = startAttack;
	prevEnd = startAttack + start->GetDuration();

	// search for notes in equal distance
	next = NOTE(start->GetNext(-1/* all Voices*/));
	while( next && !end )
	{
		nextAttack = next->GetAbsTime();
		// can't never occur if Track is sorted
#ifdef _DEBUG
		if( prevAttack > nextAttack )
			Printf("ERROR wrong sorting in CheckEqualWindow!\n");
#endif


		DiffTicks  = prevAttack - nextAttack;
		DiffTime = scoreTime2ms(
							recTempo,
							DiffTicks );

		if( abs(DiffTime) < EQUAL_TIME &&
			 !(DiffTime == 0L) )
		{
			  // Check for grace, gliss, arpeggio
			  // copy from q_track.cpp
			  DiffTicks = (prevEnd -
							  next->GetAbsTime());
			  DiffTime = scoreTime2ms(
							recTempo,
							DiffTicks);
			  if( DiffTime > 0L &&
					DiffTime < ORNAMENT_TIME )
			  {
					// provide cancel
					nextAttack = startAttack.incNum(2*EQUALTIME);
					end = 1;
			  } // if < GRACE
		}
		else // out of window
		{
			end = 1;
		}
		prevAttack = nextAttack;
		prevEnd = nextAttack + next->GetDuration();
		next = NOTE(next->GetNext(-1/*all voices*/));
	}
	// Check if overall distance <= EQUAL_DISTANCE
	DiffTicks  = startAttack - nextAttack; // nextAttack = lastAttack in window
	DiffTime = scoreTime2ms(	recTempo,
							DiffTicks);

	if( abs(DiffTime) < EQUAL_TIME )
	{ // all notes are in window
		res = (startAttack + nextAttack) / (TFrac)2;
	}
	else
	{
		res = -1;
	}
	return next;
}


/*
	Calculate StandardDeviation etc for Attackpoint and duration
*/
/*
DevValues GetDeviations( TQNOTE *start,
							 TQNOTE *end )
{
	int voice=-1;
	if(start)
		voice = start->GetVoice();


	DevValues res;
	res.BestAttDev = 0;
	res.SecondAttDev = 0;
	res.BestAttSSQ = 0;
	res.SecondAttSSQ = 0;
	res.BestDurDev = 0;
	res.SecondDurDev = 0;
	res.BestDurSSQ = 0;
	res.SecondDurSSQ = 0;


	TAbsTime BestAttack,
			SecAttack,
			 BestDuration,
			 SecDuration;


	TFrac BestAttDevSum ,
			 SecAttDevSum ,
			 BestAttSSQSum = 0L,
			 SecAttSSQSum = 0L,
			BestDurDevSum = 0L,
			 SecDurDevSum = 0L,
			 BestDurSSQSum = 0L,
			 SecDurSSQSum = 0L,
			 BestDDuration ,
			 SecDDuration;
	 double
			 Count = 0; // =double for arithmetic operations

	while( start &&
			 start != end )
	{
//	  if( start->GetDuration() > 0 ) // skip emtpy notes
//	  if( !start->Ornament() )
//	  &&		!start->Tempo()   // skip emtpy notes

	  {
		Count++;

		// Calculate distances to Attackpoint
		BestAttack = start->qAttackpoint( &SecAttack );
		BestAttack -= start->GetAbsTime();
		SecAttack -= start->GetAbsTime();
		// > 0 -> played < quantized
		// < 0 -> quantized < played


		// Calculate Deviations
		BestAttDevSum += BestAttack;
		SecAttDevSum += SecAttack;
		BestAttSSQSum += (BestAttack * BestAttack);
		SecAttSSQSum += (SecAttack * SecAttack);

		// calculate Distance for duration
		BestDuration = start->qDuration( &SecDuration );

		BestDDuration = 1 - (BestDuration.toDouble() / (start->GetDuration().toDouble()));
		SecDDuration = 1 - (SecDuration.toDouble() / (start->GetDuration().toDouble()));
		// > 0 played > quantized
		// < 0 quantized > played

		// calculate deviations for duration
		BestDurDevSum += BestDDuration;
		SecDurDevSum += SecDDuration;
		BestDurSSQSum += (BestDDuration * BestDDuration);
		SecDurSSQSum += (SecDDuration * SecDDuration);
	  }
	  // Next note
	  start = QNOTE(start->GetNext(voice));
	}

	if( Count )
	{
		// normalize deviations
		res.BestAttDev = BestAttDevSum.toDouble() / Count;
		res.SecondAttDev  = SecAttDevSum.toDouble() / Count;
		res.BestAttSSQ= sqrt(BestAttSSQSum.toDouble() / Count);
		res.SecondAttSSQ  = sqrt(SecAttSSQSum.toDouble() / Count);

		res.BestDurDev = BestDurDevSum.toDouble() / Count;
		res.SecondDurDev  = SecDurDevSum.toDouble() / Count;
		res.BestDurSSQ= sqrt(BestDurSSQSum.toDouble() / Count);
		res.SecondDurSSQ  = sqrt(SecDurSSQSum.toDouble() / Count);
	}
	res.Count = (int)Count;
	return res;
}
*/

/*
	markChords  in track
	remarks:
		- remove legato needs to be called before
		- needs to be called BEFORE SplitVoices
*/
/*
void markChords( TQTRACK *track, TVoiceCountEntry *voiceProfile )
{
// implement optional parameters for markChord
	
	TQNOTE *current,
		   *next;
	int couldBeChord = 0;

	int maxVoices,
		voiceEvents;

	if(!voiceProfile && !track)
		return;

	// evaluate voice profile
	maxVoices = voiceProfile->maxVoice();
	voiceEvents = voiceProfile->cVoiceEvents(-1);
	int *voiceCount,
		i;

	voiceCount = new int[maxVoices];
	for(i=0; i < maxVoices; i++ )
	{
		voiceCount[i] = voiceProfile->cVoiceEvents(i+1);
	}

	double comp;
	// avluate track
	current = (TQNOTE*)(track->FirstNoteObject(-1));
	TQChord *temp;
	while( current )
	{
		couldBeChord = 0;
		// check for  possible chords
		next = QNOTE(current->GetNext(-1));
		while( next && 
				next->GetAbsTime() == current->GetAbsTime() &&
			   next->GetDuration() == current->GetDuration() )
		{
			couldBeChord++;
			next = QNOTE(next->GetNext(-1));
		}
		// next points to first note after current|chord
		if(couldBeChord > maxVoices )
		{
			Printf("Error:markChords chordsize %d > maxVoices %d!\n",
					couldBeChord,maxVoices);
			couldBeChord = 0;
		}
		if( couldBeChord )
		// check if chord allowed
		{
			// compare #voices - chordsize
			comp = (double)voiceCount[couldBeChord]/(double) voiceEvents;
			if( comp < 0.3 )
			{
				// create new Chord
				// insertion into track list is done by create
				temp = new TQChord( current,
									QNOTE(current->GetNext(-1)),
									next,
									track );

			}
		}
		current = next;
	} // while
	delete [] voiceCount;
} // markChords

*/

/*
	get IOI(now,next)/IOI(next,nnext) 
*/
/*
double GetIOIRatio(TCLICKNOTE *current)
{
	double res;
	double IOI1, IOI2;
	TCLICKNOTE  *middle;
//			*next;

	if(!current)
		return 0;
	IOI1 = (double)GetIOI(current).toDouble();

	middle = CLICKNOTE(current->GetNext(-1));
	if(middle && 
	   middle->GetNext(-1) )
	{
		IOI2 = (double)GetIOI(middle).toDouble();
	}
	else // use note end
	{
		IOI2 = IOI1;
		//		IOI2 = current->GetDuration().toDouble();
	}


//	next = current->GetNext(current->GetVoice());
//	if(!next)
//		return 0;


	if(IOI1<=IOI2)
		res = IOI2/IOI1;
	else
		res = IOI1/IOI2 * -1;

	return res;

}
*/


/*
	calculate slope between current and next
	-> ~ 1. movement of (1,current) and (2,next) {x,y}
*/

double GetIOIRel(double current, double next)
{
	double res = 0;

	res = (next - current);
	
	//	1 ~ 1 = 0
	//	1 ~ 1.2 = 0.2
	//	1.2 ~ 1 = -0.2
	//	1 ~ -1.2 = -0.2
	//	-1.2 ~ -2= -0.8
	//	4 ~ -4 = -8
	//	-4 ~ 4 = 8
	

	// normalize slope

	if(next > 0 && current < 0 ) // positive slope
			res -= 2;
	else if(next < 0 && current > 0 ) // negative slope
			res += 2;

	return res;
}



/*
complete implementation and, use compareIOIList
*/
/*
void compareIOIList(TIOIList *list1, TIOIList *list2)
{
	int cEntries1,
		cEntries2;

	cEntries1 = list1->cEntries();
	cEntries2 = list2->cEntries();


	double  sum1,
			sum2,
			absSum1,
			absSum2;

	sum1 = list1->sum();
	sum2 = list2->sum();
	absSum1 = list1->absSum();
	absSum2 = list2->absSum();

	int i = 0;
	double avDistance = 0,
			sumDistance = 0,
			absSumDistance = 0;

	sumDistance = sum2-sum1;
	absSumDistance = absSum2/absSum1;

	double e1, e2;
	e1 = list1->entry(i);
	e2 = list2->entry(i);
	// compare each item
	while(e1 != 0 &&
		  e2 != 0)
	{
		avDistance += IOIDistance(e1,e2);
		i++;
		e1 = list1->entry(i);
		e2 = list2->entry(i);
	}
	// calculate average
	avDistance /= i;
	// calculate varianz! to distance
}
*/


/*
	analyse relation between succeeding IOIs in all tracks
*/
/*
void analyseIOI(TQMIDIFILE *file)
{
	FILE *out;
	out = NULL;
	#ifdef _DEBUG
		char *filenameCp,	// copy if MIDIFILENAME
			 *ending,		// ptr to .MID
			 *filename;		// new outfilename
		filenameCp = file->filename();
		filename = new char[strlen(filenameCp)+12];		
		strcpy( filename, filenameCp);
		ending = strstr(filename, ".");
		strcpy( ending, "_ioi.txt");
		out = fopen(filename,"wt");
	#endif

	Printf("Analyzing IOI...\n");
	TQTRACK *temp;
	temp = (TQTRACK *) file->FirstTrack();
	int i = 1;
	while( temp ) // for all tracks
	{
		Printf("Track %d: ",i);
		if(out)
			fprintf(out,"\nTrack %d\n", i);
		analyseIOI( temp, out );
		temp = (TQTRACK *)temp->Next();
		i++;
      Printf("\n");
	}



	if( out )
		fclose(out);
#ifdef _DEBUG
	delete [] filename;
#endif
	Printf("Analysis done.\n");
}
*/


/*
	analyse relation between succeeding IOIs in a single tracks
*/
/*
void analyseIOI(TQTRACK *track,
					 FILE *out)
{
	int cVoice, i;
	TQNOTE *current;


	if(!track)
		return;

	TIOIList *IOIList;
	TIOIratioList *TIOIratioList;

	cVoice = track->GetCVoice();

	// evaluate track
	for(i=0; i < cVoice; i++ )
	{
		Printf("Voice %d, ",i);
		current = (TQNOTE*)(track->FirstNoteObject(i));
		// get IOI list of voice i
		IOIList = new TIOIList(current, (TNOTE *)NULL);

		// get 1. Ableitung for IOIList
		TIOIratioList = new TIOIratioList(IOIList);

		// check voice
		if( out )
		{
			fprintf(out,"voice %d\n", i);
			IOIList->write(out);
			TIOIratioList->write(out);
			fprintf(out,"\n");
		}
		//------------
		fprintf(out,"----- ioi pattern------------------");
		IOIList->writePattern(out);
		fprintf(out,"----- end pattern------------------");
		delete IOIList;
		delete TIOIratioList;
	}// for
} // analyseIOI
*/
#ifdef use_TIOILIst
//-------------- list of double values ------------------
TfloatList::TfloatList(int entries)
{
	cEntriesI = 0;
	entriesI  = NULL;

	init(entries);
}

void TfloatList::init( int entries )
{
	// if 2nd init -> delete 1st List
	if(cEntriesI)
		deleteList();

	// init
	cEntriesI = entries;
	if(entries > 0)
	{
		entriesI = new double[entries];
	}
	else
	{
		entriesI = NULL;
		cEntriesI = 0;
	}
	int i;
	// init values
	for(i = 0; i < cEntriesI; i++ )
	{
		entriesI[i] = 0;
	}
} // init

TfloatList::~TfloatList(void)
{
	deleteList();
}

void TfloatList::deleteList(void)
{

	if(entriesI)
		delete [] entriesI;
	entriesI = NULL;
	cEntriesI = 0;
}

double TfloatList::entry(int i)
{
	if(i >= 0 &&
		i < cEntriesI )
	{
		return entriesI[i];
	}
	else

		return 0;
}

void TfloatList::setEntry( int i, double val)
{
	if(i >= 0 &&
		i < cEntriesI )
	{
		entriesI[i] = val;
	}
}

// return the weight of entry i
double TfloatList::weight( int i)
{
	double prev = 0,
			next = 0,
			res;
	// check limits
	if(i < 0 ||
		i >= cEntriesI )
		return 0;

	// get prev and next entry
	if(i+1 < cEntriesI)
		next = entry(i+1);
	else
		next = entry(i);

	if( i > 0 )
		prev = entry(i-1);
	else
		prev = entry(i);

	// calculate weight
//Todo implement weight function
	res = (entry(i)-prev) + (entry(i)-next);
	return res;
}

int TfloatList::isAnchor( int i)
{
	int anchor = 0;
	// mark anchor points
	if(entry(i) > 1.5)  // otherwhise can't be anchor!
	{
		// compare to previous
		if( i > 0 &&
			entry(i) > 1.49 * entry(i-1) )
			anchor++;
		
		// compare to next
		if( i < cEntriesI-1 )
		{
			if( entry(i) > 1.49 * entry(i+1) )
				anchor++;
		}
		else if(anchor) // mark last note in any case!
			anchor++;
	}
	return anchor;
} // isAnchor

int TIOIList::isAnchor(int i)
{
	return TfloatList::isAnchor(i);
}
int TIOIratioList::isAnchor(int i)
{
	return TfloatList::isAnchor(i);
}
/*
	write entry(i) to file,
	no separator will be added here
*/
void TfloatList::writeEntry(int i, FILE *out)
{
	if( i<0 )
		return;
	else if( i >= cEntriesI )
		return;



	int anchor;
	anchor = isAnchor( i );
		/*
		if( anchor > 1 )
			fprintf( out, "*");
*/
		fprintf(out,"%.3f",entry(i));
		/*
		if( anchor > 1 )
		{
			fprintf( out, "(%.3f)",weight(i));

		}
		*/

}

/*
	write complete list to file,
	separated by ' ' (space) 
*/
void TfloatList::write( FILE *out )
{
	int i;

	if( !out )
		return;
	for(i = 0; i < cEntriesI; i++ )
	{
		writeEntry(i, out );
		fprintf(out,"\n");
	}

} // write


// write in pattern syntax
void TfloatList::write(int from, 
						int to,
						FILE *out)
{
	int i,
		lock = 0; // flag

	for( i = from; i <= to; i++)
	{
		if( entry(i) != 1 )
			lock = 0;

		if( !lock )
		{
			TfloatList::writeEntry(i,out);
			fprintf(out, " "); // separate

			if( entry(i) == 1 ) // skip multiple "1"
				lock = 1;
		}

	}
	fprintf(out, "\n");
}

void TfloatList::writePattern( FILE *out)
{
	int i = 0,	// index 
		lock,	// flag
        pLength; // length of current pattern

	if( !out )
		return;

	pLength = 0;
	while( i < cEntriesI )
	{

		lock = 1;
		if( entry(i) != 1 )
			lock = 0;

		if( !lock )
		{
			pLength++;
			TfloatList::writeEntry(i,out);
			fprintf(out, " "); // separate

			if( entry(i) == 1 ) // skip multiple "1"
				lock = 1;
		}
		
		
		// end of pattern?
		while( isAnchor(i) && 
			    pLength > 3
				) 
		{
			pLength = 0;
			fprintf(out, "\n"); // next pattern
		}
		i++;
	} // while
} // write Pattern


// -------------------------------------------------------

// ---------- List of Ableitungen ----------------
//Todo rename TIOIratioList etc
/*
	entry(i) = ioi(i+1) - ioi(i)
*/
TIOIratioList::TIOIratioList( TIOIList *IOIList ) : TfloatList(0)
{
	int i;
	double val;

	if(IOIList)
		init(IOIList->cEntries()-1);

	for(i=0; i < cEntries(); i++)
	{
		// calulate relation between i and i+1
		val = GetIOIRel(IOIList->entry(i),
							 IOIList->entry(i+1));
		setEntry(i, val);
	}
#ifdef _DEBUG
	IOIListI = IOIList; // store for debug
#endif
}


void TIOIratioList::writeEntry( int i, FILE *out )
{
#ifdef _DEBUG
		IOIListI->writeEntry(i+1, out );
		fprintf(out,"; ");
#endif
	TfloatList::writeEntry(i,out);
}
void TIOIratioList::write( FILE *out )
{
	fprintf(out,"\n TIOIratioList:----------\n");
#ifdef _DEBUG
	IOIListI->writeEntry(0, out );
#endif
	fprintf(out,"---; ");
	fprintf(out,"\n");

	TfloatList::write(out);
}

// ---------------------------------------------
// -------- list of IOI values ----------------

TIOIList::TIOIList(TNOTE *from, int c) : TfloatList(c)
{
	TNOTE *current;
	int count = 0,
		i;
	double cur_IOIratio;

	count = c;

#ifdef _DEBUG
	if( count )
		notes = new PTNOTE[count+1];
	else
		notes = NULL;
	cnotes = NULL;
#endif

	// calulate
	current = from;
	for(i=0; i < count; i++)
	{
		if(current )
		{
#ifdef _DEBUG
			notes[i] = current;
#endif
			cur_IOIratio = GetIOIRatio(current);
			if(cur_IOIratio != 0)
			{
				// store cur_IOIratio in List
				setEntry(i, cur_IOIratio);
			}
			current = NOTE(current->GetNext(current->GetVoice()));
		}
		else // end
		{
			count = i;
			cEntriesI = i;
		}
	} // for
#ifdef _DEBUG
	notes[i] = current;
#endif
	// return list and count
}


/*!
	entry(i) = |attack(i-1) attack(i)| / |attack(i) attack(i+1)|
	entry(last) = |attack(i-1) attack(i)| / |attack(i) end(i)|
*/
TIOIList::TIOIList(TNOTE *from, TNOTE *to) : TfloatList(0)
{
	TNOTE *current;

	int count = 0,
		i;
	double cur_IOIratio;

	current = from;

	// count items
	while(current && current != to)
	{
		count++;
		current = NOTE(current->GetNext(current->GetVoice()));
	}

	if(count > 0)
	{
		init(count);
	}
//#ifdef _DEBUG
	if( count )
		notes = new PTNOTE[count+1];
	else
		notes = NULL;
	cnotes = NULL;
//#endif
	// calulate
	current = from;
	for(i=0; i < count; i++)
	{
		cur_IOIratio = GetIOIRatio(current);
		if(cur_IOIratio != 0)
		{
			// store cur_IOIratio in List
			setEntry(i, cur_IOIratio);
#ifdef _DEBUG
			if( notes )
				notes[i] = current;
#endif
		}
		current = NOTE(current->GetNext(current->GetVoice()));
	} // for
#ifdef _DEBUG
	if( notes )
		notes[i] = current;
#endif
	// return list and count
} //





/*!
	entry(i) = |attack(i-1) attack(i)| / |attack(i) attack(i+1)|
	entry(last) = |attack(i-1) attack(i)| / |attack(i) end(i)|
*/
TIOIList::TIOIList(lgEvent *from, lgEvent *to) : TfloatList(0)
{
	lgObject *current;

	int count = 0,
		i;
	double cur_IOIratio;

	current = from;

	// count items
	while(current 
		  && current != to)
	{
		count++;
		current = current->next();
	}

	if(count > 0)
	{
		init(count);
	}

	
	// calulate
	current = from;
	for(i=0; i < count; i++)
	{
		cur_IOIratio = ::IOIratio( dynamic_cast<lgNote *>(current));
		if(cur_IOIratio != 0)
		{
			// store cur_IOIratio in List
			setEntry(i, cur_IOIratio);
		}
		current = current->next();
	} // for
} //


void TIOIList::recalc(TCLICKNOTE *from, TCLICKNOTE *to)
{
	TCLICKNOTE *current;

	int count = 0,
		i;
	double cur_IOIratio;

	current = from;
	current = CLICKNOTE(current->GetNext(-1));


	// count items
	while(current && current != to)
	{
		count++;
		current = CLICKNOTE(current->GetNext(-1));
	}
	count--;

	if(count > 0) // re-init 
	{
		init(count); // re-init floatList
	}
	cnotes = new PTCLICKNOTE[count+2];

	// calulate
	current = from;
	for(i=0; i < count+2; i++)
	{
		cnotes[i] = current;
		current = CLICKNOTE(current->GetNext(-1));
	}
	current = from;
	for(i=0; i < count; i++)
	{
		cur_IOIratio = GetIOIRatio(current);
		if(cur_IOIratio != 0)
		{
			// store cur_IOIratio in List
			setEntry(i, cur_IOIratio);
		}
		current = CLICKNOTE(current->GetNext(-1));
	} // for
	notes = NULL;


}


/*
	Create IOIList from CLicktrack of HMidifile
*/
TIOIList::TIOIList(TCLICKNOTE *from, TCLICKNOTE *to) : TfloatList(0)
{
	recalc(from, to );
}





double TIOIList::entry(int i)
{
	double res;
	if(i >= 0 &&
		i < cEntries() )
	{
		res = TfloatList::entry(i);
	}
	else
		res = 0;
	return res;
}

void TIOIList::writeEntry( int i, FILE *out )
{
	if( i >= cEntriesI )
		return;

#ifdef _DEBUG
	// include note debug
	if(i==0 && notes)
	{
		notes[0]->GetAbsTime().Write(out);
		fprintf(out,"; ");
		notes[0]->WritePitch(out);
		fprintf(out,"; ");
		GetIOI(notes[0]).Write(out);
		fprintf(out,"; ");
		fprintf(out,"---; ");
		fprintf(out,";\n");
	}
	else if(i==0 && cnotes)
	{
		cnotes[0]->Debug(out);
		fprintf(out,"; ");
		fprintf(out,"---; ");
		fprintf(out,";\n");
	}

	
	if(notes && notes[i+1])
	{

		notes[i+1]->GetAbsTime().Write(out);
		fprintf(out,"; ");
		notes[i+1]->WritePitch(out);
		fprintf(out,"; ");
		GetIOI(notes[i+1]).Write(out);
		fprintf(out,"; ");
	}
	else if( notes && notes[i] )
	{
		fprintf(out,"*");
		notes[i]->GetAbsTime().Write(out);
		fprintf(out,"; ");
		notes[i]->WritePitch(out);
		fprintf(out,"; ");
		GetIOI(notes[i]).Write(out);
		fprintf(out,"; ");
	}
	else if(cnotes && cnotes[i+1] && i+1 < cEntriesI )
	{
		cnotes[i+1]->Debug(out);
		fprintf(out,"; ");
		GetIOI(cnotes[i+1]).Write(out);
		fprintf(out,"; ");
	}
	else if( cnotes && cnotes[i] )
	{
		fprintf(out,"*");
		cnotes[i]->Debug(out);
		fprintf(out,"; ");
		GetIOI(cnotes[i]).Write(out);
		fprintf(out,"; ");
	}
	else
		fprintf(out,"---; ");



#endif
	TfloatList::writeEntry(i,out);
	fprintf(out," ");
	if( entry(i) > 0 )
		fprintf(out,"%.3f",entry(i)-1);
	else
		fprintf(out,"%.3f",entry(i)+1);

}

void TIOIList::write( FILE *out )
{
	fprintf(out,"\n IOIList:---------------\n");
	TfloatList::write(out);
}

double TfloatList::sum( void )
{
	int i;
	double res = 0,
			temp;
	for(i = 0; i < cEntries(); i++ )
	{
		temp = entry(i);
		res += temp;
	}
	return res;
} // sum

double TfloatList::absSum( void )
{
	int i;
	double res = 0,
			temp;
	for(i = 0; i < cEntries(); i++ )
	{
		temp = entry(i);
		// normalize range
		if(temp < 0)
			temp *= -1;

		res += temp;
	}

	return res;
} // absSum
#endif
// --------------------------------


/*
	return 1 if val is nearly equal to [-1..1]
	remarks:
		val should be an IOIList entry
*/
int isOne(double val )
{
	if( val < 1.1 &&
		val > -1.1)
		return 1;

	return 0;
}





#ifdef use_TIOIList
/* 
	quantize entries to main values like 1 1.5 2 2.5 ,...

*/
TstatVals TIOIList::preQuantize( TCLICKTRACK * /*cTrack*/ ) 
{

		int i,
		iVal;
	double fVal;
	TstatVals res;
	res = TstatVals();

	for( i = 0; i < cEntries(); i++ )
	{
		// round to closest natural
		if( entry(i) > 0 )
			iVal = (int)(entry(i) + 0.5);
		else
			iVal = entry(i) - 0.5;
		fVal = iVal; // convert back to double


		if( isOne(entry(i)) )
			setEntry(i,1);
		else if( fabs( fVal - entry(i)) < 0.1 ) // compare to naturals
		{			
				setEntry(i, fVal);
		}
		else if( fabs(1.5 - entry(i)) < 0.1 ) // compare to other values
		{
			setEntry(i,1.5);
		}
		else if( fabs(-1.5 + entry(i)) < 0.1 ) // compare to other values
		{
			setEntry(i,-1.5);
		}
	} // for
	/*
	TstatCount *statCount;
	int maxStats = this->cEntries(),
		cStats = 0;

	statCount = new TstatCount[maxStats]; //  optimize array size?
	// init statistical array
	for( i = 0; i < maxStats; i++ )
	{
		statCount[i].cVals = 0;
	}

	// fill statCount

	
	delete [] statCount;
*/
	// Todo check this function
	return res;
} // TIOIList::preQuantize


/*
	compare complete list1 to list2[segStart..segEnd]
	ignore "1" entries
*/
double distance(TIOIList *list1, 
			TIOIList *list2,
			int segStart,	// index in list2
			int segEnd)		// index in list2
{
	int i1,	// index list1
		i2, // index list2
		c = 0;	// count pairs

	double res = 0;
	if(!list1 || !list2)
		return 100;

	if( segEnd < segStart )
		segEnd = list2->cEntries() - 1;

	if( segEnd > list2->cEntries() )
		return 100;

	i1 = 0;
	i2 = segStart;
	int lock1 = 0,	// flag
		lock2 = 0;	// flag
	while( i1 < list1->cEntries() &&
			i2 < list2->cEntries() &&
			 i2 <= segEnd )
	{
		// skip multiple "1"
		if( (lock1 && !isOne(list1->entry(i1))) ||
			(lock2 && !isOne(list2->entry(i2))) ||
			(!lock1 && !lock2)   ||
			(lock1 && lock2 ) )
		{
			res += fabs(IOIDistance(list1->entry(i1),
					list2->entry(i2)));
			lock1 = 0;
			lock2 = 0;
			c++;
		}

		if( isOne( list1->entry(i1) ) )
			lock1 = 1;
		else
			lock1 = 0;


		if( isOne( list2->entry(i2)) )
			lock2 = 1;
		else
			lock2 = 0;

		if( !lock1 && !lock2 )
		{
			i1++;
			i2++;
		}
		else if( lock1 && lock2 )
		{
			i1++;
			i2++;
			lock1 = 0;
			lock2 = 0;
		}
		else if( lock1 ) // keep list2
		{
			i1++;
		}
		else if( lock2 ) // keep list1
		{
			i2++;
		}
	}
	res /= c;
	return res;
}

/*
	compare complete list1 to list2[segStart..segEnd]
*/
double distance(TIOIratioList *list1, 
			   TIOIratioList *list2,
			   int segStart,	// index in list2
			   int segEnd)		// index in list2
{
	int i1 = 0,
		i2 = 0;
	double res = 0;
	if(!list1 || !list2 )
		return 100;

	i2 = segStart;

	i1 = 0;
	while( i1 < list1->cEntries() &&
		   i2 < list2->cEntries() && 
		   i2 <= segEnd )
	{
		if( i1 >= list1->cEntries() )
			res += list2->entry(i2);
		else if( i2 >= list2->cEntries() )
			res += list1->entry(i1);
		else
		{
			res += fabs(list1->entry(i1) - list2->entry(i2));
		}
		i1++;
		i2++;
	} // while

	if( !i1 ) // loop was not called
		res = 1;
	else
		res /= i1;
	return res;
}

#endif

// fill floatingAvergae with values of voiceProfile

void addValues(TFloatingAverage *floatingAverage,
			   TVoiceCountEntry *voiceProfile )
{
	int i,
		voiceEvents;

	if( !floatingAverage ||
		!voiceProfile )
		return;

	voiceEvents = voiceProfile->cEntries();
	for(i = 0; 
	    i < voiceEvents;
		i++ )
	{
		if( voiceProfile )
		{
			floatingAverage->addValue(voiceProfile->Voices());
			voiceProfile = voiceProfile->Next();
		}
#ifdef _DEBUG
		else
		{
			printf("\nError in addValues: voiceProfile == 0!\n");
		}
#endif
	}
}


#ifdef kjkh
/*
	Count number of notes which are outside the
	quantization limit
*/
int CountOverDurationLimit(
							TQNOTE *start,
							TQNOTE *end,
						  double max,	// [percent]
						  double min )	// [percent]
{
	int res = 0;
	double BestDuration,
			 SecDuration;

	while( start &&
			 start != end )
	{
//		if( start->GetDuration() ) // skip empty notes
		if( !start->Ornament() &&
			 !start->Tempo() )
		{

		}
		start = (TQNOTE*)start->GetNext(voice);
	}
	return res;
}



#endif



// return # of entries in TVoiceCountEntry list
int TVoiceCountEntry::cEntries( void )
{
	int res = 1;
	TVoiceCountEntry *temp;
	temp = Next();
	while( temp )
	{
		res++;
		temp = temp->Next();
	}
	return res;


}

// return noteI of element[pos]
TNOTE *TVoiceCountEntry::notePtr( int pos )
{
	TVoiceCountEntry *temp;
	temp = this;
	int i;
	if( pos == 0 ) // == this
		return noteI;

	// else
	i = 0;
	while( temp &&
		   i < pos )
	{
		temp = temp->Next();
		i++;
	}

	if(temp)
		return temp->notePtr();

#ifdef _DEBUG
	Printf("Warning: VoicePtr not found!\n");
#endif
	return NULL;

}

/*
	set for each entry the # of voices to the max number
	of voices during its duration
*/
void TVoiceCountEntry::recalcEntries()
{
	TVoiceCountEntry *temp,
				*temp2;
	TAbsTime endTime;
	temp = this;
	int maxVoices;
	while( temp )
	{
		// find max # of voices during duration
		maxVoices = temp->Voices();
		endTime = temp->Time() + temp->notePtr()->GetDuration();
		temp2 = temp->Next();
		while(temp2 &&
			  temp2->Time() < endTime )
		{
			if( temp2->Voices() > maxVoices )
				maxVoices = temp2->Voices();
			temp2 = temp2->Next();
		}
		temp->voicesI = maxVoices;
		temp = temp->Next();
	} // while
}

#ifdef use_TIOIList
TIOIList::TIOIList(THMIDIFILE *file) : TfloatList(0)
{
	TCLICKTRACK *track;
	TCLICKNOTE *note = NULL;
	track = file->ClickTrack;	
	if( track )
		note = track->FirstNote();
	else
	{
#ifdef _DEBUG
		printf("Error: TIOILIST(THMIDIFILE) no clicktrack");
#endif
	}


	recalc(note, NULL);
}



/*
	try to quantize movement list to grid values
*/
void TIOIratioList::preQuantize()
{
	int i,
		temp;

	double curVal,
		  delta,
		  tempFloat;
	
	for( i = 0; i < cEntries(); i ++ )
	{
		curVal = entry(i);
		// try qo quantze to naturals

		// rounding
		if( curVal > 0 )
			curVal += 0.5;
		else
			curVal -= 0.5;

		temp = curVal;
		tempFloat = temp;

		// difference?
		delta = fabs(tempFloat-temp);

		if( delta < 0.1 )	// 
			setEntry(i, tempFloat);




	}
}
#endif
