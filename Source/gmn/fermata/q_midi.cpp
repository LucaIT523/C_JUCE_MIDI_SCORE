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
|	Filename : Q_MIDI.CPP
|	Author     : Juergen Kilian
|	Date	  : 17.10.1996-98-2004, 2011
|	Implementation of TQMIDIFILE
------------------------------------------------------------------*/

//#define _DEBUG_IOI
//#define _DEBUG_LEGATO


#include <iostream>
using namespace std;
#include <string.h>
#include "patternfile.h"

#include "q_midi.h"
#include "meta_meter.h"
#include "meta_tempo.h"
#include "meta_key.h"

#include "q_chord.h"
#include "q_track.h"

#include "h_track.h"

#include "similarity.h"
#include "../lib_src/ini/ini.h"



#include "k_array.h"
#include "q_funcs.h" // ticks 2Noteface

#include "import.h"

#include "notestatlist.h"
#include "correl.h"
//-------------------------------------------------------------------

// extern int STACCATO_TIME;
// extern int ORNAMENT_TIME; 

//-------------------------------------------------------------------
TQMIDIFILE::TQMIDIFILE( const char *name ) : TMIDIFILE( name )
{
	FirstTime = 0;
};
TQMIDIFILE::TQMIDIFILE( std::string *buffer ) : TMIDIFILE( buffer )
{
	FirstTime = 0;
};

//-------------------------------------------------------------------

TQMIDIFILE::~TQMIDIFILE( void )
{
}//~TQMIDIFILE
//-------------------------------------------------------------------
void TQMIDIFILE::shiftAttacks(TFrac offset)
{
	ControlTrack->shiftAttacks( offset );
	TTRACK *track = FirstTrack();
	while( track )
	{
		track->shiftAttacks( offset );
		track = track->Next();
	}	
}

