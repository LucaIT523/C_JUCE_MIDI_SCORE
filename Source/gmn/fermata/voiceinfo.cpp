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

/*----------------------------------------------------------------
	filename: voiceinfo.cpp
	author:   Juergen Kilian
	date:     1998-2001, 2011

	Implementation of voice profile analysis functions and classes
-----------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include "voiceinfo.h"
#include <math.h> // for pow
#include "q_track.h"
#include "funcs.h"
#include "statist.h"


#include "../lib_src/ini/ini.h"
TVoiceInfo::TVoiceInfo(void): gOffset(-1)
{
	init();
}

void TVoiceInfo::init()
{
	oldVoice = -1;		// voice nr before merging
	newVoice = -1;		// voice nr after merging
//	lockedBy = -1;	    // voice is currently used?
	lastPitch  = -128;	// pitch of last note in voice, !mean
	firstPitch = -128;
	noteOff = -1L;   // noteOff of last note in voice
	start = -1L;
	meanDuration = 0L; // mean lgDuration of notes
	hold = 0L;		// sum of note durations
	rest = 0L;		// sum of rests between notes
	maxPitch = -1;		// highest pitch
	minPitch = 128;		// lowest pitch
	cNotes = 0;			// # of notes
	unused = 0;			// # times not used
	lastNoteID = -1;
	lastNote = NULL;
//	lastUsed = 0;		// count distance to last used note
}
void TVoiceInfo::addToVoice(TNOTE *now,
							double pitchDecay)
{
	double oldDecay,
		  newDecay;

	newDecay = pitchDecay;
	oldDecay = 1000 - newDecay;
	if( !now )
		return;


	if( lastPitch < 0 ) // this is the first note 
	{
		start = now->GetAbsTime();
		oldVoice = now->GetVoice();
		lastPitch = now->GetMIDIPitch();
		firstPitch = now->GetMIDIPitch();
		noteOff = now->GetAbsTime()+now->GetDuration();
//		lastUsed = 0;
		maxPitch = lastPitch;
		minPitch = lastPitch;
		cNotes = 1;

	}
	else
	{
		firstPitch = (firstPitch*0.95) + (now->GetMIDIPitch()*0.05);

		lastPitch =((lastPitch*oldDecay)+
					   (now->GetMIDIPitch()*newDecay))/1000;
							
		meanDuration = (meanDuration * 0.9) +
					   (now->GetDuration().toDouble() * 0.1);
							
		cNotes++;
		hold += now->GetDuration();
		rest += now->GetAbsTime() - noteOff;
		noteOff = now->GetAbsTime() + now->GetDuration();
		if(now->GetMIDIPitch() >  maxPitch )
				maxPitch = now->GetMIDIPitch();
							
		if(now->GetMIDIPitch() < minPitch )
				minPitch = now->GetMIDIPitch();

		if( lastNote->offset() > now->GetAbsTime() )
		{
			// overlap cut last note
			TFrac nDur;
			nDur = now->GetAbsTime() - lastNote->GetAbsTime();
			lastNote->SetDuration( nDur );
		}
						
	}
	lastNote = dynamic_cast<TQNOTE *>(now);

}

//----------------------------------------------------------------------
#ifdef GLOBAL_SPLITV 

int changeVoice( TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					int CVoice,
					int pos,			
					int mode )		// up, down
{
	int valid = 0,
		i,
		res = -1,	// undef
		count,
		link,
		newLink,
		newVoice,
		voice, 
		rRoot; // root of root
	char done = 0,
		 end = 0;	// stop if no neighbour can be found



//	root = noteList[pos].cRoot();

	// copy old values
	link = noteList[pos].linkID();
	voice = noteList->rootVoice( pos );
	int pRoot = noteList->rootID( pos );


	newVoice = voice;
//	newVoice = rand() % CVoice;
	if( mode < 0 ) // down
	{
		if( newVoice > 0 )
			newVoice--;
		else
			return 0;
	}
	else if( mode > 0 )
	{
		if( newVoice < CVoice-1 )
			newVoice++;
		else
			return 0;
	}
	else
		newVoice = voice;  // test different link
	

	// search for possible chord notes
	count = -1;
	
	
	// check must link chords to right
	i = pos+1;
	while( i < cAttacks &&
		   res < 0 )
	{
 
		if( noteList->rootVoice( i ) == newVoice &&
			noteList[i].attack->GetAbsTime() == noteList[pos].attack->GetAbsTime() )
		{
			// must link
			if( link == i &&
				voice == newVoice )
				res = 0;			// only equal voice is possible
			else
			{
				noteList->unlink( pos );
				noteList[pos].setVoice( newVoice );
				noteList->link(pos, i);
				res = 1;			// new voice, new must link
			}
			i = cAttacks; // stop
		}
		if( i < cAttacks &&
			noteList[pos].attack->offset().toDouble() > 
			noteList[i].attack->GetAbsTime().toDouble() )
		{
			i = cAttacks; // stop if too far away
		}
		i++;
	} // while

	



	i = pos - 1;
	while( i > -1 &&
		   res < 0 )	// only if note processed
	{
		rRoot = noteList->rootID( i );
		if( rRoot < 0 )
			rRoot = i;

		if( noteList->rootVoice( i ) == newVoice &&
			noteList[i].attack->GetAbsTime() == noteList[pos].attack->GetAbsTime() )
		{
			// must link
			if( link == i &&
				voice == newVoice )
				res = 0;		// must link in equal voice
			else
			{
				noteList->unlink(pos);
				noteList[pos].setVoice( newVoice );
				noteList->link(i,pos);

				res = 1;		// ok
			}
			i = -4; // stop
		}
		else if( noteList->rootVoice( i ) == newVoice &&
				 rRoot != pRoot &&
			     noteList[rRoot].attack->offset() > noteList[pos].attack->GetAbsTime() // overlapp
			   )
		{
			// take this possible link?
			pRoot = noteList->rootID( i );
	//count = i;
			if( rand() % 100 > 100 )
			{
				noteList->unlink(pos);
				noteList[pos].setVoice( newVoice );
				noteList->link(i,pos);
				i = -4; // stop
				res = 1;
			}
		}

		if( i > -1 &&
			noteList[pos].attack->GetAbsTime() - 
			noteList[i].attack->GetAbsTime() > noteList[pos].attack->GetDuration() )
		{
			i = -1; // stop if too far away
		}
		i--;
	} // while
//	if( count )
	if( voice == newVoice &&
		link == noteList[pos].linkID() )
	{
		// nothing has changed 

		if( count > -1 && // one possible link was skipped
			count != link )
		{
			noteList->unlink( pos );
			noteList[pos].setVoice( newVoice );
			noteList->link(count, pos );
			res = 1;
		}

	}
	else	// at least a single parameter has changed
	{
		noteList->unlink( pos );
		noteList[pos].setVoice( newVoice );
		res = 1; 
	}
		
	if( res  < 0) // no solution -> restore
	{
		res = 0;
	}
	return res;

	
	
	
	
	int root = noteList->rootID(pos);
	voice = noteList[pos].voice();	


	while( !valid && 
		   !end)
	{
		done = 0;
		if( mode > 0 )	 // inc voice
		{

			// 1. inc root until root == pos (==-1) then, inc voice until CVoice

		    if( root > -1 ) // this is a linked note
			{				
				// try to increment the root
				rRoot = root;

				// skip all linked notes 
				root++;
				while( root < pos &&
					   noteList->rootID( root ) == rRoot )
					root++;
				if( root == pos )
				{
					root = -1;
					// continue with voice++
				}
				else // test new root
				{
					rRoot = noteList->rootID(  root);
					if( rRoot < 0 )
						rRoot = root;
					// check for overlap to rRoot
					if( noteList[rRoot].attack->offset() > 
						noteList[pos].attack->GetAbsTime() )
					{
						valid = 1; // overlap
					}
					done = 1;
				} // else
			} // if root


			if( !done &&
				voice+1 < CVoice )
			{
				// try to inc voice
				voice++;
				if( voice < CVoice )
				{
					// check for overlappings to neighbours
					valid = 1;
					for( i = pos-1; i > -1; i--) // check left
					{
						if( noteList->rootID(  i ) != pos && // not linked to current
							noteList[i].voice() == voice )
						{
							if( noteList[i].attack->GetAbsTime() == 
								noteList[pos].attack->GetAbsTime() ) // no overlap
							{
								valid = 0; //  not possible
							}
							i = -1; // stop loop

						} // if
					} // for
					
					for( i = pos+1; i < cAttacks; i++ ) // check right
					{
						if( noteList->rootVoice( i ) == voice )
						{
							if( noteList[i].attack->GetAbsTime() == 
								noteList[pos].attack->GetAbsTime() ) // no overlap
							{
								valid = 0; // not possible
							}
							i = cAttacks; // stop loop
						} // if
					} // for
					done = 1;
				} // if < CVoice
			} // if 
			else 
			{
				// root = -1, voice == CVoice
				end = 1;
			}
		}
		else // dec voice until 0, then dec root until no overlap
		{
			if(  // root < 0 && // not linked to chord
				voice > 0 )
			{
				voice--;
				
				// check for overlappings to neighbours
				valid = 1;
				done = 1;
				for( i = pos-1; i > -1; i-- ) // check left
				{
					if( noteList->rootVoice( i ) == voice )
					{
						if( noteList[i].attack->GetAbsTime() == 
							noteList[pos].attack->GetAbsTime() ) // no overlap
						{
							valid = 0; //not possible
							done = 0;
						}
						i = -1; // stop loop
					} // if
				} // for
				
				for( i = pos+1; i < cAttacks; i++ ) // check left
				{
					if( noteList->rootVoice( i ) == voice )
					{
						if( noteList[i].attack->GetAbsTime() == 
							noteList[pos].attack->GetAbsTime() ) // no overlap
						{
							valid = 0; //not possible
							done = 0;
						}
						i = cAttacks; // stop loop
					} // if
				} // for
				if( valid )
					root = -1; 
				
			} // if 

			if( !done &&
				 root > -1 ) 
			{
				// try to decrement the root
				rRoot = noteList->rootID( pos);
				root--;
				// skip chord notes to same root
				while( root >= 0 &&
					rRoot == noteList->rootID( root) )
					root--;
				if( root >=  0 ) // Start of list reached
				{
					rRoot = noteList->rootID(  root);
					if( rRoot < 0 )
						rRoot = root;
					// check for overlap to rRoot
					if( noteList[rRoot].attack->offset() > 
						noteList[pos].attack->GetAbsTime() )
					{
						valid = 1; // overlap
					}
					else // stop searching for root in that direction
					{
						end = 1; 
					}
					done = 1;
				} // ifroot 
				else
				{
					return 0;
				}

			}
			else
			{
				// voice == 0, root == 0
				end = 1;
			}
		} // else
	} // while
		
	if( valid )
	{
		/*
		// check all notes linked to pos 
		int newRoot = -1;
		for( i = pos; i < cAttacks; i++ )
		{
			if( getRoot( noteList, i ) == pos ) // linked to current
			{
				if( newRoot < 0 )
				{
					newRoot = i; 
					noteList[i].setRoot( -1 );
					noteList[i].setVoice( rVoice(noteList, pos) );
				}
				else // at least second note of chord
				{
					noteList[i].setRoot( newRoot );
					noteList[i].setVoice( rVoice(noteList, newRoot) );
				}
			}
			if( noteList[i].attack->GetAbsTime() > noteList[pos].attack->offset() )
				i = cAttacks; // stop loop
		} // for
		*/
		

		noteList->unlink( pos );
		if( root > -1 )
			noteList->link(root, pos);

