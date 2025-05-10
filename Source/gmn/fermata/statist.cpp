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

/*---------------------------------------------------------------
	filename: statist.cpp
	author:   Juergen Kilian
	date      1998-2001-04
	IMplementation of floating avergae functions
----------------------------------------------------------------*/
#include "statist.h"
#include "portable.h"
#define DECAY_LIMIT 0.08

#define MIN_WEIGHT 0.11

/*
	average = x[1]*1/w + n[2]*1/w² + n[3]*1/w³
*/

TFloatingAverage::TFloatingAverage( int classes,
									TDecayStruct decayStruct[],
									double PBreakLimit)
{
	pBreakLimit = PBreakLimit;
	maxLeftI = -1;
	minLeftI = -1;
	maxRightI = -1;
	minRightI = -1;

	/// create Arrays
	cAvI = classes;

	currentLeftAvI = new double[classes];
	currentRightAvI = new double[classes];
	averageI = new double[classes];
	decayList = new double[classes];
	range = new int[classes];
	z = new double[classes]; 	/// normalize factor

	int i;
	// init all arrays
	for( i = 0; i < classes; i++ )
	{
		z[i] = 0;
		// init lists
		currentLeftAvI[i] = 0;
		currentRightAvI[i] = 0;
		averageI[i]  = 0;

		// copy decay info
		decayList[i] = 1-decayStruct[i].decay;
		range[i] = limitRange(decayStruct[i].range,
								decayList[i]);
		z[i] = calcNorm( range[i],
								decayList[i] );
	} // for
	valueListI = new TDoubleChunkList();
	cValuesI = 0;

	maxLeftPos = -1;
	maxRightPos = -1;
	minLeftPos = -1;
	minRightPos = -1;
	currentPos = -1;
}
TFloatingAverage::~TFloatingAverage( void )
{
	delete valueListI;	// list of values
	delete [] currentLeftAvI;	// list of current averages
	delete [] currentRightAvI;
	delete [] averageI;

	delete [] decayList;		// list of decays
	delete [] range;			// list of ranges
	delete [] z;				// normalize val

}
/*
	 range == current -> end of list
 */
void TFloatingAverage::calcRightMinMax( void )
{
	// init?
	if( currentPos >= cValuesI ||
		 currentPos < 0 )
		return;

	// init val == current
	minRightI = valueListI->get(currentPos);
	maxRightI = valueListI->get(currentPos);
	minRightPos = currentPos;
	maxRightPos = currentPos;

	// look to right until end of list
	int i;
	for( i = currentPos+1; i < cValuesI; i++)
	{
		if( valueListI->get(i) > maxRightI )
		{
			maxRightI = valueListI->get(i);
			maxRightPos = i;
		}
		else if( valueListI->get(i) < minRightI)
		{
			minRightI = valueListI->get(i);
			minRightPos = i;
		}
	} // for
}

/*
	range = current-1 -> 0
*/
void TFloatingAverage::calcLeftMinMax()
{
	// init?
	if( currentPos >= cValuesI ||
		 currentPos < 0 )
		return;

	if( currentPos >= 1 )
	{
		minLeftI = valueListI->get(currentPos-1);
		maxLeftI = valueListI->get(currentPos-1);
		minLeftPos = currentPos-1;
		maxLeftPos = currentPos-1;
	}
	else // set to invalid
	{
		minLeftI = -1;
		maxLeftI = -1;
		minLeftPos = -1;
		maxLeftPos = -1;
	}

	// look to left until head of list
	int i;
	for( i = currentPos-2; i >= 0; i--)
	{
		if( valueListI->get(i) > maxLeftI )
		{
			maxLeftI = valueListI->get(i);
			maxLeftPos = i;
		}
		else if( valueListI->get(i) < minLeftI)
		{
			minLeftI = valueListI->get(i);
			minLeftPos = i;
		}
	} // for
}


