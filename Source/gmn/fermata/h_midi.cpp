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
|	filename : H_MIDI.CPP
|	author     : Juergen Kilian
|	date	  : 17.10.1996-99-2001-03,2011
|	class-functions for class THMIDIFILE
------------------------------------------------------------------*/
#include "debug.h"

#include <iostream>
using namespace std;
#include <sstream>
#include <string>
#include <stack>

#include <string.h>

#include "h_midi.h"
#include "pattern.h"
#include "hpattern.h"
#include "h_track.h"
#include "c_track.h"
#include "funcs.h"

#include "liste.h"
#include "anvoice.h"

#include "import.h"

#include "correl.h"
#include "patternfile.h"
#include "hpatternfile.h"
#include "meta_key.h"
#include "meta_meter.h"
#include "q_chord.h"


#include "../lib_src/ini/ini.h"
//-------- for testing -----
#include "tags.h"
#include "key.h"

#include "patternfile.h"
#include "click_similarity.h"

#include "q_funcs.h"
#include "statist.h"
/// use a list of 3 best pattern for each note
#define cNotePatListSize 3

//-------------------------------------------------------------------

#ifdef _DEBUG
#define logTempoDetection
#ifdef logTempoDetection
#define TD_LOG_FNAME "_tempodetection"
#endif
#endif




THMIDIFILE::THMIDIFILE( const char *filename ) 
: TQMIDIFILE( filename )
{
	init();
	ControlTrack = new THTRACK(0, this);
};

THMIDIFILE::THMIDIFILE( std::string *buffer )
: TQMIDIFILE( buffer )
{
	init();
	ControlTrack = new THTRACK(0, this);
};
//-------------------------------------------------------------------
THMIDIFILE::~THMIDIFILE( void )
{
	if( ClickTrack )
		delete ClickTrack;
	ClickTrack = NULL;
	ClickTrackNr = -1; // turn off
	if( barlengthList )
	{
		delete barlengthList;
		delete barlengthPosList;
	}
	if( patternFile )
	{
		delete patternFile;
		patternFile = NULL;
	}
}//~TMIDIFILE
//-------------------------------------------------------------------
/*!	read a midi or .gmn file and fill data structure

res :	
1 ok
0 error
*/
char THMIDIFILE::Read(	FILE *log )
{
	int /// -1 == off; 0..127 == pitch;128..255==ctrl+128
		ClickTypeFilter = -1;  

	// read settings from inifile
	TInifile *tempIni = getInifile();
	if( tempIni )
	{
		// Check Inifile for tempo detection
		const char* val = tempIni->GetValChar( "DETECTTEMPO", 
			"MIDIFILE","Tempodetection: [OFF|HYBRID|CLICKTRACK] " );
		/// clicktrack tempo detection
		if(!strcmp(val, "CLICKTRACK") )
		{
			Printf("Tempodetection by clicktrack is on.\n");
			ClickTrackNr = tempIni->GetValInt( "CLICKTRACK", 
				"0", "[0|1...n] select MIDI tracknumber with click information" );
			if( (ClickTrackNr == 0 && 
				Type() == 1) )
			{
				cout << "Error: "<< tempIni->filename() << ": Inputfile is smf1 CLICKTRACK number must be > 0!\n";
				return 0;
			}
			else if( ClickTrackNr > 1 &&
				Type() == 0)
			{
				cout << "WARNING: " << tempIni->filename() <<": Inputfile is smf0 CLICKTRACK has been set to 0!\n";
				ClickTrackNr = 1;
			}
			// channel filter for clicktrack?
			ClickTrackChannel = tempIni->GetValInt( "CLICKCHANNEL", 
				"0","[0,1...16] MIDI channel for filtering click events, 0 == no filter" );
			if(ClickTrackChannel > 16 ||
				ClickTrackChannel < 0)
			{
				cout << "WARNING "<< tempIni->filename() <<
					": illegal CLICKCHANNEL, channel will be ignored!\n";

				ClickTrackChannel = 0;
			}


			val = tempIni->GetValChar("CLICKFILTER",
				"OFF",
				"[OFF|PITCHn|CTRLn] (n=0...127) use specific event (pitch number or ctrl number) as click");
			if( strstr(val, "PITCH") == val )
			{
				ClickTypeFilter = saveAtoi( val+5);
				if( ClickTypeFilter > 127 ||
					ClickTypeFilter < 0 )
				{
					cout << "ERROR: Invalid CLICKFILTER "<< ClickTypeFilter <<" will be ignored!" <<endl;
					ClickTypeFilter = -1;
				}
			}
			else if( strstr(val, "CTRL") == val )
			{
				ClickTypeFilter = saveAtoi( val+4) + 128;
				if( ClickTypeFilter > 255 ||
					ClickTypeFilter < 128 )
				{
					cout << "ERROR: Invalid CLICKFILTER " << (ClickTypeFilter - 128)<<" will be ignored!\n";
					ClickTypeFilter = -1;
				}
			}
			if( Type() == 0 &&
				ClickTypeFilter < 0 &&
				ClickTrackChannel == 0 )
			{
				Printf("ERROR: Inputfile is smf0, CLICKTRACKCHANNEL or CLICKFILTER must be specified for CLICKTRACK tempo detection!\n");
				return 0;

			}
			val = tempIni->GetValChar( "TACTUSLEVEL", "1/4", "beat duration of clicknotes" );

			TactusLevel = TFrac(val);
			if( TactusLevel <= 0 )
			{
				Printf("ERROR: Illegal value for TACTUSLEVEL ");
				Printf(TactusLevel);
				Printf("\n default tactuslevel = 1/4 will be used instead!");
				TactusLevel = TFrac(1,4);
				tempIni->SetEntry( "TACTUSLEVEL", "1/4");
			}
			Printf("Tactuslevel: ");
			Printf(TactusLevel);
			Printf("\n");
		}
		else if(!strcmp(val, "HYBRID")) // tempo detection by statistics
		{
			CalcTempo = 1;
		}
	} // if temp ini

	if( ClickTrackNr == 0 )
		ClickTrackNr = 1;

	int pSucc = TQMIDIFILE::Read( log );
	if( pSucc &&
		Type() != 10 &&		// no clicktrack in .gmnfiles
		ClickTrackNr > 0 ) // exists a clicktrack?
	{
		/*
		SMF-0:
		Track[0] == control track
		Track[1..16] == events of channel n
		empty tracks are deleted
		SMF-1
		-Track[n]->Tracknumber == physical tracknumber of file
		-different tracks may have the same tracknumber
		-each track contains events of only one MIDI-Channel
		*/

		// Copy Track[ClickTrackNr] to ClickTrack
		//		ClickTrack = new TCLICKTRACK( (TQTRACK *)Track[ClickTrackNr] );
		//		delete Track[ClickTrackNr];

		// search for clicktrack
		THTRACK *tempTrack = (THTRACK*)EvTracks;
		THTRACK *prev = NULL;

		// search for ClickTrackNr
		while( tempTrack &&
			ClickTrackNr != tempTrack->TrackNumber()
			)  // physical TrackNr
		{
			prev = tempTrack;
			tempTrack = dynamic_cast<THTRACK *>(tempTrack->Next());			
		} // while

		/* 
		if clicktrack was split, because of events with
		different channels, it must be merged again!!!
		should't occur
		needs to be done !!!
		*/

		//search for ClickTrackChannel
		if( ClickTrackChannel ) // Channelfilter
		{
			while( tempTrack &&
				ClickTrackNr == tempTrack->TrackNumber() &&
				ClickTrackChannel != tempTrack->Channel() )
			{
				prev = tempTrack;
				tempTrack = dynamic_cast<THTRACK *>(tempTrack->Next());
			} // while
		} // if Channel filter

		// found ?
		if( tempTrack &&
			ClickTrackNr == tempTrack->TrackNumber() &&
			(ClickTrackChannel == tempTrack->Channel() ||
			ClickTrackChannel == 0) // off
			)
		{			
			// Copy Track[ClickTrackNr] to ClickTrack
			ClickTrack = new TCLICKTRACK( RecPPQ,
				RecTempo,
				tempTrack,
				this);



			tempTrack->deleteClicks( ClickTrackChannel,
				ClickTypeFilter );
			/*
			// remove tempTrack from list
			if( prev )
			prev->SetNext( tempTrack->Next() );
			else
			EvTracks =  tempTrack->Next();

			delete tempTrack;
			*********************************************
			TrackCountI--;
			********************************************

			*/			

			// reaad from file one more time
			ClickTrack->Read( file,
				RecPPQ,
				TactusLevel, // duration of 1 click
				ClickTrackChannel, // filter
				ClickTypeFilter,
				log );
			ClickTrack->SetRecValues( RecTempo,
				RecPPQ );
		}
		else
		{
			cout << "Error: Clicktrack "<< ClickTrackNr << "not found or empty!" << endl;
			return 0;
		}		
	} // if clecktrack specified

#define HEISENBERG_DVL
#ifdef HEISENBERG_DVL

	/*
	TKeyDetermin *KeyDetermin;
	KeyDetermin = new TKeyDetermin();
	KeyDetermin->AddNotes( EvTracks->FirstNote(), NULL );
	FILE *out;
	out = fopen( "keys.txt","wt");
	KeyDetermin->Key();
	KeyDetermin->Printf(out);
	fclose(out);
	*/
#endif
	if( pSucc )
		return 1;
	else
		return 0;
}


// !!!!!!! following functions can only be executed one time !!!
// HEISENBERG
void THMIDIFILE::PreQuantize(double atAccuracy,
							 double durAccuracy )
{	

	// possible ornament duration limit in MIDI ticks
	double ornament_duration = 0;
	// calculate the ornamental duration
	//		ornament_duration = GetInifile()->GetValInt("ORNAMENT_TIME",DEF_ORNAMENT_TIME, "max duration threshold [ms] for ornaments");
	// Analyse performance note durations
	int def_ornament_duration = saveAtoi( DEF_ORNAMENT_TIME );
	double sumAll = 0, 
		sumPossible = 0;
	int cAll = 0, 
		cPossible = 0;
	double stdDev = 0;
	Printf("Calculating ornament duration: ");
	THTRACK *temp = dynamic_cast<THTRACK *>(EvTracks);
	while( temp )
	{
		TNOTE *curNote = temp->FirstNoteObject(-1);
		while( curNote )
		{
			double cDuration = curNote->durationMS(this);
			sumAll += cDuration;
			cAll++;
			if( cDuration < def_ornament_duration )
			{
				cPossible++;
				sumPossible += cDuration;
				stdDev += pow(cDuration-(sumPossible/cPossible),2);
			}
			curNote = dynamic_cast<TNOTE *>(curNote->GetNext(-1));
		}
		temp = dynamic_cast<THTRACK *>(temp->Next());
	}
	if( cAll )
	{
		if( cPossible > 1)
		{
			double avPossible = sumPossible / cPossible;
			// now get pdf
			stdDev /= cPossible - 1;
			stdDev = sqrt(stdDev);
			ornament_duration = avPossible;
		} // if cPossible
		else
		{
			// do nothing
		}
	} // if cAll			
	cout <<  ornament_duration << "ms\n";


	TTagList *Slurs = NULL,
		*UpBeats = NULL;
	temp = (THTRACK *)EvTracks;
	while( temp ) // for all tracks
	{
		// ornament detection will be called  !!
		temp->preQuantize(ControlTrack, 
			RecPPQ,
			(int)(ornament_duration+0.5));	
		TInifile *tempIni = getInifile();
		if( tempIni )
		{
			// CHeck Inifile for tempo detection
			const char*val = tempIni->GetValChar( "SLUR_OUT", "OFF", "[ON|OFF] detection and output of \\slur tags" );
			if(!strcmp(val, "ON") )
			{
				Slurs = MarkSlur( temp );
			}
#ifdef _DEBUG
			val = tempIni->GetValChar( "UPBEAT", "OFF","upbeat detection [ON|OFF]" );
			if(!strcmp(val, "ON") )
			{
				UpBeats = MarkStressTrack( temp );
			}
#endif
		} // if Ini
#ifdef _DEBUG
		if( Slurs )
		{
			FILE *temp;
			temp = fopen("slurs.txt","wt");
			Slurs->Print( temp );
			fclose( temp );
		}
#endif

		// keep Tags for later
		temp->MergeTags(Slurs);
		temp->MergeTags(UpBeats);

		//		temp->Debug();
		temp = (THTRACK *)temp->Next();
	} // while tracks
	//------ create clicktrack ------------------
	//------ tempo detecion ---------------------
	if( FirstTime ) // can only be done at first time
	{


		// do clicktrack tempo detection
		if( ClickTrackNr > 0  )	// quantize with clicktrack
		{
			cout << "Fit to clicktrack #" << (long int)ClickTrackNr << endl;

			// Todo: detect meter here!

			//ClickTrack->Write();
			//			const char *str = GetInifile()->GetValChar("UPBEAT","OFF","[ON|OFF] Upbeat detection for clicktrack tempo detection");
			FitToClick(  1 /*detect upbeat by early notes */);

			/*
			score times are still in tick-timing(!) otherwise we run into trouble
			with large denominators!
			*/			
		} // if ClickTrack
		else if( CalcTempo  )	// do statistical tempo detection
		{
			//			detectMeter();
			Printf("Tempodetection by statistics ... ");
			// Statistik ausführen un Clicktrack erzeugen
			CreateClickTrack( atAccuracy,
				durAccuracy);
			//			ClickTrack->Debug();
			Printf("done \n");
			// Todo: detect meter here!

		 	FitToClick(  2 /* upbeat detection by optimised onsets, should be already done at  create clicktrack*/);	
			/*
			score times are still in tick-timing(!) otherwise we run into trouble
			with large denominators!
			*/			
		} 
	} // if FirstTime


} // PreQuantize
//-------------------------------------------------------------------
/*!
normalize timing information by clicktrack
create tempo profile from recorded/calculated clicktrack
*/
int THMIDIFILE::FitToClick( /// 1 = manual clicktrack, 2 = auto inferred clicktrack
						   char ctrMode	 )
{
	/// todo: infer meter on score times of clicktrack!!!!
	//	TFrac meter;
	if(  ClickTrack )	// exists any clicktrack?
	{
		upBeatFound = 0;			
		if(ctrMode == 1) // check for upbeats (<-notes earlier than first click)
		{
			Printf("Upbeat detection ... ");
			TQTRACK *tempTrack = (TQTRACK*)EvTracks;
			while( tempTrack )
			{
				upBeatFound += ClickTrack->CheckForUpBeat(tempTrack);
				tempTrack = (TQTRACK*)tempTrack->Next();
			} // while
			Printf("done.\n");
		} // if 
		else if( ctrMode == 2 )
		{
			upBeatFound = 1;
		}		
		Printf( "Normalising to clicktrack ... ");
		TQTRACK *tempTrack = (TQTRACK*)EvTracks;
		while( tempTrack )
		{
			{
				// all timeing information must be still in tick timing
				ClickTrack->FitToClick( tempTrack );
				tempTrack = dynamic_cast<TQTRACK *>(tempTrack->Next());
			} // if( Check
		} // for( i =

		/*
		if( upBeatFound )
		{

		} // if upbeat		
		*/
		ClickTrack->CreateTempoTags( dynamic_cast<TQTRACK *>(ControlTrack));		
	} // if( ClickTrack
	tickTiming = 0; // all timing information are now in score time

	Printf( "done.\n");
	return upBeatFound;
} // FitToClick
//-------------------------------------------------------------------
//-------------------------------------------------------------------
/*
TFrac getDuration( TFrac cDuration, // score duration of prev note
TDoubleBinClass *tempClass) // selected IOI class
{

TFrac sDuration;
double durationF, // duration in double
rIOI;		 // resulting IOI

if( !tempClass )
Printf("Error: tempClass == NULL!\n");
durationF = (double)cDuration.toDouble();



rIOI = tempClass->FirstV();

// normalize

if( rIOI > 0 )
durationF *= (rIOI+1);
else
durationF /= ((rIOI-1)*-1);

// convert into Frac
// include 32nd=x/32. 16tripl=x/48, 5tplets=x/20  
sDuration = TFrac((long)(durationF*960+0.5), 960L);
return sDuration;
}
*/
/*!
evaluate a distance function depending on the input values
*/

