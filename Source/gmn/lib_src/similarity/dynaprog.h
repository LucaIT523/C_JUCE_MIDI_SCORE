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

// CDynaprog.h: Schnittstelle für die Klasse CDynaprog.
//
//////////////////////////////////////////////////////////////////////

#ifndef __dynaprog_h__
#define __dynaprog_h__


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef _stdcall
#define _stdcall
#endif

#ifndef min_
#define min_(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef max_
#define max_(X, Y) ((X) > (Y) ? (X) : (Y))
#endif


#define LC -1000000

/// return max of three
double _stdcall max3(double f1, double f2, double f3 );

void _stdcall MyPrintf(const char* ptr, int i, int j, int k);
void _stdcall MyPrintf(const char* ptr);

// the following functions need to be defined by the application
/*
template<class T, class U>
double _stdcall valDyna(T f1); //!output value for T

 // similarity for T
 template<class T, class U>
 double _stdcall simDyna( T f1, T f2, T gap); //![-1..0) 
 
*/
//-----------------------------------------------

/// single entry (cell) of a dynamic programming table
typedef struct{
	/// distance/penalty/cost value
	double val;
	/// location of best neighbour: 0 = (-1,-1), 1 = (-1,0), -1 = (0,-1), -2 = stop
	int minLocation;
	/// # of insertions for optimal path starting at this cell
	int cGaps;
	/// # of adjacent gaps before this cell
	int cLocalGaps;
	/// length of alignment starting at this cell
	int alLength;
} dynaProgEntry;

/// generic dynmic programming class, can be used for string alignment etc.
template<class T, class U>
class CDynaprog  
{  

	// we need some history information
	double mBestInPrevStep;
	double mBestInCurStep;
	int mXBestInPrevDiag;
	int mYBestinPrevDiag;
	int mXBestInCurDiag,
		mYBestInCurDiag;
	double mBestOfPath;
	int mStartJ,
        mStopI;
	/// # of diagonals
	int mCDiagonals;
	/// current diagonal
	int mIdxCurDiagonal;


	int *mMaxElementInColumn;
	int *mMaxElementInRow;
	int *mMinElementInColumn;
	int *mMinElementInRow;


	//! pointer to functions
    double ( _stdcall  * val1Dyna)(T f1); //!output value for T
    //! pointer to functions
    double ( _stdcall  * val2Dyna)(U f1); //!output value for T
    /// ptr to similarity func, should return [-1...0]
    double ( _stdcall * simDyna)( T f1, U f2, 
    							  /// dedicated gap value
    							  T gap,
    							  /// # of insertions for current best path
    							  int cGaps,
    							  /// # adjacent gaps for current best path
    							  int cLocalGaps,
    							  /// length pf current best path
    							  int alLength) ;
	/// the DP table
    dynaProgEntry *tableV;
    void createTable( int sizeI, int sizeJ );
	
    //! the direction in which the arrays should be aligned 0 -> left to right, <0 right to left
    int direction; 
    /// value vector 1
	const T *S1;
	/// value vector 2
	const U *S2;
	//! position up to the table is filled
	int maxPathY;
	//! position up to the table is filled
	int maxPathX;
	
	/// the dedicated gap value (ie, NULL)
	T gapVal;
	
	//! calculate new entry in table
	double calcV( int i, int j, 
					int *minLocation,
					int *cGaps,
					int *cLocalGaps,
					int *alLength,
					int *columnBlockArray,
					int *rowBlockAray);
	
	//! fill out vector
	void setS2out( int id, U val);
	//! fill out vector
	void setS1out( int id, T val );
	
public:
    /// start id in S1, always 0 
    int stjI;
    /// start id in S2, always 0
    int stiI;
	/// retrieve entry U[i]
    U e2( int i );
    /// retrieve entry S[i]
    T e1( int i );
	
	/// call delOutVal before delete or store pointers!!
	