// compare MinMax Values after a 1-step iteration
void TFloatingAverage::iterateMinMax( void )
{
	if( currentPos >= cValuesI ||
		 currentPos < 0 )
		return;

	// first entry
	if( currentPos == 1 )
	{
		minLeftI = valueListI->get(currentPos-1);
		maxLeftI = valueListI->get(currentPos-1);
		maxLeftPos = currentPos-1;
		minLeftPos = currentPos-1;
	}
	else if( currentPos == 0) // make invalid
	{
		minLeftI = -1;
		maxLeftI = -1;
		minLeftPos = -1;
		maxLeftPos = -1;
	}

	// look for right min max
	if( currentPos == 0 ||	// first entry
		currentPos > maxRightPos || // current == max
		currentPos > minRightPos ) // current == min
	{
		calcRightMinMax(); // re-calc  min and max
	}

	// look for left min max
	if( currentPos > 1 ) // look only to left
	{
		// currentPos has already been iterated
		if( valueListI->get(currentPos-1) < minLeftI )
		{
			minLeftI = valueListI->get(currentPos-1);
			minLeftPos = currentPos-1;
		}
		else if( valueListI->get(currentPos-1) > maxLeftI )
		{
			maxLeftI = valueListI->get(currentPos-1);
			maxLeftPos = currentPos-1;
		}
	} // left
} // calcMinMax

void TFloatingAverage::addValue( double newVal )
{

	// insert new value
	valueListI->set(cValuesI, newVal);
	cValuesI++;
	// look for right min max
	if( cValuesI == 1 )
	{
		minRightI = newVal;
		maxRightI = newVal;
		minRightPos = cValuesI - 1;
		maxRightPos = cValuesI - 1;
	}
	else
	{
		int pos;
		if( newVal > maxVal(&pos) )
		{
			maxRightI = newVal;
			maxRightPos = cValuesI - 1;
		}
		else if( newVal < minVal(&pos) )
		{
			minRightI = newVal;
			minRightPos = cValuesI - 1;
		}
	} // else
}

/*
	do first time calculation of all averages
*/
void TFloatingAverage::calculateAv( void )
{
//	currentPos = 0;
	if( currentPos < 0 )
		currentPos = 0;

	// for all classes
	int i;
	for(i = 0; i < cAvI; i++ )
	{
		currentRightAvI[i] = 0;
		// for all elements in range
		int to = min_(cValuesI, currentPos + range[i] - 1);
		if( to >= currentPos )
		{
			double dSum = 0;
			for( int j = currentPos; j < to; j++ )
			{
				double d = pow( decayList[i], j - currentPos );
				currentRightAvI[i] += valueListI->get(j) * d;
				dSum += d;
			} // for
			currentRightAvI[i] /= dSum;
		} // if
		currentLeftAvI[i] = 0;
		to = max_(currentPos - range[i], 0);
		if( to < currentPos )
		{
			double dSum = 0;
			for( int j = currentPos-1; j >= to; j-- )
			{
				double d = pow( decayList[i], currentPos - (j + 1) );
				currentLeftAvI[i] += valueListI->get(j) * d;
				dSum += d;
			} // for
			currentLeftAvI[i] /= dSum;
		} // if
		averageI[i] = (currentRightAvI[i]
						+  currentLeftAvI[i]) / 2-0;
	} // for all classes
}

double TFloatingAverage::currentLeftAv( int rangeID )
{
	if( rangeID >= cAvI ||
		rangeID < 0)
		return 0; // invalid
	return currentLeftAvI[rangeID];
}
double TFloatingAverage::currentRightAv( int rangeID )
{
	if( rangeID >= cAvI || rangeID < 0)
		return 0;
	return currentRightAvI[rangeID];
}

double TFloatingAverage::average( int rangeID )
{
	if( rangeID >= cAvI || rangeID < 0)
		return 0;
	return averageI[rangeID];
}