double  distance(double ioi,		// original IOI, as played
				 TCLICKNOTE *cnote, // current clicknote	
				 TCLICKNOTE *next,
				 TCLICKNOTE * /*prev*/,
				 TFrac cDuration,	// score duration of previous note
				 //				 double plDuration,	// played score duration
				 TDoubleBinClass *IOIClass,		// selected IOI class
				 //				 TFracBinClass *durationClass, // selected duration class
				 double curTempo,		  
				 TCLICKTRACK *cTrack,
				 TDoubleBinList *ioiList,  // ioi probabilities
				 TFracBinList *durationList, // duration probabilities
				 double pTempo,		// penalties
				 double pDuration,
				 double pIOI,
				 double alpha)
{
	/*
	double res,
	//		  pTempo = 0.8,
	//		  pDuration = 0,
	deltaTempo,
	tempoDist,
	newTempo;
	*/
	//		  pIOI = 0;

	// todo put values in .ini
	// get quantized duration
	TFrac sDuration = getDuration( cDuration,		// previous note
		IOIClass );	// selected IOIclass

	// calc tempo for quantized duration
	TFrac tempDuration = cnote->scoreDurationI;

	cnote->scoreDurationI = sDuration;
	double newTempo = cTrack->tempo( cnote );

	cnote->scoreDurationI = tempDuration;
	if( newTempo < 0 )
		return 100;

	double deltaTempo = fabs(newTempo-curTempo);
	double tempoDist = (deltaTempo/(deltaTempo+10));
	// todo limit tempo change

	// penalty for tempo changes during exact playing
	double res = pTempo * tempoDist;

	if( newTempo > 280 )
		res *= 100;

	// penalty for strange durations
	res += pDuration * durationDistance( sDuration,
		durationList );

	double pSynchopation = 2;
	if( next )
		res += pSynchopation *synchopationDistance( cnote->scoretime() + sDuration,
		next,
		cTrack);
	// penalty for unexact playing
	res += pIOI * IOIDistance( ioi,
		IOIClass,
		ioiList);

	// todo add penalty for increasing variance of IOI, lgDuration
	// todo add penalty for syncopated long notes
	return res;
};


double durationDistance( TFrac sDuration, // closest Class, selected duration
						TFracBinList *durationList)

{
	// todo: if distance to big -> get distance to a new class
	double res = durationList->minDist(sDuration);	
	return res;
}




/*
depends on size of denominator, weight of note 
7/8 14/16 15/16 == 33/16 
1/4 == 0, ..... 63/64 == 1
*/
double synchopationDistance( TAbsTime attack,
							TCLICKNOTE *ptr,
							TCLICKTRACK *clicktrack)
{
	long denominator = attack.denominator();
	long numerator = attack.numerator();


	// too large for synchopation
	if( denominator > 0 &&
		denominator < 4 )
		return 0;

	long mod = numerator % denominator;
	if( mod > denominator / 2 )
		mod = denominator - mod;

	// mod == distance between  attack and n*denominator/denominator
	// 1 < mod < denom/2 -> 1 > res > 0
	// 8 < denom < oo  -> 0 < res < 1 
	double resDenom = ((double)denominator - 4 )/denominator;
	double resMod = 1 / (double)(mod*mod);

	resMod = 1;
	double res = resDenom * resMod * ptr->weight(clicktrack);

	return res;
}



double IOIDistance( double ioi,
				   TDoubleBinClass *IOIClass,
				   TDoubleBinList * ioiList )
{
	// todo if distance to big -> get distance to a new class
	double res = IOIClass->distance( &ioi, 
		NULL , 
		ioiList->maxWeightOfElements(),
		ioiList->lAlpha(),
		ioiList->rAlpha(),
		ioiList->Count());

	return res;	
}


double tempoDistance( double IOIratio,
					 TDoubleBinClass *IOIClass, // selected IOI class for current note
					 double /*curTempo*/)
{
	double scoreIOI = IOIClass->FirstV();
	double playIOI = IOIratio;

	if( playIOI < 0 )
		playIOI = -1 * (1/(playIOI-1));
	else
		playIOI += 1;


	if( scoreIOI < 0 )
		scoreIOI = -1 * (1/(scoreIOI-1));
	else
		scoreIOI += 1;

	double deltaTempo;
	if( scoreIOI < playIOI )
		deltaTempo = 1 - (scoreIOI / playIOI);
	else
		deltaTempo = 1 - (playIOI / scoreIOI);

	if( deltaTempo < 0 )
		deltaTempo *= -1;

	// add IOIDistance
	// todo use relation to current Tempo
	double res = deltaTempo / (deltaTempo + 1);
	return res;
}


int createStdDurGrid( TFracBinList *durationList,
					 TFracBinClass **quarterClass,
					 const char *filename)
{

	durationList->readIni(filename);
	durationList->createDistribution();
	if( quarterClass )
	{
		TFrac tempVal(1,4);
		*quarterClass =  durationList->findExact(tempVal);
	}
	if ( durationList->Count() == 0 )
	{
		Printf("Warning: no entries found in IOIList.ini\n");
		Printf("   added defualt classes!\n");
		durationList->addClass(TFrac(1,4),2);
		durationList->addClass(TFrac(1,8),2);
		durationList->addClass(TFrac(3,8),1);
		durationList->addClass(TFrac(1,16),1);
		durationList->addClass(TFrac(1,12),1);
		durationList->addClass(TFrac(1,6),0.5);		
		durationList->addClass(TFrac(1,2));
	}
	else if( quarterClass &&
		!(*quarterClass) )
	{
		Printf("Error: quarternote class is missing in IOIList.ini!\n");
		return 0;
	}
	return 1;
}

/// add the std rIOIs to statList
int createStdIOIratioGrid( TDoubleBinList *statList,
						  TDoubleBinClass **equalClass,
						  TDoubleBinClass **doubleClass,
						  TDoubleBinClass **halfClass,
						  const char *filename)
{

	statList->readIni(filename);
	statList->createDistribution();

	if( doubleClass )
		*doubleClass = NULL;
	if( equalClass )
		*equalClass = NULL;
	if( halfClass )
		*halfClass = NULL;


	double tempVal = 0;
	TDistanceStruct dStruct = statList->closestClass(&tempVal);
	if( dStruct.distance == 0 &&
		equalClass )
		*equalClass = dStruct.classPtr;
	else if( statList->Count() == 0 )
	{
		Printf("Warning: no entries found in IOIratioList.ini\n");
		Printf("   added defualt classes!\n");
		statList->addClass(0);
		for( int i = 1; i < 9; i++ )
		{
			statList->addClass(-i);
			statList->addClass(i);
		}
		statList->addClass(1.5);
		statList->addClass(-1.5);

	}
	else if( equalClass )
	{
		Printf("Warning: equal-class is missing in IOIratioList.ini!\n");
	}


	tempVal = 1;
	dStruct = statList->closestClass(&tempVal);
	if( dStruct.distance == 0 &&
		doubleClass )
		*doubleClass = dStruct.classPtr;

	dStruct = statList->closestClass(&tempVal);
	if( dStruct.distance == 0 &&
		halfClass)
		*halfClass = dStruct.classPtr;

	return 1;

}

/// global tempo detection _alpha used in TDoubleBinClass::distance
// double _alphaBin; 

