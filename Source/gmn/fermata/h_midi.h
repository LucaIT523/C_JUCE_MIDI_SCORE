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
|	filename    : H_MIDI.H
|	author      : Juergen Kilian (kilian@noteserver.org)
|	date	    : 17.10.1996-1999-03, 2011
|   declaration of THMIDIFILE
--------------------------------------------------------------*/
#ifndef __H_MIDI_H__
#define __H_MIDI_H__
//---------------------------------------------------------------

#include <sstream>
using namespace std;

#include "pattern.h"
#include "q_midi.h"
#include "defs.h"
#include "liste.h"
#include "notestatlist.h"
#include "patternfile.h"

#include "correl.h"

#include "chunklist.h"

#define patDistanceLimit 0.131

#ifdef IMUTUS_DLL
 #include "fermata_dll/midishare.h"
#endif


//---------------------------------------------------------------
class TCLICKTRACK;
class TCLICKNOTE;
//-----------------------------------------------------------------
/// TQMIDIFILE with features of HEISENBERG stage of project
class THMIDIFILE :public TQMIDIFILE {

protected :

	int // 0 == off, 1,..,255
		ClickTrackNr;		
							
	int // 0 == all,1,..,16
		ClickTrackChannel; 

	// created drung clickTrackPatternMatching
	TPFILE *patternFile;
	/// check if a localBest or overlapping match should be selected
	TPatternDistance *acceptPatternMatch(double curDistance,
							double typMatchDistance,
							TPatternDistance *bestOvl,
							TPatternDistance *localBest
							);

	TFrac /// score duration of a clicknote, used for manual clicktrack tempo detection
		   TactusLevel;
	int // #beats for count in (in clicktrack), maybe obsolete  
		CountIn;	      
	char  // Flag marks an upbeat (pickup)
		  UpBeatI;		   
	char // Flag for beat induction/tempo detection 
		 CalcTempo;		
	/// key signature for complete file
	int	 AccidentalsI;

	virtual TTRACK *CreateNewTrack( long offset );
	/// create a clicktrack by statistical tempo detection
	void CreateClickTrack( double atAccuracy,
						   double durAccuracy );
	/// do tempo detection and ornament detection
	virtual void PreQuantize( 	double atAccuracy,
								double durAccuracy );
								
	/*! normalise timing of all notes of all tracks according to the entries in ClickTrack	
		result = 1 -> upbeat optimisation needs to be proceeded after meter detection!
	*/
	int FitToClick( /// 1 = manual played clicktrack, 2 = clicktrack by hybrid tempo detection
					 char ctrMode );
		
public :
	int clickTrackPatternMatching( TFracBinList *durationList,
								   TDoubleBinList *statList);

#ifdef IMUTUS_DLL
	THMIDIFILE( int ppq, int fileType );
	char Read( MidiSeqPtr *seqs, int size);
#endif


	
	/// return an array whith the phase in each bar, starting at from
    double * meterPhase( TTimeSignature meter, 
    					 TQNOTE *from, 
    					 int &size);
	/// copy attackpoints from all tracks into a new ClickTrack
	TCLICKTRACK *mergeToClickTrack( void );


	/// Ptr to the clicktrack
	TCLICKTRACK *ClickTrack;  
	THMIDIFILE( const char *name );
	THMIDIFILE( std::string *glnBuffer);
	virtual ~THMIDIFILE( void );

	void SetUpBeat( char c )
		{ UpBeatI = c; };
	int SetClickTrackNr( int i );
	int SetClickTrackChannel( int i );
	void SetCountIn( int i )
		{ CountIn = i; };

	void SetCalcTempo( char c )
		{ CalcTempo = c; };
	virtual char Read(   FILE *out );
	/// finalise and call ToGMN and/or midi
	virtual char Convert(ostream &gmnOutput,
						 const string &midiFilename );
	/// convert to GUIDO MusicNotation
	virtual char ToGMN( ostream &out );
	/*!
	- proceed tempo detection starting [from, ..., to)
	- stop/ return if problem notes have been found
	return:
	- to == ok, or first unproceeded note  cause by abort
	*/
	TCLICKNOTE  *tempoDetection ( TCLICKNOTE *prev,
								  TCLICKNOTE *to,
								  int direction,
//								  double curTempo,
								  TDoubleBinList *statList,
								  TFracBinList *durationList,
				  				 double threshold,
								  TFrac &start,
								  int *sucess);

private:
	void init( void );
	void inferDynamics(void);
};

class TCLICKNOTE;
double  distance(double val,		// original IOIratio
				TCLICKNOTE *cnote,
				TCLICKNOTE *next,
				TCLICKNOTE *prev,
				 TFrac cDuration,
				 TDoubleBinClass *IOIClass,		// closest class, selected IOI
				 double curTempo,
				 TCLICKTRACK *cTrack,
				 TDoubleBinList *ioiList,  // ioi probabilities
				 TFracBinList *durationList, // duration porbabilities
				 double pTempo,		// penalties
				 double pDuration,
				 double pIOI,
				 double alpha);


double synchopationDistance( TAbsTime attack,
							TCLICKNOTE *ptr,
							TCLICKTRACK *clicktrack);

double durationDistance( TFrac sDuration, // closest Class, distance
						TFracBinList *durationList); // duration porbabilities

double IOIDistance(  double ioi,
					TDoubleBinClass *IOIClass, // closest class info
					TDoubleBinList *ioiList ); // ioi probabilities

double tempoDistance( double IOIratio,		 // played IOI
					 TDoubleBinClass *IOIClass, // selected IOI class for current note
					 double curTempo );
double pSignificance( double curDistance );
					
//---------------------------------------------------------------
#endif
