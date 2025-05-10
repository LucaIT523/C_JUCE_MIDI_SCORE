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
|	filename : note_info.h
|	Autor     : Juergen Kilian
|	Datum	    : 17.10.1996-2003
|	Aufgabe   : deklaration of TNoteInfo
------------------------------------------------------------------*/
#ifndef __noteinfo_h__
#define __noteinfo_h__
//-----------------------------------------------------------------
#ifndef LOCAL_SPLITV 
#ifndef GLOBAL_SPLITV
#define LOCAL_SPLITV
#endif
#endif

#include <sstream>
using namespace std;
#include "q_note.h"

#include <stdlib.h>
#include <stdio.h>
#include "fragm.h"

/// tool class for information storing during voice separation
class TNoteInfo {
#ifdef GLOBAL_SPLITV
		/// index of root note, -1 == none
		int chordRoot;	
		/// index of closest voice  -1 == none
		int minVoice;	
		/// set root
		void setLink( int i); 
#endif
/// set root
void setLink( int i); 
	

public :
#ifdef _DEBUG
	double pitchDist,
		   gapDist,
		   ovlDist,
		   chordDist;
#endif

	int closestPitch( int id,
					  int pitch2);
#ifdef LOCAL_SPLITV
		/// index of linked note, 
		int chordRoot;	
		/// index of closest voice  -1 == none
		int minVoice;	
		void link( int prev, int next );
	void unlink( int id );
#endif

#ifdef GLOBAL_SPLITV
	void link( int prev, int next );
	void unlink( int id );
#endif

	int notRoot( int id );
		
	int mPitch( int id );
	void debug(int cAttacks,
				ostream &out);
//	int isTail( int id );
	int pitchDistance( int prev, 
						 int next,
						 int lookback );
	void copyBest( void );

	TNoteInfo( void ){chordRoot = -1; 
						minVoice = -1; 
						attack = NULL; 
						minDiff = 0;
						lOverlap = 0;
						lGap = 0;
						lPitch = 0;
						lChord = 0; 
#ifdef _DEBUG
	pitchDist = 1;
		   gapDist = 1;
		   ovlDist = 1;
		   chordDist = 1;
#endif
	
	};
	int rootID( int id ); // call from start->()
	int isRoot( int id ); // call from start->()

	/// local search distance for this note
	double Distance;		
	void setVoice( int voice );
	int gMinRoot;
	int gMinVoice;
	int linkID( void );	// id of neighbour in chord 
	int voice( void );	// return minVoice 
	int rootVoice( int i ); // voice of root
//	int rVoice( void );	// root Voice
	/// ptr to note
	TQNOTE *attack;	
	/// distance to closest voice
	double minDiff;	
	/// can be used for different purposes
	int flag;		

	double lPitch,
		  lGap,
		  lChord,
		  lOverlap;

};

    /// return number of different possible chords for noteList[ID]
    int countPChords( TNoteInfo *noteList,
                      int cAttacks,
                      int id );

    /*!
     return id of chordID's toor note  for  noteId
     remarks
     res != noteId,
     root(res) != root(noteId)
     */
    int getCChord( TNoteInfo *noteList,
                   int cAttacks,
                   int noteId,
                   int chordId );	// seeked chord range: 1..cAttacks


    void makeChords(TNoteInfo *noteList,
                    int cAttacks,
                    int CVoice,
                    TTRACK *track);

#endif
