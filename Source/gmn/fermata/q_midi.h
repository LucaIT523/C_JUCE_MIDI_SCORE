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
|	filename : Q_MIDI.H
|	Author     : Juergen Kilian
|	Date    : 17.10.1996-98, 2011
|  classdefinition for tqevent, tqtrack, tqmidi
--------------------------------------------------------------*/
#ifndef __Q_MIDI_H__
#define __Q_MIDI_H__
//---------------------------------------------------------------

#include "midi.h"
#include "defs.h"
#include "liste.h"
#include "tags.h"
#include "portable.h"
#include "chunklist.h"

//---------------------------------------------------------------


class TQTRACK;
class TPFILE;
class TFracBinList;

//---------------------------------------------------------------
/// Midifile including quantisation data and functionality
class TQMIDIFILE :public TMIDIFILE {
protected :
	/// flag indicating that upbeat shifting needs to be proceeded
	int upBeatFound;
	/// list of inferred barlengths, currently not used 
	TDoubleChunkList *barlengthList;
	/// list of inferred position of barlength changes, currently not used 
	TDoubleChunkList *barlengthPosList;
	
	/// shift attacktimes of all notes
	void shiftAttacks( TFrac offset );
	

	/// merge tracks and call setQData
	void finaliseQuantisation( void );
	/// global offset before notes
	TFrac glOffset;
    /// create a new track and prepare for read at offset position
    virtual TTRACK *CreateNewTrack( long offset );
	/// do meter detection
	TTimeSignature detectMeter( /// mode = 0 -> scoretiming, mode = 1 -> ms timing 
								int mode );
	/// shift first note to best phase position of meter signature
	TFrac meterPhaseShift( void );								
    
    /// will be called before quantisation
    virtual void PreQuantize(  	double atAccuracy,
								double durAccuracy );
    /// is 1 until quantisation/tempo detection    
    char  FirstTime;
	virtual char ToGMN( ostream &out );
    /// OFF, MIDIFILE, DETECT
    char detectMeterFlag;
    /// OFF, MIDIFILE, DETECT
    char detectKeyFlag; 	
    /// write to MIDI, all quantisation, inferring functions needs to be called before -> Use Convert(gmnName, midiName)
    int writeMIDI( const string &midiFilename );
public :
	void accuracy(double &attack, double &duration);
	virtual void preProcess( void );
	virtual void Debug( FILE *out  = NULL);
	TQMIDIFILE( const char *name );
	TQMIDIFILE( std::string *buffer );
	virtual ~TQMIDIFILE( void );
//	char GotoTrack( char track);
    /// write a GUIDO file
	virtual char Convert(ostream &gmnOutput,
						 const string &midiFilename );
	/// quantisation function
	void Quantize( void );
    /// ask user
    virtual int askDetectMeter(void);
    /// ask user
    virtual int askDetectKey( void );
    void MIR(const char *mirFName,
			 TTRACK *tempTrack,
    		 int voice );
/*
    /// move all notes in Voice from OldPos to NewPos  
    char ChangeNote( int Voice,
						  long OldPos,
						  long NewPos,
						  long NewDuration,
						  char MoveNext,
						  int  pitch );
*/
};
//---------------------------------------------------------------
#endif

