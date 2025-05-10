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

#if !defined( __TRACK_H__ )
#define __TRACK_H__
#include <string>
using namespace std;

#include "portable.h"
#include "defs.h"
#include "note.h"
#include "funcs.h"
#include "midi.h"

#include "../leanguido/lgsequence.h"




#define DEFAULT_TEMPO 100


/// flags for read track
#define ONLY_CTRL 1
/// flags for read track
#define NO_CTRL   2


/// helper class during read track
typedef struct{
	int channelCount;
	char  unreadEvents[16];
} TTrackInfo;


/// class for handling voice names
/* replaced by string
class TVoiceName{
public:
	TVoiceName( const char *ptr = NULL );
	~TVoiceName( void );
	void setName( const char *ptr );

	char *name;
};
*/

/// helper class during track read
class TNoteOnList{
public : 
		long  Time;
		TNOTE *Ptr;
		TNOTE *overhead; // used for doubled note note
};


/// generic (MIDI) Track 
class TTRACK {
protected:
	/// ptr to last used tempo control
	TMetaTempo * curTempo;
	//! number of voices
    int    CVoice; 
	//! error flag during read
    char       OK;	      	 
	int    ChannelI;
	//! Offset of trackstart in file
	long   Offset;	       
	TMIDIFILE *parent;
	TMusicalObject  *CurrentNote;
    //! ptr to list
	TMusicalObject  *Notes;	       
	TTRACK 			 *nextI;
	int    			 TrackNumberI;
	void shiftVoices(int offset);
public:
	virtual TNOTE  *append( lgTag *tag,
							int voice,
							int &maxVoice );
	/// recalc durations according to sustainOffset, should be called directly after removeLegato
	void resolveSustain( void );
	/// append the lgNote, or chord, ignore rests
	virtual void append( lgEvent *event );
	int cEvents( int voice );
	/// calculate playing accuracy
	int accuracy(double &attack, double &duration);
	/// copy global events to all voices
	void copyUnvoicedElements( void );
	string voiceName( int i );
	/// an array of voice names
	string * voiceNames;
	void setTrackName( const char *ptr);
	string trackName;
	void applyRelDuration( int relduration );
	unsigned long addNoteOn( TNoteOnList *NoteOnOff, 
					unsigned long absTime, 
					int pitch, int vel, int chan, 
					FILE *log);
	unsigned long addNoteOff( TNoteOnList *NoteOnOff, 
							unsigned long absTime, 
							int pitch, int vel, 
							int chan, 
							int relDuration,
							FILE *log);
	unsigned long addTimeSig( TNoteOnList *NoteOnOff, unsigned long absTime, int num, int denom, FILE *log);
	unsigned long addKeySig( TNoteOnList *NoteOnOff, unsigned long absTime, int ton, int minorMajor, FILE *log);
	unsigned long addSetTempo( TNoteOnList *NoteOnOff, unsigned long absTime, double bpm, FILE *log);
	unsigned long addText( TNoteOnList *NoteOnOff, unsigned long absTime, char *str, int chan, FILE *log);
	/// get a note with abstime <= evTime < offset time  
	TNOTE * noteAt( TAbsTime evTime );
	void checkChromaticScales( TMIDIFILE *midifile);
	virtual TAbsTime Convert( ostream &gmnOut,
					  TTRACK *Track0,
					  TAbsTime from,
					  TAbsTime to,
					  TAbsTime glOffset);
	/// return first key event
	TMetaKey *FirstKey( void );
	/// return first tempo event
	TMetaTempo *FirstTempo(void);
	/// return first meter sig event
	TMetaMeter *FirstMeter( void );

	virtual void Debug( FILE *out = NULL);
	int TrackNumber( void )
			{ return TrackNumberI; };
	void SetTrackNumber( int i );
	virtual void Merge(TTRACK *track2);
	TTRACK *Next( void )
			{ return nextI; };
	void SetNext( TTRACK *next )
			{ nextI = next; };
	void Insert( TTRACK *next );
	void append( TTRACK *next );

	/// MIDI Channel of events
	int Channel( void )	
		{ return ChannelI; };

	virtual TNOTE *CreateNewNote(
		TFrac abstime,
		TFrac duration,
				unsigned char note,
				unsigned char intens);
	TTRACK( long offset, TMIDIFILE *parent_ );
	virtual ~TTRACK( void );
	long Read_Header( FILE *file );
	/// read an event track of a MIDI file, or convert a low-level GUIDO sequence 
	TTrackInfo Read( FILE *file,
						  int  relduration,
						  int tempoFlag ,
						  FILE *log,
						  int ppq,
						  TTRACK *Track0,
						  int currentTrack,
						  int channel /*! filter: 0 == all,1-16 */,
						  TMIDIFILE *parent,
						  lgSequence *curSequence = NULL); 	
	void Insert( TMusicalObject *ptr );

	long GetOffset( void )
		{ return Offset; };
	void SetOffset( long offset )
		{ Offset = offset; };
	virtual TMusicalObject *FirstObject( int voice );

	/// return first note in [minAttacktimeMS, maxAttacktimeMS)
	virtual TNOTE *FirstNoteObject( int voice,
									double minAttacktimeMS = 0,
									double maxAttacktimeMS = 0);

	TMusicalObject *NextNote( int voice )
		{
			if( CurrentNote )
				CurrentNote = CurrentNote->TMusicalObject::GetNext( voice );
			return CurrentNote;
		};
	TMusicalObject *Current( void )
		{return CurrentNote;};
	void deleteNotes( TMusicalObject *from,
							TMusicalObject *to );
	void deleteSameType(TMusicalObject *from,
								TMusicalObject *to );
	void shiftAttacks( TFrac offset );

	int GetTempo( TAbsTime absTime );
	TMusicalObject *DetachedVoice(TMusicalObject *from,
											TMusicalObject *to);

	void pitchSpelling(TMIDIFILE *midifile );
	int cVoice( void ) { return CVoice;};

	void setCVoice( int c )
	{ CVoice = c; };


	TMIDIFILE *Parent( void );

	/// fill with sequence content
	int fill( lgSequence *sequence,
				TTRACK *ControlTrack,
				int &recPPQ,
				int &recTempo);
}; // TTRACK

void 	markChromaticTrill(TNOTE *sScaleStart,
						   TNOTE *sScaleEnd,
							int currentKey);
void	markCScale(TNOTE *sScaleStart,
				TNOTE *sScaleEnd,
				int currentKey,
				int prevDeltaPitch);


int isInScale( int key,
			   int pitch, 
			   int acc);

void enharmonicTransp( int *pitch,
					   int *acc,
					   int *octave,
					   int deltaPitch );



/*!
convert scale into string
	!! result must be deleted
 */
char *scale2Str( keyscale *scale );

//! parse buffer and copy values into scale
void setScale( char *buffer, keyscale *scale ); 
void readScales( TInifile *inifile );


int keyAccidental(int normKey, int midipitch);
int keyPitch( int normKey, int midipitch );

#endif
