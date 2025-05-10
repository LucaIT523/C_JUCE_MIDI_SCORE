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

/*
	file: import.cpp
	author: Jürgen Kilian
	date: 99/02/22
	function-stubs for midi2gmn interface
*/

#include <string>
#include "import.h"
#include "funcs.h"
#include "q_funcs.h"
#include <stdio.h>
#include "h_midi.h"

FILE *STDERR;
char *LOG_NAME;


int ImportMidi( char *filename,
					 char *iniFilename,
					 char *outFilename,
					 char *patternName,
					 char *maskName,

					 int playDuration, // 0%...100%...n%

					 int meterNum,
					 int meterDenom,
					 int clickNum,
					 int clickDenom,

					 char calcTempo, // 0/1 == on/off
					 int  clickTrackNr, //0 ==off, 1..255
					 int  clickTrackChannel, // 0 == off, 1..16
					 int  clickFilter //-1== off; 0..127==pitch;128..255==ctrl+128 
					 )
/*
	result:
		0 = ok
		1 = error
      2 = do it one more time
*/
{

	THMIDIFILE *Infile;	// Ptr to MIDI-file

	STDERR = fopen( "stderr.txt","wt" );

	FILE *TempOut;
#ifndef DEF_LOG_NAME
#define DEF_LOG_NAME "stderr.txt"
#endif 

	LOG_NAME = DEF_LOG_NAME;
	TempOut = fopen( LOG_NAME, "wt" );

	Infile = new THMIDIFILE(filename );
	if( !Infile )
	{
		ErrorMsg( 2 );
		return 1;
	}
	if( !Infile->Open(TempOut) )
	{
		ErrorMsg( 0 );
		return 1;
	}

	// SetParameters
	Infile->SetRelduration( playDuration );
	if( calcTempo )
	{
		Infile->SetCalcTempo( calcTempo );
	}
	else if ( clickTrackNr )
	{
		Infile->SetClickTrackNr( clickTrackNr );
		Infile->SetClickTrackChannel( clickTrackChannel );
		Infile->SetClickType( clickFilter );
	}

	TAbsTime ClickLength;
	ClickLength = Frac2Duration( clickNum, clickDenom, Infile->Ppq());

	if( !Infile->Read(  TempOut) )
	{
		ErrorMsg( 1 );
		return 1;
	}

//	Infile->PreProcess();

	Process( Infile,
				iniFilename,
				patternName,
				outFilename,
				maskName,
				meterNum,
				meterDenom );

	fclose( TempOut );
   fclose( STDERR );
	return 0;
}

char *YesNoQuestion( char *prompt,
							char *midiFilename )
/*
	-midiFIlename can be NULL
*/
{
	char answer[4];
	while(1)
	{
		printf( prompt);
		printf("[y/n]");
		scanf("%4s", answer);
		fflush(stdin);  /* flush the input stream in case of bad input */
		if(!strcmp(answer,"y"))
			return JK_YES;
		else if(!strcmp(answer,"n"))
			return JK_NO;
		if(!strcmp(answer,"Y"))
			return JK_YES;
		else if(!strcmp(answer,"N"))
			return JK_NO;

	}
	return JK_YES;
}

char *SelectQuestion( char *list,
							 char *midiFilename )
/*
	Listitems should be separated by /t
	-!!result must be deleted
	-midiFilename can be NULL
*/
{
	char *res;
	printf( list );
	res = new char[10];
	strcpy( res, "Answer" );
	return res;
}
char *InputQuestion(char *prompt,
					char *defaultStr,
							char *midiFilename)
/*
	!!result must be deleted
	midiFIlename can be NULL
*/
{
	char *res;
	printf( prompt );
	res = new char[10];
	strcpy( res, "Answer" );
	return res;
}

void ShowErrorMsg( char *str )
{
	fprintf(stderr,str);
}