	/// constructor 
    CDynaprog(const T *s1, const U *s2, 
    	/// length of s1 (if direction == dir )
		int l1, 
		/// length of s2 (if direction == dir)
		int l2,
		/// dedicated gap value, i.e. NULL
		T  gapv,
		/// alignment direction 
		int dir,
		//!output value for T
		double (  _stdcall * val1Func)(T f1), 
		 //!output value for T
		double (  _stdcall * val2Func)(U f1),
		 /// similarity function (range = (0...1])
		double (_stdcall  * simFunc)( T f1, U f2, T  gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength ) 
		);
	
	/// get val and pos (in sel) of best direct neighbour in backward search
	double getBestNeighbour(int i, int j, int *sel);
	
	void debug( FILE *out = NULL );
	void initTable( void );
	void deleteTable( void );
	int incrementTable( double &bestOfPath,
						double maxDelta,
					  /// time in sec for creating the table
					  double *processtime
						);

	//! build table, limit (-oo ... 0]
	double makeTable( /// abort sub path | if best in path - current | > delta, delta in [-oo...0)
					  double deltaLimit,
					  /// abort if best in path > -absLimit, absLimit in [-oo...0)
					  double absLimit,
					  /// time in sec for creating the table
					  double *processtime
					  );
	
	//! read alignment from table
	double retrieveAlignment(double alpha = 0,
							 int x = -1, 
							 int y = -1);
	
	
	//! set table entry
	void V(int i, 
			int j, 
			double val,
			int minLocation,
			int cGaps,
			int cLocalGaps,
			int alLength);
	//! read table entry
	double V(int i, 
			 int j,
			 int *minLocation,
			 int *cGaps,
			 int *cLocalGaps,
			 int *alLength);
	
	/// this will not be called from ~CDynaprog!!!
    void delOutVal( void )
    {
		if( S1outFl )
			delete [] S1outFl;
		if( S2outFl )
			delete [] S2outFl;
    }
	/// deletes only the table, call delOutVals separately!!
	virtual ~CDynaprog( void );
	
	void writeS1( FILE *out );
	void writeS2( FILE *out );
	void writeS1out( FILE *out );
	void writeS2out( FILE *out );
	
	/// size of S array
	int iI;
	/// size of U array
	int jI;
	/// final alignment including gaps
	U *S2outFl;
	/// final alignment including gaps
	T *S1outFl;
	//! start ID in out array, out direction is always l->r
	int outStart;
};


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
template <class T, class U>  
void CDynaprog<T,U>::createTable( int sizeI, int sizeJ )
{
	/// create a new table
	tableV = new dynaProgEntry[sizeI*sizeJ];

	// initialisation can be done during DP 
}

template <class T, class U>
CDynaprog<T,U>::~CDynaprog()
{	
	delete [] tableV;
	/// !!! call delOutVal from outside separately
}

template <class T, class U>
double  CDynaprog<T,U>::V(int i, 
						int j, 
						int *minLocation, 
						int *cGaps, 
						int *cLocalGaps, 
						int *alLength)
{
	if(i <= iI && j <= jI)
	{
		*minLocation = tableV[i*(jI+1) + j].minLocation;
		*cGaps = tableV[i*(jI+1) + j].cGaps;
		*cLocalGaps = tableV[i*(jI+1) + j].cLocalGaps;
		*alLength = tableV[i*(jI+1) + j].alLength;
		return tableV[i*(jI+1) + j].val;
	}
	else
	{
		printf("range error!\n");
		return 0;
	}
}


template <class T, class U>  
void CDynaprog<T,U>::V(int i, 
						int j, 
						double val, 
						int minLocation, 
						int cGaps,
						int cLocalGaps,
						int alLength)
{
	if(i <= iI && j <= jI)
	{
		tableV[i*(jI+1) + j].val = val;
		tableV[i*(jI+1) + j].minLocation = minLocation;
		tableV[i*(jI+1) + j].cGaps = cGaps;
		tableV[i*(jI+1) + j].cLocalGaps = cLocalGaps;
		tableV[i*(jI+1) + j].alLength = alLength;
	}	
}

