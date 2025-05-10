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
|	filename : EVENT.CPP
|	Author   : Juergen Kilian
|	Date	   : 17.10.1996-98/06-2001,2011
|	implementation of class TEVENT (MIDI-Events)
------------------------------------------------------------------*/
#include <sstream>
using namespace std;
#include <string.h>
#include "event.h"
#include "note.h"
#include "funcs.h"
#include "track.h"
#include "meta_tempo.h"

#undef UNUSED


//-------------------------------------------------------------------
TEVENT::TEVENT( void )
{
	// reset of all values
	datalength = 0;
	dtime 	   = 0;
	type 	   = 0;
	Meta  	   = 0;
	data = NULL;
} // TEVENT

//-------------------------------------------------------------------
TEVENT::~TEVENT( void )
{
	if( data )
		delete [] data;
	data = NULL;
}; // ~EVENT

//-------------------------------------------------------------------
/*!
result:
1 if EventType = SetTempo
0 else
*/
char TEVENT::tempo( void )
{
	if( Meta && (type == SET_TEMPO ) )
		return 1;
	return 0;
} // Tempo
//-------------------------------------------------------------------
/*!
result
tempo : if EventType = SET_TEMPO
-1 : else
remarks tempo is in bpm
*/
int TEVENT::GetTempo( void )
{
	long 	tempo_val;
	
	if( tempo() )
	{
		tempo_val = 0;
		tempo_val = data[0];
		tempo_val = tempo_val * 0x100 + data[1];
		tempo_val = tempo_val * 0x100 + data[2];
		// Tempo == Microseconds / quarter note
		tempo_val = 60000000 / tempo_val;
		// Tempo == Beats per Minute
		if( tempo_val == 0)
		{
			tempo_val = DEFAULT_TEMPO;
			Printf("No TempoSet == 0, Using DEFAULT_TEMPO\n");
		}
		return tempo_val;
	}
	else
		return -1; // this is no tempo event
} // GetTempo
//-------------------------------------------------------------------
/*!
result
1 : if EventType == NoteOff || (EventType = NotenOn & vel = 0)
0 : else
*/
char TEVENT::noteOff( void )
{
	char 	noteoff = 0;
	
	if( (type & 0xF0) == NOTE_OFF )			// Note Off
		noteoff = 1;
	if( ((type & 0xF0) == NOTE_ON ) && ( data[1] == 0 )) // vel == 0 -> Note Off
		noteoff = 1;
	return noteoff;
} // NoteOff
//-------------------------------------------------------------------
/*!
result
1 : if EventTpye == NoteOn
0 : else
*/
char TEVENT::noteOn( void )
{
	char noteon = 0;
	if( ((type & 0xF0) == NOTE_ON ) && ( data[1] > 0 )) // Note On
		noteon = 1;
	return noteon;
} // NoteOn
//-------------------------------------------------------------------
/*!
result :
prgChange : if EventType = PrgChange
-1 : else
*/
int TEVENT::Patch( void )
{
	if ( (type & 0xF0 ) == PROGRAM_CHANGE )
		return data[0];
	return -1;
} // Patch
//-------------------------------------------------------------------
/*!
result :
pitch : if EventType = NoteOn || NoteOff
-1 : else
*/
int  TEVENT::pitch( void )
{
	if( ((type & 0xF0) == NOTE_ON) ||
		((type & 0xF0) == NOTE_OFF))   // NoteOn oder NoteOff ?
		return data[0] & 0x7F;
	else
		return -1;
} // NoteNr
//-------------------------------------------------------------------
/*!
result :
midiChannel(1-16) of Event
0 : if EventType == Meta || SysEx
*/
char TEVENT::Channel( void )
{
	if( !Meta )
		return (type & 0xF) + 1;
	else
		return 0;
}
//-------------------------------------------------------------------
/*!
result :
1 : if EventType == EndOfTrack
0 : else
*/
char TEVENT::EndOfTrack( void )
{
	if( Meta )
		return (type == 0x2F );
	else
		return 0 ;
}
//-------------------------------------------------------------------
void TEVENT::SetDTime( long Dtime )
{
	dtime = Dtime;
}

