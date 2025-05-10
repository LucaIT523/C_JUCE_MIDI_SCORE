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
|	filename : FUNCS.H
|	Author     : Juergen Kilian
|	Date	    : 17.10.1996-98, 2011
|   Prototypes for quantize functions
|   03/05/26 code cleaning 
 ------------------------------------------------------------------*/
#ifndef __q_funcs_h__
#define __q_funcs_h__
//---------------------------------------------
#include "defs.h"
//---------------------------------------------
class TQNOTE;
class TQMIDIFILE;
class THMIDIFILE;
class TFracBinList;
class TPFILE;

void setDefaultPattern( TPFILE *pfile );
//---------------------------------------------
// validate quantisation data, select best alternative
void setQData( TQNOTE *From,
					TQNOTE *To,
					TAbsTime  EndLastMatch );
//---------------------------------------------
/// calculate alternative attackpoints
void	getBestNSecond( TFrac *delta,
						double *prob,
						int &bestPos,
						int &secondPos,
						TAbsTime time,
						int cAttacks, // fieldSize
						char *valid ); 
//---------------------------------------------
/*
void possibleAttackpoints( TAbsTime abstime,
							int  tempo,
							TFrac *delta,
							double *prob,
							int cQAttacks,
							char *valid);
							*/
//---------------------------------------------
int possibleAttackpoints( TAbsTime   abstime,
							TAbsTime  lsWindow /*![abstime - lsWindow, .. */,
                          TAbsTime rsWindow /*! ... , abstime+rSWindow */, 
                          TFracBinList *qAttacks,
//							int cQAttacks,
							char *valid,
							TFrac *delta);
//---------------------------------------------
void valueAttackpoint( TQNOTE *note,
                       char *valid,
                       TFracBinList *qAttacks,
						TFrac *delta,
						double acurateness /*! (0...1] */,
					   double *prob	);
//---------------------------------------------
/// copy best an second best attackpoint into note
/// return id of best attackpoint
int selectAttackpoint( TQNOTE      *note,
					  TFrac *delta,
					  double *prob,
						char	 *valid,		// entry is one if qDuration is inside range
                       TFracBinList *bestDurations,
                       int cAttacks);
//---------------------------------------------
int possibleDurations(
						TQNOTE *note,		// current note
						TAbsTime duration,	// played duration
						TAbsTime minQDuration,	// width of calculation window
						TAbsTime maxQDuration,	// width of calculation window
						TFracBinList *qDurations,	// list of allowed durations
						TAbsTime *delta,		// distance to each duration
						char	 *valid		// entry is one if qDuration is inside range
//						int      nDurations
						);	// number of valid durations
//---------------------------------------------
void valueDuration( TQNOTE      *note,
					char *valid,
					TFracBinList *qDurations,	// list of allowed durations
					TFrac *delta,
					double acurateness /*! (0...1] */,
					double *prob );
//---------------------------------------------
/// select the two best durations and copy them into note
/// return id of best duration
int selectDuration( TQNOTE      *note,
						TFrac *delta,
					    double *prob,
						char	 *valid,		// entry is one if qDuration is inside range
						TFracBinList *selDurations,
						int cDurations);
//---------------------------------------------
//---------------------------------------------
/*
TAbsTime diff2NextQuarter( TAbsTime start,
							  int  ppq );
*/
//---------------------------------------------
#define TDurationList TFracBinList
/// read valid durations reulst mus be deleted!
TDurationList * readDurationList( const char *fname ); 
/// read valid attackpoints reulst mus be deleted!
TDurationList * readAttackpointList( const char *fname );

int countValues(const  char *str );

//---------------------------------------------
void process( THMIDIFILE  *Infile,
				  char *Inifilename,
				  ostream &gmnOut,
				  char *Patternname,
				  char *Maskname,
				  long int  TimeSigNum,
				  long int  TimeSigDenom );
//---------------------------------------------
class TQTRACK;
void	ticksToNoteface( TQTRACK *track, int ppq );

void quantizeToNotes(TQTRACK *ControlTrack,
							 TQTRACK *tempTrack);
						
double durationalAccent(double IOI1, double IOI2, double IOI3);

double primeDenomDistance( long int attackDenom,
							long int durDenom);

#endif