/*
	Iterate averages
	pos == 0 : goto first position
	pos > 0 : goto next position
	result:
		1 : ok
		0 : end of list
*/
int TFloatingAverage::iterate(int state )
{
	if( currentPos < 0 ||  // old currentPos
		 state == 0 )
	{
		currentPos = 0;
		calculateAv(); // init
		iterateMinMax();
		return 1;
	}
	/*
		calclulate complete at each step
	*/
	currentPos++;

	// end of List?
	if( currentPos >= cValuesI )
		return 0;
	else
	{
		calculateAv(); // init
		iterateMinMax();
	}
	return 1;


	// for all classes
	int i;
	for(i = 0; i < cAvI; i++ )
	{
	/*
		j + k = 1

		An+1 = An*(1-k) + Xn+1*k

		A0   = 			 X0*k

		A1   = X0*k*j + X1*k
		A2   =    X0*k*j*j + X1*k*j + X2*k
		A3   = X0*k*j*j*j + X1*k*j*j +X2*k*j +X3*k

		A4   =              (X1*k*j*j +X2*k*j +X3*k)*j + X4*k
		A3   = X0*k*j*j*j + (A4-X4*k)/j

		An+1 = An*j + Xn+1*k

	*/
//		k = decayList[i];
//		j = (1-k);
		double k = (double)1/(double)range[i];
		double j = decayList[i];

		// curRightAv = val[cur+0]/decay^1+..val[cur+range-1]/decay^range
		// remove current pos from Right
		currentRightAvI[i] -= (((double)valueListI->get(currentPos))*k);
/*
		if( j > 0 )
			powDecay = pow( j, (range[i]-1) );
		else
			powDecay = 1;
*/
		// shift all elemnts one step right
		if( j > 0 )
			currentRightAvI[i] /= j;

		// j == decayList
		double powDecay;
		if( j > 0 )
			powDecay = pow( j, rightRange(currentPos, range[i]-1));
		else
			powDecay = 1;
		//powDecay == j^range

		// add current+range to Right
		if( currentPos + (range[i]) < cValuesI )
		{
			currentRightAvI[i] += (valueListI->get(currentPos+(range[i])))
											*k*powDecay;
		}

		if( j > 0 )
			powDecay = pow( j, leftRange(currentPos, range[i]-1));
		else
			powDecay = 1;


		// remove current-range from Left
// old		if( currentPos - (range[i]-1) >= 0 )
		if( currentPos - range[i] >= 0 )
		{
			currentLeftAvI[i] -= (valueListI->get(currentPos-range[i]))
										*k*powDecay;
		}

		// add current pos to Left
		currentLeftAvI[i] *= j;
		if( currentPos+1 < cValuesI )
			currentLeftAvI[i] += (valueListI->get(currentPos))*k;
/*
	averageI[i] = (currentRightAvI[i]+
						  (currentLeftAvI[i] -
						  valueListI[currentPos]*decayList[i]); // dont' count double time
*/
		if( currentPos+1 < cValuesI )
		{
			averageI[i] = (currentRightAvI[i]*range[i] +
						  currentLeftAvI[i]*range[i]);
/* old						  - (double)valueListI[currentPos+1])/
						  ((range[i]*2)-1) ; // dont' count double time
*/
		}
		else
			averageI[i] = 0;

	}
	currentPos++;

	iterateMinMax();
	// end of liust reached?
	if( currentPos >= cValuesI )
		return 0;

	return 1; // not at end of list
}

void TFloatingAverage::writeValues( FILE *out )
{
	if( !out )
		return;

	int pos;
	fprintf(out,"cValues: %d\n", cValuesI);
	fprintf(out,"min: %f, max: %f\n", minVal(&pos), maxVal(&pos));
	int i;
	valueListI->Debug(out);
	for( i = 0; i < cValuesI; i++)
	{
		fprintf(out,"%f, ",valueListI->get(i));
	}
	fprintf(out,"\n");
}
double TFloatingAverage::maxVal( int *pos )
{
	if( maxRight(pos) > maxLeft(pos) )
		return maxRight(pos);
	return maxLeft(pos);
}
double TFloatingAverage::minVal( int *pos )
{
	if( minRight(pos) < minLeft(pos) )
		return minRight(pos);
	return minLeft(pos);
}
void TFloatingAverage::writeAvs( FILE *out )
{
	if( !out )
		return;
	fprintf(out,"currentPos %d, val: %f ",
						currentPos,
						valueListI->get(currentPos));
	fprintf(out,"\n");
	double frac;
	int pos;
	fprintf(out,"Lmin: %f, Lmax: %f,", minLeft(&pos), maxLeft(&pos));
	fprintf(out,"Rmin: %f, Rmax: %f\n", minRight(&pos), maxRight(&pos));

	int i;
	for( i = 0; i < cAvI; i++)
	{
		fprintf(out,"decay: %f, range: %d\n", 1-decayList[i], range[i]);
		frac = (currentLeftAv(i)-currentRightAv(i))/
				(currentLeftAv(i)+currentRightAv(i));
		fprintf(out,"average: %f, left: %f, right: %f, frac: %f\n",
						average(i),
						currentLeftAv(i),
						currentRightAv(i),
						frac);
		fprintf(out,"--------------\n");
	};
		fprintf(out,"-------------------------\n");

	return;
}
int TFloatingAverage::rightRange( int pos, int range )
{
/*
	pos       0   1   2   3    4    5
	range 1   1   1   1   1
	range 2   1   2   2   2    2    2
	range 3   1   2   3   3    3    3
*/
	if( range+pos >= cValuesI )
		return cValuesI - pos;
	return range;
}