//-------------------------------------------------------------------
/*
result :
1 : ok
0 : error  ==  eof
*/
char TEVENT::ReadDTime( FILE *infile)
{
	char 	temp;
	long 	TempTime = 0;
	
	dtime = 0;
	
	if( feof( infile ) )
		return 0;
	do
	{
		temp = fgetc( infile );
		dtime = dtime * 128;
		TempTime = (temp & 0x7F);
		dtime = dtime + TempTime;
	} while ( temp & 0x80			// read until bit 8 == 1
		&& !feof( infile) );
	return 1;
} // ReadDTime
//-------------------------------------------------------------------
/*!
result :
new running state if ok
0 : error

*/
char TEVENT::Read( char Status,		// current runningstate
				  FILE *infile,
				  lgSequence *curSequence)
{
	if(data) // delete already existing data
	{
		delete [] data;
		data = NULL;
	}
	
	if( infile )
	{
		ReadDTime( infile );
		if( feof( infile ))
			return 0;
		unsigned char temp = fgetc( infile );
		if( feof( infile ))
			return 0;
		if( temp == META  )	// Meta Event
		{
			Meta = 1;
			type = fgetc( infile );
			
			datalength = 0; 
			do
			{
				temp = fgetc(infile);
				datalength = datalength * 128;
				datalength = datalength + (temp & 0x7F);
			} while( temp & 0x80); // variable length!!!
			
			data = new unsigned char[datalength];
			if( !data )
				return 0;
			// read MetaData
			for(int i = 0; i < datalength; i++ )
				if( !feof( infile ) )
					data[i] = fgetc(infile);
		}
		else if( temp == SYSEX_START ) // Sysex
		{
			Meta = 1;
			type = temp;
			datalength = fgetc( infile );
			data = new unsigned char[datalength];
			if( !data )
				return 0;
			// Read SysExData
			// read MetaData
			for(int i = 0; i < datalength; i++ )
			{
				if( !feof( infile ) )
					data[i] = fgetc(infile);
			}
		}
		else				// Channel Voice Message
		{
			Meta = 0;
			if( temp & 0x80 )  // New runningstate
			{
				type = temp;
			}
			else
			{
				type = Status;
			}
			if( (type &0xF0) == PROGRAM_CHANGE)  // Prg Change
				datalength = 1;
			else if ((type & 0xF0) == CHANNEL_AT )    // Channel Pressure
				datalength = 1;
				else                            /* Note On
												Note Off
												Key Pressure
												Control Change
												Pitch Bend
												*/
												datalength = 2;
				
				data = new unsigned char[datalength];
				if( !data )
					return 0;
				if( temp & 0x80 )  // New runningstate
				{
					// Read data from file
					// read MetaData
					for(int i = 0; i < datalength; i++ )
					{
						if( !feof( infile ) )
							data[i] = fgetc(infile);
					} // for
				}
				else
				{
					data[0] = temp;
					if( datalength == 2 )
						data[1] = fgetc( infile );
				}
		} // else
	}
	else	// get next event in curSequence
	{
		
		// get delta time in MIDI ticks?
		lgTag *curTag = curSequence->nextTag(NULL);
		if( curTag )
		{
			if(!strcmp(curTag->name().c_str(), "\\noteOn") )
			{
				type = NOTE_ON;
			}
			else if(!strcmp(curTag->name().c_str(), "\\noteOff") ) 
			{
				type = NOTE_OFF;
			}			
			else if(!strcmp(curTag->name().c_str(), "\\tempo") ) 
			{
			}			
			else if(!strcmp(curTag->name().c_str(), "\\key") ) 
			{
			}			
			else if(!strcmp(curTag->name().c_str(), "\\meter") ) 
			{
			}			
		}
		else
		{
			Meta = 1;
			type = 0x2F; // end of track
		}
	}
	return type;
} // Read
//-------------------------------------------------------------------
// this is not write into a MIDI-File!! Only for debug on text out
/* write event to log/gln file
result = current tempo
*/
int TEVENT::write( FILE * out,        // outfile
				  long LastAbsTime,	// attackpoint of prev event
				  int ppq,				// resolution
				  char *currentChannel,
				  TTRACK *Track0,
				  int tempo_val,		// current tempo == prev|default,
				  int currentTrack,
				  int /* count  */)
{
	
	
	
	unsigned char realType;
	long int ms;
	TAbsTime AbsTime,		// attackpoint of current event
		splitTime,
		restTime;		// dtime between prev and current event
	//	int tempo = DEFAULT_TEMPO; // defaultTempo
	
	//	AbsTime = LastAbsTime + dtime;
	AbsTime = LastAbsTime + dtime;
	
	restTime = dtime;
	
	//	TNOTE *currentTempoNote;
	TMetaTempo *currentTempoNote;
	
	if( Track0 &&
		Track0->Current() ) // check for tempo changes
	{
		// for calculating dtime in ms -> calc CurrentTempo
		//		currentTempoNote = (TMetaTempo *)Track0->Current(TEMPO_CLASS);
		currentTempoNote = Track0->FirstTempo();
		TAbsTime attackTime;
		
		if( currentTempoNote )
			attackTime = currentTempoNote->GetAbsTime();
		else
			attackTime = 0L;
		
		restTime = AbsTime - LastAbsTime;
		splitTime = attackTime - LastAbsTime;
		
		// write all tempo changes between prev event and current event
		while( currentTempoNote &&
			currentTempoNote->GetAbsTime() >= LastAbsTime &&
			attackTime <= AbsTime )
		{
			restTime = AbsTime - attackTime;
			if( splitTime > 0L) // dtime to tempoChange?
			{
				if(tempo_val)
				{
					ms = DTIMEtoMS( ppq,
						tempo_val, // use prev tempo for [ms]
						splitTime.toLong() );
					if( out )
						fprintf( out, "empty*%ldms ", ms );
					if( ms < 0 && out)
						fprintf( out, "empty*%ldms ", ms );
					
				}
				else
				{
					if( out )
						fprintf( out, "\nempty*%ldticks (*SetTempo is missing*)\n ",
						splitTime.toLong() );
				}
			}
			
			tempo_val = abs(currentTempoNote->GetTempo());
			if( out )
				fprintf( out, " \\tempo<\"1/4=%d\",\"1/4=%d\">(*copy*) ",tempo_val,tempo_val);
			currentTempoNote = MetaTempo(currentTempoNote->GetNext(-1/* all voices */));
			if( currentTempoNote )
			{
				splitTime = currentTempoNote->GetAbsTime() -  attackTime;
				attackTime = currentTempoNote->GetAbsTime();
			}
		}
		
#ifdef UNUSED
		// write all tempo changes between prev event and current event
		while( currentTempoNote && !ende )
		{
			nextTempoNote = currentTempoNote->GetNext();
			
			// tempo change since last note?
			if( currentTempoNote->GetAbsTime() <= LastAbsTime )
			{
				// skip tempo change
				// currentChange <= LastAbsTime < ... < AbsTime < ...
				if(!nextTempoNote)
				{
					// no more tempo changes
					tempo = abs(currentTempoNote->GetTempo());
					if( !count ) //first event write, else skip
					{
						if( out )
							fprintf( out, " \\tempo<\"1/4=%d\",\"1/4=%d\">(*copy*) ",
							tempo,tempo);
					}
					ende = 1;
				}
				else if( nextTempoNote->GetAbsTime() >= AbsTime )
				{
					// no tempochange between prev note and next note
					// currentChange <= LastAbsTIme < ABsTime <= nextChange
					tempo = abs(currentTempoNote->GetTempo());
					if( !count ) //first event write, else skip
					{
						if( out )
							fprintf( out, " \\tempo<\"1/4=%d\",\"1/4=%d\">(*copy*) ",
							tempo,tempo);
					}
					ende = 1;
				}
				else // currentChange <= LastAbstime < nextChange < absTime
				{
					// TempoChange since last event -> write
					tempo = abs(currentTempoNote->GetTempo());
					splitTime = nextTempoNote->GetAbsTime() - LastAbsTime;
					restTime -= splitTime;
					LastAbsTime = nextTempoNote->GetAbsTime();
					if( splitTime ) // dtime to tempoChange?
					{
						if( tempo )
						{
							ms = DTIMEtoMS( ppq,
								tempo,
								splitTime );
							if( out )
							{
								fprintf( out, " \\tempo<\"1/4=%d\",\"1/4=%d\">(*copy*) ",
									tempo, tempo);
								fprintf( out, "empty*%ldms ", ms );
							}
							
						}
						else
							if( out )
								fprintf( out, "\nempty*%ldticks (*SetTempo is missing*)\n ", splitTime);
							// next tempo change
							currentTempoNote = Track0->NextNote();
					}
					//					currentTempoNote = Track0->NextNote();
				} // else
			}
			else // currentChange > LastAbsTime, go back
			{
				currentTempoNote = currentTempoNote->GetPrev();
			}
		}// while
#endif
	}
	if( Channel() && 
		Channel() != *currentChannel )
	{
		if( Channel() && out )
			fprintf( out, "\\staff<%d,%d> ",
			currentTrack+1,
			Channel());
		else if( out )
			fprintf( out, "\\staff<%d> ",
			currentTrack+1);
		*currentChannel = Channel();
	}
	if( dtime != 0)
	{
		if( tempo_val )
		{
			ms = DTIMEtoMS( ppq,
				tempo_val,
				restTime.toLong() );
			if( out )
				fprintf( out, "empty*%ldms ", ms);
		}
		else if( out )
		{
			fprintf( out, "\n\\emtpy*%ldticks (*SetTempo is missing *)\n", restTime.toLong());
		}
	}
	else // 0 dtime -> don't write
	{}
	
	char PitchBuffer[10];
	if( noteOn() )
	{
		GetPitch( data[0], PitchBuffer );
		if( out ) // noteOn
			fprintf( out, "\\noteOn<\"%s\",%d> ",
			PitchBuffer, // pitch
			data[1] );  // intens
		
	}
	else if( noteOff() )
	{
		GetPitch( data[0], PitchBuffer );
		if( (type & 0xF0) == NOTE_ON && out) // NoteOn (0)
			fprintf( out, "\\noteOff<\"%s\"> ",
			PitchBuffer); // pitch
		else if( out )
			fprintf( out, "\\noteOff<\"%s\",%d> ",
			PitchBuffer, // pitch
			data[1] );  // intens
	}
	else if( tempo() )
	{
		tempo_val = abs(GetTempo());
		if( out )
			fprintf( out, "\\tempo<\"1/4=%d\",\"1/4=%d\"> ", GetTempo(), GetTempo() );
	}
	else if( Meta )
	{
		switch( type )
		{
		case  END_OF_TRACK :
			if( out )
				fprintf( out, "]" );
			break;
		case SYSEX_START :
			{
				int i;
				if( out )
					fprintf( out, "\\sysEx<" );
				// hide f7 -> datalength-1
				for(i = 0; i < datalength-1; i++ )
				{
					if( out )
					{
						if( i )
							fprintf(out, ",");
						fprintf( out, "%x", data[i]);
					}
				}
				if( out )
					fprintf( out, "> (*end sysEx*) \n" );
			}
			break;
		case  SYSEX_END :
			{
				if( out )
					fprintf( out, "> (*end sysEx*) \n" );
			}
			break;
		case SEQUENCE_NR:
			if( out )
				fprintf( out, "(*Meta: SequenceNr: %d*) \n", data[0]);
			break;
		case TEXT_1:
			{
				if( out )
					fprintf( out, "\\text<\"%s\"> ", GetText().c_str() );
			}
			break;
		case TRACKNAME:
			{
				if( out )
					fprintf( out, "\\text<\"%s\">(*TrackName*) ", GetText().c_str()  );
			}
			break;
		case TRACKNAME_2:
			{
				if( out )
					fprintf( out, "\\text<\"%s\">(*TrackName2*) ", GetText().c_str()  );
			}
			break;
		case MARKER:
			{
				if( out )
					fprintf( out, "\\text<\"%s\">(*Marker*) ", GetText().c_str() );
			}
			break;
		case LYRICS:
			{
				if( out )
					fprintf( out, "\\text<\"%s\">(*Lyrics*) ", GetText().c_str()  );
			}
			break;
			
		case TIME_SIGNATURE:
			{
				int dem = 1, count, i;
				// calculate denominator
				count = data[1];
				for( i = 0; i < count; i++ )
					dem *=2;
				
				
				if( out )
					fprintf( out, "\\meter<\"%d/%d\"> \n",data[0],dem  );
			}
			break;
		case KEY_SIGNATURE:
			{
				int key;
				key = data[0];
				if( key > 128 )
					key = key-256;
				if( data[1] == 0 && out )
					fprintf( out, "\\key<%d (*major*)> ",key );
				else if( out )
					fprintf( out, "\\key<%d (*minor*)> ",key );
			}
			break;
		default:
			if( out )
			{
				
				fprintf( out, "\n(* --- Meta<%x>%s --- *) \n", type, GetText().c_str()  );
			}
		} // switch
	}
	else
	{
		realType = type & 0xf0;
		switch( realType )
		{
		case POLY_AT :
			if( out )
				fprintf( out, "\\polyAT<keyno=%d,vel=%d> ",
				data[0],
				data[1] );
			break;
		case CONTROL_CHANGE:
			if( out )
			{
				switch( data[0])	// controller Type
				{
				case 7 :
					fprintf( out, "\\GMvolume<%d> ",
						data[1]);
					break;
				default:
					fprintf( out, "\\controller<%d,%d> ",
						data[0],
						data[1]);
				}
			}
			break;
		case PROGRAM_CHANGE:
			if( out )
				fprintf( out, "\\prgChange<%d> ", data[0] );
			break;
		case CHANNEL_AT:
			if( out )
				fprintf( out, "\\keyAT<key=%d,%d> ", data[0],data[1] );
			break;
		case PITCH_BEND:
			if( out )
				fprintf( out, "\\pitchBend<%d,%d> ", data[0], data[1]);
			break;
		default:
			if( out )
				fprintf( out, "(* Undefined type <%x> *) \n",realType );
		}
	}
	if( out )
		fflush( out );
	return tempo_val;
}
//-------------------------------------------------------------------
int TEVENT::getData( int i)
{
	if( i < datalength )
		return data[i];
	return -1; // invalid
}

//-------------------------------------------------------------------
/// event data converted to ascii text
string TEVENT::GetText( void )
{
	ostringstream res;
	//	ret = new char[datalength+1];
    for( int i = 0; i < datalength; i++ )
        res << data[i];
	return res.str();
}