/*
create a tempoprofile during tempo detection by statistics
*/
void THMIDIFILE::CreateClickTrack( double atAccuracy,
								  double durAccuracy )
{
#define _PATTERN_TEMPODETECTION
#ifdef _PATTERN_TEMPODETECTION


	// todo find threshhold for weight of usefull clicks 
	double _alphaBin = getInifile()->GetValFloat("TDALPHA","5", "for tempo detection, Don't touch");
	if( _alphaBin <= 0 )
		_alphaBin = 1;

	// create prototypes for IOIratio Classes
 	TDoubleBinList *statList = new TDoubleBinList(95,105, _alphaBin);
	const char  *IOIratioFilename = getInifile()->GetValChar("IOIratioGridFName","IOIratioList.ini","Filename for IOIratioGrid");
	/// read IOIratio .ini file
	
	createStdIOIratioGrid( statList, 
		NULL, //&equalClass,
		NULL, //&doubleClass,
		NULL, //&halfClass,
		IOIratioFilename);		


	double minW = statList->setMinMaxRel(30.0);		  
	statList->setMinweightDeltaRel( 20.0 ); // local/global evaluation

	statList->createDistribution();
	

	// create prototypes for score duration values
	/// read IOI .ini file
	TFracBinList *durationList = new TFracBinList(_alphaBin);
	const char  *IOIFilename = getInifile()->GetValChar("IOIGridFName","IOIList.ini","Filename for IOIGrid");
	createStdDurGrid( durationList,
		NULL, // = &quarterClass,
		IOIFilename);	
	minW = durationList->setMinMaxRel(30.0);
	durationList->setMinweightDeltaRel( 20.0 );
	durationList->createDistribution();
	// create clicktrack from attacks of all tracks
	if( ClickTrack )
		delete ClickTrack;
	ClickTrack = mergeToClickTrack();

	//	ClickTrack->Debug();
	// filter low-significance clicks
	ClickTrack->filterClicks(1);

	{ // create first estimation for quarter duraion at 100bpm
		// set duration for first beat in list, try reasonable durations and 'human' tempo
		double quarterIOI;
		quarterIOI = MStoDTime(RecPPQ, RecTempo, 600);
		TFrac quarter(1,4);
		durationList->updateTypicalDurations(quarter, quarterIOI,1);
	}

	// do pattern matching against database
	clickTrackPatternMatching( durationList,
		statList);
	//	ClickTrack->Debug();	
	// add IOI and IOIratios of pattern files to grid lists
	durationList->writeIni(IOIFilename);
	statList->writeIni(IOIratioFilename);



	/// ptr list for segments
	TMusObjList *startList = new TMusObjList(20);
	TMusObjList *stopList = new TMusObjList(20);

	// todo start with first durational accent!, go to left and right from this note
	// shift attack times after complete selection
	/*
	advanced start note selection strategie, did not work very well
	bestClick = ClickTrack->startNote(ClickTrack->FirstNote(),
	NULL,
	forward);
	*/	


	//	prev = searchAnchor()
	// current lgDuration, quantized
	TFrac cDuration = TFrac(1,8); 

	Printf("  Statistical tempo detection ... ");
	TCLICKNOTE * bestClick = ClickTrack->FirstNote();
	int startId = 1;
	//	double curTempo = 0;
	while( bestClick ) 
	{

		if( bestClick->startId == 0 )
			bestClick->startId = startId++;

		// set score start time to zero
		TFrac start(0,1);
		bestClick->setScoretime(start);			


		// estimate duration of first note
		double mSignificance = 0;
		if( bestClick->patternFinalID > -1 ) // use pattern duration?
		{
			// use values already set by pattern
			TPATTERN *pat = bestClick->bestPatternList[bestClick->patternFinalID].ptr;
			TPNOTE *pNote = pat->FirstNote();
			cDuration = TFrac(pNote->duration());

			mSignificance = pSignificance(bestClick->bestPatternList[bestClick->patternFinalID].distance);
			/*
			TFrac cDuration2;
			TFracBinClass *cl;
			int bestN;
			cl = durationList->closestTypicalDuration(bestClick->ioi(1,-1).toDouble(), 
			bestClick->ioi(1,-1).toDouble()*0.1,
			bestN);
			if( cl )
			{
			cDuration2 = cl->duration() * bestN;
			double pDur;
			pDur = GaussWindow(bestClick->ioi(1,-1).toDouble(),
			durationList->tickDuration(cl, bestN),
			bestClick->ioi(1,-1).toDouble()*0.1);
			if( cDuration2 != cDuration &&
			pDur > 0.95)
			{
			printf("");
			//					bestClick->patternFinalID = -1;
			//					cDuration = cDuration2;
			ClickTrack->Debug();
			}
			} // if cl		
			*/	
		} // if pattern start
		else // no pattern matched
		{
			if( bestClick->scoreDuration() <= 0L )
			{

				// closest duration might be a multiple of a class duration
				// closestDuration = bestN * cl->duration
				int bestN;

				TFracBinClass *cl = durationList->closestTypicalDuration(bestClick->ioi(1,-1).toDouble(), 
					//  bestClick->ioi(1,-1).toDouble()*0.2,
					durationList->typicalQuarterIOI * 0.3,
					bestN);
				if( durationList->typicalQuarterIOI > 0 )
				{
					// quantise performed duration to quarter and get closest typical duration
					long num = (long)(bestClick->ioi(1,-1).toDouble()/durationList->typicalQuarterIOI * 100 +0.5);
					TFrac tDur(num, 400L);
					TDistanceStruct d1 = durationList->closestClass(NULL,&tDur);
					cl = dynamic_cast<TFracBinClass *>(d1.classPtr);
					bestN = 1;

				}	  

				// give more influence to wieght than to sigma!!
				if( cl )
				{
					cDuration = cl->duration() * bestN;

					mSignificance = 0.6;
				}
				else // get highest weight class
				{
					bestClick->patScoreIOIratio = 0;
					TFracBinClass *tempClass;
					tempClass = dynamic_cast<TFracBinClass *>(durationList->getBest(NULL));
					cDuration = tempClass->duration();	
					mSignificance = 0.2;		
				}
			} // if no score Duration
			else // score duration was set before, abort with success = 1
			{
				cDuration = bestClick->scoreDuration();
				mSignificance = 0.6;
			}
		} // else of no Pattern

		bestClick->scoreDurationI = cDuration; // try to use quarter note for first note
		durationList->updateTypicalDurations(cDuration,
			bestClick->ioi(1,-1).toDouble(),
			mSignificance);

		// check for errors
#ifdef _DEBUG_CEMGIL
		{
			if( bestClick->testScoreIOI() > 0L &&
				bestClick->testScoreIOI() != cDuration )
			{
				ClickTrack->Debug();
			}   
		}
#endif


		int direction = dirforward;
		// might have changed during backward search!
		start = bestClick->scoretime();			
		TCLICKNOTE *stoppedAt = NULL;		

		int success = 0;
		stoppedAt = tempoDetection( bestClick, // start, already processed
			NULL, // end
			direction,
			statList,	
			durationList,
			0, //threshold,
			start,
			&success );

		stopList->set( stopList->count(), 
			stoppedAt );

		// start pointer missing?
		if( startList->count() != stopList->count() )
		{
			startList->set(startList->count(), bestClick);
		}

		// if successful abort, ´keep everything and go back
		if( !success )
			bestClick = stoppedAt->getNext(1,-1);	
		else
			bestClick = stoppedAt;
	} // while
	Printf(" completed\n");
	{
		// keep looping until all notes are placed

		// post processing

		ClickTrack->markStartStop( startList, stopList);	
		ClickTrack->Debug();
		startList->sort( stopList );

		// connect segments, -> shift start positions to end of prev segment
		TAbsTime prevEnd;
		TCLICKNOTE *startNote = ClickTrack->FirstNote();
		while( startNote )
		{
			if( startNote->scoretime() <= prevEnd )
			{
				TFrac delta = prevEnd - startNote->scoretime();
				ClickTrack->shiftScoretimes(startNote,
					NULL,
					delta);
			}
			prevEnd = startNote->scoretime() + startNote->scoreDuration();
			startNote = startNote->getNext(1,-1);
		}
		ClickTrack->Debug();

		/*
		this does note work for all data because errors at position n
		might be the result of wrong durations a position n-k, 
		where k is unknown! 
		-> ask user for fixing errors
		*/
		ClickTrack->fixPrimeDistanceErrors(ClickTrack->FirstNote(),
			NULL);

		ClickTrack->Debug();


		/*
		Errors might be caused by wrong selections
		for the first note of each segment!
		-> if segments don't fit, try other duration for first note
		and check if this decreases the complexity of the segment
		*/

		// scan for largest denominator
		long int largestDenom = 0;
		TCLICKNOTE *cNote = ClickTrack->FirstNote();
		while( cNote )
		{
			if( cNote->scoretime().denominator() > largestDenom )
			{
				largestDenom = cNote->scoretime().denominator();
			}
			cNote = cNote->getNext(1,-1);
		} // while


		/* error correction indicated by large tempo jumps
		is not possible
		- played tempo jumps are large
		- position of error is not always known?
		*/

		Printf("  Optimising start positions ... ");
		TFrac grid(1,8),
			resolution((long)1, largestDenom);

		// shift at segment borders connect without gaps and overlaps
		for(int j = 0; j < startList->count(); j++ )
		{	
			TCLICKNOTE *cNote = dynamic_cast<TCLICKNOTE *>(startList->get(j));
			TCLICKNOTE *cNote2 = dynamic_cast<TCLICKNOTE *>(stopList->get(j));

			// don't move if prev note was a pattern note
			/*
			TCLICKNOTE *prevStop = dynamic_cast<TCLICKNOTE *>(cNote->GetPrev(-1));											
			if( !prevStop || 
			!validIOIratio(prevStop-> patScoreIOIratio) )
			*/
			{
				/// get start of next segment
				if( cNote2 )
					cNote2 = cNote2->getNext(dirforward,-1);
				double oldQuality,
					newQuality;
				if( cNote->GetAbsTime().numerator() == 94283 )
				{
					ClickTrack->Debug();
				}					   
				// try to change duration of last note before segment
				TFrac delta = ClickTrack->optimumOffset( cNote,
					// dynamic_cast<TCLICKNOTE *>(stopList->get(i)),
					cNote2,
					grid,
					resolution,
					oldQuality,
					newQuality);

				TFrac d(1,16);
				if( delta != 0L 
					// && ( // newQuality > oldQuality * 1.2 ||
					// delta < d) 
					)
				{
					ClickTrack->Debug();
					startList->setData(j, oldQuality, newQuality, delta );
					// shift everything in (cNote...end]
					ClickTrack->shiftScoretimes(cNote,
						NULL,
						delta);

					// adapt cNote->Prev->scoreDuration
					TCLICKNOTE *pSegment = dynamic_cast<TCLICKNOTE *>(cNote->GetPrev(-1));
					if( pSegment )
					{
						pSegment->scoreDurationI += delta;

					} // if pSegment
					ClickTrack->Debug();					
				} // if change prev duration
				else // keep old selection
				{
					TFrac zero;
					startList->setData(j, oldQuality, newQuality, zero);
				}				
				/*
				Because the duration of the first note in segment is also critical
				We should test the correctness and try to optimise the following positions also

				*/
				if( 0 && cNote != cNote2 ) // don't look at single note segments
				{

					double plIOIRatio2;
					plIOIRatio2 = cNote->relScoreIOI(-1,1);
					plIOIRatio2 = normIOIratio(plIOIRatio2);

					double plIOIRatio1 = cNote->IOIratio(-1,1);
					plIOIRatio1 = normIOIratio(plIOIRatio1);

					double pIOIratio = GaussWindow(plIOIRatio1, 
						plIOIRatio2,
						0.3);

					if( pIOIratio < 0.8 )
					{


						TFrac d(1,16);
						TCLICKNOTE *pSegment;
						// adapt cNote->Prev->scoreDuration
						pSegment = dynamic_cast<TCLICKNOTE *>(cNote->GetPrev(-1));
						if( pSegment )
						{


							// we now should redo the complete segment
							/*
							TCLICKNOTE *to = cNote2->getNext(1,-1);
							while( to != pSegment )
							{
							pSegment = tempoDetection( pSegment, // start, already processed
							cNote2->getNext(1,-1), // end
							forward,
							//				curTempo,
							statList,	
							durationList,
							0, //threshold,
							pSegment->scoretime(),
							&success );
							pSegment = pSegment->getNext(1,-1);
							ClickTrack->Debug();
							}
							*/
						} // if pSegment

					} // if bad pIOIratio
				} // if not single note segment
			} // if !=
			// cNote = cNote->getNext(1,-1);
		} // for

		Printf("done\n");										
		ClickTrack->Debug();


		// get complexity  and errors
		double   *complexityList;
		double *myList,
			*sigmaList;
		{
			// calculate the error distribution for each segment
			int j;
			myList = new double[startList->count()];
			sigmaList = new double[startList->count()];
			complexityList = new double[startList->count()];
			for( j = 0; 0 && j < startList->count(); j++ )
			{
				int count = 0;

				TCLICKNOTE *startNote,
					*nextStartNote,
					*cur;
				startNote = dynamic_cast<TCLICKNOTE *>(startList->get(j));
				nextStartNote = dynamic_cast<TCLICKNOTE *>(startList->get(j+1));

				cur = startNote;
				double errorSum = 0;
				double sigmaSum = 0;


				while( cur != nextStartNote )
				{
					if( cur->IOIratio(-1,1) != 0 )
					{
						double curError = 0;
						errorSum +=  cur->IOIratioError();
						if( count )
						{
							sigmaSum += pow(errorSum/count - curError,2);
						}

						count++;
					}// if
					cur = dynamic_cast<TCLICKNOTE *>(cur->GetNext(-1));
				} // while
				myList[j] = 0;
				sigmaList[j] = 0;
				if( count )
				{
					myList[j] = errorSum/count;
					sigmaList[j] = sigmaSum/count;
				}
				complexityList[j] = denomComplexity(startNote, nextStartNote);
				if( startNote->IOIratioError() > myList[j] + sigmaList[j] &&
					complexityList[j] > 2)
				{
					// try to optimise complexity
					// select from the duration list a common duration close to
					// plIOI duration and see wht happens
					startNote->IOIratioError();
					if( startNote->IOIratio(-1,1) > startNote->relScoreIOI(-1,1) )
					{
						// note is too short
					}
					else
					{
						// note is too long

					}
					double	minComplexity = complexityList[j];
					int k;
					TDoubleBinClass *prevClass = NULL,
						*bestClass = NULL;
					// test other durations/IOIratios
					TAbsTime  prevDuration;
					prevDuration = (dynamic_cast<TCLICKNOTE *>(startNote->GetPrev(-1)))->scoreDuration();
					for( k = 0; k < statList->Count(); k++)
					{
						prevClass = statList->getBest(prevClass);
						if( prevClass )
						{
							// get new duration
							TAbsTime newDuration;
							newDuration = getDuration( prevDuration, // lgDuration of previous note
								prevClass); 
							TAbsTime durRatio =  newDuration / startNote->scoreDuration();
							double complexity = denomComplexity(startNote, nextStartNote,
								&durRatio);
							if( complexity < minComplexity )
							{
								minComplexity = complexity;
								bestClass = prevClass;
							}
						}
					} // for
					if( bestClass )
					{
						TAbsTime newDuration;
						newDuration = getDuration( prevDuration, // lgDuration of previous note
							bestClass); 
						TAbsTime durRatio =  newDuration / startNote->scoreDuration();
						// copy values into clicknotes
						TAbsTime offset;
						offset = expandDurations( startNote, nextStartNote, &durRatio);
						//						ClickTrack->Debug();
						ClickTrack->shiftScoretimes(nextStartNote, NULL, offset);
						//						ClickTrack->Debug();
					}
				} // if error
			} // for



		}

		delete [] myList;
		delete [] sigmaList;
		delete [] complexityList;

		//---------------------------------------------------		


		delete startList;
		delete stopList;
	} // block

	//ClickTrack->Debug();
	//	ClickTrack->normScoreTimes();
	ClickTrack->deleteLastClick(); // last click was generated from note offset
	// reorganize Clicktrack, remove unused click (realtime -1)
	ClickTrack->reorganize();
	// durationList->debug();
	//	statList->debug();
	string cTDebugName = filename();
	cTDebugName += ".ctr.txt";
	ClickTrack->Debug(cTDebugName);

	statList->writeIni(IOIratioFilename );
	ClickTrack->analyseQuality(filename());




	durationList->writeIni( IOIFilename );
	delete durationList;
	delete statList;

	// clicktrack self similarity
	const char *onOff = getInifile()->GetValChar("CTRACKSELFSIM","OFF","[ON|OFF] clicktrack selfsimilarity");
	if( !strcmp( onOff,"ON" ) )
	{
		Printf("Clicktrack selfsimilarity analysis ... ");
		selfSimilarity(ClickTrack->FirstNote(), 
			NULL, 
			/// window size
			4,
			/// step size
			1, ControlTrack,
			this->filename().c_str());
		Printf("done \n");
	}
}