template <class T, class U>
void  CDynaprog<T,U>::initTable( void )
{
	// init table
	mMaxElementInColumn = new int[jI+1];
	mMaxElementInRow = new int[iI+1];
	mMinElementInColumn = new int[jI+1];
	mMinElementInRow = new int[iI+1];

	// index of max elemnts in cloumn[j] or row[i]
	int j;
	for( j = 0; j <= jI ; j++ )
	{
		mMaxElementInColumn[j] = 0;
		mMinElementInColumn[j] = iI;
	}
	int i;
	for( i = 0; i <= iI ; i++ )
	{
		mMaxElementInRow[i] = 0;
		mMinElementInRow[i] = jI;
	}
	//! set start entry
    V(0,0,0, -2 /*stop*/, 0 /* cGaps */, 0 /*cLocalGaps */, 0 /*alLength*/);

	/// start position of first element in cur diagonal
	mStartJ = 0;
    mStopI = 0;
	
    mBestInPrevStep = LC; // bestVal in prevDiag
	// best elemtn in two successive diagonals
    mBestOfPath = LC; // == min(bestCur, bestPrev)

    // position of max evaluated element, == startpos for retrieval
    maxPathX = 0;
    maxPathY = 0;
	mXBestInPrevDiag = 0;
	mYBestinPrevDiag = 0;
	
	
    
	// # of diagonals
	mCDiagonals = iI + jI + 1;
	// current diagonal
	mIdxCurDiagonal = 1;

}


template <class T, class U>
int  CDynaprog<T,U>::incrementTable( double &bestOfPath,
									 double maxDelta,
								  /// time in sec for creating the table
								  double *processTime
									 )
{
	    clock_t start = clock();

		int j = min_(mStartJ, jI);
		int i = mIdxCurDiagonal - j;		
		if(	i > iI )
		{
			i = iI;
			j = mIdxCurDiagonal - i;
		}

		// flag for forst valid value in curDiagonal
        char firstValInDiag = 1;
		/// position of best element in curDiagonal
		mXBestInCurDiag = 0;
		mYBestInCurDiag = 0;

		double bestCur = LC; // bestVal in current Diag
		int minLocation; // position of best neighbour
		double bestVal = LC;
		int end = 0;
        while( !end &&
			i >= mStopI &&
			j <= jI	)
        {
        	int cGaps, cLocalGaps, alLength;
            bestVal = calcV(i,
            				j, 
            				&minLocation, 
		            		&cGaps, 
		            		&cLocalGaps, 
		            		&alLength, 
							mMaxElementInColumn, 
							mMaxElementInRow ); // calc
			
			if( bestVal > bestOfPath )
			{
				// for new optimum, can only happen if simDyna > 0 
				bestOfPath = bestVal;
			}
            else if( bestVal - maxDelta < bestOfPath 
//            	|| bestVal < absLimit 
            	)
            {
                // make it invalid, too bad value should never be in opt path
                bestVal = LC;
            }
            	
			if(bestVal > LC )
			{
				if( i > mMaxElementInColumn[j] )
					mMaxElementInColumn[j] = i;
				else if( i < mMinElementInColumn[j] )
					mMinElementInColumn[j] = i;
				if( j > mMaxElementInRow[i] )
					mMaxElementInRow[i] = j;
				else if( j < mMinElementInRow[i]  )
					mMaxElementInRow[i] = j;
			} // if valid
			
            V(i,
              j, 
              bestVal, 
              minLocation, 
              cGaps, 
              cLocalGaps, 
              alLength );  // copy to matrix
              
            if( bestVal <= LC && 	// invalid
                firstValInDiag )
            {
                if( i > 0)
                {
                    // if first element in diag is invalid, bloack first column
                     mStartJ++;
                }
            }
            else if( bestVal > LC &&
					 bestVal > bestCur )
            {
                bestCur = bestVal;
				mXBestInCurDiag = i;
				mYBestInCurDiag = j;
            }
			
            // must be a valid element			
            firstValInDiag = 0;			
			i--;
            j++;			
        } // while in valid diagonal
        // stopJ++;
		if( i >= iI &&
			j >= jI )
			end = 1; // right bottom corner reached

		if( mStopI > iI ) // all rows invalid
			end = 1; // no path possible
		if( mStartJ > jI ) // all columns invalid 
			end = 1; // no path possible
		if( mBestInPrevStep > bestCur  )
		{
//			maxPathX = prevDiagBestX;
//			maxPathY = prevDiagBestY;
			if( maxPathX == iI ||
				maxPathY == jI )
			{
				// If the very best path reaches a border, only gaps can be retrieved 
//turn on?				end = 1;
			}
		}
		else if( bestVal > LC)
		{
			// this would be best in two neighbour diagonals
//			bestOfPath = bestCur;
			
			maxPathX = mXBestInCurDiag;
			maxPathY = mYBestInCurDiag;
			if( maxPathX == iI ||
				maxPathY == jI )
			{
				// If the very best path reaches a border, only gaps can be retrieved 
//turn on?				end = 1;
			}
		} // else if
		/*
			this would be an undesired length restriction
		// stop of best of last two diagonals exceeds the limit
		if( bestOfPath < absLimit ) 
			end = 1;		
		*/
		
		/* this might eb usefull for non-optimised, comlete  DP
		// we have reached the right bottom corner within the limit
		if( curDiag == cDiag-1 )
		{
			maxPathX = iI;
			maxPathY = jI;
		}
		*/


        if( bestVal <= LC  ) // last Element in diag was invalid
        {   // block this row
            mStopI++; // 0...iI
        } // if invalid

		       
        mBestInPrevStep = bestCur;
		mXBestInPrevDiag = mXBestInCurDiag;
		mYBestinPrevDiag = mYBestInCurDiag;
//		debug();
		mIdxCurDiagonal++;

    clock_t finish = clock();
	*processTime = (double)(finish - start) / (double)CLOCKS_PER_SEC;

	return end;
}


