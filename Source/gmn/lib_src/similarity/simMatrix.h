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

#ifndef __simmatrix_h__
#define __simmatrix_h__
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

// #define _stdcall /* */
#ifdef WIN32
#define _stdcall _cdecl
#else
#define _stdcall
#endif

using namespace std;


/// tool class for x,y values
class Cxy
{
public:
	int x,
        y;
};

/// generic similarity matrix class
template <class T, class U> class CSimMatrix
{
    /// similarity function, should return [-1...0]
    double (_stdcall * sim)(T v1,
                                U v2,
                                int windowSize);
    /// eval func
    double (_stdcall *val1)( T v1);
    /// eval func
    double (_stdcall *val2)( U v1);
    
    double *tableV;
    T *vectorX;
    U *vectorY;
    int /// length of vectorX
        xI,
        /// length of vectorY
        yI; 
	/// stepsize used in calcTable
	int stepS;
public:
    CSimMatrix(int x, int y,
                double (_stdcall * s)(T v1,
                                      U v2,
                                    int windowSize),
                double (_stdcall *vl1)( T v1),
               double (_stdcall *vl2)( U v2)
               );

	virtual ~CSimMatrix( void );
        /// get table entry
        double V(int x, int y);
        /// set table entry
        void V(int x, int y, double val);
	void setXVector(T *val);
	void setYVector(U *val);
    // fill the table
    void calcTable( int windowSize,
                        int stepSize,
						double *mean,
						double *var,
						double *processTime);

	//! return list of id's where V(x,y) >= limit
	//! end of list is marked by x,y = -1
	//! result must be deleted
	Cxy *getBestIDs( double limit,
                     int windowsSize,
					  double *processTime,
					  int *listSize);

	/// calculate my and sigma
	void calcDistributionParams( double *my, 
								 double *sigma,
								 double *sigmaLess,
								 double *sigmaGreater,
								 double *minVal = NULL);

	void debug(double limit = 0, ostream *out = NULL);
};


// start of implementation ------------------------------


template <class T, class U>
CSimMatrix<T,U>::CSimMatrix(int x, int y,							
			double (_stdcall * s)(T v1,
					U v2,
					int windowSize),
			double (_stdcall *vl1)( T v1),
                            double (_stdcall *vl2)( U v2)
                            )
{
	sim = s;
	val1 = vl1;
    val2 = vl2;

    xI = x;
	yI = y;
	tableV = NULL; 
	vectorX = NULL;
	vectorY = NULL;
	stepS = 1;
}

template <class T, class U>
CSimMatrix<T,U>::~CSimMatrix( void )
{
	if( tableV )
		delete [] tableV;
}

template <class T, class U>
double CSimMatrix<T,U>::V(int x, int y)
{
	if( tableV &&
		x < xI &&
		y < yI )
	{
		return tableV[x*yI + y];
	}
	printf("range error\n");
	return 0;
}


template <class T, class U>
void CSimMatrix<T,U>::V(int x, int y, double val)
{
	if( tableV &&
		x < xI &&
		y < yI )
	{
		tableV[x*yI + y] = val;
	}

}

template <class T, class U>
void CSimMatrix<T,U>::setXVector(T *val)
{
	vectorX = val;
}
template<class T, class U>
void CSimMatrix<T,U>::setYVector(U *val)
{
	vectorY = val;
}


