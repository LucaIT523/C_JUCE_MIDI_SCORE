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

#ifndef __notestatlist_h__
#define __notestatlist_h__

#include "notestat.h"
#include "fragm.h"




///	list for duration classes with weight and probability, a list of TFracBinClass
class TFracBinList : public TDoubleBinList  
{
public:
	/// add to exact matching class, create a new class if not existing
	TFracBinClass * addExact( TFrac frac );
	/// create and add to a new class if no exact match exists, return NULL otherwhise
	TFracBinClass * addIfNew( TFrac frac );
	virtual string iniHeader( void );
	/// write as gmn syntax
	virtual int writeIni( const char *fname );
	/// read gmn file
	virtual int readIni( const char *fname );

	virtual int parseLine(string line);

	/// find an exact matching class
	TFracBinClass * findExact( TFrac &duration );
	virtual TDoubleBinClass  *addValue(TFrac duration,
							    TDoubleBinClass *classPtr = NULL);
	virtual void addValue(TFrac duration1,
						  TDoubleBinClass *classPtr1,
						  double w1,
						  TFrac duration2,
						  TDoubleBinClass *classPtr2,
						  double w2,
						  TFrac duration3,
						  TDoubleBinClass *classPtr3,
						  double w3,
						  TFrac duration4,
						  TDoubleBinClass *classPtr4
						  );
    
    
	/// get probability of duration
    double minDist( TFrac pDuration);
    /// add a new class
    TFracBinClass *addClass( TFrac duration,
                         double bias = 1);
    TDoubleBinClass *factory( TFrac duration,
                               int id,
                               double bias);
	TFracBinList(double al /*! alpha value for distance to classes */);
	virtual ~TFracBinList();
	virtual void debug( FILE *out = NULL );
    /// get class with index i
    TFrac  operator[] (int i );
    /// renumber class-id, high->low
    void sort( void );
	double probability( const TFrac &duration);
	TFracBinClass * closestTypicalDuration(double IOI,
										   double sigma,
										   /// multiple for dur of result
										   int &n);
	void updateTypicalDurations(TFrac &dur,
							    double typicalDur,
								/// [0,1] new = old * (1-significance + new * significance)
								double significance);
	/// return tick duration depending on typicalQuarter IOI
	double tickDuration(TFracBinClass *ptr, int bestN);
	double tickDuration(TFrac &scoreDuration);
	double typicalQuarterIOI;

};
#endif