/*
- proceed tempo detection starting [from, ..., to)
- stop/ return if problem notes have been found
return:
- to == ok, or first unproceeded note  cause by abort
*/
TCLICKNOTE * THMIDIFILE::tempoDetection ( TCLICKNOTE *prev,
										 TCLICKNOTE *to,
										 int direction,
										 //										 double curTempo,
										 TDoubleBinList *statList,
										 TFracBinList *durationList,
										 double, // threshold
										 TFrac &cAttack,
										 int *success)
{
	*success = 0;

	// store original start position
	TCLICKNOTE *from = prev;

	// init first note in list
	TFrac prevScoreDuration = prev->scoreDurationI;
	if( prevScoreDuration <= 0L )
	{
		Printf("\nERROR: prevScoreDuration <= 0!\n");
	}

	TFracBinClass *tClass = durationList->findExact( prevScoreDuration ); 	
	/* don't add anything! this might be "strange" durations from patterns 
	which should not be added to std classes */
	if( !tClass )
	{
		cout << "IOI class " <<
			prevScoreDuration.numerator() << "/" <<prevScoreDuration.denominator() <<
			"is missing in tempo detection IOI-grid!" << endl;
	}

	// in forward direction we can calculate the attack here 
	// in backward direction we need to to it later!
	if( direction == dirforward )
	{
		prev->setScoretime(cAttack);
		cAttack += prevScoreDuration;	// set start of next click to end of first note
	}

	// tempo of current note
	double curTempo = ClickTrack->tempo(prev);	
	// average (smoothed) tempo
	double avTempo = curTempo;

	// set current note
	TCLICKNOTE *cur = prev->getNext( direction, -1 );


#ifdef logTempoDetection
	FILE * logfile = fopen(TD_LOG_FNAME,"at");
#endif

	int abort = 0;
	double prevError = 0; // plScoreduration / newScoreDuration
#ifdef logTempoDetection
	fprintf(logfile,"----------------\n");
#endif		

	char inPattern = 0; // is 1 if in pattern
	double curPatternDistance = -1;
	char firstPatternStart = 1;
	while( cur && // do for all IOI's clicknotes ----------------------------
		cur != to 
		//		  && !abort 
		)	
	{
		// check for attackpoint shift errors
		TFrac gridRes(1,8);
		TFrac deltaP = cAttack % gridRes;
		TFrac deltaN = gridRes - deltaP;
		TFrac delta;
		// get distance to closest eighth note grid position
		if( prev &&
			deltaP.toDouble() >  prev->scoreDurationI.toDouble() * 0.5 )
		{	// don't cut previous note too much
			delta =  deltaN * -1;
		}
		else 
		{
			if( curTempo > avTempo * 1.3 ||
				curTempo > 240 )
			{
				delta =  deltaP;
			}
			else if( curTempo * 1.3 < avTempo  ||
				curTempo < 20)
			{
				// prev note was too short
				delta = deltaN * -1;				
			}
			else
			{ 
				if( deltaP < deltaN )
				{
					delta = deltaP;
				}
				else if( deltaP > deltaN )
				{
					delta = deltaN * -1;
				}
				else
				{
					// shift to a quarter position
					TFrac quarter(1,4);
					if( (cAttack - deltaP) % quarter == 0L )
					{
						delta = deltaP;
					}
					else
					{
						delta = deltaN * -1;
					}
				} // else
			} // else
		} // else




		// cur == head of new starting pattern
		if( cur->patternFinalID > -1 &&
			!validIOIratio(cur->patScoreIOIratio) && 
			//			primeDenomDistance( prev)  >= 1 )
			delta != 0L  )
		{	// all pattern must start at eight note positions
			// check if prevNote perfIOIratio > or < scoreIOIratio and decide where to shift
			/*
			if(firstPatternStart)
			{
			// shift scoretimes from beginning
			}
			else
			*/
			if( prev )
				prev->scoreDurationI -= delta;
			cAttack -= delta;
			firstPatternStart = 0;
		}
		else // if( cur->patternFinalID < 0  
			//			&& !validIOIratio(cur->patScoreIOIratio))
		{
			if( primeDenomDistance( prev)  >= 1)
			{

				TFrac gridRes(1,8);
				if( prev->scoreDurationI.denominator() % 3 )
				{	// binary value -> move to ternary
					gridRes = TFrac( 1,12 );
				}
				else
				{	// ternary value move to binary
					gridRes = TFrac( 1,16 );
				}				
				TFrac deltaP = cAttack % gridRes;
				TFrac deltaN = gridRes - deltaP;

				if( curTempo > avTempo * 1.5 &&
					prev->scoreDurationI > deltaP )
				{	// prevNote is too long
					delta = deltaP;
				}
				else if( curTempo * 1.5 < avTempo )
				{	// prevnote is too short
					delta = deltaN * -1;
				}
				else
				{	// ?? error was at another place
					// move to closest tempo position
					TFrac tempScDuration = prev->scoreDurationI;			
					prev->scoreDurationI = tempScDuration - deltaP;
					double pTempo = ClickTrack->tempo(prev);			
					prev->scoreDurationI = tempScDuration + deltaN;
					double nTempo = ClickTrack->tempo(prev);			
					prev->scoreDurationI = tempScDuration;

					if( fabs(pTempo - avTempo) <
						fabs(nTempo - avTempo) &&
						prev->scoreDurationI > deltaP )
					{
						delta = deltaP;
					}
					else
					{
						delta = deltaN * -1;
					}
					tempScDuration = 0L;
				}

				if( prev )
					prev->scoreDurationI -= delta;
				cAttack -= delta;
			} // if prime Denom
			else if( cAttack.denominator() > 16 ||
				curTempo > 240 )
			{
				// do something
				if( prev && 
					prev->scoreDurationI > delta )

					prev->scoreDurationI -= delta;
				cAttack -= delta;
			}
		} // else

		// --->>>> build a synchopation sum, reset if primeDenomDist == 0
		// if sum > x -> shift current score time


		// now update avtempo
		if( prev )
			curTempo = ClickTrack->tempo(prev);
		// prev must already include a score duration
		double patRatio = cur->patScoreIOIratio;
		if( validIOIratio(patRatio) &&
			prev->scoreDurationI != prevScoreDuration)
		{	// we are insiade a pattern
			double ratioDur;
			if( patRatio > 0 )
				ratioDur = prevScoreDuration.toDouble()* patRatio;
			else
				ratioDur =  prevScoreDuration.toDouble() / ( patRatio * -1);

			// get new ratio
			patRatio = ratioDur / prev->scoreDurationI.toDouble();
			if( patRatio < 1 )
				patRatio = -1.0 / patRatio;
			cur->patScoreIOIratio = patRatio;
		}
		prevScoreDuration = prev->scoreDurationI;
		avTempo = avTempo * 0.7 + curTempo * 0.3;

		if( prevScoreDuration <= 0L )
		{
			Printf("\nERROR: prevScoreDuration <= 0!\n");
			ClickTrack->Debug();
		}


		// attackpoint denominator is large and shift to another position
		// if  prevNote primeDenomDist  is large
		// or cAttack.denominator() is large and cur->durationalAccten is large
		// shift 


		if( cur->scoreDuration() == 0L && 
			direction == dirforward)
		{
			cur->setScoretime(cAttack);
		}



		// performed IOIratio
		double rIOI = cur->plRIOI(direction, 0);
		if( !cur->GetNext(-1) )
		{
			// there is no valid IOIratio for the least note
			double ioiPC = cur->ioi(-1,-1).toDouble();
			double ioiCN = cur->plDuration();
			rIOI = IOIratio( ioiPC, ioiCN);
		}
		// check size of IOIratio, skip small notes -> IOI < -4
		if( cur->patternFinalID < 0 ) 
		{			
			if( rIOI < -9 )
			{
			}			
			else if( rIOI > 5 )
			{
			}
		} // skip block


		// played score lgDuration = rIOI * prevScoreDuration
		// calc played score duration from prev lgDuration and playedIOI
		TFrac plScoreDuration; 
		if( rIOI < 0 )
			plScoreDuration = TFrac((long)(-960 * prevScoreDuration.toDouble() / ( rIOI ) + 0.5), 960L);
		else
			plScoreDuration = TFrac((long)(960 * prevScoreDuration.toDouble() * ( rIOI ) + 0.5), 960L);

		// convert IOI range to notestat range
		//---------- val = played rIOI
		double val = normIOIratio( rIOI );
		if( validIOIratio(cur->patScoreIOIratio) )
		{
			// if inside pattern used ioiRatio given by pattern
			inPattern = 1;
			val = normIOIratio( cur->patScoreIOIratio );
		}
		else if( inPattern ) 
		{
			// this is first note after pattern
			// abort always when leaving a pattern ?
			//			abort = 1;
			inPattern = 0;
			//			*success = 1;
			// we will get an abort if the uncertaincy gets too high
		}

		if( cur->patternFinalID > -1 ) 
		{
			curPatternDistance = (cur->bestPatternList[cur->patternFinalID]).distance;
		}
		else if( validIOIratio( cur->patScoreIOIratio ) )
		{
			// do nothing
		}
		else // not in pattern
		{
			curPatternDistance = -1;
		}


		/// get stat IOIClass
		TDistanceStruct IOIratio1d = statList->closestClass(&val);		

		// calc duration1 from [IOIratio1]
		TDoubleBinClass *tempClass = IOIratio1d.classPtr; // closest IOI class				

		/// val->[val]->[IOIclassDuration]
		TAbsTime scoreDuration1 = getDuration( prevScoreDuration, // lgDuration of previous note
			tempClass); // current IOIClass

		if( scoreDuration1 <= 0L )
		{
			cout << "ERROR: ScoreDuration1 <= 0 (" << val<< ")"
				<< scoreDuration1.toString() << " " << prevScoreDuration.toString() << endl;
		}		
		if( scoreDuration1.denominator() >= 24 ||
			scoreDuration1 <= 0L ) // no prototype!
		{
			// small notes should have been filtered out
			// quantise the IOIList duration 
			TDistanceStruct tempD = durationList->closestClass(NULL, 
				&scoreDuration1);
			scoreDuration1 = dynamic_cast<TFracBinClass*>(tempD.classPtr)->duration();

			// get the new IOIratio
			double tIOIratio = scoreDuration1.toDouble() / prevScoreDuration.toDouble();
			if( tIOIratio < 1 )
				tIOIratio = -1/tIOIratio;
			tIOIratio = normIOIratio(tIOIratio);
			// get corrected distance Struct
			IOIratio1d = statList->closestClass(&tIOIratio);	
			val = tIOIratio;

		}
		/// quantised performed IOIratio


		/// get [scoreDuration2] ---------------------------------------------
		if( cur->patternFinalID > -1 )
		{
			plScoreDuration = cur->prelScDuration;
		}

		TDistanceStruct scoreDuration2d = durationList->closestClass(NULL, 
			&plScoreDuration);
		TFrac scoreDuration2 = dynamic_cast<TFracBinClass*>(scoreDuration2d.classPtr)->duration();
		if( scoreDuration2 <= 0L )
		{
			Printf("ERROR: ScoreDuration2 <= 0");
			Printf(scoreDuration2);
		}		



		double IOIDist = IOIratio1d.distance;
		double durDist = scoreDuration2d.distance;



		// check distance to closest classes
#define DISTANCELIMIT 0.27
		if( IOIDist > DISTANCELIMIT &&
			durDist > DISTANCELIMIT &&
			cur->patternFinalID < 0)
		{

			//todo:  we need a concept for adding new classes 
		}

		char forceIOI = 0;

		if( IOIDist > 0.8 &&
			durDist > 0.8 &&  
			cur->patternFinalID < 0 && 
			cur->patScoreIOIratio == 0)
		{
			// high distance, ask user for input
			if( !strcmp(getInifile()->GetValChar("MODE",
				"INTERACTIVE",
				"[INTERACTIVE|SILENT] use SILENT for batch processing "),
				"INTERACTIVE") )
			{
				ClickTrack->Debug("_clicktrack.txt");
				durationList->write(&cout);
				statList->write(&cout);
			}
			string ioiStr;
			ostringstream prompt;
			prompt << "Please enter prototype for unknown IOIratio ";
			prompt << denormIOIratio( val );
			prompt << " at ";
			prompt << cur->GetAbsTime().numerator();
			prompt << "/";
			prompt << cur->GetAbsTime().denominator();
			prompt << "\n";

			ioiStr = InputQuestion(prompt.str().c_str(),
				"0", NULL);
			double fIOIratio;
			fIOIratio = saveAtof(ioiStr.c_str());
			if( validIOIratio( fIOIratio ) )
			{
				fIOIratio = normIOIratio( fIOIratio );
				IOIratio1d.classPtr =  statList->addIfNew( fIOIratio );
				IOIratio1d.distance = 0;
				/// val->[val]->[IOIclassDuration]
				tempClass = IOIratio1d.classPtr;
				scoreDuration1 = getDuration( prevScoreDuration, // lgDuration of previous note
					tempClass); // current IOIClass
				forceIOI = 1;
			}
			else // no prototype given
			{
				// restart with typical IOI
				// abort = 1;
				// go ahead
			}
		} // high distance


		//---------------------------------------------------------
		// optimisation code is in old cvs files!!



		// flag for range error of rIOI
		int inRange = 1;
		/// seems to be obsolete
		if( !inRange &&
			scoreDuration1 == scoreDuration2 &&
			scoreDuration1.denominator() < 32 )
		{
			inRange = 1; // use the values
		}
		/// the inferred score duration
		TFrac newCDuration;
		if( inRange &&
			//			!abort &&
			cur->patternFinalID < 0 &&
			!validIOIratio(cur->patScoreIOIratio) ) // use pattern IOIratio?
		{

			if( IOIDist < 0.1 && 
				durDist < 0.1 )
			{
				// ambigious situation -> select duration for best fit tempo
				// get resulting tempo
				newCDuration = cur->scoreDurationI;			
				cur->scoreDurationI = scoreDuration1;
				double IOITempo = ClickTrack->tempo(cur);			
				cur->scoreDurationI = scoreDuration2;
				double durTempo = ClickTrack->tempo(cur);			
				cur->scoreDurationI = newCDuration;

				if( fabs(IOITempo - avTempo) <
					fabs(durTempo - avTempo) )
				{
					newCDuration = scoreDuration1;
				}
				else
				{
					newCDuration = scoreDuration2;
				}
				if( newCDuration <= 0L )
				{
					Printf("ERROR: newCDuration <= 0!!!\n");
					Printf(newCDuration);
				}		

			} // if small distance
			else
			{
				// calc GaussDistance bet plDuration and both scoreDurations
				// use typical sigma values!!

				double IOISigma = statList->errorSigma();
				double durSigma = durationList->errorSigma();

				IOISigma = 0.3;
				durSigma = 0.0625;
				// todo: replace sigmas by calculated values?
				double pIOI = GaussWindow(val,
					IOIratio1d.classPtr->FirstV(),
					IOISigma);
				double pDur = GaussWindow(plScoreDuration.toDouble(),
					scoreDuration2d.classPtr->FirstV(),
					durSigma);

				// use binClass Distance instead of GaussDistance?
				/*
				if( durDist < 0.1 )
				pDur = 1 - durDist;
				if( IOIDist < 0.1 )
				pIOI = 1 - IOIDist;
				*/


				double pIOI2 = pIOI * IOIratio1d.classPtr->weight();
				double pDur2 = pDur * scoreDuration2d.classPtr->weight();
				/*
				double pDur2 = (1-durDist);
				double pIOI2 = (1-IOIDist);
				*/
				if( forceIOI )
				{
					pDur = 0;
					pDur2 = 0;
					pIOI = 1;
					pIOI2 = 1;
				}
				// use just the output of the binClass

				if( pDur > 0.7 &&
					pIOI > 0.7 &&
					scoreDuration1 == scoreDuration2 )
				{
					newCDuration = scoreDuration1;
				}
				else if( pDur2 > pIOI2 &&
					pDur > 0.8 )
				{
					newCDuration = scoreDuration2;
				}
				else if( pIOI2 > pDur2 &&
					pIOI > 0.8 )
				{
					newCDuration = scoreDuration1;
				}
				else if( IOIDist < DISTANCELIMIT &&
					pIOI > 0.9 )
				{
					newCDuration = scoreDuration1;
					*success = 1; // just mark startstop and come back
					abort = 1; // safety abort
				}
				else if( durDist < DISTANCELIMIT &&
					pDur > 0.9)			         
				{
					newCDuration = scoreDuration2;
					*success = 1;  // just mark startstop and come back
					//					abort = 1; // safety abort
				}
				else			         
				{
#ifdef _DEBUG
					// real error abort
					///				ClickTrack->Debug();
					//				durationList->write(stdout, NULL, &plScoreDuration);
					//				statList->write(stdout, &val);
					//				statList->debug();
#endif
					// abort = 1;
					// duration class won't give too strange durations
					// Todo other strategy?
					newCDuration = scoreDuration2; 
					/* use this ?
					double mod;
					ClickTrack->Debug();
					mod = (plScoreDuration.toDouble()+0.5) * 32;
					newCDuration = TFrac(mod, 32);
					*/
				}
				if( newCDuration <= 0L )
				{
					Printf("ERROR: newCDuration <= 0!!!!\n");
					Printf(newCDuration);
					Printf(scoreDuration1);
					Printf(scoreDuration2);
				}		
			} // else if IOIDist != durDist

		} // if not inside pattern
		else if( validIOIratio(cur->patScoreIOIratio) ) // we are inside pattern
		{
			// force the usage of pattern IOI ratio, it might be not in the list!!
			double patRatio =  cur->patScoreIOIratio;
			/*
			tempClass = IOIratio1d.classPtr;
			// calc duration with class-IOI
			newCDuration = getDuration( prevScoreDuration, // lgDuration of previous note
			tempClass); // selected IOIClass
			*/
			TFrac patRatioFrac;
			if( patRatio > 0 )
			{
				patRatioFrac = TFrac((long)(patRatio * 960 + 0.5), 960L);
				newCDuration = prevScoreDuration *patRatioFrac;
			}
			else
			{
				patRatioFrac = TFrac((long)(-patRatio * 960 + 0.5), 960L);
				newCDuration = prevScoreDuration / patRatioFrac;
			}
			if( newCDuration <= 0 )
			{
				Printf("ERROR: newCDuration <= 0!");
				Printf(newCDuration);
			}
			val = normIOIratio(cur->plRIOI(direction, 0));
		}
		else if( cur->patternFinalID > -1 )
		{
			// start of new pattern
			TPATTERN *pat = cur->bestPatternList[cur->patternFinalID].ptr;
			TPNOTE *pNote = pat->FirstNote();
			newCDuration = TFrac(pNote->duration());
		}
		else // not in range, how should that happen?
		{
			double mod = plScoreDuration.toDouble() * 16 + 0.5;

			newCDuration = TFrac((long)mod, 16L);
			if( newCDuration <= 0 )
			{
				Printf("ERROR: newCDuration <= 0!!");
				Printf(newCDuration);
			}

			{
				inRange = 1;	// perfect pl. Duration
			}
			// todo skip one more note
		} // else



		// check for illegal, small denomimator
		if( newCDuration.denominator() > 32 )
		{
			TFrac sfourth(1,64);
			if( newCDuration <= sfourth )
			{
				Printf("Warning: increased duration to 1/4!\n");
				newCDuration = TFrac(1,4);
			}
			else
			{
				Printf("\n Warning: quantised duration to 1/32!\n");
				double mod;
				ClickTrack->Debug();
				mod = (plScoreDuration.toDouble()+0.5) * 32;
				newCDuration = TFrac((long)mod, 32L);
			}
		} // if < 1/64

		/// collision between pattern value and newCDuration
		if( cur->patternFinalID > -1 &&
			cur->prelScDuration > 0L &&
			cur->prelScDuration != newCDuration )
		{
			// abort? because pattern start does not fit!!

			double pBinDur,
				pPatDur;
			// todo: replace sigmas by calculated values?
			double sigma = durationList->errorSigma();
			sigma = 0.0625;
			pBinDur = GaussWindow(log(plScoreDuration.toDouble()),
				log(newCDuration.toDouble()),
				sigma);
			pPatDur = GaussWindow(log(plScoreDuration.toDouble()),
				log(cur->prelScDuration.toDouble()),
				sigma);
			// --> restart with duration of first pattern note!
		}
		/// for safety reasons
		if( newCDuration == 0L )
			abort = 1;


		/// store for eventual reset
		TFrac old = cur->scoreDurationI;

		/// copy newCDuation into current clicknote
		double mSignificance = 0;
		//		if( !abort ||
		//			*success )
		{
			TFrac thirtytwoth(1,32);
			if( newCDuration <= 0L )
			{
				Printf("ERROR: zero CDuration!\n");
				newCDuration = TFrac(1,4);
			}
			else if( newCDuration < thirtytwoth )
				cout << "Warning: small duration " <<
				newCDuration.numerator() <<"/"<<
				newCDuration.denominator() <<endl;

			// error detection for cemgil files
			if( cur->testScoreIOI() > 0L &&
				cur->testScoreIOI() != newCDuration )
			{
				// ClickTrack->Debug();
			}   

			// set score duration for click note

			cur->scoreDurationI = newCDuration;			

			double newTempo = ClickTrack->tempo(cur);				
			// match significance
			if( curPatternDistance > -1 &&
				cur->patternFinalID > -1 )
			{
				// we are inside a pattern
				mSignificance = pSignificance(curPatternDistance);
			}		
			else // significance depends on tempo change
			{
				// +/- 30pbm == 0.6 significance
				mSignificance = GaussWindow(curTempo, newTempo, 30);		
			}

			if( newTempo <= 0 )
			{
				printf("ERROR: illegal tempo change (%f)!\n",
					newTempo);
				ClickTrack->tempo(cur);
				ClickTrack->Debug("_clicktrack.txt");
				abort = 1;
			}
			else if( newTempo   < 40 )
			{
				ClickTrack->Debug();
				if( !inPattern &&
					cur->patternFinalID < 0 )
				{	
					// abort = 1;
					// double typical durations
					durationList->typicalQuarterIOI /= 3;
				}
				mSignificance = 0;
				// see evaluation of mazurka6
			}
			if( newTempo > 200 )
			{
				ClickTrack->Debug();
				if( !inPattern &&
					cur->patternFinalID < 0 )
				{	
					// abort = 1;
					// double typical durations
					durationList->typicalQuarterIOI *= 3;
				}
				mSignificance = 0;
				//					abort = 1;
			}
			curTempo = newTempo;
		} // if !abort



		// update duration and IOI lists
		{

			double qRIOI;
			qRIOI = IOIratio(prevScoreDuration.toDouble(), newCDuration.toDouble() );
			if( validIOIratio( qRIOI ) )
			{
				qRIOI = normIOIratio( qRIOI );
				IOIratio1d = statList->closestClass(&qRIOI, NULL );
				if( IOIratio1d.distance < 0.1 )
				{	// add only if already exists
					statList->addValue(val,
						IOIratio1d.classPtr);

					// add also the inverse qRIOI
					if( qRIOI != 0)
					{
						IOIratio1d = statList->closestClass(&qRIOI, NULL );
						statList->addValue(-qRIOI,
							IOIratio1d.classPtr);
						// statList->addExact(-qRIOI);
					}
				} // if exists
			} // if valid IOIratio

			// add plScoreDuration to closest class pf newCDuration 
			scoreDuration2d.classPtr = durationList->findExact(newCDuration);
			if( scoreDuration2d.classPtr )
			{ 	// add only if already known
				durationList->addValue(plScoreDuration,
					scoreDuration2d.classPtr);
			}
			// update the current normalisation
			durationList->updateTypicalDurations( newCDuration,
				cur->ioi(1,-1).toDouble(),
				mSignificance );
		}// if inRange and ! abort


		{					
			// check for errors

			prevError = plScoreDuration.toDouble() / newCDuration.toDouble();
			prevScoreDuration = newCDuration;

			// todo !!! check for negative direction!!
			if( direction == dirforward )
				cAttack = cAttack + prevScoreDuration;
			else
			{
				cAttack = cAttack - prevScoreDuration;
				cur->setScoretime(cAttack);
			}

			// shift to next position
			TCLICKNOTE *next = cur->getNext(direction, -1);

			prev = cur;
			cur = next;
			if( cur )
			{
				next = cur->getNext(direction, -1 );
			}
			if( direction == dirforward &&
				!next )
			{
				cur = NULL; // abort
			}
		} // else (abort)
	} // while for all IOI's in range
#ifdef logTempoDetection
	fclose(logfile);
#endif

	/*
	#ifdef _DEBUG
	fclose(out);
	#endif
	*/

	TAbsTime delta;
	TFrac bestDuration(1,8);
	TFrac resolution(1,32);
	/*
	double oldQuality,
	newQuality;
	*/
	if( direction == backward )
	{
		if( cur &&
			cur->startId <= 0 )
			cur->startId--;
		// shift scoretime of startelement to 0
		// cur->setScoretime(cAttack);
		{
			// shift to zero
			ClickTrack->shiftScoretimes(prev,
				from->getNext(dirforward,-1),
				prev->scoretime() * -1);
		} // if cAttack < 0
	}
	else if( cur &&
		direction == dirforward )
	{
		if( cur->startId <= 0 )
			cur->startId--;
	}

	ClickTrack->Debug();
	return prev;

	typedef struct{ 
		int cVals;
		double inVal;	// like played
		double outVal;   // like written
		double minVal;
		double maxVal;
	} TstatCount;






	//	delete TIOIratioList;

	// -------------------------------------------------------------------------------------



	// create IOIlist/movementlist of tracks
	/*

	TIOIList *ioiList;
	TIOIratioList *TIOIratioList;

	ioiList = new TIOIList(this);
	// create TIOIratioList
	TIOIratioList = new TIOIratioList(ioiList);

	#ifdef _DEBUG
	FILE *tempOut;
	tempOut = fopen("_ioiList.txt","wt");
	fprintf(tempOut, "as played -----------\n");
	TIOIratioList->write(tempOut);
	#endif







	// quantize list values to main relations
	ioiList->preQuantize(ClickTrack);


	#ifdef _DEBUG
	fprintf(tempOut, "after preQuantize -----------\n");
	ioiList->write(tempOut);


	fclose(tempOut);
	#endif
	delete [] TIOIratioList;
	delete [] ioiList;
	*/



	// divide lists into segments
	// ------- do pattern matching for segments

	// read pattern file
	TPFILE *patternFile;
	patternFile = new TPFILE(DEF_PATTFILE);
	patternFile->Read();


	TPatternList *patternList; // array of patternnumbers
	/*
	patternList = patternFile->compare(ioiList,
	TIOIratioList,
	this->ClickTrack,
	&listSize);
	*/
	// todo finish create clicktrack by patternFile
	/*
	//? recalc attacks and durations according to matches
	//? create clicktrack according to matches
	//	ClickTrack = createClickTrack( patternList,
	patternFile,
	ioiList,
	TIOIratioList,
	this );
	*/
	patternFile->write(NULL,NULL);
	delete [] patternList;
	delete patternFile;
#endif
#ifdef _OLD_TEMPODETECTION

	TDoubleBinList     *AttackList = NULL;
	TDoubleBinClass   *Ptr;

	TCLICKNOTE *Click,
		*NextClick,
		*LastProblem,
		*PrevBar,		// 'One' of previous bar
		*FirstOne, 		// metronome beat at first 'One'
		*SecondOne;		// metronome beat at second 'One' == bar 2


	int  /* voice, */
		TimeClass;

	TAbsTime	  PartsPerNote,
		PrevPartsPerNote;
	TAbsTime RealTime,
		PrevRealTime,   // Attackpoint der vorherigen Note
		PreBar,		// Differenz zum vorherigen Takt
		NextBar,		// Differenz zum nächsten Takt
		Diff;		// Differenz zwischen NowN und NextN

	long //PlayTimeNow,	// gespielter Attackpoint von NowN
		//PlayTimeNext,	// gespielter Attackpoint von NextN
		PrevPlayDiff,
		//	     lgDuration,
		PlayDiff;

	double PPQInFirstBar;

	double Quot;
	int BeatIntens;		// Anschläge >= dieser Stärke werden als Taktbeginn erkannt


	Printf( "Start<CreateClickTrack>\n");

	if( MeterSig == 0L )
	{
		Printf("Error: Create ClickTrack::barlength == 0!\n");
		return;
	}
	if( ClickTrackNr < 0 )	// Nur sinnvoll wenn keine Metronomspur existiert
	{
		//		Taktmetrik = taktlaenge / RecPPQ;

		AttackList = new TDoubleBinList( 142,	// obere Schranke [%]
			75 ); 	// untere Schranke (%)

		//		AttackList = new TDoubleBinList( 110,	// obere Schranke [%]
		//											90 ); 	// untere Schranke (%)
		// Klassenliste der Attackpoints anlegen und Metronomspur erzeugen
		Statistik( MeterSig,
			AttackList );

		// Jetzt wird PPQ in FirstBar berechnet
		mAuftakt  = 0;
		BeatIntens = ClickTrack->GetBeatIntens();
		Click     = ClickTrack->FirstNote();
		FirstOne  = NULL;
		SecondOne = NULL;


		// Suche die 1 des ersten und zweiten Taktes
		while( Click && (SecondOne  == NULL) )
		{
			if( Click->getIntens() > BeatIntens )
			{
				if( !FirstOne )
				{
					FirstOne = Click;
				}
				else
				{
					SecondOne = Click;
				}
			} // if Intens
			Click = Click->Next();
			if( !FirstOne )	// wenn die erste Note nicht betont ist -> Auftakt
				mAuftakt = 1;
		} // while

		if( !SecondOne )	// es gibt keine zweite betonte 1
		{
			ErrorMsg( 30 );
			return;
		}

		// Jetzt ist die Länge des ersten Taktes bekannt
		Diff = SecondOne->Playtime() - FirstOne->Playtime();
		if( MeterSig == 0L )
		{
			Printf("CreateClickTrack Taktmetrik == 0\n");
			PPQInFirstBar = MAXINT;
		}
		else
		{
			PPQInFirstBar = (Diff.toDouble() / MeterSig.toDouble())/4;
		}

		// Jetzt werden den Klassen Notenwerte zugeordnet
		AttackList->FindQuarter( PPQInFirstBar ,
			RecPPQ );


		PlayDiff = Diff.toDouble();				 // Ein ganzer Takt
		PartsPerNote = MeterSig;	// Ein ganzer Takt;

		PrevPlayDiff     = 1;
		PrevRealTime     = -1;

		Quot         = MAXLONG;

		RealTime         = 0L;	// Aktueller Zeitpunkt

		AttackList->Reset();	// Liste zurücksetzen

		PreBar = 0L; // der erste Volltakt beginnt bei 0
		PrevBar = NULL;
		Click       = ClickTrack->FirstNote();
		LastProblem = Click;
		NextClick   = Click->Next();
		while( Click )
		{
			// check if click is to 1 o a new bar
			if( Click->getIntens() > BeatIntens ) // Betonung auf die 'Eins'
			{
				PreBar  = RealTime % MeterSig;		// Differenz zur 'Eins' des vorherigen Taktes
				NextBar = MeterSig - PreBar;	   // Differenz zur 'Eins' des naechsten Taktes
				if( NextBar > PreBar )	// calculate shift size
				{
					// CLick is close to previous bar
					Diff = PreBar * -1;
					if( (RealTime + Diff) <= PrevRealTime )
					{
						Diff = NextBar;
					}
				}
				else // CLick is close to following bar
				{
					Diff = NextBar;
				}
				// Diff == |Click.Realtime NextBar|
				// Diff != 0 only at errors

				//	Die Auftaktnoten müssen verschoben werden
				if( mAuftakt > 0 )
				{
					Diff = NextBar;	// Bei Auftakt immer nach hinten schieben
				} // if


				if( mAuftakt ||
					(Diff > 0L &&
					//					 (Diff < (RecPPQ / 2)) )
					(abs(Diff) < TFrac(1,8)) )
					)	// wenn 0 < Fehler < als ein Achtel
				{

					if( !mAuftakt )
					{
						ClickTrack->ShiftRealtimes( LastProblem, // Ab hier
							Click,			// bis zu dieser Note
							Diff );			// Verschiebung
					}
					else
					{
						ClickTrack->ShiftRealtimes( ClickTrack->FirstNote(), // Ab hier
							Click,			// bis zu dieser Note
							Diff );			// Verschiebung
					}
					RealTime += Diff;
					// Click ist eindeutig und kann nicht mehr verschoben werden
					upBeatI = 0; //wird nur einmal beachtet
					LastProblem = Click->Next();
				} // if
				else if( PrevBar &&
					Diff > 0L ) // Der Fehler ist größer als eine Achtelnote
				{
					// PrevBar == CLick at 1 of previous bar

					Diff = (Click->Playtime() - PrevBar->Playtime());
					// Diff == |PrevBar Click| == Barlength

					PPQInFirstBar = (Diff.toDouble() / MeterSig.toDouble())/4;

					// Die Attackpoints in diesem Takt werden jetzt nach diesem Wert berechnet
					// keep 1 of prevbar fixed
					LastProblem = PrevBar->Next();
					//					Quot = PPQInFirstBar / RecPPQ;
					Quot = PPQInFirstBar;
					RealTime = PrevBar->Realtime();
					while( LastProblem != Click )
					{
						Diff = LastProblem->Playtime() - PrevBar->Playtime();
						myAssert( !(Quot == 0) );
						Diff /= Quot;
						LastProblem->SetRealtime( RealTime + Diff );
						LastProblem = LastProblem->Next();
					} // while !=


					// set Realtime to 1 of next bar
					RealTime = RealTime + MeterSig;

					LastProblem = Click->Next();
				} // else if
				PrevBar     = Click;     // Hier beginnt ein Takt
			} // if BEATINTENS

			//			#pragma warn -sig
			// Die Nummer der Klasse wurde in Realtime gespeichert
			// Die Klasse gehört zum Abstand Click - Click->Next()
			TimeClass = Click->Realtime().toLong();
			//			#pragma warn +sig

			Click->SetRealtime( RealTime );

			Ptr  = AttackList->Find( TimeClass );
			// Ptr Zeigt auf die "TimeClass" für den Abstand Click zu NextClick
			PrevPlayDiff     = PlayDiff;     	// Abstant Playtime
			PrevPartsPerNote = PartsPerNote;		//
			PrevRealTime     = RealTime;			//

			if( Ptr )
				PartsPerNote = Ptr->PartsPerNote();
			else
				PartsPerNote = 0L;

			if( NextClick )
			{
				PlayDiff = NextClick->Playtime() - Click->Playtime();
				if( PartsPerNote == 0L)	// Der Klasse konnte kein Notenwert zugeordnet werden
				{
					LastProblem = Click->Next();	// Der nächste Attackpoint könnte falsch sein
					// den Wert von der vorherigen Note übernehmen
					if( PlayDiff == 0 )
					{
						Printf("THMIDI::CreateClickTrack PlayDiff == 0\n");
						Quot = 0;
					}
					else
					{
						Quot = PrevPlayDiff / PlayDiff;
					}
					if( !(Quot == 0) )
					{
						PartsPerNote = PrevPartsPerNote * (1/ Quot);
					}
					else	// Annahme die Note ist genausolang wie ihr Vorgänger
					{
						PartsPerNote = PrevPartsPerNote;
					}
				} // if
			} // if NextClick

			RealTime += PartsPerNote;

			Click     = NextClick;
			if( Click )
				NextClick = Click->Next();
		} // while
		//		ClickTrack->CreateTempoTrack( (TQTRACK *)Track[0],
		// Check for double call at ToGMN !!!!!
		ClickTrack->CreateTempoTrack( (TQTRACK *)ControlTrack,
			/* !!!!!*/				      RecPPQ );
	} // if ClickTrack < 0

	delete AttackList;

	FILE *tFile;
	tFile = fopen("ctrack2.txt","wt");
	ClickTrack->Write(tFile);
	fclose(tFile);
	Printf( "End<CreateClickTrack>\n");
#endif
} // CreateClickTrack