template <class T, class U>
void CSimMatrix<T,U>::calcDistributionParams( double *my, 
											  double *sigma,
											  double *sigmaLess,
											  double *sigmaGreater,
								 			  double *minVal)
{
	double mySum = 0;
	double sigmaSum = 0,
		   sigmaLessSum = 0,
		   sigmaGreaterSum = 0;
	int count = 0,
	    countLess = 0,
	    countGreater = 0;
	// exact output
	int xMax = xI;
	for( int y = 0; y < yI; y += stepS )
	{
		// do only diagonal?
		
		if( !vectorY )
		{
			xMax = y;	// write only triangonal matirx
			//	fprintf(out,"%3.3f| ", val1(vectorX[y]));
		}
		else
		{
			// fprintf(out,"%3.3f| ", val2(vectorY[y]));
		}
		
		// print all or lower triangonal matirx
		for( int x = 0; x < xMax; x += stepS )
		{
			double v = V(x,y);
			if( !minVal ||
				v > (*minVal) )
			{
				mySum += v;
				
				if( count )
				{
					// sigmaSum = (av - x)^2
					double lAv = mySum/(double)(count);
					double lErr = pow( lAv - v,2);
					sigmaSum += lErr;
					if( v <= lAv )
					{
						sigmaLessSum += lErr;
						countLess++;
					}
					if( v >= lAv )
					{
						sigmaGreaterSum += lErr;
						countGreater++;
					}
				} // if				
				count++;
			} // if
		} // for 
	} // for 
	if( count )
	{
		sigmaSum /= (double)count;
		mySum/= (double)count;
	}
	if( countLess )
	{
		sigmaLessSum /= (double)countLess;
	}
	if( countGreater )
	{
		sigmaGreaterSum /= (double)countGreater;
	}
	*my = mySum;
	*sigma= sigmaSum;
	*sigmaLess = sigmaLessSum;
	*sigmaGreater = sigmaGreaterSum;
}

template <class T, class U>
void CSimMatrix<T,U>::calcTable( int windowSize,
								 int stepSize,
								 double *mean,
								 double *var,
								 double *processTime)
{
	*processTime = 0;
	tableV = new double[xI*yI];
	int x,y;
	stepS = stepSize;
	*mean = 0;
	*var = 0;
	// init
	for( y = 0; y < yI; y++ )
	{
		for( x = 0; x < xI; x++ )
		{
			V(x,y,0);
		};
	}

    clock_t start = clock();

	int count = 0;
	if( vectorY == NULL ) // only corner needed!
	{
		for( y = 0; y+windowSize-1 < yI; y += stepSize )
		{
			for( x = 0; x < y; x += stepSize )
			{
				V(x,y,sim(vectorX[x],dynamic_cast<U>(vectorX[y]),windowSize) );
//				printf("%d ",x);
				*mean += V(x,y);
				count++;
				if( count > 1 )
				{
					*var += pow(*mean/(double)(count) - V(x,y),2);
				}
			}
//			printf("\n%d-------------\n",y);
		}// for
	}
	else
	{
		for( y = 0; y+windowSize-1 < yI; y += stepSize )
		{
			for( x = 0; x+windowSize-1 < xI; x += stepSize )
			{
				V(x,y,sim(vectorX[x],vectorY[y],windowSize) );
				*mean += V(x,y);
				count++;
				if( count > 1 )
				{
					*var += pow(*mean/(double)(count) - V(x,y),2);
				}
			}
		} // for
	}
	if( count > 0 )
	{
		*mean /= (double)count;
		if( count > 1 )
		{
			*var /= (double)(count-1);
		}
	} // if
   clock_t finish = clock();
   *processTime = (double)(finish - start) / CLOCKS_PER_SEC;

}

