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
|	filename : ornament.H
|	Autor     : Juergen Kilian
|	Datum	    : 17.10.1996-2003
|	Aufgabe   :  deklaration of TOrnament
------------------------------------------------------------------*/
#ifndef __ornament_h__
#define __ornament_h__
//-----------------------------------------------------------------
#include <sstream>
using namespace std;
#include <stdlib.h>
#include "portable.h"
#include "q_note.h"
#include <stdio.h>
#include "fragm.h"




/// class for ornamental notes, base note + separate of "played" notes
class TOrnament : public TQNOTE
{
protected:
	/// list of hidden ornamental notes
	TQNOTE *asPlayed;
	/// glissando, grace, trill, turn, ....
	const char   *typeI;
public:
	TQNOTE *playedNotes( void )
	{ return asPlayed; };
	virtual void pitchSpelling( TMIDIFILE *midifile);
	/// constructor, [asPlayedFrom...asPlayedTo) will be removed from track!
	TOrnament( TQNOTE *baseNote,
				  TQNOTE *asPlayedFrom,
				  TMusicalObject *asPlayedTo,
				  TTRACK *track,
				  const char *type);
	virtual ~TOrnament(void);
	virtual void Debug( ostream &out);
	virtual TFrac Convert( ostream &gmnOut,
				TAbsTime preAbstime,
				TFrac   prev,
				TTRACK *Track0);

	virtual void ticksToNoteface( int ppq );
};

#endif
