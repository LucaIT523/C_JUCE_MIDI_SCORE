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
|	filename : SMF2SAL.CPP
|	author     : Juergen Kilian
|	date	    : 17.10.1996-98,2011
|	global functions
------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "portable.h"

#include <ctype.h>

#include "midi.h"
#include "track.h"
#include "note.h"
#include "funcs.h"
#include "import.h"

//---------------------------------
// Flag for compiler
// must be declared if scale is used
//#define USE_NOTENWERTE

const char *chromaticScale[12] = { "c", "c#", "d", "e&", "e", "f", "f#",	"g", "g#","a","b&","b"};
//-------------------------------------------------------------------

//----------------------------------
// TInifile *Inifile = NULL;
// extern FILE *LOG_FILE;


#if defined (Windows)
	#include <owl\owlpch.h> 
	#include <owl\except.h>
#endif

// error messages --------------------------------------
#define MAXERROR 31

const char *ErrorStr[MAXERROR] = {
/*0*/	"Can't open input file!\n",
/*1*/	"Can't read input file!\n",
/*2*/	"Not enough memory!\n",
/*3*/	"Invalid filename!\n",
/*4*/	"Error at conversion\n",
/*5*/	"Illegal duration\n",
/*6*/	"Filename is missing\n",
/*7*/	"\n",
/*8*/	"Syntax error in pattern file!\n",
/*9*/	"Not enough memory for pattern!\n",
/*10*/  "Can't open pattern file\n",
/*11*/  "Range error in quantize!\n",
/*12*/  "Can't open mask file!\n",
/*13*/	"Syntax error in mask file!\n",
/*14*/  "Can't write gmn-file!\n",
/*15*/  "\nerror at parameter M !\n",
/*16*/  "\nerror at parameter C !\n",
/*17*/	"\np M is missing !\n",
/*18*/	"\nclicktrack doesn't exist !\n",
/*19*/	"\ncan't open file \n",
/*20*/	"\nnot trackstart !\n",
/*21*/	"\nvoice doesn't exist !\n",
/*22*/	"\nnot doesn't exist !\n",
/*23*/	"\nerror: overlapping notes !\n",
/*24*/	"\nwrong sorting!\n",
/*25*/	"\ndenominator == 0!\n",
/*26*/	"\nwrong barlen!\n",
/*27*/	"Error in clicktrack!\n",
/*28*/	"\ncan't solve overlapping!\nplease prove settings!\n",
/*29*/	"\naccenting was played wrong!\n"
/*30*/	"Illegal range for d!\n"
	};


//functions --------------------------------------------


void writeIntens( ostream &file,
				  double intens )
{
	if( intens <= 0 ||
		!file )
		return;

	file << "\\i<\"";
	if( intens < 0.16 )
	{
		file << "ppp";
	}
	else if( intens < 0.31 )
	{
		file << "pp";
	}
	else if( intens < 0.47 )
	{
		file << "p";
	}
	else if( intens < 0.63)
	{
		file << "mp";
	}
	else if( intens < 0.78)
	{
		file << "mf";
	}
	else if( intens < 0.91)
	{
		file << "f";
	}
	else if( intens < 1 )
	{
		file << "ff";
	}
	else 
	{
		file << "fff";
	}
	file << "\","<< intens <<  "> ";
}


/// return the last "." position of a string
const char * lastDot(const char *str )
{
	const char *temp = strstr(str, "." );
	while( temp &&
		   strstr( temp+1, "." ) )
	{
		temp = strstr(temp+1,".");
	}
	return temp;
}
 
const char *getPostfix( const char * str,
					    const char *pUpr,
						const char *pLwr)
{
	const char *temp = lastDot( str );
	if( temp )
	{
		// temp[0] == "."
		temp++;
		int i;
		for( i = 0; i < 3; i++ )
		{
			if( temp[i] == 0 ||
				( temp[i] != pUpr[i] &&
				  temp[i] != pLwr[i])
				)
			{
				temp = NULL;
				i = 4; // abort
			}
		} // for
	}
	return temp;
}

/// return ptr to ".mid" postfix
const char *getMidPostfix( const char * str)
{
	return getPostfix( str, "MID","mid");
}

/// return ptr to ".gmn" postfix
const char * getGmnPostfix( const char *str)
{
	return getPostfix( str, "GMN","gmn");
}


/*
	DTIMEToBPM
	Parameter :
		difftime : time[ms] for a quarter beat
	result:
		tempo for difftime = 1/4

*/
double quarterMSToBPM( long quarterMS )
{
	double res;

	if( quarterMS != 0 )
		res = 60000.0 / (double)quarterMS;
	else
		res = 0;
	return res;
}


//-------------------------------------------------------------------
/*!
	converts musical scoretimes into ms, where tempo is used
	result = ms
*/
TFrac scoreTime2ms(	int  tempo,
		            TFrac dtime )
{
	TFrac ms;
	if(tempo )
	{
		ms = 60000L / tempo;
        ms *= dtime;
	}
	else
	{
		Printf( "\nError: Division by tempo == 0!\n");
		ms = 0L;
	}
	if( dtime > 0L && ms < 0L )
		Printf("ERROR: Overflow in scoreTime2ms!\n");
	return ms;
} // scoreTime2ms
//-----------------------------------------------------------
TFrac ms2ScoreTime( int  tempo,	
					TFrac ms )