//		noteList[pos].setRoot( root );
//		noteList[root].incLink();
//			noteList[root].setMaxBackLink( pos );
		
		if( root < 0 )
			noteList[pos].setVoice( voice );
		else
			noteList[pos].setVoice( noteList->rootVoice( root));
		return 1;
	}
	return 0; // no sulution found
}
#endif

//----------------------------------------------------------------------
double voiceDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *voiceInfo,  // current used voices
					int CVoice,				// size of array
					int /*changeID*/,			// id of changed note
					int /*oldLinkID*/,
					int /*oldVoice*/,
					double &pChord,			// cost values
					double &pPitch,			// cost values
					double &pGap,				// cost values
					double &pOverlap,
					int loockBack,
					int emptyVoiceEv,
					double gapThreshold,
					TMIDIFILE *theMidifile)

{
#ifdef GLOBAL_SPLITV
	/* if voice or link of noteList[changeID] 	has changed:
		- recalc all costs of oldLink,
				 all costs of newLink
				 all costs of next in old voice
				 all costs of next in new voice */
	int i,
		oldRoot = -1,
		newRoot = -1,
		curRoot,
		nextInOldVoice = -1,
		nextInNewVoice = -1;


	double left = 0,
		  right = 0,
		  temp = 0;




	// get all root id's
	oldRoot = oldLinkID;
	if( oldLinkID > -1 )
	{
		oldRoot = noteList->rootID( oldLinkID );

	}

	newRoot = changeID;
	if( changeID > -1 )
		newRoot = noteList->rootID( changeID );



	// init voice id's
	i = changeID + 1;
	while( i < cAttacks && 
			changeID > -1 )
	{
		if( nextInOldVoice < 0  &&
			noteList[i].voice() == oldVoice )
		{
			nextInOldVoice = i;
		};
		if( nextInNewVoice < 0  &&
			noteList[i].voice() == noteList[changeID].voice() )
		{
			nextInNewVoice = i;
		};

		if( nextInNewVoice > -1 &&
			nextInOldVoice > -1 )
			i = cAttacks; // stop loop
		i++;
	}

	// sum all untouched left id's
	for( i = 0; i < changeID; i++ )
	{
		curRoot = noteList->rootID(i);
		if( (oldRoot < 0 || 
			 curRoot != oldRoot) &&
			(newRoot < 0 ||
			 curRoot < newRoot) )
		{
			left += pOverlap * noteList[i].lOverlap;
			left += pGap * noteList[i].lGap;
			left += pChord * noteList[i].lChord;
			left += pPitch * noteList[i].lPitch;
		} // if
	} // for

	int state;
	i = 0;
	for( state = 0; state < 4; state++ )
	{

		if( changeID > -1 )
		{
			switch( state )
			{
			case 0: i = changeID; 
						state++;
						if( oldLinkID < 0 ||
							noteList[changeID].linkID() == oldLinkID )
							state++; // skip1
						if( nextInOldVoice < 0)
							state++; // skip
						if( nextInNewVoice < 0 )
							state++; // skip break;
						break;
			case 1: i = oldLinkID; 
						state++;
						if( nextInOldVoice < 0)
							state++; // skip
						if( nextInNewVoice < 0 )
							state++; // skip break;
			case 2: i = nextInOldVoice;
						state++;
						if( nextInOldVoice == nextInNewVoice )
							state++; // stop 
						if( nextInNewVoice < 0 )
							state++; // skip break;
						break;
			case 3: i = nextInNewVoice; 
						state++;
						break;
			}; // switch
		}
		else
		{
			i = -1;
		}

		// set lPitch
		pitchDistance( noteList,
							 cAttacks,
							 voiceInfo,
							 CVoice,
 							 i,
							 loockBack,
							 emptyVoiceEv);
		// set lChord
		chordDistance( noteList,
								cAttacks,
								voiceInfo,
								CVoice,
								i);
		// set lGap
		gapDistance( noteList,
								cAttacks,
								voiceInfo,
								CVoice,
								i);
		// set lOverlap
		overlapDistance(noteList,
									cAttacks,
									voiceInfo,
									CVoice,
									i);
	

		if( changeID < 0 )
			state = 5; // stop loop
		else
		{
			temp += pOverlap * noteList[i].lOverlap;
			temp += pGap * noteList[i].lGap;
			temp += pChord * noteList[i].lChord;
			temp += pPitch * noteList[i].lPitch;
		}
	}; // for state


	// sum all untouched right id's
	for( i = changeID+1; i < cAttacks; i++ )
	{
		curRoot = noteList->rootID(i);
		if( (oldRoot < 0 || 
			 curRoot != oldRoot) &&
			(newRoot < 0 ||
			 curRoot < newRoot)  )
		{
			right += pOverlap * noteList[i].lOverlap;
			right += pGap * noteList[i].lGap;
			right += pChord * noteList[i].lChord;
			right += pPitch * noteList[i].lPitch;
		} // if
	} // for



	return left + right + temp;
#endif
#ifdef LOCAL_SPLITV
		double res = 0;
//          printf("<c"); fflush(stdout);		
#ifdef _DEBUG
	   noteList[0].chordDist = res;
#endif

		if( pChord > 0 )
			res = (pChord * chordDistance( noteList,
				cAttacks,
				voiceInfo,
				CVoice, -1 ));
#ifdef _DEBUG
	   noteList[0].chordDist -= res;
	   noteList[0].pitchDist = res;
#endif

//            printf("c>"); fflush(stdout);		

            // printf("<p"); fflush(stdout);		
			
		if( pPitch > 0 )
			res += (pPitch * pitchDistance( noteList,
				cAttacks,
				voiceInfo,
				CVoice, 
				loockBack,
				emptyVoiceEv));
#ifdef _DEBUG
	   noteList[0].pitchDist -= res;
	   noteList[0].gapDist = res;
#endif
        // printf("p>\n"); fflush(stdout);		
  //      printf("<g"); fflush(stdout);		

		if( pGap > 0 )
			res += (pGap * gapDistance( noteList,
				cAttacks,
				voiceInfo,
				CVoice, -1,
				gapThreshold,
				theMidifile));
    //    printf("g>"); fflush(stdout);		

      //  printf("<o"); fflush(stdout);		
#ifdef _DEBUG
	   noteList[0].ovlDist = res;
	   noteList[0].gapDist -= res;
#endif

		if( pOverlap )	
			res += (pOverlap * overlapDistance(noteList,
				cAttacks,
				voiceInfo,
				CVoice, -1,
				theMidifile));
//        printf("o>"); fflush(stdout);		
#ifdef _DEBUG
	   noteList[0].ovlDist -= res;
#endif
	
		return res;
#endif
}

double unusedDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *voiceInfo,  // current used voices
					int CVoice )
/*
	
	penalty for usage of voices with high unused counter
*/
{
	int i,
		j,
		voice,
		cNotes = 0;

	double res,
		maxUnused = 1,
		unusedVal = 0;
	
	for(i=0;i<CVoice;i++)
	{
		if(voiceInfo[i].unused > maxUnused )
			maxUnused = voiceInfo[i].unused;
		for( j =0; j <cAttacks;j++)
		{
			voice =  noteList[j].voice();
			if( voice ==i )
			{
				unusedVal += voiceInfo[voice].unused;
				cNotes++;
				j = cAttacks; // stop
			}
		} // for
	}
	if(cNotes )
	{
		unusedVal /= cNotes;
		unusedVal /= maxUnused;
	}
	res = unusedVal;
	return res;
}


void sortLinks( TNoteInfo *noteList, // current chord to test
					int cAttacks )         // size of array
/*
	remove circular chord links, and re-set voices in linked notes
*/
{
	int rootID,
		i;

	// unlink the root note of each chord
	for( i = 0; i < cAttacks; i++ )
	{
		if( noteList[i].linkID() > - 1 )
		{
            /*
            if( noteList[i].linkID() > i )
				printf("");
             */
             rootID = noteList->rootID(i );

			if( rootID == i ) // linked to itself
				rootID = -1; 
			else
			{
//				noteList[rootID].setRoot( -1);  // root note must not be linked
				// copy root note voice to all notes of chord
				noteList[i].setVoice( noteList->rootVoice( rootID));		
			}
			// make direct link to root
			noteList->link(rootID, i);
//			noteList[i].setLink(rootID);
		}
	} // for
}
		

int valid(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo * /*voiceInfo*/,  // current used voices
					int CVoice )
/*
	!! links must be sorted and checked before !!
*/
{
	int voice,
		attack,
		attack2;



	// check distance to voiceInfo
	for( attack = cAttacks - 1; attack > 0; attack-- )
		// go from right to left
	{

		voice = noteList->rootVoice( attack);
//		if( voiceInfo[voice].noteOff > noteList[attack].attack->GetAbsTime() )
//			return 0; // overlapp with selected voice

		attack2 = noteList[attack].linkID();
		if(  attack2 == attack )
			return 0; // don't link note to itself
/*
		// check chord links to non overlapping notes
		if( attack2 > -1 &&
			noteList[attack2].attack->GetAbsTime() > noteList[attack].attack->offset() && // no overlap
			noteList[attack].attack->GetAbsTime() > noteList[attack2].attack->offset() )   // no overlap
			return 0; // can not occur inside a slice!!!
*/


		attack2 = attack - 1 ;
//		for( attack2 = attack+1; attack2 < cAttacks; attack2++ )
		
		// check for notes with equal attacks and equal voices!
		while( attack2 > -1 &&
			noteList[attack].attack->GetAbsTime() == noteList[attack2].attack->GetAbsTime() )
		{

			if( noteList[attack2].voice() == voice )
			{
				if( noteList[attack2].linkID() < 0 &&		// no chord
					noteList[attack].linkID() < 0 )		// no chord	
					return 0; // equal attack and same voice
				else if( noteList->rootID( attack ) == attack2 )
				{
					// ok do nothing
				}
				else if( noteList->rootID(attack2) != noteList->rootID(attack) )	// not linked					
					return 0; // equal attack and same voice
			} // if same voice			
			attack2--;			
		} // for

		if( noteList->rootVoice( attack) >= CVoice )
			printf("error");

	} // for

	


	return 1;

}



		
/*
	  ?costs ~ ambitus(chord)
	  ?costs ~ distance of notes with different voices
	  costs ~ intervalsize in chord
	  remarks:
		- remove legato must be called before
		- noteList must be sorted by attacktime 
		- notes MUST overlapp
*/
double chordDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *,  // current used voices
					 int /*CVoice*/,
					 int /*id*/)
{
//#define GLOBAL_SPLITV

#ifdef GLOBAL_SPLITV
	int i, 
		j,
		start = 0,
		end = cAttacks,
		root,
		minPitch,
		maxPitch,
		pos;

	double res = 0,
		  pAmbitus;

	return 0;
	// reset flags
	for( i = 0; i < cAttacks ; i++ )
		noteList[i].flag = 0;



	if( id > -1 ) // do only on chord
	{
		start = id;
		end = start + 1;
	}

	for( i = start; i < end ; i++ )
	{

		pAmbitus = 0;
		pos = i;
//		pos = cAttacks - i - 1; // go from right to left
		j = noteList[pos].linkID();
		if( j > -1 &&
			!noteList[pos].flag )
		{
			minPitch = noteList[pos].attack->GetMIDIPitch();
			maxPitch = minPitch;

			noteList[pos].flag = 1;
			while( j != pos ) // follow one cyle links
			{
				if( noteList[j].attack->GetMIDIPitch() > maxPitch )
					maxPitch = noteList[j].attack->GetMIDIPitch();
				else if( noteList[j].attack->GetMIDIPitch() < minPitch )
					minPitch = noteList[j].attack->GetMIDIPitch();
				noteList[j].flag = 1;
		
				j = noteList[j].linkID();
			}
			// | min, max | == ambitus

			pAmbitus = ((double)abs( minPitch - maxPitch))/128;
#define base 2.13
			pAmbitus = (pow(base, pAmbitus ) - 1)/base;
/*
				pAmbitus /= 24;
			if( pAmbitus > 1 )
				pAmbitus = 1;
*/
			res += pAmbitus;
		} // if
	} // for

return res;


#endif
#ifdef LOCAL_SPLITV

	double res = 0,
		  sumDistance = 0,  // penalty distance for complete lice
		   pVoice,
			temp;
	int i,
		j,
		note1Pitch = -1, // lowest of chord
		note2Pitch = -500,		// highest of chord
		tempPitch,
		cChords = 0,
		cNotesOfChord; // first note in chord

	
	TFrac  earliestAttack,
		    latestAttack,
			latestOffset,
			earliestOffset,
//		    longestDuration,
//			shortestDuration,
			tempFragm;

	res = 0;

	// 22.8.02 changed attack/duration penalty into 
	// penalty = (earliestOffset - latestAttack) / (latestOffset - earliestAttack)
	

	cChords = 0;
	if( cAttacks > 1 )
	{
		for(j=0; j < cAttacks; j++) // search for chord in voice j
		{
			pVoice = 0;
			cNotesOfChord = 0;

			if( // noteList[j].linkID() < 0 ||  // this is a root
				noteList[j].linkID() > j )   // only root points to >j
			{
				note1Pitch = -1;
				double minIntens = noteList[j].attack->getIntens();
				double maxIntens = minIntens;

				// go through link circle
				// for(i=0; i < cAttacks; i++ )
				 i = j;
				do {

// todo add delta, attack distance to chord distance,
//		therefore normalize chords to earliest root before
					/*
					if( noteList[i].cRoot() == j ||   // linked to root
						i == j ) // the root itself
					*/
					{
						double intens = noteList[i].attack->getIntens();
						if( intens < minIntens )
							minIntens = intens;
						else if( intens > maxIntens )
							maxIntens = intens;

						// [i] is in same chord as root == j
						// calc ambitus
						if( note1Pitch < 0 ) // first note of chord
						{
							note1Pitch = noteList[i].attack->GetMIDIPitch();		
							note2Pitch = note1Pitch;

							earliestAttack = noteList[i].attack->GetAbsTime();
							latestAttack = earliestAttack;

							/* obsolete
							longestDuration = noteList[i].attack->GetDuration();
							shortestDuration = longestDuration;
							*/

							earliestOffset = earliestAttack +
											 noteList[i].attack->GetDuration();
							latestOffset = earliestOffset;

						}
						else // compare to min/max values
						{
							// values for ambitus distance
							tempPitch = noteList[i].attack->GetMIDIPitch();		
							if( tempPitch < note1Pitch )
								note1Pitch = tempPitch;
							else if( tempPitch > note2Pitch )
								note2Pitch = tempPitch;
							
							// values for attack/duration distance
							tempFragm = noteList[i].attack->GetAbsTime();
							if( tempFragm < earliestAttack )
								earliestAttack = tempFragm;
							else if( tempFragm > latestAttack )
								latestAttack = tempFragm;

							// get offset
							tempFragm = tempFragm + noteList[i].attack->GetDuration();
							if( tempFragm > latestOffset )
							{
								latestOffset = tempFragm;
							}
							else if( tempFragm < earliestOffset )
							{
								earliestOffset = tempFragm;
							}
							
							/*
							if( tempFragm > longestDuration )
								longestDuration = tempFragm;
							else if( tempFragm < shortestDuration )
								shortestDuration = tempFragm;
							*/
				
							cNotesOfChord++;

						} // else
					} //if 
					i = noteList[i].linkID(); // next pos in cycle
				} // for
				while( i != j );

				// eval only if a real chord
				if( cNotesOfChord)
				{
					
					// ambitus distance
					temp = ((double)abs( note1Pitch - note2Pitch));

					temp = chordPenalty( temp );

					// ambitus distance 
					pVoice = temp;
					
					// Attack, lgDuration Distance -------------------------
					/*
					tempFragm = latestAttack - earliestAttack;
					attackDistance = tempFragm.toDouble() / longestDuration.toDouble();
					if( attackDistance > 1 )
						printf("");

					durationDistance = 1 - (shortestDuration.toDouble() / longestDuration.toDouble());
					//pAttacks = (double)(cNotesOfChord+1) / (double)cAttacks;
					//pVoice *= pAttacks;			

					// y = x + (1-x)*b => max(a,b) < x <= 1
					// y = x + b -bx = (1-b)x + b

					attackDistance = attackDistance + durationDistance - (attackDistance*durationDistance); 

					// give extra penalty for atackpoint moves
					// x = a + (1-a)*b => max(a,b) < x <= 1
					*/
					double 		   durationDistance;
					
					// Kernel duration / total duration
					durationDistance = (earliestOffset.toDouble() - latestAttack.toDouble()) /
									(latestOffset.toDouble() - earliestAttack.toDouble());
					
					// give more influece
					/*
					attackDistance *= attackDistance;

					attackDistance = 1 - attackDistance;
					*/
					durationDistance = 1 - GaussWindow( log(durationDistance), log(2.0));
													  						
													  						
													  												
					double intensPenalty = 1 - GaussWindow( minIntens, maxIntens, 2 );

					double chordProb;

					intensPenalty *= 0.3;			
					
					// penalty for single chord
					chordProb = (pVoice + durationDistance) - (pVoice*durationDistance);
					chordProb = (chordProb + intensPenalty) - (chordProb * intensPenalty);

					// chord penalty for complete selection
					sumDistance = (sumDistance + chordProb) - (sumDistance*chordProb);
					cChords++;
				}
			} // if root
		} // for voice
	} // if attacks
	

	res = sumDistance;
	if( res < 0 || res > 1 )
		Printf("Chord distance error\n");

return res;
#endif


	  
}