template <class T, class U>
void CSimMatrix<T,U>::debug(double limit, ostream *out)
{
	char mustClose = 0;
	if( !out )
	{
		ofstream fOut;
		out = &fOut;
		((ofstream *)out)->open("_simmatrix.txt");
		mustClose = 1;
	}

	int x;
	if( vectorX )
	{
//		fprintf(out,"xxxxxx ");
		(*out) << "lenx = " << xI << endl;
		for( x = 0; x < xI; x++ )
			(*out) <<  val1(vectorX[x]) << " ";
		(*out) << "\n\n";
	}

	int y;
	if( vectorY )
	{
//		fprintf(out,"xxxxxx ");
		(*out) << "lenx = " << yI << endl;
		for( y = 0; y < yI; y++ )
			(*out) <<  val2(vectorY[y]) << " ";
		(*out) << "\n\n";
	}

	// exact output
	int xMax = xI;
	for( y = 0; y < yI; y += stepS )
	{
		// do only diagonal?		
		if( !vectorY )
		{
			xMax = y;	// write only triangonal matirx
		}
		else
		{
		}
		
		// print all or lower triangonal matirx
		for( x = 0; x < xMax; x += stepS )
		{
			(*out) << V(x,y) << " ";
		}
		// print upper triangoal matrix
		for( x = x; x < xI; x += stepS )
		{
			if( x == y )
				(*out) << "1.000 ";
			else if( x > y )
				(*out)<< "1.000 ";
			else
				(*out) <<  V(y,x) << " ";
		}
		(*out) << "\n";
	}
	(*out) << "\n";

	// summary outPut
	xMax = xI;
	int value;
	for( y = 0; y < yI; y += stepS )
	{
		// do only diagonal?
		if( !vectorY )
		{
			xMax = y;
		}

		for( x = 0; x < xMax; x+= stepS )
		{			
			if( V(x,y) > limit )
			{
				value = (int)(V(x,y)*10);
				(*out) <<  value-1;
			}
			else
				(*out) << ".";			
		}
		(*out) << "\n";
	}
	(*out) << "\n";
	out->flush();
	if( mustClose )
	{
		((ofstream *)out)->close();
		/// delete out;
	}
}

/*
	get the high scoring entries of the similarity matrix
	a HSP is inferred if 
		c(i,j) > tau && c(i+k,j+k) > tau
*/
template <class T, class U>
Cxy *CSimMatrix<T,U>::getBestIDs( double limit,
								 int windowSize,
								 double *processTime,
								 int *cur)
{
	*processTime = 0;
	*cur = 0;
	if(xI < 1 || yI < 1 )
		return NULL;

	Cxy *res;
	if( vectorY ) // we need a m x n matrix
		res = new Cxy[xI*yI+1];
	else	// a upper triangle matrix is enough
		res = new Cxy[(xI*yI)/2+1];


    clock_t start = clock();

	// look diagonal by diagonal (tl->br)
	int xStart,
		yStart;
	if( vectorY )
	{
		// righmost top corner element
		xStart = xI-1;
		yStart = 0;
	}
	else
	{
		// tl corner
		xStart = 0;
		yStart = 1;
	}
	int prevMaxPos; // diag distance to preMax
	int prevHSPPos;
	//! we need to close HSP in this distance
	#define maxHSPDistance 10 
	// two triggering HSP must have a minimum distance
	#define minHSPDistance 10
	while(yStart < yI )
	{
		prevMaxPos = 0;
		prevHSPPos = 0;
		int x = xStart;
		int y = yStart;
		// do one diagonal
		while( x < xI &&
			   y < yI )
		{
			if(prevMaxPos > 0 &&
			   prevHSPPos == 0 &&	// do not evaluate close HSP in same diagonal!
			   V(x,y) > limit )
			{				
				if( prevMaxPos < maxHSPDistance )
				{	
					if(vectorY ||
						abs(x-y) >= windowSize ) // ommit too large overlappings
					{
						res[*cur].x = x;
						res[*cur].y = y;
						(*cur)++;
					}
					prevHSPPos = minHSPDistance;
				}
				prevMaxPos = 1;
/*				else // better than prev?
				{
					if(V(x,y) > V(res[cur-1].x,
								  res[cur-1].y) )
					{
						// overwrite prev
						res[cur-1].x = x;
						res[cur-1].y = y;
					}
					else // skip this max
					{
					}
				}
*/
			}
			else if(  prevMaxPos == 0 && V(x,y) > limit  )
			{
				prevMaxPos++;
			}
			else if( prevMaxPos > 0 && V(x,y) <= limit )
			{
				prevMaxPos++;
			}			
			if( prevHSPPos )
				prevHSPPos--;
			x++;
			y++;
		} // while, for all valid entries

		// next diagonal
		if( xStart > 0 ) 
			xStart--;
		else
			yStart++;
		
	} // while
	// mark end of list
	res[*cur].x = -1;
	res[*cur].y = -1;

    clock_t finish = clock();

	*processTime = (double)(finish - start) / CLOCKS_PER_SEC;
	return res;
}



#endif