//-----------------------------------------------------------------
//-------------------------------------------------------------------



#ifdef OLD_SETTEMPO
void THMIDIFILE::SetTempo( void )
{
	if( ClickTrack )
		ClickTrack->CreateTempoTrack( (THTRACK*)ControlTrack,
		RecPPQ );

	TQMIDIFILE::SetTempo();
} // SetTempo
#endif

char THMIDIFILE::ToGMN( ostream &out )
{
	/*
	if( ClickTrack  )
	ClickTrack->CreateTempoTrack( (THTRACK*)ControlTrack,
	RecPPQ );
	todo turn on create tempo track in HMIDI::ToGmn
	*/
	inferDynamics();
	return TQMIDIFILE::ToGMN(out);
}
//-------------------------------------------------------------------
TTRACK *THMIDIFILE::CreateNewTrack(
								   long offset ) // offset at MIDI file
								   /*
								   factory function for new track
								   */

{
	THTRACK *temp = new THTRACK( offset, this );
	return temp;
}
//-------------------------------------------------------------------
int THMIDIFILE::SetClickTrackNr( int i )
/*
i = tracknr, SMF1: 1,..,16;  SMF0: 0,1
off = -1
result
1 : Track[i] exists
0 : Track[i] doesn't exist
*/
{
	// search for Track[i]
	TTRACK *temp = EvTracks;
	while( temp &&
		temp->TrackNumber() != i )
	{
		temp = temp->Next();
	}

	// Track[i] found?
	if( temp )
	{
		if( ClickTrackNr )
			mDirty = 1;
		ClickTrackNr = i;
		return 1;
	}
	// not found
	ClickTrackNr = i;
	return 0;
};
//-------------------------------------------------------------------
int THMIDIFILE::SetClickTrackChannel( int i )
/*
result:
1 : At Track[ClickTrackNr] exist events of Channel i
0 : At Track[ClickTrackNr] exist no events of Channel i

remarks:
SetClickTrackNr must be called before
*/
{
	ClickTrackChannel = i;

	if( ClickTrackNr < 1 ) // ClickTrackNr not set
		return 0;

	// search for Track[i]
	TTRACK *temp = EvTracks;
	while( temp &&
		temp->TrackNumber() != i )
	{
		temp = temp->Next();
	}

	// search for channelfilter track
	while( temp &&
		temp->TrackNumber() == ClickTrackNr &&
		temp->Channel() != i )
	{
		temp = temp->Next();
	}
	// found?
	if( temp &&
		temp->TrackNumber() == ClickTrackNr &&
		temp->Channel() != i )
	{
		ClickTrackChannel = i;
		return 1;
	}
	// error
	return 0;
};

