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
|	filename : PATTERNFILE.H
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	    : 17.10.1996-98-03
|	classdefinition
|					und TPFILE (PATTERN.CPP)	(= Datei mit Pattern )
|	2003/06	added similarity functions etc
-----------------------------------------------------------*/
#ifndef __PATTERNFILE_H__
#define __PATTERNFILE_H__
//---------------------------------------------------------------
#include <iostream>
#include <sstream>

#include "spatternfile.h"
#include "pattern.h"

using namespace std;
//---------------------------------------------------------------
//-----------------------------------------------------------------
/// PatternFILE
class TDoubleBinList;
class TCLICKTRACK;
class TFracBinList;

/// pattern file class including a list if TPATTERN
class TPFILE  : public TSPFILE{ 
private :
	/// dynamic bin list for pattern usage
	TDoubleBinList *weightList;

public :
	/// get best pattern for quantisation matching
	TPATTERN *bestQMatch( TQNOTE *note , 
						TFrac &minStart, 
						double  &distance, 
						double atAccuracy, 
						double durAccuracy,
						TFrac patternLength,
						TFrac curMeter);
	/// get best matching pattern
	TPATTERN * bestIOIratioMatch( TCLICKNOTE *from, 
						 /// distance to best match
						 double &distance,
						 TDoubleBinList *IOIratioList,
						 TFracBinList *durationList,
						 TCLICKTRACK *clicktrack,
						 /// array for sorted distances, size MUST be == CPattern()
						 TPatternDistance *pArray,
						 /// size of array
						 int lSize,
						 /// last perfIOI, scoreIOI relation
						 double normF,
						 double normFSigma
						 );
	/// return aPriori probability depending on how many times this pattern was used before
	double pUsedBefore( TPATTERN *pat );
	/// return probability depending on how many times this pattern was used at the current processing
	double pUsedCur( TPATTERN *pat);
	/// return pUsedCur + pUsedBefore
	double pUsed(TPATTERN *pat);
	/// increment used at current file counter
	void incCUsed( TPATTERN *pat,
				   int delta = 1 );
	/// write cUsedBefore + cUsedCurrent into tag/file
	void finaliseCUsed( void );

	TPFILE( const char *name );
	virtual ~TPFILE( void );

	/// old similarity function
	/*
	virtual TPATTERN *Compare( TQNOTE *note, 
							   TFrac length );
	*/
	/// retrieve best alignment and similarity between played notes and pattern database
	virtual CDynaprog<TQNOTE *, TPNOTE *> *similarity( TQNOTE *note,
														double &sim /* alginment similarity 0 <= sim <= 1*/,
														TFrac &minStart,
														int &patternID,
														double atAccuracy,
														double durAccuracy);

	/// read .gmn file
    char 	 Read( void );

	virtual void debug( FILE *out = NULL);
	double typicalMatchDistance;
	double updateTypicalMatchDistance( double newDistance);
protected:
	/// # of total matches pattern->cUsedCur()
	double cUsedCur;
	/// total sum of pattern->cUsed()
	double cUsed;
}; //TPFILE



//-----------------------------------------------------------------
#endif
