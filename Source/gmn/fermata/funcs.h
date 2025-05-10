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
|	filename  : FUNCS.H
|	Author    : Juergen Kilian
|	Date	    : 17.10.1996 - 98,2011
|  tools functions
------------------------------------------------------------------*/
#ifndef __funcs_h__
#define __funcs_h__
//---------------------------------------------

#include <string>
using namespace std;

//! version string
#ifdef _DEBUG
#define VERSION_STR "midi2gmn v2.1 (FERMATA) debug"
#else
#define VERSION_STR "midi2gmn v2.1 (FERMATA)"
#endif
/// builds:
/// 180: 2003/05/06 linux compile with makefile
/// 181: removed TNOTE > TNOTE bug
/// 182: optimisation and profiling
/// 183: remove gub in removeLegato (caused circular links
/// 184: remove negative duration causing bug in SetQData
/// 185: revised quantisation functions
/// 186: improved quantisation functions
/// 187: improved pattern quantisation, prototype for pattern extraction
/// 188: removed getFirstTempo bug
/// 189: updated qunatisation windows size, insert of 1/64 into grid for machine files
/// 190: removed bug in getBestNSecond
/// 191: removed voiceNames bug 2003/08/18
/// 192: removed durationIsIOI bug, added gapThreshold
/// 193: 25.9.03, improved voice separation, imrpoved fittoclick
/// 194: 10.11.03, _*0/0 bugfix, tempo detection, faster ReadTrack
/// 195: 25.11.03, removed wrong false positive machine file detection
/// 197: bug fixes and improvements for split voices
/// 198: revised tempo detection and quantisation
/// 199: DonByrd settings
/// 200: removed zero voice offset
/// 201: machine played detection by IOIDurRatio, improved TQTRACK::setQData duration List creation
/// 202: removed "hidden"  user input question bug, re changed machine duration to 0.8 because of 80% settings!
#define VERSION_INT 203



int validIOIratio( double ratio);

//#include "defs.h"
//#include "../lib_src/ini/ini.h"
//#include <stdio.h>
//#include "fragm.h"

class TAbsTime;
class TFrac;


string getVersion( void );
int getVersionInt( void );

#include "fragm.h"
/// convert tick timing into un-normalsied score timing
void convertDuration( 	/// duration in midiTicks (denom might be 1)
						TAbsTime  tickDuration,
						/// MIDI resolution
						int   ppq,
						/// score time
						int   *numerator,
						/// score time 
						int   *denominator );
//---------------------------------------------
/// convert score timing into tick timing
long frac2Ticks( long int scoreNumerator,
				 long int scoreDenominator,
				 int ppq );
//---------------------------------------------
/// convert delta tick timing into ms
double DTIMEtoMS( /// if ppq == 0 it is assumed that dtime is in score-time units!
				  double ppq,
				  double  tempo,
				/// MIDI delta time
			       double dtime );

//---------------------------------------------
/// convert score timing into ms
TFrac scoreTime2ms( int  tempo,
				    TFrac duration );
/// convert ms into score timing, not tested for overflows!
TFrac ms2ScoreTime( int  tempo,	
					TFrac ms );
//---------------------------------------------
/// convert ms into MIDI delta tick timing
double MStoDTime( /// if ppq == 0 it is assumed that result should be in score-time units!
				  double ppq,
				  double tempo,
				  double ms);
//---------------------------------------------
/// calculate bpm form delta time [ms] for a quater note
double quarterMSToBPM( long quarter_note_ms_length  );
/// display usage string
void usage( void );
//---------------------------------------------
void ErrorMsg( int Nr );
/// old interface function
void sal2svw ( char *inFilename );
/// convert midiPitch into guidoPitch, buffer should be > 5 char, res = 1 ok
int GetPitch( int midiPitch, char *guidoPitchBuffer );

//#define Printf printf

void Printf(const char *str);
void Printf(TFrac &frac);

/// return ptr to ".mid" postfix
const char *getMidPostfix( const char * str);
/// return ptr to ".gmn" postfix
const char * getGmnPostfix( const char *str);

class lgTag;
/// create all parameters of a  \note<> tag, used for input of other ASCCI formats
int readLNote( lgTag *noteTag,
				double &perftime,
				double &perfduration,
				double &susduration,
				/// scoretime int ticks with 96ppq
				long int &scoreTime,
				int &pitch,
				int &velolicty,
				int &voice,
				int &bar,
				int &beat);

/// write intensity tag to file
void writeIntens( ostream &gmnOut,
				  double intens );
#endif
