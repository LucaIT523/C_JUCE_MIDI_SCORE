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
#include <string>
#include <stdlib.h>
#include "note_info.h"
#include "funcs.h"
#include "../lib_src/ini/ini.h"
//----------------------------------------------------------------------
 
#undef UNUSED
//----------------------------------------------------------------------


void TNoteInfo::setLink( int id )
{
	chordRoot = id;
}

void TNoteInfo::link(int prev, int next)
{
	if( next < 0 ||
		next == prev )
		Printf("warning link error \n");

	int temp;
	if( prev > next ) // swap
	{
		// swap
		temp = next;
		next = prev;
		prev = temp;
	}

/*
	if( this[next].linkID() > -1 ) // already linked
 	{
		unlink( next );
	}
*/
	if( prev < 0 ) // maybe old link
	{
		this[next].setLink( -1 );
		return;
	}


	if( this[prev].linkID() > -1 &&	// chord
		this[next].linkID() > -1 )	// chord
	{
		int temp,
			root2,
			root;
		root = this->rootID( next );
		root2 = this->rootID( prev );
		if(  root == root2 )
			return; // already linked
		temp = root;
		do
		{
			next = this[temp].linkID();
			unlink( temp );
			link( prev, temp );
			temp = next;
		}while( temp != root );
		return;

	}

	if( this[prev].linkID() < 0 &&	// single note
		this[next].linkID() > -1 )	// chord
	{
		if( this->isRoot( next ) > 0 ) 
		{
			this[prev].setLink( this[next].linkID() ); // new root
			this[next].setLink( prev );
		}
		else
		{
			int root;
			root = this->rootID(next);
			link( root, prev );
		}
		return;
	}


	TNoteInfo *noteInfo;
	noteInfo = this; // should be [0]

	// find root of chord
	int root = -1;
	if( noteInfo[prev].linkID() < 0 ) // new chord
	{
#ifdef _DEBUG
		if( this[next].voice() != this[prev].voice() )
			Printf("Link voice error\n");
#endif

		
		
		noteInfo[prev].setLink( next );
		noteInfo[next].setLink( prev );
	}		
	else  // go through chord
	{
		if( noteInfo->isRoot( prev )  )
			root = prev;
		else
			root = noteInfo->rootID( prev );

		int p, n;
		p = root;
		n = noteInfo[root].linkID();
#ifdef _DEBUG
		if( this[n].voice() != this[p].voice() )
			Printf("Link voice error\n");
#endif

		while( noteInfo[p].linkID() > next )
		{
			p = n;
			n = noteInfo[n].linkID();
#ifdef _DEBUG
		if( this[n].voice() != this[p].voice() )
			Printf("Link voice error\n");
#endif

		}
		// insert p -> next -> n
		if( p == next ||
			next == n )
			Printf("Warning link error\n");
		noteInfo[p].setLink( next );
		noteInfo[next].setLink( n );
	} // else

}



void TNoteInfo::unlink( int id )
{
	int root, 
		p, 
		n;

	if( this[id].linkID() < 0 ) // not linked
		return;


	root = rootID( id );
	if( isRoot( id ) )
	{
		// search for pointer to root 
		p = root;
		while( this[p].linkID() != root )
		{
			p = this[p].linkID();
		}
		n = this[root].linkID() ;
		if( p == n ) // only two notes in chord
			n = -1;
		this[p].setLink( n );

	}
	else // search for pointer to id
	{
		p = root;
		while( this[p].linkID() != id )
		{
			p = this[p].linkID();
		}
		n = this[id].linkID();
		if( p == n ) // only two notes in chord
			n = -1;		// p == root -> unlink

		this[p].setLink( n );

	}
	this[id].setLink( -1 );
	

}


int TNoteInfo::linkID()
{
	return chordRoot;
}

/*
void TNoteInfo::setRoot( int i )
{
	chordRoot = i;
}
*/

int TNoteInfo::isRoot( int id )
{
	if( this[id].linkID() > id )
		return 1;
	else if( this[id].linkID() < 0 ) // no chord 
	{
		return -1;
#ifdef GLOBAL_SPLITV
		return -1;
#endif
#ifdef LOCAL_SPLITV
		return 1;
#endif
	}
	
	return 0;
}
int TNoteInfo::rootID( int i )
{
	TNoteInfo *noteList;
	noteList = this;
	if( i < 0 )
		return -1;

	if( noteList[i].linkID() < 0 ) // no chord
		return -1;

	while( noteList[i].linkID() < i )
	{
		i = noteList[i].linkID();
		if( i < 0 )
			Printf("error");
	}
#ifdef LOCAL_SPLITV_aaaa

	
	while( noteList[i].linkID() < i &&
		   noteList[i].linkID() > -1 )
	{
		i = noteList[i].linkID();
	}

#endif
	return i;
}