/*
	converts musical scoretimes into ms, where tempo is used
	result = ms
*/
{
	/*
	ms = dtime * (60000L / tempo)
	dtime = ms * tempo / 6000L
	*/
	TFrac dtime;
	dtime = ms * tempo / 60000L;
	return ms;
} // ms2ScoreTime
//-----------------------------------------------------------
/*
	DTIMEtoMS
	converts Dtimes from [ticks] to [ms]
	result = ms
*/
double DTIMEtoMS( double ppq,		// length of quarter note
				  double  tempo,		// current temp in bpm
				  double dtime )	// tick timing information
{
	if( ppq <= 0 )
	{
		// input filre was a GUIDO file -> ticktiming = scoretiming
//		Printf("Error: DTIMEToMS:: ppq == 0\n");
		ppq = 0.25;
	}
	double ms = ( dtime * 60000 )  /  ppq; // ms = [Quarter]
	if( tempo > 0)
	{
		ms = ms / tempo;		// ms = [millisekunden]
	}
	else
	{
		Printf( "\nError: Division by tempo == 0!\n");
		ms = 0;
	}
	return ms;
} // DTIMEtoMS
//-------------------------------------------------------------------
/*
	MStoDTime
	converts time[ms] into MIDI-Ticks
*/
double MStoDTime( double ppq,
				  double tempo,
				double ms )
{
	if( ppq <= 0 )
	{
		// no ppq -> scoretime duration output is expected
		ppq = 0.25;
	}

	double dtime = ms * ppq * tempo; // ms * Parts / min
	dtime = dtime / 60000;
	return dtime;	
} // MStoDTime
//-------------------------------------------------------------------
/*
	converts MIDI-Pitch (0.127) into GUIDO-Pitch-str.
	result 1: ok
			0:error
	the pitch will be copied into str
*/
int GetPitch( int pitch, char *str )
{
	int key    = ((pitch & 0x7F) % 12 );
	int octave = ((pitch & 0x7F) / 12 );
		  octave = octave - 4;
		  strcpy( str, chromaticScale[key] );
		  sprintf( str + strlen(str), "%d", octave );
	return 1;
}
//-------------------------------------------------------------------
/*
	converts fraction-durationn into MIDI-Tick-duration
*/
long frac2Ticks( long int numerator,
						long int denominator,
						int ppq )		// Aufl”sung
{
	if( denominator == 0 )
		return 0;
	double res = ( 4.0 * (double)ppq ) / (double)denominator;
	res *= numerator;
	return (long)(res + 0.5);
} // Frac2Duration
//-------------------------------------------------------------------
/*
	converts MIDI-tick-duration into fraction
*/
void convertDuration( TAbsTime lgDuration,
				  int  ppq,
				  long int  *numerator,
				  long int  *denominator )
{
	TFrac PPQ(1,ppq*4);
	PPQ *= lgDuration;
	*numerator = PPQ.numerator();
	*denominator = PPQ.denominator();
} // ConvertDuration
//-------------------------------------------------------------------
void ErrorMsg( int Nr )
{
	if( Nr < MAXERROR )
		ShowErrorMsg( ErrorStr[Nr]);	// in fmbrose.cpp
	else
	   	ShowErrorMsg( "Undefined Error\n");
} // ErrorMsg
//-------------------------------------------------------------------
//-------------------------------------------------------------------------
//-----------------------------------------------------------------


int getVersionInt( void )
{
    return VERSION_INT;
}
string getVersion( void )
{
    ostringstream res;
    res << VERSION_STR;
    res << "(" << getVersionInt() << ")";
    return res.str();
};


#if defined( Windows ) || defined( _WIN32V_ ) 
//|| defined( _MACV_ )
// don't do screen output!!
#undef printf
void Printf(const char *s1 )
{
	fprintf( STDERR, s1 );
	fflush( STDERR );
}

void Printf(const char *s1, const char *s2 )
{
	fprintf( STDERR, s1, s2 );
	fflush( STDERR );
}
void Printf(const char *s1, unsigned const char *s2)
{
	fprintf( STDERR, s1, s2 );
	fflush( STDERR );
}


void Printf(const char *s1, int s2)
{
	fprintf( STDERR, s1, s2 );
	fflush( STDERR );
}
void Printf(const char *s1, double s2)
{
    fprintf( STDERR, s1, s2 );
    fflush( STDERR );
}

void Printf(const char *s1, int s2, int s3)
{
	fprintf( STDERR, s1, s2,s3 );
	fflush( STDERR );
}
void Printf(const char *s1, long s2, double s3)
{
	fprintf( STDERR, s1, s2,s3 );
	fflush( STDERR );
}

void Printf(const char *s1, long s2, long s3)
{
	fprintf( STDERR, s1, s2,s3 );
	fflush( STDERR );
}
void Printf(const char *s1, long s2, long s3, long s4)
{
	fprintf( STDERR, s1, s2,s3,s4 );
	fflush( STDERR );
}
void Printf(const char *s1, long s2)
{
	fprintf( STDERR, s1, s2 );
	fflush( STDERR );
}
void Printf(TFrac f)
{
	fprintf( STDERR, "%ld/%ld",
							f.numerator(),
							f.denominator() );
	fflush( STDERR );
};

#else // commandline output
void Printf(const char  *s1 )
{
	if( s1 != NULL )
	{
		std::cout << s1;
		std::cout.flush();
	}
}
void Printf(TFrac &frac )
{

		std::cout << frac.numI << "/" << frac.denomI << endl;
		std::cout.flush();

}

#endif