int TFloatingAverage::leftRange( int pos, int range )
{
/*
	cVal = 6
	pos       0   1   2   3    4    5
	range 1   				 1		1	  1
	range         2   2   2    2    1
	range 3  3    3   3   3    2    1
*/
	if( range > pos )
		return pos+1;
	return range;
}


/* do first time calculation of all avergares,
 calculate averages at currentPos
 */
void TDecayAverage::calculateAv( void )
{
//	currentPos = 0; 
	if( currentPos < 0 )
		currentPos = 0;
	if( currentPos < -1 ) // guard
	{
		currentPos = 0;
	}

	if( currentPos >= cValuesI )
		return;

	// for all classes
	int i;
	for(i = 0; i < cAvI; i++ )
	{
		double j = decayList[i];  //0...1

		// limit the range
		if(range[i]-1 >= cValuesI )
		{
			range[i] = cValuesI;
			z[i] = calcNorm( range[i],
								  j );
		}

		//---- Right Average -----
		// fill from right to left
		// right: 0...range-1
		if( currentPos + range[i]-1 < cValuesI )
			currentRightAvI[i] = valueListI->get(currentPos + range[i]-1); // init
		else
			currentRightAvI[i] = 0;
		int n;
		for( n = currentPos + range[i]-2; n >= currentPos; n-- )
		{
			if( n < cValuesI )
			{
				currentRightAvI[i] = currentRightAvI[i]*j
											+ (valueListI->get(n));
			}
		} // for
		// currentRightAv == val[0]+val[1]*j + ... + val[range-1]*j^range-1
		//   j = decay

		// normalize
//		currentRightAvI[i] /= z[i];

		// Left averagre
		//---- Right Average -----
		// fill from left to right
//old		if( currentPos - range[i] + 1 >= 0 )
		if( currentPos - range[i] >= 0 )
			currentLeftAvI[i] = valueListI->get(currentPos - range[i]); // init
		else
			currentLeftAvI[i] = 0;

//old		for( n = currentPos - range[i] + 2; n <= currentPos; n++ )
		for( n = currentPos - range[i] + 1; n < currentPos; n++ )
		{
			if( n >= 0 )
			{
				currentLeftAvI[i] = currentLeftAvI[i]*j
											+ (valueListI->get(n));
			}
		} // for

//		currentLeftAvI[i] = valueListI[0]; // init

		// normalize
//		currentLeftAvI[i] /= z[i];

		// global average
		if( currentPos < cValuesI )
		{
			averageI[i] = (currentRightAvI[i] +
						  currentLeftAvI[i]);
/*old			- (double)valueListI[currentPos])/
						  ((z[i]*2)-1) ; // dont' count double time
						  */
		}
		else
			averageI[i] = 0;
//		averageI[i] = currentRightAvI[i]; ///z[i];
	} // for i
	iterateMinMax();
}// calculateAV