void TNoteInfo::setVoice(int voice)
{
	minVoice = voice;
	if( linkID() > -1 )
	{
		minVoice = voice;
			
	}
}


int TNoteInfo::voice( void )
{
	return minVoice;
}

int TNoteInfo::rootVoice( int pos )
{
	while( isRoot( pos ) == 0 ) // not inside chord
	{
		pos = this[pos].linkID();
	}
	return this[pos].voice();
		

}

/*
void TNoteInfo::incLink()
{
	if( chordRoot < 0 )
		chordRoot--; // #links = abs( chordRoot ) -1
	else
		Printf("Warning: chord link error!\n");
}

void TNoteInfo::decLink()
{
	if( chordRoot < -1 )
		chordRoot++;
	else
		Printf("Warning chord link error!\n");
}
*/


void TNoteInfo::copyBest()
{
	chordRoot = gMinRoot;
	minVoice = gMinVoice;
}

/*
	return pitch distance between this[prev] and this[next]
	if prev == chord return distance to closest note of chord

  !lookback can't be evaluated outside notelist in here!
*/
int TNoteInfo::pitchDistance(int prev,
							   int next,
							   int lookback)
{
		int note1pitch,
			note2pitch,
			pRoot,
			cRoot,
			j,
			pitch,
			cur;

		int res = 0,
			  temp;
#ifdef _DEBUG
		if( this[prev].linkID() > -1 &&
			rootID(prev) == rootID( next ) )
			Printf("Error: distance inside chord!\n");
#endif
		note1pitch = this->mPitch( next );

		note2pitch = this->closestPitch( prev,
										 note1pitch);

		j = prev-1;
		pRoot = this->rootID( prev );

		while( lookback && 
			   j > -1)
		{
			cRoot = this->rootID( j );
			// search for prev of prev 
			if( this[j].voice() == this[prev].voice() &&	// equal voice
				( this[j].linkID() < 0 ||						// single note
				  cRoot != pRoot )	)						// different chord
			{
				pRoot = cRoot;
				pitch = this->closestPitch( j,
											note1pitch );
				note2pitch = (double)note2pitch*0.8 + (double)pitch*0.2;
			}
			j--;
			lookback--;
		}// while



		res = abs(note1pitch - note2pitch);
		return res;


// old version searching for closest note inside chord
		note2pitch = this[next].attack->GetMIDIPitch();
		note1pitch = this[prev].attack->GetMIDIPitch();
		res = note1pitch - note2pitch;

		if( this[prev].linkID() > -1 ) // search in chord
		{
			cur = this[prev].linkID();
			while( cur != prev ) // one cycle
			{
				note1pitch = this[cur].attack->GetMIDIPitch();
				temp = note1pitch - note2pitch;
				if( abs(temp) < abs(res) )
					res = temp;
				cur = this[cur].linkID();

			} // while
		} // if 

		return res;
}