/*
	costs ~ |gap|/(|note|+|gap|)
	remarks:
		- only 1 note in voice (=chord note) will be evaluated
*/
double gapDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *voiceInfo,  // current used voices
					int CVoice,
					int /*id*/,
					double gapThreshold,
					TMIDIFILE *theMidifile
					)
{
#define LOCAL_SPLTV
#ifdef GLOBAL_SPLITV

	double res = 0,
		  gap,
		  maxGap,
		  cur;
	int i,j,
		start = 0,
		end = cAttacks;

	int voice,
		chordRoot;

	return 0;
	
	TAbsTime offset; // attack of first note
	offset = noteList[0].attack->GetAbsTime();

	double earliestOffset,
		temp;

	// init voiceInfo
	for( i = 0; i < CVoice; i++ )
	{
		voiceInfo[i].noteOff = offset;
	}

	if( id > -1 )
	{
		start = 0;
		end  = start+1;
	}

	for(i = 0; i < end; i++) 
	{
		voice = noteList->rootVoice( i );
		chordRoot = noteList->rootID( i );
		
		if( id < 0 ||
			i == id )
		{
			// get earliest note off
			earliestOffset = voiceInfo[0].noteOff.toDouble();
			for( j = 1; j < CVoice; j++ )
			{
				temp = voiceInfo[j].noteOff.toDouble();
				if( temp < earliestOffset )
					earliestOffset = temp;
			}
	
			gap = noteList[i].attack->GetAbsTime().toDouble() - voiceInfo[voice].noteOff.toDouble();
			maxGap = noteList[i].attack->GetAbsTime().toDouble() - earliestOffset;

			cur = 0;
			if( maxGap > 0 )
				cur = gap/maxGap;
			res += cur;

		}
		// add new value to voiceInfo
		voiceInfo[voice].noteOff = noteList[i].attack->offset(); 

	} // for
asdasd
	return res;
#endif

#ifdef LOCAL_SPLITV
	double res,
		  curAttack,
			temp,
			gap = 0,
			voiceEnd;

	TQNOTE *firstInVoice;

	// position of first note in all voices
	int VoiceID,
		cNotes = 0,
		i,
		j; 
	res = 0;

	/* gap distance MUST depend on abstime
	double start = -1;

	// search for earliest NoteOff in all voices
	TFrac tempFragm = -1;

	char oneEmpty = 0,
		 allEmpty = 1;

	// all empty -> start, earliest = - duration
	// one empty -> earliest duration = start
	tempFragm = voiceInfo[0].noteOff;
	for( j =0; j <CVoice;j++)
	{
		if( cAttacks == 3 )
			printf("");
		if(	voiceInfo[j].noteOff > -1 )
		{
			allEmpty = 0;
			if( tempFragm.toDouble() < 0 ) // first value
				tempFragm = voiceInfo[j].noteOff;
			else if( voiceInfo[j].noteOff < tempFragm ) // earlier
				tempFragm = voiceInfo[j].noteOff;
			
			//if( start < 0 )
			//	start = voiceInfo[j].start.toDouble(); // first non empty voice
			//else if( voiceInfo[j].start.toDouble() < start )
			//	start = voiceInfo[j].start.toDouble(); // first non empty voice
			//
		}
		else // if still one voice is empty
		{
			oneEmpty = 1;
		}
	}
	if( tempFragm > 0L )
		start = tempFragm.toDouble();
	if( start > gapThreshold )
		start -= 0.5 * gapThreshold;
		*/
	/*
	if( allEmpty )
	{
		tempFragm  = noteList[0].attack->GetDuration() * -1L; 
		start = tempFragm.toDouble();
	}
	else if( oneEmpty )
	{
		tempFragm = start;
	}
	*/
	/*
	earliestNoteOff = (double)tempFragm.toDouble();
	*/
// 	start = earliestNoteOff;

	/*
	if( earliestNoteOff < 0 ) // at least one empty voice
	{
		//use earliest attack <- first note may start much later than 0!
		tempFragm = noteList[0].attack->GetAbsTime();
		for(j=1; j<cAttacks; j++ )
		{
			if( noteList[j].attack->GetAbsTime() < tempFragm )
				tempFragm = noteList[j].attack->GetAbsTime();
		}

		for(j=0; j<CVoice; j++ )
		{
			if( voiceInfo[j].start < tempFragm &&
				voiceInfo[j].start >= 0L )
				tempFragm = voiceInfo[j].start;
				}
				start = tempFragm.toDouble();
				} // if
				earliestNoteOff = (double)tempFragm.toDouble();
	*/
	
	for( i = 0; i < CVoice; i++ )
	{
		firstInVoice = NULL;
		for( j =0; j <cAttacks;j++)
		{
			VoiceID = noteList->rootVoice(j);
			if( (noteList[j].linkID() < 0 ||  
				noteList[j].linkID() > j) && // gap only betwenn last note in voice and first note in noteList
				VoiceID == i )
			{
				
				//				curAttack = (double)firstInVoice->GetAbsTime().toDouble();
				curAttack = (double)noteList[j].attack->GetAbsTime().toDouble();
				cNotes++;
				
				/*				maxGap = curAttack - earliestNoteOff;
				// we have her still tick timing everything 
				
				  // if maxGap is very small we could get hight gap penalties for insignificant gaps
				  // if pPitch is >> pGap this would cause bad results
				  // if maxGap is <= gapThreshold we expand maxGap which will limit pGap!
				  if( maxGap > 0 &&
				  maxGap < gapThreshold )
				  maxGap = maxGap * (gapThreshold - maxGap);
				  
					if( maxGap > 0 ) // 
				*/
				{
					if( !firstInVoice )
						voiceEnd = (double)voiceInfo[i].noteOff.toDouble();
					else
						voiceEnd = (double)(firstInVoice->GetDuration()+firstInVoice->GetAbsTime()).toDouble();
					
					if( voiceEnd < 0 ) // empty voice 
					{
						voiceEnd = 0;
						/*
						voiceEnd = voiceInfo[0].gOffset.toDouble();
						voiceEnd = start;
						if( start < 0 )
							voiceEnd = curAttack;
						*/
					}
					gap = curAttack - voiceEnd;
				} // block
				firstInVoice = noteList[j].attack;
			} // if
			else if( 0 && VoiceID == i && // chord gap(all notes in chord)  = gap(first note in chord)
				noteList[j].linkID() > -1 )
			{
				// gap penalty only for complete chord
				
				cNotes++;
				
				curAttack = (double)noteList[noteList[j].linkID()].attack->GetAbsTime().toDouble();
				/*
				maxGap = curAttack - earliestNoteOff;
				if( maxGap > 0 )
				*/
				{
					if( !firstInVoice )
						voiceEnd = (double)voiceInfo[i].noteOff.toDouble();
					else
						voiceEnd = (double)(firstInVoice->GetDuration()+firstInVoice->GetAbsTime()).toDouble();
					
					if( voiceEnd < 0 )
					{
						voiceEnd = 0;
						
						// voiceEnd = voiceInfo[0].gOffset.toDouble();
						/*
						voiceEnd = curAttack;
						voiceEnd = start;
						if( start < 0 )
							voiceEnd = curAttack;
							*/
					}
				}
				gap = curAttack - voiceEnd;
				
			} // else
			
			if( gap < 0 )
				gap = 0;
			
			// old: temp = gap / maxGap;
			//					temp = 1 - exp(-0.25 * (gap / gapThreshold));
			
			temp = 1 - GaussWindow( gap/gapThreshold, 1.9);

			
			//old Average: 
			//					res += temp;
			res = res + (1-res)*temp;
			gap = 0;
			
			
			
		} // for attacks
	} // for voice

//	res /= cAttacks;

	/* used for old average*/
	/*
	if( cNotes > 0 )
		res /= cNotes;
	*/	

	if( res > 1 || res < 0 )
		cout << "Gap distance error ("<< res << ")!" << endl;
		
	return res;
#endif
}

double overlapDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *voiceInfo,  // current used voices
					int CVoice,
					int /*id*/,
					TMIDIFILE *theMidifile)
/*
	costs ~ 1 - ( new duration(n) / old duration ( n )
	remarks:
		- noteList must be sorted by ascending attacks
		- all notes in noteList MUST overlapp
*/
{
	TQNOTE *note1,
			*note2;

    TFrac duration1;

	int cNotes=0;

	int i,
		j;

//#define GLOBAL_SPLITV

#ifdef GLOBAL_SPLITV
	

	int pos,
		voice;
	int root;

	return 0; // todo needs to be changed

	for( i = 0; i < cAttacks-1; i++ )
	{
		pos = cAttacks - i - 1; // go from right to left
		if( noteList[pos].linkID() < 0 ) // root or not linked, skip chord notes
		{
			note1 = noteList[pos].attack;
			voice = noteList[pos].voice();

			note2 = NULL;
			// search to left until first note in same voice
			for( j = pos-1; j > -1; j-- )
			{
				if( noteList->rootVoice( j) == voice )
				{
					root = noteList->rootID(  j);
					if( root < 0 )
						root = j;
					note2 = noteList[root].attack;
					j = -1; // stop loop;
				}
			}
			if( note2 )
			{
				res += note2->overlapDistance(note1->GetAbsTime());
			}
		}// if
	}

	return res;


#endif

#ifdef LOCAL_SPLITV
	double res = 0;

	for( i =0; i < CVoice; i++) // search through all voices
	{
		note1 = NULL;
		note2 = NULL;

		note1 = voiceInfo[i].lastNote;		

		/*
		if(note1) // check for overlapp to prev note in voice
		{
		
			duration1 = note1->GetDuration();

			if( note1->GetAbsTime() + note1->GetDuration() <= 
				noteList[0].attack->GetAbsTime() )
			{
						// no overlapp
						note1 = NULL;
			}
		}
		*/

		
		TQNOTE *latestOffsetNote;
		TAbsTime latestOffset;
		int j2;

		latestOffsetNote = note1;
	
		for( j =0; j < cAttacks;j++)
		{
			if( noteList[j].voice() == i  &&
				(noteList[j].linkID() < 0 ||  // no chord linked notes
				 noteList[j].linkID() > j) ) // take only root note of chord
				// add distance for each note in this voice
			{
				if( !note1 ) // j == first note in voice i
				{
					// no overlap to existing voice
					note1 = noteList[j].attack;
					duration1 = note1->GetDuration();

					latestOffsetNote = note1;

					j2 = noteList[j].linkID(); 
					if(  j2 > -1 )
					// search for latest offset note in chord
					{
						latestOffset = latestOffsetNote->GetAbsTime()+
									   latestOffsetNote->GetDuration();
						while( j2 != j )
						{
							if( note1->GetAbsTime() + note1->GetDuration() < latestOffset )
							{
								latestOffsetNote = note1;
								latestOffset = latestOffsetNote->GetAbsTime()+
									   latestOffsetNote->GetDuration();
							}
							j2 = noteList[j2].linkID();
	
						} // while
					} // if
				}
				else // j >= second note in voice
				{
					note2 = noteList[j].attack;
			
					double cRes = latestOffsetNote->overlapDistance(note2->GetAbsTime(),
															 note2->GetDuration(),
															 theMidifile);
					res = res + cRes - res*cRes;
					/*
					if( res > 0 )
						latestOffsetNote->overlapDistance(note2->GetAbsTime(),
																 note2->GetDuration() );
					*/

					/*
					gap = note2->GetAbsTime() - note1->GetAbsTime();
					// gap <= duration because of sort
					res += 1 - (gap.toDouble() / duration1.toDouble());

					// inc position
					*/
					note1 = note2;
					duration1 = note1->GetDuration();
					latestOffsetNote = note1;

					j2 = noteList[j].linkID(); 
					if(  j2 > -1 )
					// search for latest offset note in chord
					{
						latestOffset = latestOffsetNote->GetAbsTime()+
									   latestOffsetNote->GetDuration();
						while( j2 != j )
						{
							if( note1->GetAbsTime() + note1->GetDuration() < latestOffset )
							{
								latestOffsetNote = note1;
								latestOffset = latestOffsetNote->GetAbsTime()+
									   latestOffsetNote->GetDuration();
							}
							j2 = noteList[j2].linkID();
	
						} // while
					} // if

					
					
					cNotes++;
				}
			} // if

		} // for
	} // for
	/*
	if( cNotes > 0 )	// normalize range to 0..1
		res /= cNotes;
	*/
	return res;
#endif
}

// penalty for ambitus of chord
double chordPenalty( double ambitus )
{
	double temp;
	temp = ambitus;
	//					temp = temp/24;

					//--- total exp version -------------
					// -> output values are very small
					/*
					
					temp = temp / 128; // [0...1] 
					temp = (exp(3 * temp )  - 1)/ (exp(3)-1); // 0...1 
					*/



	// chordFunc := 1-exp(-((x/15)^6))
	/*
	ambitus = fabs(ambitus)/15;
	ambitus = pow( ambitus,6 );

	temp = 1-exp(-ambitus);
	*/
	temp = 1 - kGaussWindow(ambitus, 12, 4);
	return temp;
// switch between exp and linaer
#define cEXPTHRESH 24
// penalty at switchpoint
#define cbreakP 0.8
// exponential influence
#define cnX 3
					if( temp >= cEXPTHRESH )
					// use linear function
					{
						temp = cbreakP + (temp-cEXPTHRESH)/(128-cEXPTHRESH)*(1-cbreakP);
					}
					else 
						// use exp function
					{
						double a;
						a = log(1+cbreakP)/(cnX * cEXPTHRESH);
						temp = exp(cnX * temp * a) - 1;
					}
					
					if( temp > 1 )
						temp = 1;
					else if( temp < 0 )
						temp = 1;

	
	return temp;
}

double pitchPenalty( double interval )
{
	// use a GaussWindow!
	double temp;

	temp = interval;
	// chordFunc := 1-exp(-((x/15)^6))
	/*
	temp = fabs(temp)/15;
	temp = pow( temp,6 );
	temp = 1-exp(-temp);
	*/
	temp = 1 - GaussWindow(interval, 12);

	return temp;


// old linear version
//					temp = (double)temp / 128;
					//				res += temp;

// new exp-lin version 23.8.02
// switch between exp and linaer
#define pEXPTHRESH 15
// penalty at switchpoint
#define pbreakP 0.8
// exponential influence
#define pnX 2
					if( temp >= pEXPTHRESH )
					// use linear function
					{
						temp = pbreakP + (temp-pEXPTHRESH)/(128-pEXPTHRESH)*(1-pbreakP);
					}
					else 
						// use exp function
					{
						double a;
						a = log(1+pbreakP)/(pnX * pEXPTHRESH);
						temp = exp(pnX * temp * a) - 1;
					}
					
					if( temp > 1 )
						temp = 1;
					else if( temp < 0 )
						temp = 1;

	return temp;
}

/*
	costs ~ |lastPitch in voice, pitch of new note|
	remarks:
 - only 1 note in voice (=chord note) will be evaluated
 */