void TQMIDIFILE::Debug( FILE *out )
{
#ifdef _DEBUG_MEMORY
	char mustClose = 0;
	if( ! out )
	{
		mustClose = 1;
		out = fopen("_qmidifile.txt","wt");
	}
	fprintf(out,"TQMIDIFILE\n");
	// debug control track
	fprintf(out,"ControlTrack\n");
	if(ControlTrack)
		ControlTrack->Debug(out);
	// debug event tracks
	fprintf(out,"EventTracks\n");
	TTRACK *temp = EvTracks;
	while(temp )
	{
		temp->Debug(out);
		temp = temp->Next();
	}
	if( mustClose )
		fclose( out );
#endif
}
//-------------------------------------------------------------------
void TQMIDIFILE::Quantize( void )
{
	
	// flags for mode
	detectMeterFlag = UNKNOWN;
	detectKeyFlag	= UNKNOWN;
	/// read parameters from .ini
	EQUAL_TIME	= saveAtoi(DEF_EQUAL_TIME);
	LEGATO_TIME = saveAtoi(DEF_LEGATO_TIME);
//	STACCATO_TIME = saveAtoi(DEF_STACCATO_TIME);
	
	/// mark quantisation pattern regions
	char markQPattern = 0;
	
	TInifile *tempIni = getInifile();
	if( tempIni )
	{	
		// read time constants
		EQUAL_TIME = tempIni->GetValInt("EQUAL_TIME",DEF_EQUAL_TIME,"max distance [ms] for two equal attackpoints");
		LEGATO_TIME = tempIni->GetValInt("LEGATO_TIME",DEF_LEGATO_TIME, "min overlap duration [ms] for legato");
		
		// read key tag settings		
		const char *tempVal = tempIni->GetValChar("DETECTKEY","MIDIFILE","key detection [MIDIFILE|DETECT|ON|OFF] ");
		if(!strcmp(tempVal,"ON") )
		{
			detectKeyFlag = UNKNOWN;
		}
		else if(!strcmp(tempVal,"OFF") )
		{
			detectKeyFlag = OFF;
		}
		else if(!strcmp(tempVal,"MIDIFILE") )
		{
			detectKeyFlag = MIDIFILE;
			Printf("Key-signatures of midifile will be used.\n");
		}
		else if(!strcmp(tempVal,"DETECT") )
		{
			detectKeyFlag = DETECT;
			Printf("Detect key=on\n");					
		}
		else
		{
			cout << "Warning: unknown value " << tempVal <<
					" for DETECTKEY in fermata.ini!\n";
			detectKeyFlag = MIDIFILE;
		}
		
		tempVal = tempIni->GetValChar("MARKQPATTERN","OFF","[ON|OFF] mark pattern quantised regions" );
		if( !strcmp( tempVal, "ON") )
		{
			markQPattern = 1;
		}	
		
		// read meter tag settings		
		tempVal = tempIni->GetValChar("DETECTMETER","MIDIFILE","meter detection [MIDIFILE|DETECT|ON|OFF]"); // detect||file
		if(!strcmp(tempVal,"ON") )
		{
			detectMeterFlag = UNKNOWN;
		}
		else if(!strcmp(tempVal,"OFF") )
		{
			detectMeterFlag = OFF;
			Printf("Meter detection: off\n");
		}
		else if(!strcmp(tempVal,"MIDIFILE") )
		{
			detectMeterFlag = MIDIFILE;
			Printf("Meter detection: use midifile\n");
		}
		else if(!strcmp(tempVal,"DETECT") )
		{
			detectMeterFlag = DETECT;
			Printf("Meter detection: on\n");
//			Printf("Warning: meter detection not implemented!\n");
		}
		else
		{
			cout << "Warning: unknown value "<< tempVal <<
					" for DETECTMETER in fermata.ini!\n" ;
			detectMeterFlag = MIDIFILE;
		}
		
	} // if inifile
	
	
	// needed for multi pass usage 
	if( mDirty )			// Todo some reset?
	{
		// needs to be checked
		//		file = fopen(name, "rb");
		//		if( !file )
		//			return;
		//	Read();
		//		fclose( file );
	};
	
	
	int mergeTracks = 0;
	if( tempIni )
	{
		// pre-processing: handle smf1 as smf0 -> merge all tracks	
		const char *val = tempIni->GetValChar("MERGETRACKS","0" ,"[ON|OFF]only for testing",1 /*hide*/);
		if( !strcmp(val,"ON") ||
		    !strcmp(val,"1") )
		    mergeTracks = 1;
		if( mergeTracks )
		{
			Printf("Warning: Merge tracks is on!!\n");
			TQTRACK *nextTrack;
			TQTRACK *tempTrack = (TQTRACK*)EvTracks;
			if( tempTrack )
				nextTrack = dynamic_cast<TQTRACK *>(tempTrack->Next());
			else
				nextTrack = NULL;
			while( nextTrack ) // for all tracks
			{
				tempTrack->Merge(nextTrack);
				TQTRACK *delTrack = nextTrack;
				nextTrack = dynamic_cast<TQTRACK *>(nextTrack->Next());
				delete delTrack;
			} // while
		} // if merge
	} // if ini
	
	//--------------------  quantisation step1 -------------------------------------------------------------
	// do  Voice separation 
//	FILE *voice_log = NULL;
	
#ifdef _DEBUG_VOICE_LOG
	voice_log = fopen( "_voice.log","wt" );
#endif
	
	/// ----------------- do voice separation for all tracks ----------------------------------
	if( RecPPQ <= 0 ) // input file was a .gmn file
	{
		RecTempo = Tempo(NULL);
		if( RecTempo <= 0 ) // not \tempo tags
		{
			RecTempo = DEFAULT_TEMPO;
		}
	}
	TQTRACK *tempTrack = dynamic_cast<TQTRACK *>(EvTracks);
	while( tempTrack ) // for all tracks
	{
		if( tempTrack->GetCVoice() == 0 )	// First call?
		{
			// remove small overlappings and small rests
			tempTrack->Debug();
            tempTrack->mergeEqualAttackpoints( RecTempo,
												RecPPQ );
			tempTrack->Debug();
            tempTrack->removeLegatoOverlaps( RecTempo,
											RecPPQ );
			tempTrack->Debug();
			/*			
			analyseVoices(tempTrack,
			voice_log);
			*/
			
			FirstTime = 1;
			tempTrack->CheckSort();
			if( Type() != 10 || /// gmn input -> no split voices
				mergeTracks) // don't call for .gmn files
			{
				//			   	tempTrack->Debug();
				tempTrack->SplitVoices( RecTempo, RecPPQ, this);
				//				tempTrack->Debug();
			}
			else // we have read a .gmn file
			{
				tempTrack->setCVoice(1);
				TMusicalObject *tObj = tempTrack->FirstNoteObject(-1);
				while( tObj )
				{
					tObj->SetVoice(0);
					tObj = tObj->GetNext(-1);
				}
			} // else
//			tempTrack->Debug();

		} // if CVoice == 0
		tempTrack = dynamic_cast<TQTRACK *>(tempTrack->Next());
	} // for i ( all tracks
	
	
	// retrieve/analyse accuracy ----------------------------------------------------------
	double atAccuracy,
		   durAccuracy;	
	accuracy(atAccuracy, durAccuracy );
// memory safe 	

	//   ------------ quantisation step2 --------------------------------------------------------------
	// call HEISENBERG -> tempodetection
	// call virtual functions: MarkGrace, ...

	 PreQuantize(atAccuracy,
 		        durAccuracy);
	
	
	// --------------- self similarity ----------------------------------------------------
    const char *tempC = getInifile()->GetValChar("SIMILARITY","OFF", "[ON|OFF] similarity analysis");
    if( !strcmp(tempC, "ON") /* && 
		Type() != 10*/ ) // don't do for .gmn files?
    {
		// delete existing selfsimilarity file
		FILE *tempf = fopen(SIMMATRIX_FNAME(this->filename().c_str(),"sim.txt").c_str(), "wt");
		fclose( tempf );
		
		int windowSize = getInifile()->GetValInt("SIM_WINDOWSIZE","4","" );
		int stepSize = getInifile()->GetValInt("SIM_STEPSIZE","3","" );
		
		TTRACK *tTrack = EvTracks;
        // do analysis for each track and every voice
        while( tTrack )
		{			
			int v = tTrack->cVoice();
			for( int v2 = 0; v2 < v; v2++ )
			{
				cout << "Selfsimilarity analysis: track " << tTrack->TrackNumber();
				cout << " voice " << (v2+1);
				cout << "window="<<windowSize<<", step="<< stepSize << endl;
					
				TNOTE *temp = tTrack->FirstNoteObject(v2);				
				selfSimilarity( temp, NULL, // [temp ... NULL) == complete voice
					windowSize /*window*/,
					stepSize /* step*/,
					ControlTrack,
					this->filename().c_str()
					);
			} // for
			tTrack = tTrack->Next();
		} // while
		Printf("done.\n");
	} /// similarity block
	
	// create ms note duration map 
	{
		const char *filename = getInifile()->GetValChar("DURATION_MAP","",NULL,1 /*hide */);
		if( strcmp(filename, "" ) )
		{
			cout << "Creating ms table " << filename << "...";
			THTRACK *tTrack = dynamic_cast<THTRACK *>(EvTracks);
			while( tTrack )
			{
                // convert to GUIDO LowLevelNotation
                tTrack->toGLN(filename,
					this->Ppq(),
					ControlTrack);
				tTrack = dynamic_cast<THTRACK *>(tTrack->Next());
			} // while
			Printf("done.\n");
		} // if			
	}// block
	
	
    //------------------------------------------------------------------------
	/// even after tempo detection ticktiming is still used! 
	// otherwhise trouble with too large denominators
	Printf("Convert tick-timing into score-timing ... ");	
	// convert control track
	if( tickTiming ) 
	{
		// convert tick timing into noteface timing
		ticksToNoteface((TQTRACK*)ControlTrack, RecPPQ);
		tickTiming = 0;
	}
	else
	{
		Printf("Score-time information available!\n");
	}
	tempTrack = dynamic_cast<TQTRACK *>(EvTracks);
	while( tempTrack )
	{
		// convert tick-timing into noteface-timing
		ticksToNoteface( tempTrack, RecPPQ);
		tempTrack = (TQTRACK*)tempTrack->Next();
	}
	Printf("done.\n");
	
    //----- detect meter ? -------------------------------------------------------------------
	const char *detectVal = tempIni->GetValChar( "DETECTMETER", "MIDIFILE", "meter detection [MIDIFILE|DETECT|ON|OFF]" ); 
	if( !strcmp(detectVal,"ON" ) ||
		!strcmp(detectVal,"DETECT" ) )
	{
		detectMeter( 0 /* score time mode */);
		//!! phase shift needs to be called AFTER quantization
	}
	else if ( !strcmp(detectVal, "MIDIFILE" ) )
	{
		// check if any meter inside midifile
		try{
			TMetaMeter *tempMeter = NULL;
			if( ControlTrack != NULL )
				tempMeter = ControlTrack->FirstMeter();
			if( !tempMeter )
			{
				Printf("MIDI-File contains no meter information!\n");
				detectMeter(0 /* score time mode */);
				//!! phase shift needs to be called AFTER quantization
			}
		}
		catch(char* e)
		{
			ShowErrorMsg(e);
		}
		catch( ...)
		{
			ShowErrorMsg("unknown Exception");
		}
	} // else		
    
    TMetaMeter *tempMeter = NULL;
	if( ControlTrack )
	{
		tempMeter = ControlTrack->FirstMeter();
	}
	if( tempMeter != NULL )
	{
		if( upBeatFound )
		{
			TFrac meter(tempMeter->numerator,
						tempMeter->denominator );
			// upBeat found might be set by THMIDIFILE::FitToClick
			upBeatFound = 0;
			Printf("Upbeat detection for meter = ");
			Printf(meter);
			Printf(" ... ");

			TFrac thirtyTwo = meter / 16;
			double oldQuality,
					newQuality;
			// do a phase check of current time signature (meter)
			TFrac delta = 0L;
			if( EvTracks )
			{
				((TQTRACK *)EvTracks)->optimumOffset(
														(TQNOTE *)EvTracks->FirstNoteObject(-1),
														NULL,
														-1,
														meter,
														thirtyTwo,
														oldQuality,
														newQuality);
			}
			if( delta > 0L &&
				newQuality > oldQuality )
			{
				Printf(" offset = ");
				Printf(delta);
				/// shift attackpoints in complete midifile
				shiftAttacks(delta);
				/*
					ClickTrack->shiftScoretimes(ClickTrack->FirstNote(),
						NULL,
						delta);
				*/
			} // if
			Printf(" done.\n");
		} // if upbeat
	}

	//------------- pattern processing ------------------------------------------------
	//----- check pattern length ------
	const char *pName = getInifile()->GetValChar("QPATTERN",DEF_PATTFILE,"[filename|OFF] filename of pattern database for quantisation");
	TFrac pLength;
	TPFILE *PFile = NULL;
	if( strcmp(pName,"OFF") ) // if not off
	{
		PFile = new TPFILE(pName);
		if( PFile->Read() )
		{
			// if no parse error write 
			PFile->write();
		}
		if( PFile->CPattern()  <= 0 )
		{
			setDefaultPattern(PFile);
		}
		if( PFile && 
			PFile->cLength() > 1 )
		{
			if( tempMeter )
			{
				pLength = TFrac(tempMeter->Meter());
			}
			else
			{
				string str;
				str = InputQuestion("Patternfile contains patterns of different lengths! \nPlease enter pattern length to use: ", "0/1", NULL);
				pLength = TFrac(str.c_str());
			}
		}
		else if( PFile ) // single pattern length
		{
			pLength = PFile->Length();
		}
		
		if( PFile &&
			PFile->CPattern() )
		{

			cout << "Quantisation pattern database=" << PFile->name << endl;
			if( pLength != 0L )
			{	// if a patternlength is selected check if in database
				cout << " pattern length =" <<
						pLength.numerator() <<"/" <<
						pLength.denominator()<< endl;
				if( tempMeter && 
					pLength != tempMeter->Meter() )
				{
					Printf("Warning: Patternlength != Time Signature!\n");
				}
				if( !PFile->CPattern( &pLength ) )
				{
					cout << "Warning: " << PFile->name <<
							" does not contain pattern with this length!\n";
					delete PFile;
					PFile = NULL; // don't use
				}
			} // if pLength
		}
		else
		{
			Printf("No quantisation patternfile available!\n");
			if( PFile )
				delete PFile;
			PFile = NULL;
		}
	} // if pattern file
// memory leak safe

	// -------------- Step 3 ------------------------------------------------
	// Quantise notes in all tracks -------------------------------------------
	// CDynaprog<TQNOTE *, TPNOTE *> *dynaprog;
	/// check accuracy vs grid
	/// Todo: eliminate also attachpoints which are very close to each other (1/12, 1/16), which one?
	const char  *AttackFilename = getInifile()->GetValChar("AttackGridFName","IOIList.ini","Filename for IOIGrid");
	TFracBinList *qAttacks = readAttackpointList( AttackFilename);
	qAttacks->createDistribution();
	
	/// size of qAttacks list
	int cAttacks = 	0;
	if( qAttacks )
		cAttacks = qAttacks->Count();
	if( atAccuracy < 0.85 )
	{
		// low accuracy -> remove small durations
		TFrac minNote(1,16);
		const char *state = NULL;
		for( int i = 0; i < cAttacks; i++ )
		{
			if( (*qAttacks)[i] < minNote )
			{
				if(  !state )
				{
					state = YesNoQuestion("Attack-accuracy is low, limit attack-grid to 1/16?",JK_YES, NULL);
				}
				if( !strcmp(state, JK_YES) )
				{
					qAttacks->remove( i );
					//!! qAttacks->Count() and class numbering has changed!
					i--;
					cAttacks--;
					// qAttacks->write(stdout);
				}
			} // if
		} // for
	} // atAccuracy
	
	const char  *DurationFilename = getInifile()->GetValChar("DurationGridFName","IOIList.ini","Filename for IOIGrid");
	TFracBinList *qDurations = readDurationList( DurationFilename );
	qDurations->createDistribution();
	
	int cDurations = 0;
	if( qDurations )
		cDurations = qDurations->Count();
	
	if( durAccuracy < 0.8 )
	{
		// low accuracy -> remove small durations
		TFrac minNote(1,16);
		const char *state = NULL;
		for(int i = 0; i < cDurations; i++ )
		{
			if( (*qDurations)[i] < minNote )
			{
				if(  !state )
				{
					state = YesNoQuestion("../leanguido/lgDuration-accuracy is low, limit duration-grid to 1/16?",JK_YES, NULL);
				}
				if( !strcmp(state, JK_YES) )
				{
					qDurations->remove( i );
					//!! qAttacks->Count() and class numbering have changed!
					i--;
					cDurations--;
                }
			} // if
		} // for
	} // if durAccuracy
	
	/* Quantize the file ------------------------------------*/
	//------------------ loop all tracks ----------------------------------
	
	int cTrack = 1;
	const char *mirFName = getInifile()->GetValChar("MIR","OFF","[filename|OFF]");
	tempTrack = (TQTRACK *)EvTracks;
	int PMatch = 0;
	while( tempTrack )
	{
		if( !FirstTime ) // reset ?
		{
			tempTrack->DelTempoNotes();
			tempTrack->ResetDiffs();
		}
		
		int voice;
		for( voice = 0; voice < tempTrack->GetCVoice(); voice++ )	// for all Voices
		{
			/// init at begin of voice
			TQNOTE *lastNoMatch = NULL;
			TQNOTE *firstMatch = NULL;	

			MIR( mirFName, tempTrack, voice );			
			
			tempTrack->Debug();
			/// earliest possble pattern start
			TFrac endPrev( 0,0 ); 
			
			// do pattern match or calc alternatives fo attackpoint and duration------------------------------------------
			// start at first note in voice
			TQNOTE *From = dynamic_cast<TQNOTE *>(tempTrack->FirstNoteObject( voice ));
			TQNOTE *To   = NULL;   	  // until end of track
			TFrac Diff = 0L;
			TQNOTE *Ptr = From;
			if( Ptr )
			{		
				if( format ) // SMF 1
					cout << "Quantisation for track "<< cTrack <<", voice "
					<<voice+1<<" ... ";
				else // SMF 0
					cout << "Quantisation for track 0, voice " <<
							voice+1 << " ... ";
				
				double typicalPDistance = 0.22; 
				while( Ptr ) // for all notes
				{
					/// try pattern matching first
					if( PFile &&
						atAccuracy < 1 &&	// no pattern matching for machine played files!
						durAccuracy < 1 )
					{
						From = NULL; // stop no pattern quantisation
						/// Try pattern match
						double pDistance;
						// get a best pattern file
						TPATTERN *Pattern = PFile->bestQMatch( Ptr, 
																endPrev, 
																 pDistance,
																atAccuracy,
																durAccuracy,
																pLength,
																tempMeter->Meter().toFrac());
/*						if( Pattern && 
							pDistance < 0.2 &&
							pDistance >= typicalPDistance * 1.2 )
						{
							typicalPDistance = typicalPDistance * 0.8 
											  + pDistance * 0.2;
						}
						else */
						if( // dynaprog && 
							Pattern &&
							pDistance < typicalPDistance*1.2 ) 	// match?
						{
							// update the dynamic pattern distance
							typicalPDistance = typicalPDistance * 0.8 +
											   pDistance * 0.2;
							// quantize Ptr, ... alogn dynaprog
							// int i = dynaprog->outStart;
							PMatch++;
							/*
							Printf("\n");
							Printf("\nPattern match %d," , patternID);
							Printf("%f:", sim);
							*/
							PFile->incCUsed( Pattern );
							
							/// copy pattern to qNotes
							//	while( i < dynaprog->iI + dynaprog->jI )
							TPNOTE *pnote = Pattern->FirstNote();
							TQNOTE *qnote = Ptr;
							int i = 0;
							while( qnote &&
								pnote )
							{
								if( pnote &&
									qnote)
								{
									// store observed attackpoint in pattern note
									pnote->calcStatistics(Pattern->cUsed() /* + Pattern->cUsedCur()*/,
															qnote->GetAbsTime());
									pnote->copyTo(qnote );
									/// pnotes were  shifted to correct positions during IOIdistance
									endPrev = TFrac(pnote->pos() + pnote->duration());

									
									// add to qdurations liste
									TFrac dur = TFrac(pnote->duration()); 
									TFracBinClass *exactClass = qDurations->findExact( dur );
									// don't add "strange" pattern durations to grid!!!
									/*
									if( !exactClass )
									{
										exactClass = qDurations->addClass( pnote->duration() );
										
									}
									*/
									if( exactClass )
										qDurations->addValue( qnote->GetDuration(), exactClass );
									// qDurations->write(stdout);
									
									/// add to qattacks, largest duration first!
									int k;
									for( k = cAttacks - 1; k > -1; k--)
									{
										dur = (*qAttacks)[k];
										if( (TFrac(pnote->pos()) % dur) == 0L )
										{
											TFracBinClass *atClass = qAttacks->findExact( dur );
											if( atClass )
											{
												qAttacks->addValue(dur,
																   atClass);
												k = -2; // stop loop
											}
										}
									} // for
									if( k == -2) // nothing found, out of grid
									{
										// todo add to grid?
									}
									// qAttacks->write(stdout);
								} // if pattern note
								if( qnote )
								{
									if( markQPattern )
									{
										if( !i ) // first note
										{
											qnote->color = 2;
										}
										else
										{
											qnote->color = 0;
										}
									}
									if( lastNoMatch && 
										!firstMatch)
										firstMatch = qnote;
									Ptr = QNOTE(qnote->GetNext(Ptr->GetVoice() ));
									// qnote->Debug(stdout);
									
								} // if qnote
								qnote = dynamic_cast<TQNOTE*>(qnote->GetNext(qnote->GetVoice()));
								pnote = pnote->GetNext();
								i++;
							} // while
							From = lastNoMatch; // -> do non pattern match
							To = firstMatch;
							
			//				tempTrack->Debug();
							// Printf("--------------------\n");
						} // if similarity
						else  // no pattern found
						{
							if( Ptr )
							{
								// min shift position for next pattern start
								TFrac ioi = Ptr->ioi(1, Ptr->GetVoice());
								TFrac offset(ioi.numerator(),
											 ioi.denominator() * 2L);

								TFrac eighth(1,8);
								if( offset > eighth)
									offset = eighth; 
								endPrev = Ptr->GetAbsTime() + ioi - eighth;
								// Printf("Pattern no match (%f)", sim );
								// Ptr->Debug(stdout);
								
								// Printf("\n");
							}
							
							// Todo: try gapped alignment?
							// problem duration similarity?
							if( !lastNoMatch )
							{
								lastNoMatch = Ptr;	// first unprocessed note
							} // if ( ! From
							From = NULL; /// set to lastNoMatch and next match or end of voice
							Ptr = QNOTE(Ptr->GetNext( voice ));
						} // else
						//					Ptr = dynamic_cast<TQNOTE*>(Ptr->GetNext( voice ));
						if( !Ptr ) // end of list ?
						{
							From = lastNoMatch;
							To = NULL;
						}
					} // if
					else // no PFile
					{
						if( PFile )
						{
							Printf("Pattern quantisation skipped for machine played files!\n");
						}
						/// do nothing
						From = Ptr;
						To = NULL;
						Ptr = NULL;
					}
					
					///---------------------- solve gap or everything without pattern ------------------
					if( From )
					{		
						tempTrack->QuantizeAttackpoints( From,
							To,
							Diff,	// = 0
							qAttacks,
							qDurations,
							atAccuracy);
//						tempTrack->Debug();
						// TodO reset qdurations?
						tempTrack->QuantizeDurations(
										From,
										To,
										qDurations,
										durAccuracy);
						tempTrack->Debug();
						if( markQPattern )
						{
							TQNOTE *tQNote = From;
							while( tQNote &&
									tQNote != To )
							{
								tQNote->color = 1;
								tQNote = QNOTE(tQNote->GetNext( tQNote->GetVoice()) );
							} // while
						} // if amrQPattern

						lastNoMatch = NULL;
						firstMatch = NULL;
					} // if From
					else if( !PFile ) // error
					{
						Printf("TMIDIFile::Quantize: From == NULL!\n");
					}
					From = To;
					To = NULL;
				} /// while				
			} /// if Ptr -> if any note in voice 
			Printf("done. \n");
		} // for all voices
				
#ifdef _DEBUG
		//        qDurations->write(stdout);
		//       qAttacks->write(stdout);
#endif
			tempTrack->Debug();

		tempTrack = (TQTRACK *)tempTrack->Next();
		cTrack++;
	} // for all tracks
if( qAttacks )
	{
		qAttacks->writeIni( AttackFilename );
		delete  qAttacks;
		qAttacks = NULL;
	}
	if( qDurations )
	{
		qDurations->writeIni( DurationFilename );
		delete  qDurations;
		qDurations = NULL;
	}
	
	// store updated information for .ini file
	if( PFile )
	{
		 PFile->finaliseCUsed();
		// write to read name
		PFile->write(NULL,NULL);
		delete PFile;
		PFile = NULL;
	}
	

	
#ifdef features
	fclose(tFile);
	fclose(tFile2);
#endif
	
	//	dynamic_cast<TQTRACK *>(controlTrack)->QuantizeTempo( TAbsTime(1,16) );
	
	FirstTime = 0;
}// Quantize
//-------------------------------------------------------------------
//-------------------------------------------------------------------
/*
Convert midi file into gmn.
Quantization must be done before
result:
1 : ok
0 : error
*/
char TQMIDIFILE::Convert(ostream &file,
                     const string &midiFilename )
{
	cout <<"Start conversion to buffer " << endl;
	char 	res = 0;
	
	file << "%%This file was generated by  " << getVersion() << ".\n";
	file << "%%Please report all problems to kilian@noteserver.org\n";
	file << "%%Please visit http://www.noteserver.org for more information on GUIDO MusicNotation.\n";
	file << "{";
	// select best alterbative for attack and durations, and merge all voices of all tracks into a single track
	finaliseQuantisation();
	// call conversion function
	res = ToGMN( file );
	
	file << "}" ;
	if( midiFilename.length() > 0 )
		writeMIDI( midiFilename.c_str() );
	Printf( "End<TQMIDI::Convert>\n" );
	return res;
}