/*! fill dynamic programming table until str end or cost limit
is reached
return = costs for element maxPath(x,y)
*/
template <class T, class U>
double  CDynaprog<T,U>::makeTable(//! limit for abort, in (-oo...0)
									double deltaLimit,	
									/*! max cost for path, in (-oo...0) */ 
									  double absLimit, 
									  double *processTime)
{
	*processTime = 0;	

//	double alpha = 0;
 // to BLAST optimised filling

	initTable();

    clock_t start = clock();

    char end = 0;
    while( !end  &&
		   mIdxCurDiagonal < mCDiagonals )
    {
		end = incrementTable( mBestOfPath,	// call by reference
								deltaLimit,
								processTime);		
    } // while !end, for all diags
	
    clock_t finish = clock();
	*processTime = (double)(finish - start) / (double)CLOCKS_PER_SEC;

	return mBestOfPath;
}

template <class T, class U> 
void CDynaprog<T,U>::deleteTable(void )
{
	delete [] mMaxElementInRow;
	delete [] mMaxElementInColumn;
}


template <class T, class U> 
void CDynaprog<T,U>::debug(FILE *out)
{	
	char mustClose = 0;
	if( !out )
	{
		out = fopen("_dynaprog.txt","wt");
		mustClose = 1;
	}
	fprintf(out,"              ");
	int i;
	for( i = 0; i < maxPathX; i++ )
	{
		fprintf(out, "%2.3f  ", val1Dyna(e1(i)) );		
	} // for
	fprintf(out, "\n");
	// write table with numbers
	double _best = -1000000;
	int startColumn = 0;
	int stopColumn = 0;
	// debug routing
	long cEvalCells = 0;
	
	int j;
	for( j = 0; j <= maxPathY; j++ )
	{
		for( i = 0; i <= maxPathX; i++ )
		{
			// table heading
			if( i == 0 )
			{			
				if( j > 0 )
				{
					fprintf(out, "%2.3f:", val2Dyna(e2(j-1)) );
				}
				else
				{
					fprintf(out, "     :");			
				}
			}
			double val = LC;
			if( i >= startColumn )
			{
				int dummy, cGaps, cLocalGaps, alLength;
				val = V(i,j, &dummy, &cGaps, &cLocalGaps, &alLength) ;
				if( val > LC &&
				    val >= 0)
					fprintf(out, " %2.3f ", val);
				else if( val > LC )
					fprintf(out, "%2.3f ", val);
				else
					fprintf(out, " ----- ");

				if( i >= maxPathX &&
					j >= maxPathY )
				{
					i = iI+1;
					j = jI+1;
				}
				if( val > _best )
					_best = val;
				// debug routing
				cEvalCells++;
			}
			else
			{
				val = LC;
				fprintf(out, " ----- " );
			}

			if( val <= LC && 
				i > startColumn &&
				i >= stopColumn) // Stop row
			{
				stopColumn = i;
				i = iI +1;
			}
			else if( val <= LC && 
					i == startColumn)
			{
				startColumn++;
			}

		} // for 
		fprintf(out, "\n");
	}
	fprintf(out, "\n");
	fprintf(out, "Best of Path = %f\n",_best);
	fprintf(out,"Cells %ld/%d\n",cEvalCells, (jI+1)*(iI+1));
	
	// write table
	startColumn = 0;
	stopColumn = 0;
	for( j = 0; j <= jI; j++ )
	{
		for( i = 0; i <= iI; i++ )
		{
			double val = LC;
			if( i >= startColumn )
			{
				int dummy, cGaps, cLocalGaps, alLength;
				val = V(i,j,&dummy, &cGaps, &cLocalGaps, &alLength) ;
				if( val > LC  )
				{
					int selDir = tableV[i*(jI+1) + j].minLocation;
					switch(selDir)
					{
					case 0 : 
						fprintf(out, "\\");
						break;
					case 1 : 
						fprintf(out, "-");
						break;
					case -1 : 
						fprintf(out, "|");
						break;						
					default :
						fprintf(out, "?");
						break;												
	
					} // switch
				}
				else
					fprintf(out, ".");
				
				if( i >= maxPathX &&
					j >= maxPathY )
				{
					startColumn = iI+2;
				}
			}
			else
			{
				val = LC;
				fprintf(out, ".");
			}

			if( val <= LC && 
				i > startColumn &&
				i >= stopColumn) // Stop row
			{				
				stopColumn = i;
				for( i = i+1; i <= iI; i++ )
					fprintf(out,".");
			}
			else if( val <= LC && 
					i == startColumn)
			{
				startColumn++;
			}

		} // for 
		fprintf(out, "\n");
	}
	fprintf(out, "\n");
	fflush(out);
	
	writeS1( out );
	fprintf(out, "\n");
	fflush(out);
	writeS2( out );
	fprintf(out, "\n");
	fflush(out);

	writeS1out( out );
	fprintf(out, "\n");
	fflush(out);
	writeS2out( out );
	fprintf(out, "\n");
	fflush(out);

	
	fprintf(out, "maxX/Y= %d/%d\n",maxPathX, maxPathY);
	if( mustClose )
		fclose(out);
}

