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
	filename: tags.cpp
	author:   Juergen Kilian
	date:     1998-2001, 2011
	Implementation of list-classes for guido tags
-----------------------------------------------------------------*/
#include "debug.h"
#include <sstream>
using namespace std;
#include <string.h>
#include "q_note.h"
#include "h_track.h"
#include "h_midi.h"
#include "funcs.h"
#include "k_array.h"
#include "tags.h"

#include "../lib_src/ini/ini.h"

// Limit for recognition of different durations
#define UP_BEAT_LIMIT (TFrac(11,10))
//-------------------------------------------------------------
// default values for markStressByIntens
#define DEFAULTINTENS 0.7
#define DEFINSTR "0.7"

#undef UNUSED

// limit for staccto detection
// quant/play > STACLIMIT -> staccato
#define STACLIMIT TFrac(3,2)

//--------------- TTagList ----------------

TTagList::TTagList( int type,
					 TNOTE *start)
{
		nextI = NULL;
		TypeI = type;
		StartPtr = start;
		EndPtr = start;
}

TTagList::TTagList( int type,
					 TNOTE *start,
					 TNOTE *end)
{
		nextI = NULL;
		TypeI = type;
		StartPtr = start;
		EndPtr = end;
}

#ifdef _DEBUG
void TTagList::PrintSingle( FILE *out )
// For Debug: Print tag information
{
	fprintf(out,"Type:%d, Start%ld/%ld, End: %ld/%ld, Voice:%d\n",
			Type(),
			StartPtr->GetAbsTime().numerator(),
			StartPtr->GetAbsTime().denominator(),
			(EndPtr->GetAbsTime()+
			EndPtr->GetDuration()).numerator(),
			(EndPtr->GetAbsTime()+
			EndPtr->GetDuration()).denominator(),
			StartPtr->GetVoice() );
}

void TTagList::Print( FILE *out )
// For debug: Printf complete tag list
{
	TTagList *temp;
	PrintSingle( out ); // printf this
	temp = Next();
	while( temp )
	{
		temp->PrintSingle( out );
		temp = temp->Next();
	}
}
#endif


void TTagList::SetEnd( TNOTE *end )
{
	EndPtr = end;
}
TNOTE *TTagList::Start( void )
{
	return StartPtr;
}
TNOTE *TTagList::End( void )
{
	return EndPtr;
}
void TTagList::SetNext( TTagList *next )
{
	nextI = next;
}

//--------------------------------------------------------------
TTagList  *Merge( TTagList *current1, TTagList *current2 )
// Merge two TagLists sorted bei start-times
{
	TTagList *res,
				*min,
				*prev;

	TAbsTime start1,
				start2;
	res = NULL;
	prev = NULL;
		
	if(!current2)
		return current1;
	else if(!current1)
		return current2;
	

	// Merge until end of one list
	while( current1 && current2 )
	{
		start1 = current1->Start()->GetAbsTime();
		start2 = current2->Start()->GetAbsTime();
		if(  start1 <= start2 )
		{
			min = current1;
			current1 = current1->Next();
		}
		else
		{
			min = current2;
			current2 = current2->Next();
		}

		if( prev ) // attach to previous elements
		{
			prev->SetNext( min );
			prev = min;
		}
		else // first element of list
		{
			res = min;
			prev = min;
		}
	} // while
	// Check for unattached tails
	if( current1 )	// -> current2 == NULL
		min = current1;
	else if( current2 ) // ->current1 == NULL
		min = current2;
	else
		min = NULL;

	// append rest of list
	if( prev )
	{
		prev->SetNext( min );
	}
	else // current2 must have been empty
	{
		res = min;
	}
	return res;
}
//----------------------------------------------

void DeleteTags( TTagList *start )
// delete complete list of tags
{
	TTagList *temp;
	while( start ){
		temp = start->Next();
		delete start;
		start = temp;
	}
}
//----------------------------------------------

TTagList *DoForAllVoices( TQTRACK *track,
							PPROC fptr)