void TQMIDIFILE::finaliseQuantisation( void )
{
	TQTRACK *tempTrack = NULL;
	TTRACK *nextTrack = NULL;
	
	
	if( EvTracks )
	{
		// merge all tracks in single track voice separated voices
		tempTrack = (TQTRACK *)EvTracks->Next();
		EvTracks->copyUnvoicedElements();
	}
	while( tempTrack )
	{
		EvTracks->Merge(tempTrack);
		nextTrack = tempTrack->Next();
		//	Printf("d2\n");
		
		delete tempTrack;
		tempTrack = (TQTRACK *)nextTrack;
	}
	
	
	// EvTracks->SetNext(NULL); is done inside of Merge
	// now only 1 EvTrack exists!!!
	
	// quantize control events of Track0 to attackpoints of other tracks
	tempTrack = (TQTRACK *)EvTracks;
	//	tempTrack->Debug();
	if( tempTrack )
		tempTrack->SetQData(); // select single alternative for notes
	//	tempTrack->Debug();
	
}

/*!
write gmn file of "this"
quantisation and finaliseQuantisationmust be called before
result:
1 : ok
0 : error
*/
char TQMIDIFILE::ToGMN( ostream &file )
{	
	if( !EvTracks )
	{
		file << "(* No event tracks available! *)\n";
		return 0;
	}
	
	
	// check for tempo setting in .ini
	TInifile *tempIni;
	if( ControlTrack == NULL )
	{
		ControlTrack = new TQTRACK(0, this);
	}
	tempIni = getInifile();
	const char *tempVal;
	tempVal = tempIni->GetValChar("TEMPO_OUT","ON","[ON|HIDDEN|OFF] output of \\tempo tag");
	if( !strcmp(tempVal,"OFF") )
	{
		ControlTrack->deleteSameType(ControlTrack->FirstTempo(),NULL);		
	}
	else if( !strcmp(tempVal,"HIDDEN") )
	{
		TMetaTempo *temp = ControlTrack->FirstTempo();
		while( temp )
		{
			temp->hidden=1;
			temp = dynamic_cast<TMetaTempo *>(temp->GetNext(temp->GetVoice()));
		}
	} // else if
	
	// check for key setting in .ini
	tempVal = tempIni->GetValChar("DETECTKEY","MIDIFILE","key detection [MIDIFILE|ON|OFF|DETECTOFF]");
	if( !strcmp(tempVal,"OFF") )
	{
		ControlTrack->deleteSameType(ControlTrack->FirstKey(), NULL);
	}
	
	// check for key meter in .ini
	tempVal = tempIni->GetValChar("DETECTMETER","MIDIFILE","meter detection [MIDIFILE|ON|OFF|DETECT]");
	if( !strcmp(tempVal,"OFF") )
	{
		ControlTrack->deleteSameType(ControlTrack->FirstMeter() ,NULL);
	}
	
	// check for redundant meter / tempo / key
	{
		TTimeSignature  prevSig;
		prevSig.initSig(0, 0, 0.0 /*weight*/);
		TMetaMeter *ctrl,
			*temp,
			*prev;
		
		// read first meter signature of track
		prev = ControlTrack->FirstMeter();
		if( prev )
		{
			prevSig = prev->Meter();
			ctrl = MetaMeter(prev->GetNext(-1));
		}
		else
			ctrl = NULL;
		
		char end = 0;
		const char *answer = NULL;
		// compare successive meter signatures, remove? if no changes
		while( ctrl && !end)
		{
			if( ctrl->Meter() == prevSig )
			{
				if( !answer )
				{
					answer = YesNoQuestion(" Remove redundant meter signatures?","y",NULL);
				}
				
				if( !strcmp(answer, JK_YES) )
				{
					// keep prev
					temp = MetaMeter(ctrl->GetNext(-1));
					ControlTrack->deleteNotes(ctrl, ctrl->TMusicalObject::GetNext(-1) );
					ctrl = temp;
				}
				else
				{
					end = 1;
				}
			}
			else // not equal, keep 
			{
				
				prev = ctrl;
				prevSig = prev->Meter();
				ctrl = MetaMeter(ctrl->GetNext(-1));
			}
		} // while 
	} // meter, removal
	
	// check for redundant key
	{
		TMetaKey *ctrl,
			*temp,
			*prev;
		
		// read first meter signature of track
		prev = ControlTrack->FirstKey();
		if( prev )
		{
			ctrl = MetaKey(prev->GetNext(-1));
		}
		else
			ctrl = NULL;
		
		char end = 0; 
		const char *answer = NULL;
		// compare successive meter signatures, remove? if no changes
		while( ctrl && !end)
		{
			if( ctrl->nAccidentals() == prev->nAccidentals() &&
				ctrl->minorMajor() == prev->minorMajor()  )
			{
				if( !answer )
				{
					answer = YesNoQuestion(" Remove redundant key signatures?","y",NULL);
				}
				
				if( !strcmp(answer, JK_YES) )
				{
					// keep prev
					temp = MetaKey(ctrl->GetNext(-1));
					ControlTrack->deleteNotes(ctrl, ctrl->TMusicalObject::GetNext(-1) );
					ctrl = temp;
				}
				else
				{
					end = 1;
				}
			}
			else // not equal, keep 
			{
				prev = ctrl;
				ctrl = MetaKey(ctrl->GetNext(-1));
			}
		} // while 
	} // meter, removal
	
	// check for redundant tempo
	{
		TMetaTempo *ctrl,
			*temp,
			*prev;
		
		// read first meter signature of track
		prev = ControlTrack->FirstTempo();
		if( prev )
		{
			ctrl = MetaTempo(prev->GetNext(-1));
		}
		else
			ctrl = NULL;
		
		char end = 0;
		const char 	*answer = NULL;
		// compare successive meter signatures, remove? if no changes
		while( ctrl && !end && prev)
		{
			if( ctrl->GetTempo() == prev->GetTempo()  )
			{
				if( !answer )
				{
					answer = YesNoQuestion(" Remove redundant tempo information?","y",NULL);
				}
				
				if( !strcmp(answer, JK_YES) )
				{
					// keep prev
					temp = MetaTempo(ctrl->GetNext(-1));
					ControlTrack->deleteNotes(ctrl, ctrl->TMusicalObject::GetNext(-1) );
					ctrl = temp;
				}
				else
				{
					end = 1;
				}
			}
			else // not equal, keep 
			{
				prev = ctrl;
				ctrl = MetaTempo(ctrl->GetNext(-1));
			}
		} // while 
	} // tempo removal

	/// quantise control information to attackpoints of notes
	quantizeToNotes(dynamic_cast<TQTRACK *>(ControlTrack),
		dynamic_cast<TQTRACK *>(EvTracks));
	
	
	/// everything should be merged into EvTracks
	TQTRACK *tempTrack = dynamic_cast<TQTRACK *>(EvTracks);
	
	if( Type() != 10 ) // don't do pitch conversion for original .gmn files
	{
		// read scale information
		readScales(getInifile());
		
		// calc key depending accidentals
		mCurControl = NULL;
		tempTrack->pitchSpelling(this);
		tempTrack->checkChromaticScales(this);
		mCurControl = NULL;
	}
						fprintf(stdout,"start convert"); fflush(stdout);
	
	// ##tempTrack->Debug();
	// write control information <= Abstime(0)
	tempTrack->Convert( file,
						  ControlTrack,	// convert track
						  -1, 			// from-Abstime
						  MAXLONG,		// to-abstime
						  glOffset
						  ); 
	// count notes
	int cnotes = 0, cstems = 0;
	TMusicalObject *curObj;
	curObj = tempTrack->FirstObject(-1);
	while( curObj )
	{
		TQChord *tChord = dynamic_cast<TQChord *>( curObj );
		if( tChord )
		{
			cstems++;
			cnotes += tChord->cNotes();
		}
		else if( dynamic_cast<TQNOTE *>(curObj ) )
		{
			cnotes++;
			cstems++;
		}
		curObj = curObj->GetNext(-1);
	}
	int cGVoice = 0, cUVoice = 0;
	cGVoice = tempTrack->cVoice();
	for( int i = 0; i < cGVoice; i++)
	{
		if( tempTrack->FirstNoteObject(i) )
			cUVoice++;
	}
	
	int maxV = getInifile()->GetValInt("MAXVOICES","-1","");
	file << "\n(*Debug info \n";
	file << "filename=" <<  filename() << endl;
	file << "cstems="<< cstems 
		<<"\ncnotes="<< cnotes 
		<<"\ncslices="<< cVoiceSlices
		<<"\nusedvoices="<< cUVoice
		<<"\ncreatedvoices="<< cGVoice
		<<"\nmaxVoice="<<maxV <<"\n*)\n";
	return OK;
} // Convert
//-------------------------------------------------------------------

