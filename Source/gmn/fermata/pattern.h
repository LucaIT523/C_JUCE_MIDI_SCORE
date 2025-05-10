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
|	filename : PATTERN.H
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	    : 17.10.1996-98-03
|	classdefinition
|					TPATTERN (PATTERN.CPP)         (= Pattern )
|	2003/06	similarity functions
-----------------------------------------------------------*/
#ifndef __PATTERN_H__
#define __PATTERN_H__
//---------------------------------------------------------------
#include "spattern.h"
#include "q_note.h"
#include "../lib_src/similarity/dynaprog.h"
//---------------------------------------------------------------
//-----------------------------------------------------------------
class TPFILE;
/// helper class for storing pattern ptr and distance measures
class TPatternDistance
{
public:
	TPatternDistance( void )
	{
		ptr = NULL;
		distance = -1;
		normDistance = -1;
		pos = NULL;
		id = -1;
	}
	int id;
	/// pattern
	TPATTERN *ptr;
	/// selected startpos for pattern
	TCLICKNOTE *pos;
	/// penaliest distance
	double distance;
	/// original distance, equivalent to typicalDistance of TPATTERN
	double normDistance;
};

class TCLICKTRACK;

/// class for gmn patterns including compare functions
class TPATTERN  : public TSPATTERN{
	friend class TPFILE;
	
	/* not used anymore
	virtual TQNOTE  *Compare( TQNOTE *note,
			 TFrac  *diffLength,
			 TFrac  *diffStart,
			 char  *pass,
			 char  mode,	/// test or copy
			 TAbsTime  *endLastMatch);
	*/
public:
	/// durational distance between note and first clicknote
	double typicalDistance;

	double headDurationDistance( TCLICKNOTE *note,
                             TDoubleBinList *IOIratioList,
                             TFracBinList *durationList,
                             TCLICKTRACK *clicktrack,
							 double normF,
							 double normFSigma);
							
	/// IOIratio distance between 2...n notes of pattern 
    double IOIratioDistance( TCLICKNOTE * from
							 /// last perfIOI, scoreIOI relation
							 );
	int IOIratioPrefix( TPatternDistance *, int size, int delta);
	char IOIratioSuffix(TPATTERN *p2,  int delta);
	int IOIratioSuffix( TPatternDistance *, int size, int delta);
	char IOISuffix(TPATTERN *p2,  int delta);
	int IOISuffix( /// list of pattern pointer with distance
					TPatternDistance *patList, 
					/// size of list
					int size, 
					/// delta position inside pattern
					int delta);
	/*
	double distance( TCLICKNOTE * from,
						 TDoubleBinList *IOIratioList );
	*/
	/// distance measure used for pattern quantisation
	double IOIDistance( TQNOTE *start,
						  TFrac &minStart,
						TFrac meter,
						double atAccuracy,
						double durAccuracy);
	/// prior depending on cUsed counter
    virtual double aPriori( double sum );

	// # of matches during current processing
	double curWeight( void );
	/// how often was this pattern used?
	double cUsed( void );
	
	/// write cUsed+cUsedCur to .gmn
	void finaliseCUsed( void );
	
	TPATTERN( void );
	virtual ~TPATTERN( void );

	/*
	virtual TQNOTE  *Set( TQNOTE *ptr,
			  TFrac  *diff,
			  int   ppq,
			  TFrac  *endLastMatch );

	virtual TQNOTE  *Compare( TQNOTE *note,
			 TFrac  *diffLength,
			 TFrac  *diffStart,
			 char  *pass,
			 TAbsTime  *endLastMatch);
	*/
	/// similarity function, 
	CDynaprog<TQNOTE *, TPNOTE *> *similarity( TQNOTE *start, 
												double &res,
												TFrac &minStart,
														double atAccuracy,
														double durAccuracy
														);

	/// write contents to .gmn file
	virtual void write( FILE *out,
						lgVoice *v = NULL );

	virtual string toString(lgVoice *v);
	//void ChangePpq( int oldppq, int newppq );
	
	/// set a new typical distance
	double updateTypicalDistance(double newDistance);
	/// get penalty for distance higher than typical distance
	double distancePenalty(double newDistance, double sigma);
protected:
	/// ptr to binclass of TPFILE
	TDoubleBinClass *binclass;
	/// increment cUsedCurrent counter
	void incCUsed( int delta = 1 );
	/// # of matches in current file
	int cUsedCurrent;
	/// shift pattern start time
	void shiftTo( TAbsTime pos );
}; // TPATTERN

double normIOIratio( double IOIratio );
double denormIOIratio( double IOIratio );

#endif