/*!
fill out arrays with best alignment retrieved backwards
from table
result: simDyna for best (=retrieved) alignment
*/
template <class T, class U>
double CDynaprog<T,U>::retrieveAlignment(double alpha, //! influence of length
										 int x /* start position */,
                                         int y /* start position */)
{
	
	//! make output arrays
	//! in worst case the length might be iI+iJ
	{
		S1outFl = new T [iI+jI+1];
		S2outFl = new U [iI+jI+1];
	}
	
	
	int steps = iI+jI;	
	int i,j;
	
	//! start at given position?
	if( x < 0 || y < 0 )
	{
		i = maxPathX;
		j = maxPathY;
	}
	else
	{
		i = x;
		j = y;
	}
	
	
	int k = 0;
	while( i > 0 || j > 0 )
	{
		int sel; // -1 = (-1,0), 0 = (-1,-1), 1 = (0,-1)
		// ! look for best neighbour in (i-1,j-1), (i,j-1), (i-1,j)
		getBestNeighbour(i,j, &sel);
		// sel gives position of best neighbour
		switch(sel) 
		{
		case -1 : if( i-1 > -1 )
				  {					
					  setS1out(steps-k-1, e1(i-1));
				  }
			      setS2out(steps-k-1, (U )gapVal);
			      i--;
			break;
		case 1 :  setS1out(steps-k-1, gapVal);
			      if( j-1 > -1 )
				  {
				      setS2out(steps-k-1, e2(j-1));
				  }			
			      j--;
			break;
		case 0: 
			if( i-1 > -1 )
			{
					setS1out(steps-k-1, e1(i-1));
			}
			if( j-1 > -1 )
			{
				setS2out(steps-k-1, e2(j-1));
			}
			i--;
			j--;
			break;	
		default :
			std::cerr << "Illegal step in dynaprog" << std::endl;
			break;
		} // switch
		k++;
	} // while
	
	// Total costs are  in V(iI, jI);
	double sum = 1;
	for( j = outStart; j < jI+iI; j++)
	{
		int cGaps = 0,
		    cLocalGaps = 0,
		    alLength = 1;
		sum *= (simDyna(S1outFl[j], S2outFl[j], 
						gapVal,
						cGaps,
						cLocalGaps,
						alLength )+1);
	}
	sum *= pow(k, alpha);
 	return sum;
}

