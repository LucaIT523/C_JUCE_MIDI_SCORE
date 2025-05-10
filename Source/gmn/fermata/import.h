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

#if !defined( __import_h__ )
#define __import_h__


#include <string>
using namespace std;
//----- exit codes -----------------
#define EXIT_OK 0
#define EXIT_FILE_NOT_FOUND 1
#define EXIT_WRONG_PARAMETERS 2
#define EXIT_UNKNOWN_OPTION 3
#define EXIT_WRONG_CALLING 4
#define EXIT_CANT_OPEN_FILE 5
#define EXIT_CANT_READ_FILE 6
#define EXIT_CANT_CONVERT 7
//----------------------------------


#define JK_YES "yes"
#define JK_NO "no"


#define SILENT 0
#define VERBOSE 1

int ImportMidi( char *filename,
					 char *outFilename,
					 char *iniFilename,
					 char *patternName,
					 char *maskName,

					 int playDuration, // 0%...100%...n%

					 int meterNum,
					 int meterDenom,
					 int clickNum,
                int clickDenom,

					 char calcTempo, // 0/1 == on/off
					 int  clickTrackNr, //0 ==off, 1..255
					 int  clickTrackChannel // 0 == off, 1..16
					 );


const char *YesNoQuestion( const char *prompt, 
					 const char *defualt,
					 const char *midiFilename );

const char *SelectQuestion( const char *list, 
					 const char *defualt,
					  const char *midiFilename  );
// result must be deleted
string InputQuestion(const char *prompt,
					 const char *def	,
					const char *midiFilename );
void ShowErrorMsg( const char *str );





#endif
