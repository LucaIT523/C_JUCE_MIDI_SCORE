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
|	Datum	    : 17.10.1996-03,2011
|	Aufgabe   : deklaration of  TDoubleBinClass und TDoubleBinList 
|				  
------------------------------------------------------------------*/
#ifndef __LISTE_H__
#define __LISTE_H__
//----------------------------------------------------------------
#include "element.h"
#include <stdio.h>
#include "fragm.h"
//-----------------------------------------------------------------
class TPATTERN;

/// helper class
typedef struct{
	/// distance to classPtr
	double distance;
	/// bestfitting class
	TDoubleBinClass *classPtr;
} TDistanceStruct;


//-----------------------------------------------------------------
/// a list of TDoubleBinClass used for statististical classes of score-durations or abs-durations or IOIRatios
class TDoubleBinList {	
protected :
	double precR;
	double precL;


//    double MaxWeightI;
    double MinWeightI;

	/// is 1 after adding of new values
	char lpDirty;
	char rpDirty;


    /// # of elemnts
    int CountI;	
    /// Set range for each class

    /// max upper limit [%]
    int UpperboundI;
    /// max lower limit [%] 
    int LowerboundI;

    /// normalising factor
    double Mult;		
    int firstValue;

    TDoubleBinClass *Head;
    /// todo ???
    int CompareValue( double value1,
                      double value2 );

	/// alpha value for distance calculation
	double alpha;
	const char *prefix;
	double avErrorSumI;
	double errorSigmaSumI;
	int cValsI;

    
public :
	// variance for single values 
	double valueSigma;
	/// add to exact matching class, create class if necessary
	TDoubleBinClass * addExact( double val );
	/// create and add if no exact class already exists, return NULL otherwhise
	TDoubleBinClass * addIfNew( double val );
	virtual void addPattern( TPATTERN *pattern);
	virtual int parseLine( string line );
    /// create distribution (set weights) depending on bias values
    void createDistribution( void );
	/// distance of val or duration to ptr class
	double distance( TDoubleBinClass *ptr, 
					 double *val, 
					 TFrac *duration);
	/// remove a class
	int remove( int id );
//	double ex2;
//	double ex;
	double rAlpha( void );
	double lAlpha( void );
//	double precise( void );
	double preciseR(void);
	double preciseL( void );
    TDoubleBinList( double MaxUp /*! threshold in % for upper limit */,
                    double MaxLow /*! threshhold in % for lower limit */,
			double al /* alpha value for distance to classes */);
    
	
	virtual int readIni( const char *fname );
	virtual int writeIni( const char *fname );
	virtual string iniHeader( void );
    
	double deltaI;
	void setMinweightDeltaRel( double rel );
    double maxWeightOfElements( void );
    double avWeight( void );
	// double resetMaxWeight( double maxWeight );
    double setMinWeight( double minWeight );
    void normClasses( void );
    void insert( TDoubleBinClass *ptr );

    /// return best TDoubleBinClass with weight <= ptr->weight
    TDoubleBinClass *getBest( TDoubleBinClass *ptr = NULL);
    
	TDoubleBinClass * factory( double value, 
							    int ID,
                             double bias);
	TDoubleBinClass * addClass(double val,
                        double bias = 1);

    /// find closest class to val or duration
    TDistanceStruct closestClass( double *val,  
								  TFrac  *duration = NULL );
	virtual ~TDoubleBinList( void );
    /// normalise value and add  to it's closest class
    
    int  Add( double Value );
    /*
    int  Add( double Value )
    {
        double temp = Value;
        return Add( temp );
    }
     */
    
    /// add the value to it't closest class
    virtual TDoubleBinClass *addValue(double val, TDoubleBinClass *ptr = NULL);

	void addValue(double duration1,
						  TDoubleBinClass *classPtr1,
						  double w1,
						  double duration2,
						  TDoubleBinClass *classPtr2,
						  double w2,
						  double duration3,
						  TDoubleBinClass *classPtr3,
						  double w3,
						  double duration4,
						  TDoubleBinClass *classPtr4
						  );



    /// seems to be similar to addValue
    /// todo: is it still used
    double Insert( double value,
               int  *classId );
    TDoubleBinClass *Find( int classId );
    /// todo ???
    void MakeTempo( int filePpq,
			int fileTempo );
	void Reset( void  );
	char FindQuarter( double PlayPPQ,
                   int ppq );
    /// get the number of existing classes
    int Count( void ) 
    { return CountI; };
    double Average( void );
	double SSQ( void );
    virtual void write( ostream *out = NULL,
						double *val = NULL,
						TFrac *duration = NULL);
	virtual void debug( FILE *out = NULL );
	double avError(void);
	double errorSigma(void);
	double setMinMaxRel(double minMaxRel);
	double maxWeight(void);
private:
}; // TDoubleBinList



//-----------------------------------------------------------------


#endif