/*! get best neighbour in (i-1,j)(i-1,j-1)(i,j-1)
result: entry at best neighbour pos
sel shows position of best neighbour
0  = i-1,j-1
1  = i,j-1
-1 = i-1, j
*/				
template <class T, class U>
double CDynaprog<T,U>::getBestNeighbour(int i, int j, int *sel)
{
	double best = 0;
	int dummy, cGaps, cLocalGaps, alLength;
	if( i > 0  || j > 0 )
	{
		int minLocation = tableV[i*(jI+1) + j].minLocation;
		switch( minLocation )
		{
		case 0 : 
			best = V(i-1,j-1, &dummy, &cGaps, &cLocalGaps, &alLength);
			*sel = 0;
			break;
		case -1 :
			best = V(i,j-1, &dummy, &cGaps, &cLocalGaps, &alLength);
			*sel = 1;
			break;
		case 1 :
			best = V(i-1,j, &dummy, &cGaps, &cLocalGaps, &alLength);
			*sel = -1;
			break;
		default:
			std::cerr << "ERROR: in Dynaprog direction = " << minLocation << " (" << i << "," << j << ")!" << std::endl;
			debug();
			exit(1);
		} // switch
	}
	return best;
}

template <class T, class U>
CDynaprog<T,U>::CDynaprog(const T *s1, 
						  const U *s2, 
						  int l1,  //! length of s1
						  int l2,	//! length of s2
						  T gapv,
						  int dir, // alignment direction
						  double (_stdcall  * val1Func)(T f1), //!output value for T
						  double (_stdcall  * val2Func)(U f1), //!output value for T
						  double (_stdcall  * simFunc)( T f1, U f2, T gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength )  //!(0..1] 
						  )
{
	gapVal = gapv;
	
	simDyna = simFunc;
	val1Dyna = val1Func;
    val2Dyna = val2Func; 
	S1 = s1;
	S2 = s2;

	iI   = l1;
	jI   = l2;


	direction = dir;
	// make it save
	if(dir >= 0 )
	{
		direction = 1;
	}
	else
	{
		direction = -1;
	}
	stiI = 0 ;
	stjI = 0;
	
	
	
	//! points to first valid position in out vector
	//! out vector get filled from right to left because
	//! retrieving the alignment is a backwords
	outStart =iI+jI+1;
	
	
	S1outFl = NULL;
	S2outFl = NULL;
	
	//! init table with 0
	createTable( iI+1, jI+1);
}