double pitchDistance(TNoteInfo *noteList, // current chord to test
					int cAttacks,         // size of array
					TVoiceInfo *voiceInfo,  // current used voices
					int CVoice,
					int lookBack,
					int EmptyVoiceIv)
{
//#define GLOBAL_SPLITV

#ifdef GLOBAL_SPLITV
	int i,j,
		count,
		chordRoot,
		chordRoot2,
		note1Pitch,
		note2Pitch,
		voice;
	double res = 0,
			temp,
		  cur;

	int EmptyVoiceIv = 12;
	TInifile *tempIni;
	int lookBack = 0;

	tempIni = GetInifile();
	if(tempIni ) 
	{
		lookBack = tempIni->GetValInt("PITCHLOOKBACK","2"
		EmptyVoiceIv = tempIni->GetValInt("EMPTYVOICEIV","11; start interval in empty voice");
	}


	

	// init voice info
	for( i = 0; i < CVoice; i++ )
	{
		voiceInfo[i].lastNote = NULL;
		voiceInfo[i].lastNoteID = -1;
		voiceInfo[i].lastPitch = 0;
	}

	for( i = 0; i < cAttacks; i++ )
		noteList[i].flag = 0;

	int end = cAttacks;

	if( id > -1 )
		end = id + 1;

	int link;
	for( i = 0; i < end; i++ )
	{
		cur = 0;
		temp = -1;
		voice = noteList->rootVoice( i );
		chordRoot = noteList->rootID( i );

		// search for pitch  of prev notes
		if( !noteList[i].flag &&
			( id < 0 ||
			  id == i)  )
		{
			j = voiceInfo[voice].lastNoteID;
			// do for one cycle in chord 
			link = i;

			do
			{
				if( j > -1 )
				{
					temp = noteList->pitchDistance( j,
													link,
													lookBack );
				}
				else
				{
					temp = EmptyVoiceIv;
				}

				cur = (double)abs( temp ) / 128;

				noteList[link].lPitch = cur;
				noteList[link].flag = 1;

				res += cur;
				link = noteList[link].linkID();

			} while( link > -1 &&
				      link != i );
		} // if

		if( noteList->isTail( i ) ) // add to chord if last note in chord
			voiceInfo[voice].lastNoteID = i;

	} // for
	return res;
	
#endif
#ifdef LOCAL_SPLITV
	double res,
		temp;
	
	
	int mode = 0, // 0 = prevNote == outside notelist, 1 == inside noteList
		i,j,
		c,
		VoiceID,
		minPitch,
		notePitch;
	
	
	TQNOTE *prevNote = NULL;
	int prevID = -1; // id of prev note, -1 == outside tnoteList
	minPitch = 0;
	c = 0;
	
	res = 0;
	
	// init flags
	for( i= 0; i < cAttacks; i++ )
	{
		noteList[i].flag = 0;
	}
	
	// check distance for all voices
	for( i =0; i < CVoice; i++)
	{
		prevNote = voiceInfo[i].lastNote;
		mode = 0; // outside noteList
		for( j = 0; j < cAttacks; j++) 
		{
			VoiceID = noteList[j].voice(); // must be 			
			// evaluate each attack only once
			if( !noteList[j].flag && 
				VoiceID == i)
			{
				notePitch = noteList[j].attack->GetMIDIPitch();		
				noteList[j].flag = 1; // mark note j				

				if( !prevNote  )	// must be an empty voice!
				{
					
					if( !voiceInfo[VoiceID].lastNote ) // first note in voice?
						// should always be true
					{
						temp = EmptyVoiceIv; // Interval size for new voice

						/*
							- noteList[j] == single note -> use EmptyVoiceIv as start distance
							- noteList[j] == chord: use 1xEmptyVoiceIv and mark all chord notes

							- set prevNotes as root of noteList[j]
						*/
						if( noteList[j].linkID() < 0 ) // not linked, no chord
						{
								prevNote = noteList[j].attack;
								mode = 1; // inside noteList
								prevID = noteList->rootID( j );
								if( prevID < 0 )
									prevID = j;
						}
						else // noteList[j] is part of chord
						{
								int rootID;
								// mark all other notes of chord
								rootID = noteList->rootID( j );
								minPitch = noteList->mPitch( rootID );

								c = noteList[j].linkID();					
								while( c != j ) // one cycle
								{
									if( c != rootID ) // add penalty for all non root notes
									{
										// get midi pitch of c
										notePitch = noteList->mPitch( c );
										// add penalty for ambitus!!
										temp = abs( minPitch - notePitch );

										// temp = (double)temp / 128;
										temp = pitchPenalty( temp/2 );
										res = res + (1-res)*temp;
			
									}
									noteList[c].flag++;
									c = noteList[c].linkID();
								}

								// Todo: add pitch penalty (ambitus?) for non root notes? 

								prevNote = noteList[j].attack;
								mode = 1; // inside noteList
								prevID = noteList->rootID( j );
								if( prevID < 0 )
									prevID = j;

								temp = EmptyVoiceIv; // add penalty for root
						} // else (chord)
					} // if last note 
					// initial gaps will be penalt in gapDistance
					else 			
					{
						// todo: make weighted pitches in lookback
						notePitch = noteList->mPitch(j); // = MIDIPitch
						temp = voiceInfo[VoiceID].lastNote->deltaPC( notePitch, lookBack );
					}
					// here 0 <= temp <= 128
					// normalize to 0 <= temp <= 1
					//				temp = (double)(log((temp*2)+1)/log(257));				
					//				temp = (double)(log((temp)+1)/log(129));				

					temp = pitchPenalty( temp );

					
					
					
					
					res = res + (1-res)*temp;
					
					/*
					if( !prevNote ) // new root, or first in voice
					{
					prevNote = noteList[j].attack;
					prevID = noteList->rootID( j );
					}
					*/
					
				}
				else if( prevNote )
				{
					/*
						- noteList[j] == single note -> pitchDistance to prev Note
						- noteList[j] == chord -> pitchDistance for all notes of chord

						- prevNote = root(noteList[j])
					*/

					c = j; // start with j, maybe go through chord
                    // printf("<do"); fflush(stdout);
					do // for complete chord
					{						
						notePitch = noteList->mPitch(c);
                        // printf("<pd%d",mode); fflush(stdout);
						if( mode == 0 ) // outside noteList
							temp = prevNote->deltaPC( notePitch, lookBack );
						else
							temp = noteList->pitchDistance( prevID,
															j,
															lookBack );

                        // printf("pd>"); fflush(stdout);
						
						// here 0 <= temp <= 128
						// normalize to 0 <= temp <= 1
						//				temp = (double)(log((temp*2)+1)/log(257));				
						//				temp = (double)(log((temp)+1)/log(129));				
						// temp = (double)temp / 128;
						temp = pitchPenalty( temp );

						//				res += temp;
						res = res + (1-res)*temp;
						c = noteList[c].linkID();

						if( c > -1 ) // mark as processed
							noteList[c].flag++;
					} while( c > -1 &&
						     c != j );
                    // printf("do>"); fflush(stdout);

                    prevNote = noteList[j].attack; // this is last note in voice
					mode = 1;
					prevID = noteList->rootID( j );
					if( prevID < 0 )
						prevID = j;

				} // else if prev Note
			} // if !flag && VoiceID
		} // for attacks
	} // for voice
	
#ifdef _DEBUG
	if( res < 0 || res > 1 )
		Printf("pitchDistance out of range!\n");
#endif
	
	
	//	res /= cAttacks;
	return res;
#endif
}



double voiceDistance(TNoteInfo *noteInfo,
					TVoiceInfo *voiceInfo,
					TVoiceInfo * /*maxValues*/)
// k-NN Distance from noteInfo to voiceInfo
{
	double res;

#define pitchweight 1
#define restweight 1
#define lastusedweight 2
#define cnotesweight 2
#define MAXDISTANCE 45

	// read from ini

	// --------------------- check tonal distance --------------
	int midiPitch = noteInfo->attack->GetMIDIPitch();
	// range = 0..128 * pitchweight
	// note distance  0..12 ..24 .. 128
	// result         0..0.5 .0.8 ,, 1
	if( voiceInfo->lastPitch >= 0)
	{
		
		// linear distance between pitches
		res = (fabs( midiPitch -
			 voiceInfo->lastPitch));

		/*
		if( res <= 12  )
			res /= 24;
		else if( res <= 24 )
			res = 0.2 + (res/40);
		else
			res = 0.7538 + (res/520);
		*/

		// convert linear into log
		res = (double)(log((res*2)+1)/log((double)257));


		/*
		if( voiceInfo->lastUsed < 0 )
		{
		// range 0...1 * lastusedweight
		res += (voiceInfo->lastUsed/(maxValues->lastUsed)) * lastusedweight;
		}
		
		  //	range =    (0 ..1 * weight
		  res += ((maxValues->cNotes)/(voiceInfo->cNotes+1)) * cnotesweight;
		*/
		
		// check ambitus ------------------------------------
		// range  = 0..128
/*		
		if( voiceInfo->minPitch > midiPitch &&
			voiceInfo->minPitch <= voiceInfo->maxPitch ) // valid?
		{
			minmaxRes = (voiceInfo->minPitch - midiPitch);
			minmaxRes = log( minmaxRes + 1) / log(129);
		}
		else if(  voiceInfo->maxPitch < midiPitch &&
				   voiceInfo->minPitch <= voiceInfo->maxPitch ) // valid?
		{
			minmaxRes = (midiPitch - voiceInfo->maxPitch);
			minmaxRes = log( minmaxRes + 1) / log(129);
		}
		else 
			minmaxRes = 0;
		
		// resDur 0   ..  |note|  .. 2*|note|  ... infinite
		//	      0         0.5         0.8           1
		double restDuration,
			noteDuration; // duration of rest between last note and current note
		restDuration = (noteInfo->attack->GetAbsTime() - 
			voiceInfo->noteOff).toDouble();
		noteDuration =  noteInfo->attack->GetDuration().toDouble();
		
		
		if( restDuration < noteDuration )	// 0 .. 1 ->  0 .. 0.5
			resDuration = (restDuration / noteDuration ) * 0.5;
		else if( restDuration < 3 * noteDuration) // 1..3  -> 0.5 .. 0.8
			resDuration = 0.5 + (((restDuration / noteDuration) - 1 ) *  0.15);
		else // 3 .. oo  (= 0.33 -> 0 ) -> 0.7 .. 1
			resDuration = 1 - ((noteDuration / restDuration) * 0.476);
		
*/		
			/*
			if(restDur > noteInfo->attack->GetDuration() )
			res += restDur.toDouble() / noteInfo->attack->GetDuration().toDouble();
		*/
		
		/*
		if( res > 0.5 )
			res = res + ((1-res) * (minmaxRes * 0.5));
			*/
//		res = res + (resDuration/4);
	} // if last pitch
	else
		res = 0; // try not to use empty voices?
	return res;

}


//----------------------------------------------------------------------
TVoiceInfo GetMaxVoiceValues(TVoiceInfo *voiceInfo,
								 int  CVoice )		// size of array

// return max Voice Info Values
{
	int i;

	int Voice = 0;

	TVoiceInfo res;
	res.cNotes = voiceInfo[Voice].cNotes;
	res.hold = voiceInfo[Voice].hold;
	res.lastPitch = -1;
//	res.lastUsed = voiceInfo[Voice].lastUsed;
	res.maxPitch = voiceInfo[Voice].maxPitch;
	res.meanDuration = voiceInfo[Voice].meanDuration;
	res.minPitch = voiceInfo[Voice].minPitch;
	res.rest = voiceInfo[Voice].rest;
	for(i = Voice+1; i < CVoice; i++)
	{
		if( voiceInfo[i].cNotes >
			res.cNotes )
			res.cNotes = voiceInfo[i].cNotes;

		if( voiceInfo[i].hold >
			res.hold )
			res.hold = voiceInfo[i].hold;

/*		if( voiceInfo[i].lastUsed < 
			res.lastUsed )
			res.lastUsed = voiceInfo[i].lastUsed;
			*/
		if( voiceInfo[i].maxPitch >
			res.maxPitch ) 
			res.maxPitch = voiceInfo[i].maxPitch;
		if( voiceInfo[i].meanDuration >
			res.meanDuration )
			res.meanDuration = voiceInfo[i].meanDuration;
		if( voiceInfo[i].minPitch <
			res.minPitch )
			res.minPitch = voiceInfo[i].minPitch;
		if( voiceInfo[i].rest >
			res.rest )
			res.rest = voiceInfo[i].rest;
	} // for
	return res;
}



//----------------------------------------------------------------------

void findClosestVoices(TNoteInfo *noteList,  // array
					  int cAttacks,
					TVoiceInfo *voiceInfo, // array
					int CVoice)				// maxVoice
/* 
	- set closest voice for each noteList[i]
	- use only pitch distance as cost function
	- all note with equal closest voice will be linked to chord
*/
{
//#define GLOBAL_SPLITV

#ifdef GLOBAL_SPLITV
	int i,
		root;
	for( i=0; i < cAttacks; i++ )
	{
		noteList[i].setVoice( CVoice/2 ); // set all notes to first voice
		if( i > 0 ) // check if chord with prev
		{
			if( noteList[i].attack->GetAbsTime() == noteList[i-1].attack->GetAbsTime() )
			{
				// equal attack -> chord
				root = noteList->rootID( i-1);
				if( root < 0 ) 
					root = i-1;
				noteList->link(i-1, i );
			} // if equal
		} // if i
	} // for



#endif

#ifdef LOCAL_SPLITV
	int j,i,
		earliestVoiceID = -100,
		minVoiceIndex;

	double curDiff;


	// calc closest voice for each attack
	// select for each note the closest voice of pitch distance
	for(i = 0; i < cAttacks; i++ )
	{

		minVoiceIndex = -1;
		// array-position empty?
		if( noteList[i].attack ) 
		{
			int pitch2;
			pitch2 = noteList[i].attack->GetMIDIPitch();

			//calculate feature vector and do k-NN search
			noteList[i].minDiff = MAXINT;
			noteList[i].minVoice = -1; // curVoice
			earliestVoiceID = -1;
			// find closest voice for noteList[i]
			for(j = 0; j < CVoice; j++ ) // 0 ..(Voice -1) is already processed
			{

				// todo maybe use here pitch-closest instead of earliest
				if( voiceInfo[j].lastNote && 
					earliestVoiceID == -1 )
					earliestVoiceID = j;
				else if( voiceInfo[j].lastNote )
				{
					TNOTE *n1,*n2;
					n1 = voiceInfo[earliestVoiceID].lastNote;
					n2 = voiceInfo[j].lastNote;
					if( !n1 || !n2 )
						printf("NULL Pointer !\n");
					if( *n1 > *n2 )  
						earliestVoiceID = j;
				}
				
				//!! be carefull with empty voices!!
				if(  voiceInfo[j].lastNote &&  // only distance to used voices
					 voiceInfo[j].noteOff <=	// overlapping?
						noteList[i].attack->GetAbsTime())
				{
					// calc pitch-distance

					noteList[i].minVoice = 0;  // temp index for compare

					curDiff = voiceInfo[j].lastNote->closestPitch( pitch2 );
					curDiff = fabs( curDiff - pitch2 );

/*
					curDiff = pitchDistance( &(noteList[i]),
											 1, // #attacks
											 &(voiceInfo[j]),
											 1, // #voices
											 -1); // dummy id
*/
					if( curDiff < noteList[i].minDiff )
					{
						noteList[i].minDiff = curDiff;
						minVoiceIndex = j;

					}
				} // if !locked
			} // for Voice j
		} // if attack



		if( minVoiceIndex > -1 )
		{
			noteList[i].minVoice = minVoiceIndex;
		}
		else if( !voiceInfo[0].lastNote ) // all voices are unused
		{
			noteList[i].minVoice = 0;
		
		}
		else	// no empty voice,
		{
			// make last note (earliest of all voices) in one voice shorter, do again
			// use voice with minimal overlap
			/*

			TFrac IOI;
			IOI = noteList[i].attack->GetAbsTime() - 
				  voiceInfo[earliestVoiceID].lastNote->GetAbsTime();	
			voiceInfo[earliestVoiceID].lastNote->SetDuration(IOI);
			*/
			noteList[i].minVoice = earliestVoiceID;
			
		}

	} // for attack

	// change ovelapping notes in same voices into chords
	int note1ID;
	for(j = 0; j < CVoice; j++)  // search for chords
	{
		note1ID = -1; // root note
		for( i = 0; i < cAttacks; i++ )
		{
			if( noteList[i].minVoice == j ) // root note
			{
				if( note1ID < 0 )  // first note in voice
				{
					note1ID = i;
					noteList->unlink(i);
//					noteList[i].setLink( -1 ); // i becomes the root
				}
				else // second note in voice
				{
					noteList->link(i, note1ID);
//					noteList[i].setLink( note1ID ) ;
				}
			}
		} // for attack
	} // for voice

#endif
} // findClosestVoices

/*
//----------------------------------------------------------------------
void collissionOptimize( TNoteInfo *noteList,  // array
					     int cAttacks,
					     TVoiceInfo *voiceInfo, // array
					     int CVoice)			// maxVoice
{
	int *fixedVoices,
		*fixedNotes,
		*selectedVoices,
		*minVoices,
		run,
		cRuns,
		voice,
		i,j,cUnfixed;

	double curDistance,
		  minDistance = -1;
	fixedVoices = new int[CVoice];
	fixedNotes  = new int[cAttacks];
	selectedVoices = new int[cAttacks];
	minVoices = new int[cAttacks];

	cUnfixed = 0;
	// search for ambigous voices -> keep unfixed
	for(j = 0; j < CVoice; j++ )
	{
		if( voiceInfo[j].tempCount == 1 )
			fixedVoices[j] = 1;
		else
		{
			fixedVoices[j] = 0;
			cUnfixed++;
		}
	}
	if( cUnfixed == 1 )
	{
		Printf("Warning: collissionOptimize only 1 unfixed voice!\n");
		return;
	}

	// mark fixed/unfixed notes
	for(i = 0; i < cAttacks; i++ )
	{
		if( noteList[i].attack &&
			noteList->rootVoice( i ) > -1 &&
			fixedVoices[noteList->rootVoice( i )] ) // link to fixed voice
		{
			fixedNotes[i] = 1;
			selectedVoices[i] = noteList->rootVoice( i );
		}
		else if( !noteList[i].attack )		// empty position
		{
			fixedNotes[i] = 1;
			selectedVoices[i] = noteList->rootVoice( i );
		}
		else							// can be changed
		{
			fixedNotes[i] = 0;
			selectedVoices[i] = -1;
		}
	}

	// now all unique notes & voices have fixedNotes/Voices to 
	cRuns = cUnfixed * 10;
	run = 0;
	while( run < cRuns ||
		   minDistance < 0 )
	{

		run++;
		// reset temp fixed notes and voices
		for( j = 0; j < CVoice; j++ )
		{
			if( fixedVoices[j] == 2 )
				fixedVoices[j] = 0;
		}
		
		// find for all unfixed notes an unfixed voice
		for( i = 0; i < cAttacks; i++ )
		{
			if( noteList[i].attack &&
				!fixedNotes[i] )
			{
				
				voice = -1;
				while( voice < 0 )
				{
					
					voice = rand() % CVoice;
					
					// select only unfixed voices
					if( fixedVoices[voice] )	// voice avaliable
						voice = -1;
					else if( !fixedVoices[voice] &&	// overlapping
							 noteList[i].attack->GetAbsTime() <
							 voiceInfo[voice].noteOff )
					{
						voice = -1;
					}
				} // while

				// lock temporarly if unfixed before
				fixedVoices[voice] = 2;
				selectedVoices[i] = voice;
			} // if attack
		} // for i

		// calc global distance for this selection
		curDistance = 0;
		for(i = 0; i < cAttacks; i++ )
		{
			if( noteList[i].attack &&
				!fixedNotes[i])
				curDistance += voiceDistance(&(noteList[i]),
											 &(voiceInfo[selectedVoices[i]]),
											 NULL); // not used

		}
		// keep selection with min distance
		if( minDistance < 0 ||
			curDistance < minDistance )
		{
			minDistance = curDistance;
			for( i = 0; i < cAttacks; i++ )
				minVoices[i] = selectedVoices[i];
		}


	} // for run

	// copy selected voices
	for(i = 0; i < cAttacks; i++ )
	{
		noteList[i].setVoice( minVoices[i] );
	}

	delete [] selectedVoices;
	delete [] minVoices;
	delete [] fixedVoices;
	delete [] fixedNotes;
}
*/
//----------------------------------------------------------------------
#ifdef asdaksjdhs
 int findBestAttack(TNoteInfo *noteList,		// array
				   int cAttacks)
// return best next note for Voice
{




	int minAttack = -1,
		i;

// for testing
	for(i = 0; i < cAttacks; i++ )
		if( noteList[i].attack )
			return i;



	double minDiff = MAXINT;

	for(i = 0; i < cAttacks; i++ )
	{
		if( noteList[i].attack  )
		{
			if( // noteList[i].minVoice == Voice &&
				noteList[i].minDiff < minDiff )
			{
				minDiff = noteList[i].minDiff;
				minAttack = i;
			}
			else if( // noteList[i].minVoice == Voice &&
				noteList[i].minDiff == minDiff )
			{
				// Strategie ???
#ifdef _DEBUG
				printf("equal voice distance!\n");
#endif
			}
			
		} // if attack
	} // for cAttack i 
	return minAttack;
} // findBestAttack
#endif

int getVoiceIndex( int voice,
				   TVoiceInfo *voiceInfo, // array
				   int nrOfVoices )
/*
	return index of voice with oldVoice == voice
	or first empty voice
*/
{
	int index = 0;
	while( index < nrOfVoices &&
		   voiceInfo[index].oldVoice > -1 && // use first empty voice	
		   voice != voiceInfo[index].oldVoice )
	{
		index++;
	}
	return index;
}

int findClosestVoices( TVoiceInfo *unmerged, // unmerged Voices
							  int /*nrOfVoices*/,
							  int segStart,
							  int segEnd,
							  TVoiceInfo *target, //  targetVoices[segSize], 
							  int segSize,
							  TSearchInfo *result)	// closestVoice/distance[segSize]

/*
	Find for each unmerged[segSart..segEnd-1]
	the closest voice in target[0..segSize-1]
	return:
		1 : 1 or more possible voices found
		0 : no voice found
*/
{
	if( !unmerged )
		return -1;


	int j,
		ret,
		ambitus1,
		ambitus2;

	double ambitusOverlapping,
		  pitchDistance;


	// init result
	ret = 0;
	for( j = 0; j < segSize; j++ )
	{
	  // init result
		result[j].sourceIndex = -1;
	  result[j].pitchIndex = -1;
      result[j].pitchDistance = 300;

	  result[j].ambitusIndex = -1;
	  result[j].maxAmbitus = -1;
	} // for

	int index;
	// find closest voice for all voices in voiceList
	for( index = segStart; index < segEnd; index++)
	{
		if( unmerged[index].newVoice < 0 )
		{
			for(j = 0; j < segSize; j++ )
			{
				// overlapping in range?
				if( target[j].noteOff <= unmerged[index].start  )	
				{
					ret = 1;
					// calc pitch
					if( target[j].lastPitch > -1 )
						pitchDistance = fabs(target[j].lastPitch -
										unmerged[index].firstPitch );
					else
						pitchDistance = 128; // keep undependend from played pitch

					if( pitchDistance < result[index-segStart].pitchDistance )
					{
						result[index-segStart].pitchDistance = pitchDistance;
						result[index-segStart].pitchIndex = j;
						result[index-segStart].sourceIndex = index;
					}
					
					// calc ambitus
					ambitus1 = unmerged[index].maxPitch - unmerged[index].minPitch;
					ambitus2 = target[j].maxPitch - target[j].minPitch;
					if( ambitus2 > ambitus1)
						ambitus1 = ambitus2;
					if( unmerged[index].minPitch <= target[j].maxPitch &&
						unmerged[index].maxPitch >= target[j].minPitch )
					{
						ambitusOverlapping = (unmerged[index].maxPitch - 
							target[j].minPitch)/
							ambitus1;
					}
					else if( target[j].minPitch <= unmerged[index].maxPitch &&
						target[j].maxPitch >= unmerged[index].minPitch )
					{
						ambitusOverlapping = (target[j].maxPitch 
							- unmerged[index].minPitch)/ambitus1;
					}
					else
						ambitusOverlapping = 0;
					
					if( ambitusOverlapping > result[index-segStart].maxAmbitus )
					{
						result[index-segStart].maxAmbitus = ambitusOverlapping;
						result[index-segStart].ambitusIndex = j;
						result[index-segStart].sourceIndex = index;
					}
				} // if
			} // for
		} // if
	}// for
	return ret;
}

//----------------------------------------------------------------------
int selectClosestVoice( TVoiceInfo *unmerged, // unmerged Voices
					  int /*nrOfVoices*/,		// array size
					  int /*segStart*/,			// start index of current segment
					  int /*segEnd*/,			// end index current segment
					  TVoiceInfo *target, //  targetVoices[segSize], 
					  int segSize,			// array size of *target, *distance
					  TSearchInfo *distance)	// closestVoice/distance[segSize]

/*
*/
{
	int j,
		minPitchIndex = -1,
		maxAmbitusIndex = -1,
		targetAmbitusVoice = -1,
		targetPitchVoice = -1,
		closestIndex;

	double minPitchDistance = 300,
		  maxAmbitus = 0;

	// find closest pair in distance
	for( j = 0; j < segSize; j++)
	{
		if( distance[j].pitchIndex > -1 &&
			distance[j].pitchDistance < minPitchDistance )
		{
			minPitchIndex = distance[j].sourceIndex;
			targetPitchVoice = distance[j].pitchIndex;
			minPitchDistance = distance[j].pitchDistance;
		}

		if( distance[j].ambitusIndex > -1 &&
			distance[j].maxAmbitus > maxAmbitus )
		{
			maxAmbitusIndex = distance[j].sourceIndex;
			targetAmbitusVoice = distance[j].ambitusIndex;
			maxAmbitus = distance[j].maxAmbitus;
		}
	}

	closestIndex = targetPitchVoice;
	// following needs to be improved
	if( targetPitchVoice == targetAmbitusVoice )
	{
		closestIndex = targetPitchVoice;
	}
	if( minPitchIndex > -1 )
	{
		unmerged[minPitchIndex].newVoice = targetPitchVoice;
		target[targetPitchVoice].addToVoice(&(unmerged[minPitchIndex])); 
	}
	else // no closest pair found
	{
		// find empty target voice for one unmerged element
#ifdef _DEBUG
		Printf("SelectClosest Voice: no voice found!");
#endif
	}
	return 1;
}

void TVoiceInfo::addToVoice(TVoiceInfo *voiceInfo )
// merge this and *voiceInfo
{
	this->cNotes += voiceInfo->cNotes;
	this->lastPitch = voiceInfo->lastPitch;
	if( maxPitch < voiceInfo->maxPitch )
		this->maxPitch = voiceInfo->maxPitch;

	if( voiceInfo->minPitch < minPitch )
		minPitch = voiceInfo->minPitch;

	this->noteOff = voiceInfo->noteOff;

}



 /*!
 add now to feature vector of corresponding voice
  */
 void addToVoice( TNOTE *now,
                  TVoiceInfo *voiceInfo, /// array
                  int nrOfVoices 		/// size of array
                  )
 {
     int voice,
     index;
     if( !now ||
         !voiceInfo )
         return;
     	index = 0;
        voice = now->GetVoice();
        index = getVoiceIndex ( voice ,
                                voiceInfo,
                                nrOfVoices );
        	if( index < nrOfVoices )
            {
                voiceInfo[index].addToVoice(now,
                                            500); // PitchDecay
            }
#ifdef _DEBUG
            else
            {
                Printf("Error addToVoice: voice not found\n");
            }
#endif
 } // addToVoice