int TNoteInfo::notRoot( int id )
/*
	return 1 if not linked or not root
*/
{
	int tLink;
	tLink = this[id].linkID();
	if( tLink < 0 ||
		tLink < id )
		return 1;

	return 0; 

}
/*
int TNoteInfo::isTail(int id)
	return 1 if id == last note of chord
{
	int root;
	root = rootID( id );

	if( root < 0 )
		return 1;

	if( this[root].linkID() == id )
		return 1;

	return 0;

}
*/
void TNoteInfo::debug( int cAttacks,
				ostream &tOut )
{

	/*
	char mustClose = 0;
	if( !tOut )
	{
		tOut = fopen("_noteList.txt","wt");
		mustClose = 1;
	}
	*/
	int i;
	int v = 0, maxV=1;

	char *flags;
	flags = new char[cAttacks];
	for( i = 0; i < cAttacks; i++ )
		flags[i] = 0;
	while( v < maxV )
	{ // sort by voice 


		tOut << "\n " << v << ": ";
		for( i = 0; i < cAttacks; i++ )
		{
			char inChord = 0;
			int cur = i;
			if( this[i].voice() == v && 
				!flags[i] )
			{
				while( !flags[i] )
				{
					if( this[i].linkID() > -1 )
					{
						if( !inChord )
							tOut << "{";
						inChord = 1;
					}
					
					/*
					fprintf(tOut,"id=%d, voice=%d,", i, this[i].voice() );
					if( this[i].linkID() > -1 )
						fprintf(tOut, "linked to%d; ",
						this[i].linkID());

					fprintf(tOut, "\n");
					*/
					if( this[i].attack )
					{
						this[i].attack->WritePitch(tOut);
						tOut << ", ";
					}

//						this[i].attack->Debug( tOut );
					flags[i] = 1;
					if( this[i].linkID() > -1 )
					{
						i = this[i].linkID();
					}
				} // while
			} // if voice 
			else if( this[i].voice() >= maxV )
				maxV = this[i].voice() + 1;
			if( inChord )
			{
				tOut << "}";
				inChord = 0;
			}
			i = cur;
		}
		v++;
	} // while
	tOut << "\n";
	delete [] flags;
	/*
	if( mustClose )
		fclose(tOut);
		*/
	
}

/*
single note -> return MIDI Pitch
chord note -> return avPitch of chord
*/
int TNoteInfo::mPitch(int id)
{
	//!!! obsolete 
	return this[id].attack->GetMIDIPitch();
	
	if( this[id].linkID() < 0 ) // single note
		return this[id].attack->GetMIDIPitch();
	
	// get av pitch og chord
	int cur,
		count = 1;
	double av;
	cur = id;

	av = this[id].attack->GetMIDIPitch();
	cur = this[id].linkID();
	while( cur != id ) // one cycle
	{
		av += this[id].attack->GetMIDIPitch();
		cur = this[cur
			].linkID();
		count++;
	}

	av /= count;
	return av;
}


int TNoteInfo::closestPitch(int id,
							int pitch2)
{

	if( this[id].chordRoot < 0 ) // not linked
		return this[id].attack->GetMIDIPitch();

	// else search for closest pitch inside chord
	int delta,
		deltaMin,
		p,
		p2,
		nextID;

	p = this[id].attack->GetMIDIPitch();
	p2 = p;
	delta = abs( p - pitch2 );
	deltaMin = delta;

	nextID = this[id].chordRoot;
	while( this[nextID].chordRoot > -1 &&	// 1 cycle
		   this[nextID].chordRoot != id )
	{
		p = this[nextID].attack->GetMIDIPitch();
		delta = abs( p - pitch2 );
		if( delta < deltaMin )
		{
			deltaMin = delta;
			p2 = p;
		}
		nextID = this[nextID].chordRoot;
	} // while 
	return p2;

}


/*
	return id of chordID's toor note  for  noteId
	remarks
 res != noteId,
 root(res) != root(noteId)

 */
 int getCChord( TNoteInfo *noteList,
                int cAttacks,
                int noteId,
                int chordId )	// seeked chord range: 1..cAttacks
 {
     int res = -1;
     int i = 0,
         noteRoot;

     noteRoot = noteList->rootID( noteId );	// -1 or 0..cAttacks
     while( chordId &&
            i < cAttacks )
     {
         // look through noteList and count possible roots
         if( noteList->isRoot(i)   )
         {
             if( (noteRoot < 0 ||	// note not linked
                  noteRoot != i) &&	// note linked to different chord
                 noteId != i )		// can't be linked to itself
             {
                 res = i;
                 chordId--;
             }
         }
         i++;
     } // while
#ifdef _DEBUB
     if( res >= cAttacks ||
         res < 0 )
         printf("Error: Range error in getCChord!\n");
#endif
     	return res;
 }


 /*!
  return number of different possible chords for noteList[ID]
  notes must be sorted by ascending attacktimes!
  */
  int countPChords( TNoteInfo *noteList,
                   int cAttacks,
                   int id )
 {


	 TAbsTime prevAbsTime;

     int res = 0,
     i,
     idRoot;
     idRoot = noteList->rootID( id );
     // count nr off different root
     for( i = 0; i < cAttacks; i++)
     {
         if( noteList->isRoot(i)   )
         {
             if( (idRoot < 0 || // note not linked
                  idRoot != i) &&	// note linked to different chord
                 id != i )		// can't be linked to itself
                 res++;
         }
     }
     return res;
 }

