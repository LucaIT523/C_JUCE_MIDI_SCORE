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
|	filename : LISTE.H
|	Autor     : Juergen Kilian
|	Datum	    : 17.10.1996-03, 2011
|	Aufgabe   : deklaration of  TDoubleBinClass und TDoubleBinList 
|				  
------------------------------------------------------------------*/
#ifndef __ELEMENT_H__
#define __ELEMENT_H__
//-----------------------------------------------------------------
#include <iostream>
using namespace std;
#include <stdio.h>
#include "fragm.h"
#include <math.h>
//-----------------------------------------------------------------


class TListe;
class TFrac;

/// a single element of the TDoubleBinList class TDoubleBinClass represents a stastical value-class for the binStatistics modul
class TDoubleBinClass
{	
	friend class TDoubleBinList;

private :
	/// normal deviation
	double myI;
	/// normal deviation
	double sigma2I;

    /// average value, exponential smoothed
    double avI; 

    /// norm value of class
    double FirstVI;			// first/initial value used for this class
    /// generic weight of class
    double weightI;
    /// #entires attached to class
    int   CountI,
        /// bpm if class represents a quarter note
        PossibleTempoI,
        /// unique id 0...CountI -1
        IDI;	

    /// score duration of class
    TFrac PartsPerNoteI;	
    TDoubleBinClass *nextI;	
    TDoubleBinClass * PrevI;


public :
	virtual string toString( void );
        /// bias for arbitrary distribution, default = 1
        double bias;
    double sigma( void )
	{ return sqrt( sigma2I); };
	double my( void )
	{ return myI; };
	int cLocalMax;
	double stdDev( void );
	double phi( double val );
	double ex2;
	double ex;
	
	double V( void );
	double E( void );
	virtual void debug( ostream *out );
    double variance( void );
	double lVariance( void )
	{
		return preciseL;
	}
	double rVariance( void )
	{
		return preciseR;
	}
    double weight();

    TDoubleBinClass * Prev( void );
    void SetPrev( TDoubleBinClass *prev);
    TDoubleBinClass *Next( void );
    TDoubleBinClass *SetNext( TDoubleBinClass *ptr );


    /// distance between val and norm value of class
    virtual double distance( double *val, 
							TFrac   *dummyDuration,  
							double maxWeight,
							double lAlpha,
							double rAlpha,
							int cClasses);

    TDoubleBinClass( /// value of class
             double value,
              /// class id
              int  ID ,
              /// bias
              double b);
    virtual ~TDoubleBinClass(void){};
    void Reset( void );

//    void debug( FILE *out );	
    /// merge with next ptr
    void AddNext( void );	
    /// todo ???
    // void Change( double quot );
    /// todo ???
    void MakeTempo( int filePpq,
                    int fileTempo );
    char Find( int id );

    virtual double Value( void )		
    { return FirstVI; };
    virtual double FirstV( void )	
    { return FirstVI; };
    void SetPartsPerNote( TFrac ppn )	
    { PartsPerNoteI = ppn; };
    TFrac  PartsPerNote( void )			
    { return PartsPerNoteI; };
    int  Tempo( void )            	
    { return PossibleTempoI; };
    int  ID( void )			
    { return IDI; };
    int Count( void )				
    { return CountI; };
    void setID( int i )
    { IDI = i; };
protected:

    double preciseL;
    double preciseR;

    /// inc element counter
    virtual void addValue(double val, double delta);
    double normalize( double delta,
                     double minWeight);

    void setWeight( double weight);
    /// add a value
    double CountUp( double newvalue );
}; // TDoubleBinClass
//-----------------------------------------------------------------
/// get a new duration by prev duration and IOIratio
TFrac getDuration( /// duration of previous note
				   TFrac prevDuration, 
				   /// selected IOIratio class
				   TDoubleBinClass *tempClass); 


#endif
