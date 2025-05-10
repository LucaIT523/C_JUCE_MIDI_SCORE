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

#if !defined( __Q_TRACK_H__ )
#define __Q_TRACK_H__
/*!
	TTRACK including quantisation data
	Juergen Kilian
*/

#include "track.h"
#include "q_note.h"
#include "tags.h"
#include "fragm.h"


class TVoiceCount;
class TVoiceInfo;
class TFracBinList;
//----------------------------------------------------
/// used at Track::Read
enum{ ONLY_TEMPO  = 1,
    NO_TEMPO =  2};

/// time for recognition of equal attackpoints [ms]
#define DEF_EQUAL_TIME "60"
/// time for recognition of legato overlapping[ms]
#define DEF_LEGATO_TIME "152"
/// time for recognition of staccato rests [ms]
//#define DEF_STACCATO_TIME "150"
/// time for recognition of grace notes and legato [ms]
#define DEF_ORNAMENT_TIME "130"


/// Miditrack including quantisation data
class TQTRACK : public TTRACK {
protected:
	virtual TQChord * CreateNewChord( lgChord *chord);
	virtual TNOTE * CreateNewNote( lgEvent *);
    //!!! result must be delted res[i] == index of ith voice
	int * sortVoices(void);
    /// ptr to a list of tags (slur, stacc, ...)
	TTagList *Tags;
    /*
    /// get som av vales for a voice, used for old voice spearation!
	TVoiceInfo *getVoiceFeatures( int *nrOfVoices );
     */
	int countVoices( void ); // real # of voices
public:
	/// append the lgNote, or chord, given by \note<> tag
	virtual TNOTE *append( lgTag *tag,
						int voice,
						int &maxVoice );
	/// append the lgNote, or chord, ignore rests
	virtual void append( lgEvent *event );

	/// do some pre processing before quantisation, might be re-defined in inherited classes
    virtual void preProcess( void );
	virtual TMusicalObject *FirstObject(int voice  /*! -1 = all voices */
										)
	{
		TMusicalObject *temp;
		temp = TTRACK::FirstObject(voice);
		if(temp)
		{
			CurrentNote = temp;
			return dynamic_cast<TQuantizedObject *>(temp);
		}
		return NULL;
	};

    /// note factory
	virtual TNOTE *CreateNewNote(
				TFrac abstime,
				TFrac duration,
				unsigned char pitch,
				unsigned char intens);

	TQTRACK( /// MIDI file offset
	        long offset, TMIDIFILE *parent_ ) : TTRACK( offset, parent_)
		{	
			CVoice = 0;		   // #voices = 0
			Tags = NULL;
		};
	virtual ~TQTRACK( void );
//	char QuantizeTempo(TAbsTime ppq);

/// attackpoint quantisation function for [from, ... , to)
void QuantizeAttackpoints(/// start 
						TQNOTE  *from,			
						TQNOTE  *to,	
						///  |perfAttack, quantisedAttack| for previous note		
						TFrac   diff,			
						/// list/grid of valid attackpoints
						TFracBinList *qAttacks ,
						 /// list of valid durations
						TFracBinList *qDurations,
						/// overall attackpoint acurracy
						double accuracy);
						
	/// duration quantisation in [from, ..., to)
	void QuantizeDurations( TQNOTE     *from,	
							TQNOTE     *to,		
							/// list/grid of valid durations
						TFracBinList *qDurations ,
						/// played durational accuracy
						double accuracy);
	/// convert to .gmn files
	virtual TAbsTime Convert( ostream &gmnOut,
						/// control track
					  TTRACK *Track0,			
					  TAbsTime from,
					  TAbsTime to,
					  TAbsTime offset);

	void removeLegatoOverlaps( int recTempo,
                              int recPPQ );

    void mergeEqualAttackpoints( int recTempo,
                                int recPPQ );
    /*
	long ChangeNote( int Voice,
						  long OldPos,
						  long NewPos,
						  long NewDuration,
						  char MoveNext,
						  int  Pitch );
     */
    //! # of voice in this track, not valid until SplitVoices has been called
	int GetCVoice( void )	
			{ return CVoice; };
    /// remove all set tempo events from control track
	void DelTempoNotes( void );
    /// reset quantisation values of all notes
	void ResetDiffs( void );
    /// re-sort the note list
	void CheckSort( void );
    /// validate quantisation data
	void SetQData( void );
    /// validate quantisation data for all voices
	void SetQData( int voice );
    /// merge two tag lists
	void MergeTags( TTagList *tags)
	{
		Tags = ::Merge(tags,Tags);
	}

    /// merge notelist and taglist with track2
	virtual void Merge(TTRACK *track2);
    /// renumber voiceIDs
    ///void mergeVoices(int offsetSize /* size of voiceOffset */ );


    /// do voice separation, main function
    void SplitVoices( int recTempo,
                  int recPPQ,
                  TMIDIFILE *theMidifile);
    /// voice separation sub-function, should only be called from inside SplitVoices
     void splitVoices(TNOTE *from, 
					TNOTE *to, 
					/// nr of start voice
					int offset_ID,     
					double &pChord,	// cost for chord detection
					double &pPitch,   // cost for pitch distance
					double &pGap,     // cost for gaps
					double &pOverlap,// cost for overlapping
					/// insiginicance threshold
					double gapThreshold,
					/// colorize voices?
					char colorizeVoices,
					TMIDIFILE *theMidifile);
     TFrac optimumOffset(TQNOTE *from, 
							TQNOTE *to, 
							int voice,
							TFrac barlength,
							TFrac resolution,
							double &oldQuality,
							double &newQuality );
}; // TQTRACK

#endif