template <class T, class U>
T CDynaprog<T,U>::e1(int i)
{
	int id;
	id = stiI+i*direction;
#ifdef _DEBUG
	if( abs(id) >= iI )
	{
		printf("Range error in e1()id=%d, i=%d, dir=%d, stiI=%d\n",
					id, i, direction, stiI);
	}
#endif
	return *(S1+id);
	
}

template <class T, class U>
U CDynaprog<T,U>::e2(int j)
{
	int id;
	id = stjI+j*direction;	
#ifdef _DEBUG
	if(  abs(id) >= jI )
	{
		printf("Range error in e2()id=%d, j=%d, dir=%d, stjI=%d\n",
							id, j, direction, stjI);
	}
#endif
	return *(S2+id);
}


template <class T, class U> 
void CDynaprog<T,U>::setS1out(int id, T val)
{
	if( id < outStart )
		outStart = id;
	
	S1outFl[id] = val;
}

template <class T, class U> 
void CDynaprog<T,U>::setS2out(int id, U val)
{
	if( id < outStart )
		outStart = id;
	
	//	if( S2outStr )
	//		S2outStr[id] = val;
	//	else
	S2outFl[id] = val;
	
}

template <class T, class U> 
void CDynaprog<T,U>::writeS1(FILE *out)
{
	int i;
	if( direction > 0 )
	{
		for( i = 0; i < iI; i++ )
		{
				fprintf(out, "%3.3f, ", val1Dyna(e1(i))) ;
		}
	}
	else
	{
		for( i = iI-1; i >= 0; i-- )
		{
				fprintf(out, "%3.3f, ", val1Dyna(e1(i))) ;
		}
	}
}
template <class T, class U>
void CDynaprog<T,U>::writeS2(FILE *out)
{
	int i;
	if( direction > 0 )
	{
		for( i = 0; i < jI; i++ )
		{
				fprintf(out, "%3.3f, ", val2Dyna(e2(i))) ;
		}
	}
	else
	{
		for( i = jI-1; i >= 0; i-- )
		{
				fprintf(out, "%3.3f, ", val2Dyna(e2(i))) ;
		}
	}
	
}
template <class T, class U> 
void CDynaprog<T,U>::writeS1out(FILE *out)
{
	int i;
	double temp;
	// print dots for empty positions
	for( i = 0; i < outStart; i++)
		fprintf(out, ".");		
	if( direction > 0 )
	{
		
		for( i = outStart; i < iI+jI; i++ )
		{
			temp = val1Dyna(S1outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	} // if
	else
	{
		for( i = iI+jI-1; i >= outStart; i-- )
		{
			temp = val1Dyna(S1outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	}
	
}
template <class T, class U> 
void CDynaprog<T,U>::writeS2out(FILE *out)
{
	int i;
	double temp;
	for( i = 0; i < outStart; i++)
		fprintf(out, ".");
	
	if( direction > 0 )
	{		
		double temp;
		for( i = outStart; i < iI+jI; i++ )
		{
			temp = val2Dyna(S2outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	}
	else
	{
		for( i = iI+jI-1; i >= outStart; i-- )
		{
			temp = val2Dyna(S2outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	} // if
}

/*! calc entry for table pos (i,j)
pos(i,j) = max(gap/s2(j-1),
s1(i-1)/gap,
s1(i-1)/s2(j-1))
*/
template <class T, class U>
double CDynaprog<T,U>::calcV(int i, 
							int j, 
							int *minLocation,
							int *cGaps,
							int *cLocalGaps,
							int *alLength,							
							int *maxEInColumn,
							int *maxEInRow
							)
{
	double bestVal = LC; // invalid
    char unvalid = 1;
	int cGaps1, cGaps2, cGaps3;
	int cLocalGaps1, cLocalGaps2, cLocalGaps3;
	int alLength1, alLength2, alLength3;
    
    int dummy;
    double a;
	if( i == 0 && j == 0 ) // start position
	{				
		bestVal = V(0,0, 
					minLocation, 
					cGaps,
					cLocalGaps,
					alLength);
					
        unvalid = 0;
//		*minLocation = -2; // stop
	}
	else if( i == 0 )
	{
		a = V(0,j-1, &dummy, &cGaps1, &cLocalGaps1, &alLength1);
		if( a  > LC )// valid entry
		{
			bestVal = a + simDyna(gapVal, e2(j-1), gapVal,
    							  cGaps1,
    							  cLocalGaps1,
    							  alLength1 );
	        unvalid = 0;
			*minLocation = -1;
			*cGaps = cGaps1 + 1;
			*cLocalGaps = cLocalGaps1 + 1;
			*alLength = alLength1 + 1;
		} 
	} // if i == 0
	else if( j == 0 )
	{
		a  = V(i-1,0, &dummy, &cGaps2, &cLocalGaps2, &alLength2);
		if( a > LC )
		{
			bestVal = a + simDyna(e1(i-1), (U)gapVal, gapVal,
    							  cGaps2,
    							  cLocalGaps2,
    							  alLength2 );
	        unvalid = 0;
			*minLocation = 1;
			*cGaps = cGaps2 + 1;
			*cLocalGaps = cLocalGaps2 + 1;
			*alLength = alLength2 + 1;
		} 
	} // if j == 0
	else
	{
		double v1 = LC, // very expensive!
			v2 = LC, // very expensive!
			v3 = LC; // very expensive!
		
		double bestNeighbour = LC;
         a = V(i-1,j-1, &dummy, &cGaps1, &cLocalGaps1, &alLength1);
		if( j-1 <= maxEInRow[i-1] && 
			i-1 <= maxEInColumn[j-1] &&
			a > LC  		    			
			)
        {
			bestNeighbour = a;
			v1 = a + simDyna(e1(i-1), e2(j-1), gapVal,
    							  cGaps1,
    							  cLocalGaps1,
    							  alLength1 );
			unvalid = 0;
        }
        a = V(i-1,j, &dummy, &cGaps2, &cLocalGaps2, &alLength2);
		if( j <= maxEInRow[i-1] &&			
			i-1   <= maxEInColumn[j] &&
			a > LC )
        {
			if( a > bestNeighbour )
			{
				bestNeighbour = a;
			}
			v2 = a + simDyna(e1(i-1), (U)gapVal, gapVal,
    							  cGaps2,
    							  cLocalGaps2,
    							  alLength2 );
            unvalid = 0;
        }
        a = V(i,j-1, &dummy, &cGaps3, &cLocalGaps3, &alLength3);
		if( j-1 <= maxEInRow[i] &&			
			i <= maxEInColumn[j-1] &&
			a > LC )
        {
			if( a > bestNeighbour )
			{
				bestNeighbour = a;
			}
			v3 = a + simDyna(gapVal,e2(j-1), gapVal,
    							  cGaps3,
    							  cLocalGaps3,
    							  alLength3 );
            unvalid = 0;
        }
		/*
		bestVal = max3(v1,
			v2,
			v3);
		*/
		if( v1 >= v2 &&
			v1 >= v3 )
		{
			bestVal = v1;
			*minLocation = 0;
			*cLocalGaps = 0;
			*cGaps = cGaps1;
			*alLength = alLength1 + 1;
		}
		else if( v2 >= v1 &&
			     v2 >= v3 )
		{
			bestVal = v2;
			*minLocation = 1;
			*cGaps = cGaps2 + 1;
			*cLocalGaps = cLocalGaps2 + 1;
			*alLength = alLength2 + 1;
		}
		else
		{
			bestVal = v3;
			*minLocation = -1;
			*cGaps = cGaps3 + 1;
			*cLocalGaps = cLocalGaps3 + 1;
			*alLength = alLength3 + 1;
		}
	}// else
    if( unvalid )
	{
        bestVal = LC;
		*minLocation = -3;
	}
	return bestVal;
}



#endif 
