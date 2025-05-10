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

#if !defined( __VOICEINFO_H__ )
#define __VOICEINFO_H__


#include "defs.h"
#include "q_note.h"

class TNOTE;
class TQNOTE;

///  for voice storing information about opened voices during voice separation
class TVoiceInfo{
protected:
		void init( void );
public:

    int lastNoteID;
		TVoiceInfo( void );
		void addToVoice( TNOTE *now,
						 double pitchDecay ///  decay for new pitch [0..1000]
                   );
        void addToVoice( TVoiceInfo *voiceInfo);
                   

        int ///  voice nr before merging
            oldVoice;		
		int ///  voice nr after merging
            newVoice;		
//		int ///  noteId links to this?
  //          lockedBy;	    
		double ///  pitch of last note in voice, !average
            lastPitch;	
		double firstPitch;
		TAbsTime
             ///  noteOff of last note in voice
            noteOff;  
		TAbsTime
            ///  attack of first event
            start;		
		double ///  av Duration of notes
            meanDuration; 
		TAbsTime  ///  sum of note durations
            hold;		
		TAbsTime ///  sum of rests between notes
            rest;		
		double ///  highest pitch
            maxPitch;		
		double ///  lowest pitch
            minPitch;		
		double ///  # of notes
            cNotes;		
		int   ///    increase for each time voice wasn't used
            unused;		
		int ///  #of closest notes for voice
            tempCount;		
		TQNOTE ///  current last note in voice
            *lastNote;	
		TAbsTime gOffset;
};

///   tool struct for storing info during voice sorting
typedef struct {
	int sourceIndex;
	int pitchIndex;
	double pitchDistance; 
	int ambitusIndex;
	int maxAmbitus;
} TSearchInfo;

///  get for each note in noteList the closest voice in voiceInfo
void findClosestVoices(TNoteInfo *noteList,		///  array
					   int cAttack,		///  size of array
					   TVoiceInfo *voiceInfo, ///  array 
					   int cVoice///  size of arry
                       );	
///  unused??? todo check???  remove collisions in voices
/*
void collissionOptimize(TNoteInfo *noteList,		///  array
					   int cAttack,		///  size of array
					   TVoiceInfo *voiceInfo, ///  array 
					   int cVoice);	///  size of arry

 */

///  find closest voice for all unmerged in target
int findClosestVoices( TVoiceInfo *unmerged, ///  unmerged Voices
							  int nrOfVoices,	///  array size of *unmerged
							  int segStart,		///  startIndex of current segment
							  int segEnd,		///  stopIndex of current segment
							  TVoiceInfo *target, ///    targetVoices[segSize], 
							  int segSize,			///  array size of *target
							  TSearchInfo *result	///  closestVoice/distance[segSize]
                       );

///  set closest voice for all unmerged in target
int selectClosestVoice( TVoiceInfo *unmerged, ///  unmerged Voices
					  int nrOfVoices,		///  array size
					  int segStart,			///  start index of current segment
					  int segEnd,			///  end index current segment
					  TVoiceInfo *target, ///    targetVoices[segSize], 
					  int segSize,			///  array size of *target, *distance
					  TSearchInfo *distance	///  closestVoice/distance[segSize]
                        );

/* unused
int findBestAttack(TNoteInfo *noteList, ///  array 
				   int cAttacks);		///  size of array
*/

///  distance between voice and notes
double voiceDistance(TNoteInfo *noteInfo,    ///  info about current note
					TVoiceInfo *voiceInfo,	///  array
					TVoiceInfo *maxValues); ///  max values of array

///  re-sort chord links
void sortLinks(TNoteInfo *noteList, ///  current chord to test
					int cAttacks);         ///  size of array

///  check if valid voice separation
int valid(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice );

double pitchPenalty( double interval );
double chordPenalty( double ambitus );

double pitchDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice,
					int lookback,
					int emptyVIv);
double chordDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice,
					int id );
double gapDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice,
					int id,
					double gapThreshold,
					TMIDIFILE *theMidifile);
double unusedDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice );

double overlapDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice,
					int id,
					TMIDIFILE *theMidifile );
double voiceDistance(TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					TVoiceInfo *voiceInfo,  ///  current used voices
					int CVoice,				///  size of array
					int changeID,			///  id of changed note
					int oldLink,
					int oldVoice,
					double &pChord,			///  cost values
					double &pPitch,			///  cost values
					double &pGap,				///  cost values
					double &pOverlap,
					int loockback,
					int emptyVoiceEv,
					double gapThreshold,
					TMIDIFILE *theMidifile);

///  copy voice links of noteList into *notes
/*
int changeVoice( TNoteInfo *noteList, ///  current chord to test
					int cAttacks,         ///  size of array
					int CVoice,			///  maxVoices
					int pos,			
					int mode 		///  up, down
                 );
*/

TVoiceInfo GetMaxVoiceValues(TVoiceInfo *voiceInfo, ///  array
								 int  CVoice 		///  size of array
                             );


int getVoiceIndex( int voice,	///  search for voice
				   TVoiceInfo *voiceInfo, ///  array
				   int nrOfVoices ///  size of array
                   );

///   get chord root of note startID
int getRoot( TNoteInfo *noteList, ///  current chord to test
			 int  startID,
			 int cAttacks);         ///  size of array

/*!
add now to feature vector of corresponding voice
 */
void addToVoice( TNOTE *now,
                 TVoiceInfo *voiceInfo, ///  array
                 int nrOfVoices 		///  size of array
                 );
#endif