/*
	Iterate average
	pos == 0 : goto first position
	pos > 0 : goto next position
	result:
		1 : ok
		0 : end of list
*/
int TDecayAverage::iterate(int pos )
{
	if( currentPos < 0 ||
		 pos == 0 )
	{
		currentPos = 0;
		calculateAv();
		return 1;
	}
	// end of List?
	if( currentPos >= cValuesI )
		return 0;

	// for all classes
	int i;
	for(i = 0; i < cAvI; i++ )
	{
		double j = decayList[i];

		// denormalize
//		currentRightAvI[i] *= z[i];

		// curRight = val[cur]*decay^0 + val[cur+1]*decay +...
		//								..+val[cur+range-1]*decay^range-1

		// remove current pos from Right
		currentRightAvI[i] -= valueListI->get(currentPos);

		// curRight = val[cur+1]*decay +...
		//								..+val[cur+range-1]*decay^range-1

		// shift right
		if( j > 0 )
			currentRightAvI[i] /= j;

		// curRight = val[cur+1]* +...
		//								..+val[cur+range-1]*decay^(range-2)


		// powDecay = j^range[i]-1


		double powDecay;			
		if( j > 0 )
//			powDecay = myPow( j, range[i]-1); // -1
			powDecay = DECAY_LIMIT;
		else
			powDecay = 1;

		// add val[current+range] to Right
		if( currentPos + (range[i]) < cValuesI )
		{
			currentRightAvI[i] += valueListI->get(currentPos+(range[i]))
										  *powDecay;
		}
		// curRight = val[cur+1]* +...
		//								..+val[cur+range-1]*decay^(range-2)
		//								+val[cur+range-1+1]*decay^(range-1)

		// normalize
//		currentRightAvI[i] /= z[i];

		if( j > 0 )
//			powDecay = myPow( j, range[i]-1);
			powDecay = DECAY_LIMIT;
		else
			powDecay = 1;

		// un-normalize
//		currentLeftAvI[i] *= z[i];

		// remove val[current-range] from LeftAv
		if( currentPos - range[i] >= 0 )
//old		if( currentPos - (range[i]-1) >= 0 )
		{
			currentLeftAvI[i] -= (valueListI->get(currentPos-(range[i])))
										*powDecay;
		}

		// shift right
		currentLeftAvI[i] *= j;

		// add current pos to Left
		if( currentPos+1 < cValuesI )
//old			currentLeftAvI[i] += ((double)valueListI[currentPos+1]);
			currentLeftAvI[i] += (valueListI->get(currentPos));

		// normalize
//		currentLeftAvI[i] /= z[i];


		if( currentPos+1 < cValuesI )
		{
			averageI[i] = (currentRightAvI[i] +
						  currentLeftAvI[i]);
//old						  - (double)valueListI[currentPos+1])/
//						  ((z[i]*2)-1) ; // dont' count double time
		}
		else
			averageI[i] = 0;
	} // for

	currentPos++;
	iterateMinMax();
	// end of liust reached?
	if( currentPos >= cValuesI )
		return 0;

	return 1; // not at end of list
}

double calcDecay( int range )
// res^range == DECAY_LIMIT
{

	double res;

	if( range > 1 )
#ifdef powl
		res = powl( DECAY_LIMIT, (long double)1/(range-1));
#else
                res = pow( DECAY_LIMIT, (double)1/(range-1));
#endif
        else
		res = 0;
	return res;
}

int limitRange( int oldRange,
				double decay )
{
	int newRange = calcRange( decay );
	if( newRange < oldRange )
		return newRange;

	return oldRange;
}

int calcRange( double decay )
{
	// check bounds
	if( decay >= 1)
		return MAXINT;

	int range = 1;
	double weight = decay;
	while( weight > MIN_WEIGHT )
	{
		weight *= decay;
		range++;
	}
	return range;
}

double calcNorm( int range,
					  double decay )
{
		int l;
		double tempJ = 1;
		double z = 1;
		for( l = 1; l < range; l++ )
		{
			// z[i] = d^0+..+d^(range-1)
			tempJ = tempJ*decay;
			z += tempJ;
		} // for
      return z;
}

/*
double myPow( double base, int expt )
{
#ifdef powl
	return powl( ( double)base, expt);
#else
	return pow( ( double)base, expt);
#endif
	int res = 1;
	int i;
	for( i = 1; i < expt; i++ )
	{
			res *= expt;
	}
	return res;
}
*/


/*
	 returns a list of breakpoints in entry values,
	 size of list will be set in listSize
	 !! result must be deleted!!!

 */