/*
Convert midi file into gmn.
Quantization must be done before
result:
1 : ok
0 : error
*/
char THMIDIFILE::Convert( ostream &gmnOutput,
						 const string &midiFilename )
{

	// -------------------------------------------------------

	cout << "Start conversion to stream " << endl;


	gmnOutput << "%%This file was generated by " << getVersion() << endl;
	gmnOutput << "%%Please report all problems to kilian@noteserver.org\n";
	gmnOutput << "%%Please visit http://www.noteserver.org for more information on GUIDO MusicNotation.\n";


	gmnOutput << "{";

	// write key
	TMetaKey *key = NULL;

	// later: write master control track
	TInifile *tempIni = getInifile();
	if( tempIni )
	{
		const char *detectVal = tempIni->GetValChar( "DETECTKEY", "MIDIFILE", "key detection [MIDIFILE|DETECT|ON|OFF]" ); 
		if( !strcmp(detectVal,"ON" ) ||
			!strcmp(detectVal,"DETECT" ) )
		{
			// delete existing key-sigs
			TMetaKey *tempKey = ControlTrack->FirstKey();
			ControlTrack->deleteSameType( tempKey, NULL );
			/// todo: detect key changes			
			AccidentalsI = DetectKey(this);
			key = new TMetaKey( 0L, // attack
				AccidentalsI,
				0 ); // minor/major == undef
		}
		// check for available meter information

		TMetaMeter *tempMeter = ControlTrack->FirstMeter();
		if( !tempMeter )
		{
			Printf("MIDI-File contains no meter information!\n");
			detectMeter(0 /* score time mode */);
			meterPhaseShift();
		}
		const char *val = tempIni->GetValChar( "DETECTMETER", "MIDIFILE", "meter detection [MIDIFILE|DETECT|ON|OFF]" ); 
		if( !strcmp(val, "ON") ||
			!strcmp(val,"DETECT") )
		{
			meterPhaseShift();
		}

	} // if ini
	// create new TMetaKey and insert in control track

	if(ControlTrack && key)
		ControlTrack->Insert(key);
	else if( !ControlTrack )
	{
		Printf("h_midi.cpp Control Track is missing!\n");
		if( key )
			delete key;
	}
	// merge tracks, setQData, ...
	finaliseQuantisation();
	char res = ToGMN( gmnOutput ); // TQMIDIFILE

	gmnOutput <<  "}" ;
	if( midiFilename.length() > 0 )
		writeMIDI( midiFilename );
	Printf( "Conversion completed\n" );
	return res;
}