TTRACK *TQMIDIFILE::CreateNewTrack( long offset ) // offset at MIDI file
												  /*
												  factory function for new TQTrack
												  */
												  
{
	TQTRACK *temp;
	temp = new TQTRACK( offset, this );
	return temp;
}
//-------------------------------------------------------------------
/*
virtual function
can be used at inherit classes, (call HEISENBERG)
*/
void TQMIDIFILE::PreQuantize( 	double ,
							 double )
{
}
//----------------------------------------------------------
void TQMIDIFILE::preProcess( void )
// virtual function, can be overriden be inherit classes
{
	
	
	
	
	TQTRACK *temp;
	temp = (TQTRACK *)EvTracks;
	while( temp )
	{
		temp->preProcess();
		temp = (TQTRACK *)temp->Next();
	}
}
//----------------------------------------------------------

/*
Ask the user for using meter information of MIDIfile
res infer : ON
use SMF information : MIDIFILE
no : OFF
parameter can be set in 
*/
int TQMIDIFILE::askDetectMeter(void)
{
	const char *answer;
	if( detectMeterFlag == UNKNOWN ) // ask only 1 time
	{
		answer = YesNoQuestion( "Use meter information of MIDI-file?", "y", NULL );
		if(!strcmp(answer,JK_YES) )
			detectMeterFlag = MIDIFILE;
		else if(!strcmp(answer,JK_NO) )
		{
			detectMeterFlag = DETECT;
			Printf("Detect meter=on\n");
			//			Printf("Warning: meter detection not implemented!\n");
		};
	}
	return detectMeterFlag;
}
//----------------------------------------------------------