int *TFloatingAverage::getBreaks(int *listSize)
{
	// use GA for optimizing of isBreakpoint!!

	// needs to be optimzed

	if( !cValuesI )
		return NULL; 

	*listSize = cValuesI;
	int *res = new int[*listSize];

	// init result
	int i;
	for( i=0; i < *listSize; i++)
		res[i] = -1;

#ifdef _DEBUG
// #define _DEBUG_STATIST
#endif

#ifdef _DEBUG_STATIST
	FILE *out;
	out = fopen("_statist.txt","wt");
	this->writeValues(out);
#endif
	// init
	// calculate avregares
	double *rAvList = new double[cValuesI*cAvI];
	double *lAvList = new double[cValuesI*cAvI];
	double *deltaList = new double[cValuesI*cAvI];
	int pos = 0;
	i = 0;
	while( iterate(pos) )
	{
		pos = 1; // iterate
		for( int r = 0; r < cAvI; r++ )
//		for( int r = 0; r < cValuesI; r++ )
		{
			rAvList[(r*cValuesI)+i] = currentRightAv(r);
			lAvList[(r*cValuesI)+i] = currentLeftAv(r);
			deltaList [(r*cValuesI)+i] = pow(rAvList[(r*cValuesI)+i] -
										lAvList[(r*cValuesI)+i]
										,2);
		} // for
		i++;
	} // while

	// search for positions with local min/max in every range 


	
	i = 0;
	// first note is always a brea;
	res[i] = 0;
	i++;
	for( int j = 0;j < cValuesI; j++ )
	{
		if( j > 0 ) // a left value is needed
		{
			double lMCount = 0;
			for( int r = 0; r < cAvI; r++)
			{
//				if( rAvList[(r*cAvI)+j] > rAvList[(r*cAvI)+j+1] &&
//					rAvList[(r*cAvI)+j] > rAvList[(r*cAvI)+j-1] )
				if( deltaList[(r*cValuesI)+j] > deltaList[(r*cValuesI)+j-1] &&
					deltaList[(r*cValuesI)+j] > deltaList[(r*cValuesI)+j-1] ) 				
				{
					lMCount += 1;
				}
//				else if( rAvList[(r*cValuesI)+j] < rAvList[(r*cValuesI)+j+1] &&
//					     rAvList[(r*cValuesI)+j] < rAvList[(r*cValuesI)+j-1] )
				else if( deltaList[(r*cValuesI)+j] < deltaList[(r*cValuesI)+j-1] &&
						 deltaList[(r*cValuesI)+j] < deltaList[(r*cValuesI)+j-1] ) 				
				{
					lMCount -= 1;
				}
			} // for all ranges
			if( lMCount > (double)cAvI * 0.8 )
			{
				// mark as av 
				res[i] = j;
				i++;
			}
		} // if
	} // for
	delete [] deltaList;
	delete [] rAvList;
	delete [] lAvList;
#ifdef _DEBUG_STATIST
	fclose(out);
#endif
	*listSize = i;
	return res;
}// getBreaks

/* 
	return 1 if currentPos == breakPoint
	else 0
*/
/*
int TFloatingAverage::isBreakPoint(void)
{

	// no breakpoint at first or last position
	if( // currentPos == 0 ||
		currentPos == cValuesI-1 )
		return 0;
	// pos0 is always a breakpoint, calling function can ignore if needed
	if( currentPos == 0 )
		return 1;

	// compare left to right
	double pBreak = 0,
		  temp = 0; // probability for break
//	temp  /= cAvI;
	pBreak += temp;


	int maxLPos,
		maxRPos;

	temp = 0;
	// compare left-left and  right-right
	avI = minLeftAv() + maxLeftAv();
	if( avI > 0 )
	{
		temp += fabs( (minLeftAv() - maxLeftAv() ) / avI );
	}

	avI = minRightAv() + maxRightAv();
	if( avI > 0 )
	{
		temp += fabs( (maxRightAv() - minRightAv() ) / avI );
	}

	temp  /= 2;
	pBreak = (pBreak+temp)/2;
	
	// compare max values
	temp = 0;
	avI = maxLeft(&maxLPos) + maxRight(&maxRPos);
	if( (currentPos == maxLPos ||
		currentPos == maxRPos) &&
		avI > 0 )
	{
		temp = 1-fabs((maxLeft(&maxLPos) - maxRight(&maxRPos)) / avI );
		pBreak = (pBreak+temp)/2;
	}

// #define pBreakLimit 1.4

	if( pBreak > pBreakLimit )
		return 1;
	return 0; // else 
	
}
*/