/*
copy all attackpoints of all tracks into a new clicktrack
Todo replace by createAttacklist !!
*/
TCLICKTRACK *THMIDIFILE::mergeToClickTrack()
{	
	// determine duration structure
	double minDuration  = -1,
		maxDuration  = 0;
	double minIntens = 128;

	// analyse durations of track----------------------
	TTRACK *track = FirstTrack();
	while( track )
	{
		TNOTE *note = track->FirstNoteObject(-1);
		while( note )
		{
			if( RecPPQ <= 0 ) // input was a .gmn file
			{
				note->SetAbsTime( note->GetAbsTime() * (960L *4) );
				note->SetDuration( note->GetDuration() * (960L*4) );
			}
			if( note->getIntens() < minIntens )
				minIntens = note->getIntens();

			// fDuration = note->GetDuration().toDouble();
			double fDuration = note->ioi(1, note->GetVoice() ).toDouble();
			if( fDuration <= 0 ) // no IOI available -> last note
			{
				fDuration = note->GetDuration().toDouble();
			}
			if( fDuration > maxDuration )
				maxDuration = fDuration;
			else if( minDuration < 0 ||
				fDuration < minDuration )
				minDuration = fDuration;

			note = NOTE(note->GetNext(-1));
		} // while
		track = track->Next();
	} // while
	if( RecPPQ <= 0)
	{
		RecPPQ = 960;
		tickTiming = 1;
	}
#define durationWeightLimit 0.2
	double norm = (minDuration/durationWeightLimit) - minDuration;

	// add all notes and offset of last note to clicktrack
	TCLICKTRACK *newClicktrack = new TCLICKTRACK(RecPPQ,
											RecTempo,
											NULL,
											this);
	TFrac lastOffset; // noteOff of prev note
	track = FirstTrack();
	while( track )
	{
		TNOTE *note = track->FirstNoteObject(-1);
        TCLICKNOTE *endClick = NULL;
		while( note )
		{
			int lastIntens = note->getIntens();
			// duration = note->GetDuration();
			TFrac duration = note->ioi(1, note->GetVoice() );
			if( duration <= 0L )
				duration = note->GetDuration();

			// weight depends on duration and intensity
			// minWeight = minDur / (minDur + norm);
			// norm = (minDur - minDur*minWeight)/minWeight

			double weight = duration.toDouble() / (duration.toDouble() + norm); // range = 0...oo -> 0...1
			weight = weight + ((1-weight)*((lastIntens-minIntens)/512)); // range = 0..128 -> 0.5...1

			// 0.9/0.1 -> 0.9 + 0.1*0.1 = 
			//			-> 0.1 + 0.9*0.9 = 


			TCLICKNOTE *click = new TCLICKNOTE( note->GetAbsTime().toLong(),
                                    note->GetAbsTime(),
                                    lastIntens,
                                               //				weight, // weight
                                    duration); // duration
			endClick = new TCLICKNOTE( note->GetAbsTime().toLong()+
                                       note->GetDuration().toLong(),
                                               note->GetAbsTime()+note->GetDuration(),
                                               lastIntens,
                                               //				weight, // weight
                                               duration); // duration
			if( note->createTag )
			{
				click->createTag = note->createTag;
			}
			//			click->testScorePos = note->testScorePos;

			TQChord *tempChord = dynamic_cast<TQChord *>(note);
			if( tempChord )
			{
				click->cNotes = tempChord->cNotes();
			}
			// todo calc weight for clicknote 

			newClicktrack->Insert(click); // check for double trigger will be done in here
			//			lastOffset = note->GetAbsTime() + note->GetDuration();
			note = NOTE(note->GetNext(-1));
		} // whilte notes in track		
        if( endClick != NULL )
        { // insert click for the very last offset of the track
           
            newClicktrack->Insert(endClick); // check for double trigger will be done in here
        }
		track = track->Next();
	} // while tracks
#ifdef OLD_STRAT

	// add a virtual click note at end of track-------------------
	TCLICKNOTE *click = newClicktrack->FirstNote();
	if( !click )
	{	
		return NULL;
	}
	// search for last click
	while( click->GetNext(-1) )
	{
		click = dynamic_cast<TCLICKNOTE *>(click->GetNext(-1));
	}

	if( click->GetPrev(-1) )
	{
		long lastIOI = click->GetAbsTime().toLong() - 
			click->GetPrev(-1)->GetAbsTime().toLong();
		long minOffset = click->plOffset();

		// set last clicknote to onset of last clicknote + n*lastIOI
		long lastOffset = click->GetAbsTime().toLong() + lastIOI;
		while( lastOffset < minOffset )
			lastOffset += lastIOI;

		// insert last offset as last click
		click = new TCLICKNOTE( lastOffset,
			TFrac(lastOffset,1L),
			click->Intens(),
			//				weight/2, // use weight of attack
			TFrac(click->plDuration(),1L)
			); 
		newClicktrack->Insert(click);
	}
#endif
	return newClicktrack;
	//	newClicktrack->Check(RecPPQ,					  RecTempo);
}


/*!
return an array whith the phase in each bar, starting at from
end of array is marked by a negativ phase
*/
double * THMIDIFILE::meterPhase(TTimeSignature /*meter*/, 
								TQNOTE * /*from*/, 
								int & /* size */)
{
	return NULL;
}


#ifdef IMUTUS_DLL
char THMIDIFILE::Read(MidiSeqPtr *seqs, int size)
{

	int i;
	MidiSeqPtr cur;
	THTRACK *curTrack;


	// seqs[0] should be track 0
	if( seqs[0] )
		ControlTrack = new THTRACK( seqs[0] );


	for( i = 1; i < size; i++ ) // process all tracks
	{
		cur = seqs[i];

		if( cur )
		{
			curTrack = new THTRACK( seqs[i] );
			curTrack->applyRelDuration( Relduration );
			addTrack( curTrack );
		}
	}
	return 0;
}
THMIDIFILE::THMIDIFILE(int ppq, int fileType) 
: TQMIDIFILE( NULL )
{
	init();
	RecPPQ = ppq;
	format = fileType;

}


#endif


double pSignificance( double curDistance )
{
	if( curDistance > patDistanceLimit )
		return 0;
	// 60% of patDistanceLimit == 0.6 significance
	double res = GaussWindow( curDistance, 
		patDistanceLimit * 0.6);
	return res;
}

void THMIDIFILE::init( void )
{
	barlengthList = NULL;
	barlengthPosList = NULL;	
	AccidentalsI = -8;		// < -7 -> invalid
	ClickTrack    = NULL;
	ClickTrackNr  = -1;		// -> invalid
	ClickTrackChannel = -1;	// -> invalid
	TactusLevel = 0L;
	CountIn       = 0;
	CalcTempo = 0;			// Tempo infering == off
	UpBeatI = 0;			// no pickup
	patternFile = NULL;
}

/// infer a dynamics profile for all voices and all tracks
void THMIDIFILE::inferDynamics(void)
{
	const char *val;
	val = getInifile()->GetValChar("DYNAMICS","ON","[ON|OFF] infer intensity profile");
	if( !strcmp( val, "ON" ) )
	{
		TDecayStruct decaySet[2];
		decaySet[0].decay = 0.2;
		decaySet[0].range = 7;
		decaySet[1].decay = 0.2;
		decaySet[1].range = 11;

		TTRACK *temp = EvTracks;
		while( temp )
		{
			int voice;
			for( voice = 0; voice < temp->cVoice(); voice ++ )
			{
				TFloatingAverage *floatingAverage = new TFloatingAverage(2,
					&(decaySet[0]),
					0.1);								 
				TNOTE *cNote = temp->FirstNoteObject(voice);
				while( cNote )
				{
					floatingAverage->addValue( cNote->getIntens() );
					cNote = QNOTE(cNote->GetNext( voice ));
				} // while note
				int listSize = 0;
				int *breakIdList = floatingAverage->getBreaks( &listSize );
				if( breakIdList )
				{
					int pID = 0;
					int i = 0;
					double prevIntens = 0;
					TNOTE *cNote = temp->FirstNoteObject(voice);
					while( cNote &&							
						pID < listSize &&
						breakIdList[pID] > -1 )
					{
						if( i == breakIdList[pID] )
						{
							double newIntens = floatingAverage->rightAv(0, i)/127;
							if( newIntens < prevIntens / 1.14 ||
								newIntens > prevIntens * 1.14 )
							{
								// write only larger changes
								cNote->setIntens(newIntens);
								prevIntens = newIntens;
								pID++;
							}
							else
							{
								cNote->setIntens(-1);
							}
						}
						else
						{
							// make invalid
							cNote->setIntens(-1);
						}
						i++;
						cNote = QNOTE(cNote->GetNext( voice ));
					} // while note
					// reset remaining notes
					while( cNote )
					{
						cNote->setIntens(-1);
						cNote = QNOTE(cNote->GetNext( voice ));
					}
					delete [] breakIdList;
				} // if
				delete floatingAverage;
			} // for voice
			temp = temp->Next();
		} // while
	} // if
	else // make all intensity settings invalid
	{
		TTRACK *temp = EvTracks;
		while( temp )
		{

			TNOTE *cNote = temp->FirstNoteObject(-1);
			while( cNote )
			{
				cNote->setIntens(-1);
				cNote = QNOTE(cNote->GetNext( -1 ));
			} // while note		
			temp = temp->Next();
		} // while
	} // else
}