/*
Ask the user for using key information of MIDIfile
res infer : ON
use SMF information : MIDIFILE
no : OFF
parameter can be set in 
*/
int TQMIDIFILE::askDetectKey( void )
{
	const char *answer;
	if( detectKeyFlag == UNKNOWN ) // ask only 1 time
	{
		answer = YesNoQuestion( "Use key information of MIDI-file?", "y",NULL );
		if(!strcmp(answer,JK_YES) )
			detectKeyFlag = MIDIFILE;
		else if(!strcmp(answer,JK_NO) )
		{
			detectKeyFlag = DETECT;
			Printf("Detect key=on\n");
		}
	}
	return detectKeyFlag;
}

void TQMIDIFILE::MIR(const char *mirFName,
					TTRACK *tempTrack,
		    		int voice )
{
				if( strcmp(mirFName, "OFF" ) &&
					strcmp(mirFName, "" ) )
				{
					int windowSize = getInifile()->GetValInt("SIM_WINDOWSIZE","4","" );
					int stepSize = getInifile()->GetValInt("SIM_STEPSIZE","3","" );
					
					TNOTE *temp = tempTrack->FirstNoteObject(voice);
					if( temp )
					{
						retrieveSequence(temp, NULL,
							windowSize, // windowsize
							stepSize,	// stepsize
							ControlTrack,
							mirFName,
							this->filename().c_str());
						// todo: store all  found occurrencies into .gmn file						
					} // if
				}	// if MIR
}
//----------------------------------------------------------
//----------------------------------------------------------


