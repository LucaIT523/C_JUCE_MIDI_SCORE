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
|	filename : event.h
|	Autor     : Juergen Kilian
|	Date	    : 17.10.1996-98/06
|   			2003/05/21 replaced char *GetText bei string GetText
 | classdeclaration for TEVENT
------------------------------------------------------------------*/
#ifndef __event_h__
#define __event_h__
//-----------------------------------------------------------------
#include <string>
using namespace std; 

#include <stdlib.h>
#include "portable.h"
#include "defs.h"
#include <stdio.h>

#include "../leanguido/lgsequence.h"

#define DEFAULT_TEMPO 100



//--------Channel Voice Mesagge---------------------------------------------------------


#define END_OF_TRACK 0x2f
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define POLY_AT 0xa0
#define CONTROL_CHANGE 0xb0
#define PROGRAM_CHANGE 0xc0
#define CHANNEL_AT 0xd0
#define PITCH_BEND 0xe0
#define SYSEX_START 0xf0
#define SYSEX_END  0xf7
#define META 0xFF
//-------------Controller----------------------------------------------------
#define MOD_WHEEL 0x01
#define DATA_ENTRY 0x06
#define VOLUME 0x07
#define PAN 0x0a
#define SUSTAIN 0x40
//--------------Meta Events---------------------------------------------------
#define SEQUENCE_NR 0x00
#define TEXT_1 0x01
#define TRACKNAME 0x03
#define TRACKNAME_2 0x04
#define LYRICS 0x05
#define MARKER 0x06
#define SET_TEMPO 0x51
#define TIME_SIGNATURE 0x58
#define KEY_SIGNATURE 0x59
//-----------------------------------------------------------------
class TTRACK;

/// MIDI event class
class TEVENT {	
private :
    /// deltatime of event
    long dtime;
    /// flag for Metaevents
    unsigned char Meta;
    /// eventtype
    unsigned char type;
    /// number of databytes
    int datalength;
     /// event data
    unsigned char *data;
    /// read delta time
    char ReadDTime(  FILE *infile);

public :
	TEVENT( void );
	virtual ~TEVENT( void );
    /// set delta time
    void SetDTime( long dtime );
	char Channel( void );
    /// return 1 if end of track event
    char EndOfTrack( void );
	int  pitch( void );
	int  Patch( void );
	char noteOff( void );
	char noteOn( void );
    /// return 1 if tempo event
    char tempo( void );
	int  GetTempo( void );
    /// -1 == error, 0..127 == data[i]
    int getData( int i);
    /// read event from file
    char Read(  char Status /* cur running status */,
			 FILE *outfile,
			 lgSequence *curSequence);
    /// Eventtyp == RunningStatus
    char GetStatus( void )		
		{ return type; };
	int Type( void )
		{ return type; };
    /// Deltatime
    long DTime( void )		
		{ return dtime;};
    /// get intensity for noteOn, noteOff
    unsigned char Intens( void )		
		{ return data[1]; };
    /// debug output
    int write( FILE *out,
					long AbsTime,
					int ppq,
					char *currentChannel,
					TTRACK *Track0,
               int tempo,
					int currentTrack,
					int count);
    /// get event data as text
    string GetText( void );
}; // TEVENT
//-----------------------------------------------------------------
//-----------------------------------------------------------------
#endif