/// check if a localBest or overlapping match should be selected
TPatternDistance *THMIDIFILE::acceptPatternMatch(double curDistance,
												 double typMatchDistance,
												 TPatternDistance *bestOvl,
												 TPatternDistance *localBest
												 )
{
	if( !bestOvl &&
		!localBest )
		return NULL;

	if( curDistance < 0 )
	{
		// we have no running pattern
		if( bestOvl == NULL )
		{		
			double pLocal = GaussWindow( localBest->distance,
				1.5 * typMatchDistance );
			if( pLocal > 0.6 )
			{
				return localBest;
			}
		}
		else if( localBest == NULL )
		{
			// if ovl and curDistance < 0 -> test 	for possible accept ovl
			double pOvl = GaussWindow( bestOvl->distance,
				1.8 * typMatchDistance );
			if( pOvl > 0.6 )
			{
				return bestOvl;
			}

		}
		else
		{
			// there can not be an OVL outside a pattern !!
			Printf("ERROR ERROR ERROR!\n");			
		}
	}
	else 		// there is a current pattern distance
	{
		if( bestOvl == NULL )
		{
			// no overlap possible
			// select if much better than current distance
			double pLocal = GaussWindow( localBest->distance,
				curDistance * 0.2 );
			if( pLocal > 0.6 )
			{
				return localBest;
			}
		}
		else if( localBest == NULL )
		{
			// accept overlap if it is < limit
			double pOvl = GaussWindow( bestOvl->distance,
				typMatchDistance * 1.8 );
			if( pOvl > 0.6 )
			{
				return bestOvl;
			}
		}
		else
		{
			// accept localBest if << bestOvl and << current Distance
			double pLocal = 1 - GaussWindow( localBest->distance,
				bestOvl->distance,
				curDistance * 0.2);

			double pLocal2 = GaussWindow( localBest->distance,
				curDistance * 0.7 );

			if( localBest->distance > bestOvl->distance ||
				pLocal2 < 0.6 )
				pLocal = 0;	

			double pOvl = GaussWindow( bestOvl->distance,
				typMatchDistance * 1.7 );

			if( pLocal > 0.6 &&
				pLocal > pOvl )							    
			{
				return localBest;
			}
			else if( pOvl > 0.6 )
			{
				return bestOvl;
			}		
		}
	} // curDistance < 0
	return NULL;
}


int THMIDIFILE::clickTrackPatternMatching( TFracBinList *durationList,
										  TDoubleBinList *statList)
{

	// load pattern database
	const char *pdatabaseName = getInifile()->GetValChar("TPATTERN","tpatternbase.gmn","[filename|OFF]");
	if( strcmp(pdatabaseName, "OFF") )
	{
		Printf("\n   Clicktrack pattern matching ... \n    ");
		patternFile = new TPFILE(pdatabaseName);
		patternFile->Read();
		if( patternFile->CPattern() <= 0 )
		{
			// patternFile->writeFile(pdatabaseName);
			//			Printf("WARNING: Can't open tempo pattern file %s!\n", pdatabaseName );
			cout << pdatabaseName << " didn't exist, created std. patternfile!\n";
			//			delete patternFile;
			//			return 0;
			setDefaultPattern(patternFile);
		}
		FILE *out = fopen(pdatabaseName,"wt");
		patternFile->write(out);
		fclose(out);

		// add all missing IOIratios and IOI's to the grid-lists
		// ignore very uncommon notes -> don't do automatically add operation!



		// get a matching list for all patterns  for each clicknote

		int delta = 1; // offset between current note and last pattern start
		long int dToEnd = 0;
		long cn = 0; 



		TPatternDistance *bestSuffixStart = NULL,
			*bestLocalStart = NULL;

		// relation between last scoreIOI and last perfIOI
		double normF = 0;
		double normFSigma = 0;			
		double firstMatchPerfIOI;
		TFrac firstMatchScoreIOI;

		// flag == 1 as long no pattern match
		int firstPatternMatch = 1;
		/// end note of last complete matched pattern	
		TCLICKNOTE *lastPatternEnd = NULL;
		TPATTERN *curPattern = NULL;
		TPNOTE *curPNote = NULL;

		TCLICKNOTE *cur = ClickTrack->FirstNote();
		double curDistance = -1;
		/// get a best patternlist for all notes
		while( cur )
		{
			if( !curPNote ) // no running pattern
			{
				curPattern = NULL;
				// increase normFSigma
			}
			int lSize = min(cNotePatListSize, patternFile->CPattern());
			// store a list of best patterns for each note
			TPatternDistance *patternDistance = new TPatternDistance[ cNotePatListSize];
			double distance = -1;
			// distance to all pattern for current note
			patternFile->bestIOIratioMatch( cur, 
				distance,
				statList,
				durationList,
				ClickTrack,
				patternDistance,
				lSize,
				normF,
				normF * normFSigma);

			if( cur->GetAbsTime().numerator() == 88929 )
				ClickTrack->Debug();
			//			patternFile->weightList->write(stdout);
			// cur processed before?
			if( cur->bestPatternList )
			{
				delete [] cur->bestPatternList;
				cur->bestPatternList = NULL;
				//				ClickTrack->Debug();
			}

			cur->bestPatternList = patternDistance;
			cur->patternFinalID = -1;

			// find a new pattern 
			if( !curPattern )
			{
				TPatternDistance  *newMatch =  acceptPatternMatch(
					curDistance, // curdistance
					patternFile->typicalMatchDistance, // typ distance
					bestSuffixStart, // best Ovl
					bestLocalStart);
				if( newMatch &&
					newMatch == bestLocalStart )
				{
					// jump back to local min match duraing last pattern

					TCLICKNOTE *ivMatch = cur;
					while(  ivMatch &&
						ivMatch != lastPatternEnd  )
					{
						ivMatch->patternFinalID = -1;
						ivMatch->patScoreIOIratio = 0L;
						ivMatch->prelScDuration = 0L;
						ivMatch->scoreDurationI = 0L;
						ivMatch = ivMatch->getNext(-1,-1);
						//							ClickTrack->Debug();
					}
					cur = newMatch->pos;
					// make invalid
					cur->patScoreIOIratio = 0;
					// this means last selected curPattern was wrong!
					// make everything between prev match and cur invalid!

					bestSuffixStart = NULL;						
				} // if new Match == bestloclaStart
				bestLocalStart = NULL;
				//				bestLocalDist = -1;
				// index in patterndistance array
				int bestId = 0;
				// was there a good suffix pattern match
				if( newMatch &&
					newMatch == bestSuffixStart )
				{
					lastPatternEnd = cur->getNext(-1,-1);
					// jump back to suffix id
					cur = bestSuffixStart->pos;
				}
				else
				{
					lastPatternEnd = cur->getNext(-1,-1);;
				}
				//				bestSuffixDist = -1;
				bestSuffixStart = NULL;
				//				bestSuffixId = -1;
				if( !newMatch ) // no jump back by ovl or localBest
				{
					newMatch =  acceptPatternMatch(
						-1, // curdistance
						patternFile->typicalMatchDistance, // typ distance
						NULL,
						&(cur->bestPatternList[0]) );
					if( newMatch )
					{
						newMatch->id = 0;	
					}
				}	
				if( newMatch )
				{
					if( firstPatternMatch )
					{
						//					curPattern = cur->bestPatternList[bestId].ptr;
						curPattern = newMatch->ptr;
						// init typical duration and restart
						TFrac dur = curPattern->FirstNote()->IOI();
						firstMatchScoreIOI = dur;
						firstMatchPerfIOI = cur->ioi(1,-1).toDouble();
						durationList->updateTypicalDurations( firstMatchScoreIOI,
							firstMatchPerfIOI,
							1);						
						firstPatternMatch = -1;
						normFSigma /= 2;
						normF = cur->ioi(1,-1).toDouble() / curPattern->FirstNote()->IOI().toDouble();
						patternFile->incCUsed(curPattern);
						curPattern = NULL;
						// restart from begin with new typical durations
						bestSuffixStart = NULL;
						//						bestSuffixDist = -1;
						//						bestSuffixId = -1;
						cur = ClickTrack->FirstNote();
						//				ClickTrack->Debug();
					}
					else // select a new pattern
					{
						cur->patternFinalID = newMatch->id;
						//						cur->patternFinalID = bestId;
						//						curPattern = cur->bestPatternList[bestId].ptr;
						curPattern = newMatch->ptr;
						curPattern->updateTypicalDistance(cur->bestPatternList[bestId].normDistance);
						cn = curPattern->cNotes();
						curPNote = curPattern->FirstNote();
						TFrac dur = curPNote->IOI();
						durationList->updateTypicalDurations( dur,
							cur->ioi(1,-1).toDouble(),
							pSignificance(curDistance));
						dToEnd = cn;
						patternFile->incCUsed(curPattern);

						cur->prelScDuration = curPNote->IOI();

						normF = cur->ioi(1,-1).toDouble() / curPNote->IOI().toDouble();
						normFSigma = 0.05;
						delta = 0;
						curDistance = newMatch->distance;
						patternFile->updateTypicalMatchDistance(curDistance);			
					} // else new pattern
				} // if < distanceLimit
				newMatch = NULL;
			} // if ! cur Pattern
			else if( curPattern )// check for suffix
			{
				if( validIOIratio(curPNote->IOIratio() ) )
				{
					// we are inside pattern

					double normF2 = cur->ioi(1,-1).toDouble() / curPNote->IOI().toDouble();
					double mSignificance = GaussWindow( normF, normF2, normF*0.1);
					normF = normF2;
					normFSigma = 0.05;
					cur->patScoreIOIratio = curPNote->IOIratio();
					TFrac dur = curPNote->IOI();
					durationList->updateTypicalDurations( dur,
						cur->ioi(1,-1).toDouble(),
						mSignificance);
				}  // if inside pattern
				// cur->prelScDuration = curPNote->IOI();

				// check if there is a suffix, get id of best overlap matching pattern of list
				int suffixId = curPattern->IOISuffix( cur->bestPatternList,
					lSize,
					delta );
				if( suffixId > -1 )
				{
					if( acceptPatternMatch(	-1, // curdistance
						patternFile->typicalMatchDistance, // typ distance
						&(cur->bestPatternList[suffixId]),
						NULL) )										
					{
						if( !bestSuffixStart ||
							// prefer early suffix matches against late ones
							cur->bestPatternList[suffixId].distance * 1.1< bestSuffixStart->distance)
						{
							bestSuffixStart = &(cur->bestPatternList[suffixId]);
							bestSuffixStart->id = suffixId;
							//							bestSuffixDist = cur->bestPatternList[suffixId].distance;
							//							bestSuffixId = suffixId;
							bestSuffixStart->pos = cur;
						}
					} // if accept pattern
				} // if suffixId
				else if( acceptPatternMatch(curDistance, // curdistance
					patternFile->typicalMatchDistance, // typ distance
					NULL,
					&(cur->bestPatternList[0])) )
				{
                    if( bestLocalStart != NULL )
                        bestLocalStart->id = 0;
					//					bestLocalDist = cur->bestPatternList[0].distance;
					bestLocalStart = &(cur->bestPatternList[0]);
					bestLocalStart->pos = cur;
				} // if best local match
				cur->patternFinalID = -1;
			} // else if curPattern

			if( curPNote )
			{
				curPNote = curPNote->GetNext();
			}
			else
			{
				normFSigma += 0.1;
			}

			if( firstPatternMatch > -1 )
			{
				cur = cur->getNext(1,-1);
				delta++;
				dToEnd--;
			}
			else
			{
				firstPatternMatch = 0;
			}

			// we have reached a new pattern
			if( dToEnd < 0 )
				dToEnd = cn - delta;
		} // while
		//		ClickTrack->Debug();
		Printf("   completed.\n");

		ClickTrack->Debug();
		{
			FILE *out = fopen(pdatabaseName,"wt");
			patternFile->write(out);
			fclose(out);
		}
		if( !firstPatternMatch ) // reset the typical duration
		{
			durationList->updateTypicalDurations( firstMatchScoreIOI,
				firstMatchPerfIOI,
				1);
		}
		/* Don't delete this here, there might be references from bestPatternList
		if( patternFile )
			delete patternFile;
		*/
	} // if

	return 1;
}
