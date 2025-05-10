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

#if !defined( __statist_h__ )
#define __statist_h__
#include <math.h>
#include <stdio.h>

#include "chunklist.h"
/// tool class for TFloatingAverage
typedef struct
{
	double decay;
	int range;
} TDecayStruct;


/// "ranged"-bidirectional average of double series
class TFloatingAverage
{
protected:
    /// limit for detection of "break points" nside the series 
	double  pBreakLimit;	
	double minRightAv( void );
	double maxRightAv( void );
	double minLeftAv( void );
	double maxLeftAv( void );
	double maxRightI,
			 maxLeftI,
			 minRightI,
			 minLeftI;

	int maxRightPos,
	 	maxLeftPos,
		minRightPos,
        minLeftPos;

    /// current position in list, -1 == undef
	int 	currentPos;			

    /// get real, valid range (!end of list)
	int rightRange( int pos, int range );
    // get real, valid range (!start of list) 
	int leftRange( int pos, int range );

    /// list of values
	TDoubleChunkList *valueListI;
    /// current size of list
    int cValuesI;

    /// # of averages
	int cAvI;
    /// list of summed weights
    double *z;
    /// list of current averages
    double *currentLeftAvI;
    /// list of current averages
    double *currentRightAvI; 
     /// list of current averages    
    double *averageI;

    /// list of decay values
	double *decayList;
    /// list of ranges
	int    *range;			

	virtual void calculateAv( void );
	void iterateMinMax( void );
	void calcRightMinMax( void );
    
public:
	double rightAv( int rangeID, int id);
    /// retrieve list of breakpoints, result must ve deleted!
	virtual int * getBreaks(int *listSize);
	void calcLeftMinMax(void);
	// virtual int isBreakPoint( void );
	TFloatingAverage( /// # of average classe
					  int classes, 
					  /// array of decay/range vals
					  TDecayStruct decayStruct[],  
					  /// threshold for break detection
					  double PBreakLimit);
	virtual ~TFloatingAverage( void );
	/// insert a new value into the list
	void addValue( double newVal );
	double currentLeftAv( int pos );
	double currentRightAv( int pos );
	/// total average
	double average( int pos );
	virtual int iterate(  //// 0 == goto first pos, 1 == next pos 
						 int pos  );
	/// debug
	void writeAvs( FILE *out );
	/// debug
	void writeValues( FILE *out );
	double maxVal( int *pos );
	double minVal( int *pos );
	double maxRight( int *pos )
		{ *pos = maxRightPos;
		  return maxRightI; };
	double minRight( int *pos )
		{ *pos = minRightPos;
		  return minRightI; };
	double maxLeft( int *pos )
		{ *pos = maxLeftPos;
		  return maxLeftI; };
	double minLeft( int *pos )
		{ *pos = minLeftPos;
		  return minLeftI; };
};

/// TFloatingAverage with "decay" approach isntead of range approach
class TDecayAverage : public TFloatingAverage
{
protected:
	virtual void calculateAv( void );

public:
//	virtual int * getBreaks(int *listSize);
	TDecayAverage( /// size of array
					int classes, 
					/// range/decay array
					TDecayStruct decayStruct[],
					double pBreakLimit):
		TFloatingAverage( classes, 
					  decayStruct,
					  pBreakLimit ){};
	virtual int iterate( /// 0 == first pos, 1 == next pos
						int pos );
};

/// calc resulting range if deacy is applied to each element
int calcRange( double decay );
/// limit a range to a decay
int limitRange( int oldRange,
				double decay );
/// get resulting decay for desired range
double calcDecay( int range );

double calcNorm( int range,
				  double decay);
				  
// double myPow( double base, int expt );

/// calc propability density function of normal distribution, 
double normal_pdf(double x,
				  double mean,
				  double var );
/// return exp(-(x-mean)^2 / (2*sigma^2)
double GaussWindow(double x,
				  double mean,
				  double sigma);

/// exp(-0.5*pow(x/sigma,2)
double GaussWindow(double x,
				  double sigma);
/// exp(-0.5*pow(x/sigma,k)
double kGaussWindow(double x,
				  double sigma,
				  int k);
				  
/// ini function for nDimAngle 				  
void initNDimAngle( double &XxY,
					double &XxX,
					double &YyY,
					/// initval 0 or 1
					double initVal = 0);
/// add value for nDimAgle
void updateNDimAngle( double Xi,
					  double Yi,
					  double &XxY,
					  double &XxX,
					  double &YyY);
/// get angle between n-dinmensional angle: acos(X*Y/(||X|| * ||Y||)
double nDimAngle(  double XxY,
				  double XxX,
				  double YyY);

#endif