double TFloatingAverage::minRightAv( void )
{
	double res = currentRightAv(0);
	int i;
	for(i=1; i < cAvI; i++ )
	{
		if( currentRightAv(i) < res )
			res = currentRightAv(i);
	}
	return res;
}

double TFloatingAverage::maxRightAv( void )
{
	double res = currentRightAv(0);
	int i;
	for(i=1; i < cAvI; i++ )
	{
		if( currentRightAv(i) > res )
			res = currentRightAv(i);
	} // for
	return res;
}

double TFloatingAverage::minLeftAv( void )
{
	double res = currentLeftAv(0);
	int i;
	for(i=1; i < cAvI; i++ )
	{
		if( currentLeftAv(i) < res )
			res = currentLeftAv(i);
	} // for
	return res;
}

double TFloatingAverage::maxLeftAv( void )
{
	int i;
	double res;
	res = currentLeftAv(0);
	for(i=1; i < cAvI; i++ )
	{
		if( currentLeftAv(i) > res )
			res = currentLeftAv(i);
	}
	return res;
}




double GaussWindow(double x,
				  double mean,
				  double sigma
				  )
{
	double res = kGaussWindow(x-mean,sigma,2);
	/*
	res = exp( -pow(x-mean,2) / (2*pow(sigma,2)));
	*/
	return res;
	
}

double kGaussWindow(double delta,
				  double sigma,
				  int k)
{
	double res = exp( - pow(delta,k) / (2*pow(sigma,k)));
	return res;
}


double GaussWindow(double delta,
				  double sigma
				  )
{
	
	double res = kGaussWindow(delta,sigma,2);
//	res = exp( - pow(delta,2) / (2*pow(sigma,2)));
	return res;
}

/// calc propability density function of normal distribution
#ifndef M_PI
#define M_PI	(acos(-1.0))
#endif
//#define M_PI		3.1416
double normal_pdf(double x,
				  double mean,
				  double var // ==sigma^2
				  )
{
	double x2, v1,v2;
	double  sigma = sqrt(var);
	x2 = (x-mean) / sigma;

	v1 = 1.0/(sigma*sqrt(2.0*M_PI));

	v2 = exp(-0.5 * pow(x2,2));
	return v1*v2;	
}



/// ini function for nDimAngle 				  
void initNDimAngle( double &XxY,
					double &XxX,
					double &YxY,
					double initVal)
{
	XxX = initVal;
	XxY = initVal;
	YxY = initVal;
}
/// add value for nDimAgle
void updateNDimAngle( double Xi,
					  double Yi,
					  double &XxY,
					  double &XxX,
					  double &YxY)
{
	XxY += Xi * Yi;
	XxX += pow(Xi,2);
	YxY += pow(Yi,2);
}
/// get angle between n-dinmensional angle: acos(X*Y/(||X|| * ||Y||)
double nDimAngle(  double XxY,
				  double XxX,
				  double YxY)
{
	XxX = sqrt(XxX);
	YxY = sqrt(YxY);
	double res;
	if( XxX != 0 && YxY != 0)
		res = XxY / (XxX * YxY);
	else
		res = 1;
		
	if( res > 1 )
		res = 1;
	else if( res < -1 )
		res = -1;

	res = acos( res );
	return res;
}

double TFloatingAverage::rightAv(int rangeID, int id)
{
	if( rangeID > cAvI )
		return 0;

	int to = min_(cValuesI, id + this->range[rangeID]);
	if( id >= to )
		return 0;

	double res = 0;
	double dSum = 0;
	for( int j = id; j < to; j++ )
	{
		double d = pow(decayList[rangeID], j-id);
		res += valueListI->get(j) * d;
		dSum += d;
	}
	res /= dSum;
	return res;

}
