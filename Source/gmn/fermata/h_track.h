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

#if !defined( __H_TRACK_H__)
#define __H_TRACK_H__


#include "q_track.h"
#include "q_note.h"

//prefix used at ini-file
#define ORNAMENT_PREFIX "ORNAMENT"

#ifdef IMUTUS_DLL
#include "fermata_dll/midishare.h"
#include "C:\users\saldvl\MidiShare 1.86\MidiFile\MidiFile.h"
#endif
/// advanced TQTRACK
class THTRACK : public TQTRACK
{
public:
	int deleteClicks( int channel, int clickType );
	/// create a attack, duration, ioi ms table for each note
	void toGLN( const char *filename,
				int ppq,
		        TTRACK *ctrlTrack);
#ifdef IMUTUS_DLL
	THTRACK( MidiSeqPtr );
#endif

	virtual void preQuantize( TTRACK *Track0, 
							  int ppq,
							  int ornament_duration);
/*
    virtual TNOTE *CreateNewNote(
		TFrac abstime,
		 TFrac duration,
				unsigned char note,
				unsigned char intens);
*/
	THTRACK( long offset, TMIDIFILE *parent_ ) : TQTRACK( offset, parent_)
    {};
	virtual ~THTRACK( void )
    {};
	void markOrnament( TTRACK *Track0, 
						int ppq,
						double ornament_duration);
	void deleteInVoice( TQNOTE *from, TQNOTE *to );
};

double CompareKNN( int *list1,
						 int *list2,
						 double *weights,
						 int items );


void ReadFeatures( int *features, int i, TInifile *inifile);
char *GetNextVal(char *valStr, char **tail);
int parseVal( char *valStr);

/// do a kNN classification of the ornament type between (Start
int retrieveOrnamentType( TQNOTE *Start,
                       TQNOTE *Succ,
                       ostream &log,
                       TInifile *inifile);


#endif