#ifdef OLD_SETTEMPO
void TQMIDIFILE::SetTempo( void )
/*
copy tempo profile from track0 to all tracks,
notes can be splitted
*/
{
	char change; // changes have happen
	
				 /*
				 Need not to be done, TempoChanges are shifted to NoteEnd in CheckTempo
				 // Quantize Track0 to 1/8
				 if( Track[0] )
				 ((TQTRACK *)Track[0])->QuantizeTempo( TEMPO_QUANTIZE);
	*/
	
	// make alternatives in track i valid
	TQTRACK *tempTrack;
	tempTrack = (TQTRACK*)EvTracks;
	
	//	for ( i = 1; i < TrackCount; i++ )
	while( tempTrack )
	{
		//			tempTrack = (TQTRACK *)Track[i];
		tempTrack->SetQData();
		tempTrack = (TQTRACK *)tempTrack->Next();
	}// for
#ifdef DEBUG
	FILE *tFile;
	tFile = fopen("setq.txt","wt");
	fprintf(tFile,"AFter SetQData\n");
	Debug( tFile );
	fclose( tFile );
#endif
	
	// check for collision between notes and tempochanges
	do	// until no changes occur
	{
		change = 0;
		tempTrack = (TQTRACK*)EvTracks;
		
		//		for ( i = 1; i < TrackCount; i++ ) // all tracks
		while( tempTrack )
		{
			//			tempTrack = (TQTRACK *)Track[i];
			//			change  += tempTrack->CheckTempo( Track[0] );
			change  += tempTrack->CheckTempo( ControlTrack );
			tempTrack = (TQTRACK*)tempTrack->Next();
		}
	}
	while( change );
	
	tempTrack = (TQTRACK*)EvTracks;
	
	// copy tempo changes to all tracks
	//	for ( i = 1; i < TrackCount; i++ )
	while( tempTrack )
	{
		//			tempTrack = (TQTRACK *)Track[i];
		tempTrack->SetTempo( ControlTrack );
		tempTrack = (TQTRACK*)tempTrack->Next();
	}
} // SetTempo
#endif