// call fptr for all voices of track
{
	TTagList *res,
				 *temp;
	res = NULL;
	int i;
	if( !track )
	{
		Printf("NULL Pointer in DoForAllVoices!\n");
		return NULL;
	}
	TQNOTE *start;
	for(i=0; i < track->GetCVoice(); i++)
	{
		// MarkUpBeats in Voice i
		start = QNOTE(track->FirstNoteObject(i));
		temp = fptr(start, track->Parent()->getInifile());
		res = Merge( temp, res );
	}
	return res;
}

//-------------------------------------------------------------
TTagList *DoForAllTracks( THMIDIFILE *file,
							PPROC fptr)
// call fptr for all tracks of file
{
	TTagList *res,
			 *temp;

	res = NULL;

	if( !file )
	{
		Printf("NULL Pointer in DoForAllTracks!\n");
		return NULL;
	}
	THTRACK *current;

	current = (THTRACK *)file->FirstTrack();
//	for( i = 0; i < file->TrackC(); i++ )
	while( current )
	{
			temp = DoForAllVoices(current,
								fptr);

			res = Merge( temp, res );
			current = (THTRACK *)current->Next();
	}
	return res;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
TTagList *MarkSlur(THMIDIFILE *file )
{
	return DoForAllTracks( file, MarkSlur );
}
//-------------------------------------------------------------
TTagList *MarkStaccato(THMIDIFILE *file )
{
	return DoForAllTracks( file, MarkStaccato );
}
//-------------------------------------------------------------
//-------------------------------------------------------------
TTagList *MarkSlur(TQTRACK *track )
{
	return DoForAllVoices( track, MarkSlur );
}
//-------------------------------------------------------------
TTagList *MarkStaccato(TQTRACK *track )
{
	return DoForAllVoices( track, MarkStaccato );
}
//-------------------------------------------------------------
TTagList *MarkStressTrack(TQTRACK *track )
{
	TTagList *lgDuration;

	if(!track) 
		return NULL;
	TQNOTE *startNote = NULL;
	TNOTE *temp;
	temp = track->FirstNoteObject(-1);
	if(temp)
		startNote = dynamic_cast<TQNOTE *>(temp);

	lgDuration = DoForAllVoices( track, MarkStressByDuration );

//	Intens   = MarkStressByIntens(startNote);
//	Number   = MarkStressByNumber(startNote);
	
//	lgDuration = Merge(Intens, lgDuration);
//	lgDuration = Merge(Number, lgDuration);
	return lgDuration;
}
//-------------------------------------------------------------

//------------------------------------------------------------
TTagList *MarkStressByIntens(TQTRACK *track )
{
	return DoForAllVoices( track, MarkStressByIntens );
}
//------------------------------------------------------------
TTagList *MarkStressByNumber(TQTRACK *track )
{
	return DoForAllVoices( track, MarkStressByNumber );
}
//------------------------------------------------------------
TTagList *MarkStressByDuration(TQTRACK *track )
{
	return DoForAllVoices( track, MarkStressByDuration);
}

//-------------------------------------------------------------
//-------------------------------------------------------------

TTagList *MarkSlur( TQNOTE *start, TInifile *inifile )
/*
	mark slur if small overlapping of notes
*/
{

	TTagList *prevTag,
				*firstTag,
				*newTag;
	TNOTE *now,
			 *next;

	prevTag = NULL;
	firstTag = NULL;	// head of tag list

	int state,
		 prevState;

	#define TagOFF  0
	#define TagON  1

	state = TagOFF;
	prevState = TagOFF;

	now = start;
	while( now )
	{
		next = NOTE(now->GetNext(now->GetVoice()));
		if( next )
		{
			// check legato(now,next)
			if( (now->GetAbsTime()+now->GetDuration()) >=
					next->GetAbsTime())
			{
				// Turn Slur on
				state = TagON;
				if( prevState == TagOFF )
				{
					// state has changed
					newTag = new TTagList( TAG_SLUR,
											  now );
					// attach to taglist
					if( prevTag )
					{
						prevTag->SetNext( newTag );
					}
					else	// create List
					{
						firstTag = newTag;
					}
					prevState = TagON;
					prevTag = newTag;
				} // if prev state
			}
			else // no overlapping
			{
				state = TagOFF;
				if( prevState == TagON )
				{
					// state has changed
					prevState = TagOFF;
					if( prevTag )
						prevTag->SetEnd( now ); // last note of group
					prevState = TagOFF;
				}
			} // else
		} // next == NULL
		else if( state == TagON ) // Slur On at end of piece
		{
			// state has changed
			prevState = TagOFF;
			if( prevTag )
				prevTag->SetEnd( now ); // last note of group
		}
		now = next;
	} // while
	return firstTag;
}
//-------------------------------------------------------------
TTagList *MarkStressByIntens( TQNOTE *start,
								TInifile *tempInifile)
{
	TQNOTE *current;

	TTagList *prevTag = NULL,
			*firstTag = NULL,
			*newTag = NULL;


	int  i=0,
		 j=0; 
	double level = 0,
		  sum,
		  av;

	if(!start)
		return firstTag;

	// read intenslevel from .ini
	if(tempInifile)
	{
		level = tempInifile->GetValFloat("STRESSINTENS",DEFINSTR,"Threshold for intensity stressed notes");
	}
	else
	{
		Printf("Can't open inifile!\n");
	}

	// analyse intensity of track
	sum = 0;
	current = start;
	while(current)
	{
		sum += current->getIntens();
		current = QNOTE(current->GetNext(current->GetVoice()));
		i++;
	}
	av = sum / i;

	level = av+((127-av)*level); //????
	// find/mark stressed notes
	current = start;
	while(current )
	{
			if( current->getIntens() > level)
			{
				newTag = new TWeightedTag( TAG_STRESS,
										current,
										current,
										current->getIntens()/127);
				if( prevTag )
				{
					prevTag->SetNext( newTag );
					prevTag = newTag;
				}
				else // start new list
				{
					prevTag = newTag;
					firstTag = newTag;
				}
				j++;
			}
			current = QNOTE(current->GetNext(current->GetVoice()));
	}
	return firstTag;
}

//-------------------------------------------------------------

TTagList *MarkStaccato( TQNOTE *start, TInifile *inifile )
{

	TTagList *prevTag,
				*firstTag,
				*newTag;
	TQNOTE *now,
			 *prev;

	prevTag = NULL;
	firstTag = NULL;	// head of tag list

	int state,
		 prevState;

	#define TagOFF  0
	#define TagON  1

	state = TagOFF;
	prevState = TagOFF;

	now = start;
	prev = now;

	TAbsTime playDuration, 	// duration as played
			 quantDuration;	// duration as quantized
	while( now )
	{
		playDuration = now->GetDuration();
		// skip virtual notes
		if( playDuration > 0L )
		{

			quantDuration = now->qDuration();
			// if qDuration > 150%(pDuration) -> MarkStac
			if( quantDuration / playDuration > TFrac(3,2) )
			{
		// use \stac for each single note
					// state has changed
					newTag = new TTagList( TAG_STACCATO,
												  now );
					newTag->SetEnd( now ); // one note per tag
					if( prevTag )
					{
						prevTag->SetNext( newTag );
					}
					else	// create List
					{
						firstTag = newTag;
					}
					prevTag = newTag;

			}
		} // if playDuration
		prev = now;
		now = QNOTE(now->GetNext(now->GetVoice()));
	} // while, next == NULL
	if( state == TagON ) // Slur On at end of piece
	{
		// state has changed
		prevState = TagOFF;
		if( prevTag )
			prevTag->SetEnd( prev ); // last note of group
	}
	return firstTag;
}
//-------------------------------------------------------------

TTagList *MarkStressByDuration( TQNOTE *start, TInifile *inifile )
/* Mark if duration(next) > duration(now)
	if a < b < c -> mark only c !!!
	if a < b > c -> mark b
	if a = b > c -> mark b
*/
{
	TTagList *firstTag,
				*prevTag,
				*newTag;

	TNOTE *now,
		 *next,
		 *nnext;

	firstTag = NULL;
	prevTag  = NULL;

	TAbsTime d1,d2,d3;
	// Mark if duration(next) > duration(now)
	now = start;
	while( now )
	{
		next = QNOTE(now->GetNext(now->GetVoice()));
		if( next ) // mark next?
		{
			nnext = QNOTE(next->GetNext(next->GetVoice()));
			d1 = GetIOI(now);
			d2 = GetIOI(next);
			if( nnext )
			{
				d3 = GetIOI(nnext);
			}
			else
			{
				d3 = 0L;
			}
			if( d1 > 0L &&
				((d2 > (d1*UP_BEAT_LIMIT) && // a < b
				 d2 > (d3*UP_BEAT_LIMIT) ) || // b > c 
				 ( (d1*UP_BEAT_LIMIT) > d2 && //a==b
				   (d2*UP_BEAT_LIMIT) > d1 && //b==a
				   d2 > (d3*UP_BEAT_LIMIT) ) // b > c
				))
			{
//				next->MarkUpBeat();
				newTag = new TWeightedTag( TAG_STRESS,
											  next, // tag start
											  next, // tag end 
											  (d2/(d1+d2)).toDouble());
				if( prevTag )
				{
					prevTag->SetNext( newTag );
					prevTag = newTag;
				}
				else // start new list
				{
					prevTag = newTag;
					firstTag = newTag;
				}
			} // if upbeat duration
		} // if next
		now = next;
	} // while now
	return firstTag;
}

/*
	Mark notes by # of ATPts at the same time
*/
TTagList *MarkStressByNumber( TQNOTE *start, TInifile *inifile )
{

	TQNOTE  *current,
			*stressEnd,
			*next;

	TTagList *prevTag = NULL,
			*firstTag = NULL,
			*newTag = NULL;


	int  i=0;

	if(!start)
		return firstTag;

	// find/mark stressed notes
	current = start;
	while(current )
	{
		i = 0;
		// count Notes at Attack(current)
		next = QNOTE(current->GetNext(current->GetVoice()));
		stressEnd = next;
		while( next && 
			sameIOI( next->GetAbsTime(),
					 current->GetAbsTime()) )
		{
			i++;
			stressEnd = next;
			next = QNOTE(next->GetNext(-1 /* all Voices*/));
		}
		// create new tag
		if( i > 1)
		{
			newTag = new TWeightedTag( TAG_STRESS,
									current,
									stressEnd,
									i/10); // weight
			if( prevTag )
			{
				prevTag->SetNext( newTag );
				prevTag = newTag;
			}
			else // start new list
			{
				prevTag = newTag;
				firstTag = newTag;
			}
			// skip tagged notes
			current = stressEnd;
		} // if stressed
		current = QNOTE(current->GetNext(-1 /* all Voices*/ ));
	}
	return firstTag;
}
//-------------------------------------------------------------
int ListSize( TTagList *tags)
// #Tgas in List
{
	int res = 0;
	while(tags)
	{
		res++;
		tags = tags->Next();
	}
	return res;
}
TagPtr *CreateOnsetIndex( TTagList *tags )
/*
	create array of pointers to tags, sorted by onset times
	!!result needs to be deleted
*/
{
	TagPtr *res = NULL;
	int listSize;

	if( !tags ) 
		return NULL;

	// create ptr array
	listSize = ListSize( tags ); 
	if( !listSize )
		return NULL;
	res = new TagPtr[listSize];
	int i;

	// fill array with pointers to tags
	for( i=0; i < listSize; i++)
	{
		res[i] = tags; // tag list already sorted by onset times
		tags = tags->Next();
	}
	return res;
}

TagPtr *CreateOffsetIndex( TTagList *tags)
/*
	create array of pointers to tags, sorted by offset times
	!!result needs to be deleted
*/
{
	TagPtr *res = NULL;

	int listSize;

	if( !tags ) 
		return NULL;

	listSize = ListSize( tags ); 
	if( !listSize )
		return NULL;

	// create and fill array
	res = CreateOnsetIndex( tags );

	// sort array
	int i,j;
	TTagList *current,
			 *temp,
			 *prev; 
	i = 0;
	while( i < listSize)
	{
		prev = res[i];
		for(j = i+1; j < listSize; j++)
		{
			current = res[j];
			if(prev->End()->GetAbsTime() < 
				current->Start()->GetAbsTime() )
			{
				// stop search, 
				// List is sorted by onset -> if offset(prev) < onset(current) -> stop
				j = listSize;
				i++;
			}
			else if( prev->End()->GetAbsTime() <
				current->End()->GetAbsTime() )  
			{
				// sort ok, keep searching
				if( j+1 == listSize )
					i++;

			}
			else if( prev->End()->GetAbsTime() >
				current->End()->GetAbsTime() )
			{
				// swap, start search again
				temp = res[i];
				res[i] = res[j];
				res[j] = temp;

				j = listSize;
			}
		} // for
	} // while
	return res;
}



TTagType GetTags(TTagList *tags,
				TAbsTime start,
				int voice )
/*
	return Tag-Bit-Array at position start in voice 
*/
{
	TTagType res = 0;
	TAbsTime end;
	while( tags &&
			 tags->Start()->GetAbsTime() <= start )
	{
		if( tags->Start()->GetVoice() == voice )
		{
			end = tags->End()->GetAbsTime() +
					tags->End()->GetDuration();
			if( end > start ) // check if start is inside tag range
			{
				// note is in tag range
				res |= tags->Type();
			}
		}
		tags = tags->Next();
	}
	return res;
}

void PrintTags(  ostream &gmnOut,
				 TTagType PrevTags,
				 TTagType CurTags)
/*
	Print GMN output ot tag change
*/
{
	if( (PrevTags & TAG_SLUR) &&
		!(CurTags & TAG_SLUR) )
	{
		gmnOut << ") ";
	} 

	//  use begin/end
	if( (PrevTags & TAG_STACCATO) &&
		 !(CurTags & TAG_STACCATO) )
	{
//		fprintf( file,") ");
		gmnOut << "\\staccEnd ";
	}

	if( !(PrevTags & TAG_SLUR ) &&
		  (CurTags & TAG_SLUR) )
	{
		gmnOut << "\\slur(";
	}
//	use begin/end
	if( !(PrevTags & TAG_STACCATO ) &&
		  (CurTags & TAG_STACCATO) )
	{
//		fprintf( file, "\\stacc(");
		gmnOut << "\\staccBegin ";
	}

#ifdef _DEBUG
	if( !(PrevTags & TAG_STRESS ) &&
		  (CurTags & TAG_STRESS) )
	{
		gmnOut << "\\noteFormat<color=\"red\"> ";
	}

	if( (PrevTags & TAG_STRESS) &&
		 !(CurTags & TAG_STRESS) )
	{
		gmnOut << "\\noteFormat<color=\"black\"> ";
	}
#endif
	
/*
	if( !(PrevTags & TAG_GRACE ) &&
		  (CurTags & TAG_GRACE) )
	{
		fprintf( file, "\\grace( ");
//		fprintf( file, "\\graceBegin");
	}

	if( (PrevTags & TAG_GRACE) &&
		 !(CurTags & TAG_GRACE) )
	{
//		fprintf( file,") ");
		fprintf(file, ") ");
	}
*/

}


//////////////////////////////////////////////////////////////////////
// TWeightedTag Klasse
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
TWeightedTag::TWeightedTag( int type,
			 TNOTE *start,
			 double weight) : TTagList(type, start)
{
	WeightI = weight;
}

TWeightedTag::TWeightedTag( int type,
			 TNOTE *start,
			 TNOTE *end,
			 double weight) : TTagList(type, start, end)
{
	WeightI = weight;
}


TWeightedTag::~TWeightedTag( void )
{

}

double TWeightedTag::weight( void )
{
	return WeightI;
}