//-----------------------------------------------------------------
#ifdef UNUSEDFUNCTIONS
char TQMIDIFILE::ChangeNote( int Voice,			// Stimme in der die Note vorkommt
							long OldPos,		// Bisheriger Attackpoint
							long NewPos,		// Neuer Attackpoint
							long NewDuration,	// Neue Tondauer
							char MoveNext, 		// Flag == 1 -> Die nachfolgenden Noten auch verschieben
							int  Tonhoehe )
							/*
							Attackpoint und Tondauer einer Note können mit
							dieser Funktion manuell festgelegt werden.
							Die Funktion wird vom Noten-Editor aufgerufen
							*/
{
	int MaxVoice = 0,	// letzte Stimme +1 in Track[i]
		MinVoice = 0,	// erste Stimme in Track[i]
		i,
		SearchTrack = -1;
	
	long erg;
	
	TQTRACK *tempTrack;
	// Spur suchen in der Voice vorkommt
	for( i = 1; i < TrackCount; i++ )
	{
		tempTrack = (TQTRACK *)Track[i];
		
		//			if( i != ClickTrackNr )
		{
			MinVoice = MaxVoice;
			MaxVoice += tempTrack->GetCVoice();
			if( MaxVoice > Voice )
			{
				SearchTrack = i;
				i = TrackCount;	// Abbruch
			} // if MinVoice
		} // if i
	} // for ( i
	if( SearchTrack < 0 )
		ErrorMsg( 21 );	// Stimme nicht gefunden
	
	
	erg = ((TQTRACK *)Track[SearchTrack])->ChangeNote( Voice - MinVoice,
		OldPos,
		NewPos,
		NewDuration,
		MoveNext,
		Tonhoehe );
	
	// erg gibt an um wieviele Ticks die nachfolgenden Noten verschoben werde
	
	if( MoveNext )
	{
		NewPos = OldPos + erg;
		// Jetzt noch die nachfolgenden Noten in den anderen Spuren verschieben
		for( i = 1; i < TrackCount; i++ )
		{
			if( //(i != ClickTrackNr) &&
				(i != SearchTrack) )
			{
				tempTrack->ChangeNote( -1, // nicht suchen
					OldPos,
					NewPos,
					NewDuration,
					MoveNext,
					Tonhoehe );
			} // if i
		} // for ( i
	} // if MoveNext
	
	return erg;
} // Change Note
#endif

  /*
  char TQMIDIFILE::readIni(TInifile *inif)
  {
  
	return TMIDIFILE::readIni( inif );
	}
*/


/*! return estimate acury values of midifile */
void TQMIDIFILE::accuracy(double &attack, double &duration)
{
	Printf("Accuracy analysis: ");
	
	// search for track with most events
    attack = -1;
    duration = -1;
	if( !FirstTrack() )
	{
		Printf("ERROR: No event track available!\n");
		return;
	}
	
        int maxEv = -1;
	TQTRACK *temp = dynamic_cast<TQTRACK*>(FirstTrack());
	while( temp )
	{
		
                double mAt,
                        mDur;
		int ev = temp->accuracy(mAt, mDur);
		// printf("%d, %f, %f\n",i,mAt,mDur);
		if( ev > maxEv )
		{
			maxEv = ev;
			attack = mAt;
			duration = mDur;
		} // if new max
		temp = dynamic_cast<TQTRACK*>(temp->Next());
	} // while tracks
	
	std::cout << "at-accuracy " << attack ;
	std::cout << " dur-accuracy " << duration ;
	std::cout << " #events " << maxEv << endl;

}

/*
detect the meter by using autocorrelation (J. Brown)
  Todo: implement that algor can be used before quantization
  */
TTimeSignature TQMIDIFILE::detectMeter( /// mode = 0 -> scoretiming, mode = 1 -> ms timing 
										int mode)
{
	Printf("Meter detection ... ");
	// delete existing (default) meter sigs
	if( ControlTrack != NULL )
	{
		TMetaMeter *tempMeter = ControlTrack->FirstMeter();
		ControlTrack->deleteSameType( tempMeter, NULL );
	}
	else
	{
		Printf("Skipped Meter detection");
		TTimeSignature res;
		res.initSig(4,4,1);
		return res;
	}
	
	// do meter Detection
#define meterSigSize 11
	
	TTimeSignature *meterSigs = new TTimeSignature[meterSigSize+2];
	meterSigs[0].initSig(4,4,1.1);
	meterSigs[1].initSig(3,4,1);
	meterSigs[2].initSig(3,8,0.7);
	meterSigs[3].initSig(6,8,1);
	meterSigs[4].initSig(2,4,1);
	meterSigs[5].initSig(5,4,0.8);
	meterSigs[6].initSig(7,4,0.7);
	meterSigs[7].initSig(9,8,0.9);
	meterSigs[8].initSig(5,8,0.75);
	meterSigs[9].initSig(11,8,0.7);
	meterSigs[10].initSig(12,8,0.5);
	meterSigs[11].initSig(3,2,0.9);
	meterSigs[12].initSig(6,16,0.7);
//	meterSigs[11].initSig(3,16,0.8);
//	meterSigs[12].init(13,8);
	
	int i;
	for( i = 0; i < meterSigSize; i++ )
		meterSigs[i].prob = 0;
	
	
	const char* iniVal = getInifile()->GetValChar("TIMESIGINTEGRSIZE","8/1",
					"[Fraction] Integration size for time signature autocorrelation");
	TFrac integrSize(iniVal);
	if( integrSize <= 0L )
	{
		integrSize = TFrac(8,1);		
	}
		
		
	TNOTE *moreNotes = NULL;	
	/// data elements of TQMIDIFILES might be used as meter signature
	/// changes in future version
	/// inferred barlengths
//	barlengthList = new TDoubleChunkList();
	/// inferred time positions for barlengths
//	barlengthPosList = new TDoubleChunkList();

	
	/// for ms timing, barlength = prevBarlengthId * index

	double startPosMS = 0;
	double integrSizeMS = 10000, // N = 10s
		   maxBarlengthMS = 6000, // m <= 6s
		   resolutionMS = 30;
	/// array of possible barlengths size = integrSizeMS/resolutionMS
	double *probsMS = NULL;

	int msArraySize = 0;

	int cValues = 0;
	do
	{
		moreNotes = NULL;
		
		// evaluate in all tracks [startPosMS, startPosMS+windowSizeMS)
		TTRACK *track = FirstTrack();
		while( track )
		{
			msArraySize = 0; // reset

			int voice = -1; 
//			for( voice = 0; voice < cVoice; voice++ )
			{
				// get the first note in voice at a given start position
				// make sure that notes of all voice are in a certain time window
				TNOTE *note = track->FirstNoteObject(voice);
				/* use for ms working 
				if( !moreNotes )
					moreNotes = track->FirstNoteObject(voice,
														startPosMS + windowSizeMS );
				*/
				if( note )
				{
					startPosMS =  note->AbsTimeMS(this);
					cValues++;
					// Todo: meter detection, test at different positions in piece -> find meter changes
					// calculate a probability for each TimeSignature in th array
					// TFrac curIntegrSize = integrSize - note->GetAbsTime();
					double *probsTrack = autoCorrelation( QNOTE(note),  // calculate matching factor for each barlenth
														  voice,	 	
															meterSigSize,
															meterSigs,
															integrSize,  // normalizer
															this );

					// copy current values
					for( i = 0; i < meterSigSize; i++ )
						meterSigs[i].prob += probsTrack[i];
					if( probsTrack )
						delete [] probsTrack;

					/// get a list of probs for ms barlengths
					double *probsTrackMS = autoCorrelationMS(QNOTE(note),
											voice,
											integrSizeMS, 
											maxBarlengthMS,
											resolutionMS,
											/// call by reference, will be changed if <= 0 
											&msArraySize,
											this);
					/// ms ArraySize should be integrSize / resolution
					/// first call
					if( !probsMS && 
						msArraySize)
					{
						probsMS = new double[msArraySize];
						for( i = 0; i < msArraySize; i++ )
						{
							probsMS[i] = 0;
						} // for
					} // if
					
					// copy current ms calues
					for( i = 0; i < msArraySize; i++ )
					{
						probsMS[i] += probsTrackMS[i];
					}
					if( probsTrackMS )
						delete [] probsTrackMS;
				} // if note
			} // for voice
			track = track->Next();
		} // while track		
		
		
		/*
		if( probsMS )
		{
			// do averaging
			for( i = 0; i < msArraySize; i++ )
			{
				probsMS[i] /= (double)cValues;
				if( i > 0 &&
					resolutionMS*(i+1) > 750 &&
					probsMS[i] > probsMS[maxIndex] )	
				{
					maxIndex = i;
				}
			} // for
			if( prevBarlengthId != maxIndex )
			{
				for( i = 0; i < msArraySize; i++ )
				{				
					printf("%f = %f\n",
						resolutionMS * (i+1),
						probsMS[i] );				
				}				
				prevBarlengthId = maxIndex;
				barlengthList->add( resolutionMS * (maxIndex+1) );
				barlengthPosList->add( startPosMS);
			} // for

			// reset
			for( i = 0; i < msArraySize; i++ )
			{
				probsMS[i] = 0;
			}
		} // if probsMS
		*/
		startPosMS += integrSizeMS;			
	}
	while( moreNotes 
		// && startPosMS < 20 
		); 
	
/*
#ifdef _DEBUG	
		FILE *out = fopen("_barlength.txt","at");
		for( int c = 0; c < barlengthList->count(); c++ )
		{
			fprintf(out, "%f : %f\n", 
				barlengthPosList->get(c),
				barlengthList->get(c));
		}
		fclose(out);
#endif
*/
	// todo: add probs of a merged track, see Bach examples
	
	// find best score time TimeSignature
	qsort(meterSigs, 
		  meterSigSize, 
		  sizeof(TTimeSignature), 
		  compareTimeSig );	
		  
	int maxIndex = meterSigSize-1;
	// Todo: ask user if not significantly the best value!
/*	
	for( i = 1; i < meterSigSize; i++ )
	{
		//		if( probs[i]*meterSigs[i].weightI > probs[maxIndex]*meterSigs[i].weightI )
		if( probs[i] > probs[maxIndex] )
			maxIndex = i;
	}
*/	
	
#ifdef _DEBUG
	FILE *out = fopen("_metersigs.txt","at");
	for(i=0; i < meterSigSize; i++ )
		fprintf(out,"sign: %d/%d, prob:%f, weighted%f\n",
		meterSigs[i].numeratorI,
		meterSigs[i].denominatorI,
		meterSigs[i].prob,
		meterSigs[i].prob * meterSigs[i].weightI);
	
	fclose(out);
#endif
	if( probsMS )
		delete [] probsMS;

	TTimeSignature TimeSignature = meterSigs[maxIndex];
	// Todo: normalize to most seen duration
	//	TimeSignature.normalize(); 12/8->6/8 2/4 -> 4/4
	TMetaMeter *meterSigEvent = new TMetaMeter(0L, 
								TimeSignature.numeratorI,
								TimeSignature.denominatorI );
	if( ControlTrack != NULL )
	{
		TMetaMeter *temp = ControlTrack->FirstMeter();
		ControlTrack->deleteSameType( temp, NULL );
		// skip other information at time zero!
		TMusicalObject *pObj = NULL;
		TMusicalObject *tObj = ControlTrack->FirstObject(-1);
		if( tObj &&
			tObj->GetAbsTime() <= 0L )
		{
			pObj = tObj;
			tObj = tObj->TMusicalObject::GetNext(-1);
		}
		if( pObj )
		{
			pObj->SetNext( meterSigEvent );
			meterSigEvent->SetNext( tObj );
		}
		else
		{
			ControlTrack->Insert(meterSigEvent);
		}
	}
	if( meterSigs )
		delete [] meterSigs;
	cout << TimeSignature.numeratorI << "/" << TimeSignature.denominatorI << " done. \n";
	return TimeSignature;
}
	

TFrac TQMIDIFILE::meterPhaseShift( void )
{
	Printf("Timesignature offset calculation ... ");
	TMetaMeter *temp = ControlTrack->FirstMeter();
	if( !temp )
		detectMeter(1 );
	temp = ControlTrack->FirstMeter();
	
	TTimeSignature TimeSignature(4,4,1.0);	
	if( temp )
		TimeSignature = temp->Meter();

	// ToDo:  pickup detection -> phase check for different starting points
	// count possible pickup points
	TFrac epsilon( getInifile()->GetValChar("MATCHWINDOW","1/24","[fraction] epsilon size for meter detection autocorrelation"));
	// get optimum offset
	TFrac offset(-1,1);	
	// there should be only a single track at this point!
	TTRACK *track = FirstTrack();
	while( track )
	{			

//		TFrac *offsets = new TFrac[cVoice];
		TFrac offset;
		int voice = -1; // all voices
//		for( voice = 0; voice < cVoice; voice++ )
		{
			TNOTE *note = track->FirstNoteObject(voice);
			if( note )
			{
//				i = 0;
				TFrac barLength( TimeSignature.numeratorI,
								TimeSignature.denominatorI );

/*
				if( offsets[voice] >= barLength )
					offsets[voice] -= barLength;
*/
				if( offset > barLength )
					offset -= barLength;
				Printf(offset);
			}
			else
			{
				offset = -1;
				// offsets[voice] = -1;
			}
		} // for
		// check for best offset
//		for( voice = 0; voice < cVoice; voice++ )
		voice = -1; // all voices
		{
			/*
			if( offset < 0L &&
				offsets[voice] >= 0L )
			{
				offset = offsets[voice];
			}
			else if( offset >= 0L &&
					 offsets[voice] >= 0L &&
					 offset != offsets[voice] )
			{
				Printf("No unique upbeat offset!\n");
				offset = -1;
				voice = cVoice;
			}
			*/
		}
		// delete [] offsets;
		track = track->Next();
	} // while
	
	// keep globale offset and shift everything during output
	if( offset > 0 )
	{
		glOffset = offset;
//		Printf("%ld/%ld ", glOffset.numerator(), glOffset.denominator() );
	}	
	// todo: meter changes: phase check for every bar, -> segmentation
	Printf(" done \n");	
	return TimeSignature;
}



int TQMIDIFILE::writeMIDI( const string &midiFilename )
{
	string MidiFilename;
	FILE *out;
	out = fopen( midiFilename.c_str(), "rb");
	if( out )
	{
		MidiFilename = "_";
		MidiFilename +=  midiFilename;
		fclose(out);
	}
	else
	{
		MidiFilename = midiFilename;
	}
	out = fopen(MidiFilename.c_str(),"wb");
	if( out )
	{
		// write file header
		// write tracks
		// go back and write file length
		Printf("Sorry, TQMIDIFILE::writeMIDI is not implemented yet!\n");
		fclose(out);
		return 0;
	}
	return 1;
}

