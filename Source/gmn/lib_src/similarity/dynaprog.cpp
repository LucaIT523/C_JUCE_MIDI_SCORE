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

// dynaprog.cpp: Implementierung der Klasse dynaprog.
//
//////////////////////////////////////////////////////////////////////

kjhkjhkjhkjh
//#include "stdafx.h"
#include "dynaprog.h"
#include <stdio.h>
#include <string>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
double max3(double f1, double f2, double f3)
{
	double res = f1;

	if( f2) > res )
	{
		res = f2;
	}
	if( f3 > res )
	{
		res = f3;
	}

	return res;
}

/*
template <class T>dynaprog<T>::dynaprog(const char *s1, 
										const char *s2)
{

	gapVal = '_';
	// length
	iI = strlen(s1);
	jI = strlen(s2);

	// start index
	stiI = 0;
	stjI = 0;

	// copy strings into destination
	S1fl = NULL;
	S2fl = NULL;
	S1str = s1;
	S2str = s2;


	S1outStr = NULL;
	S2outStr = NULL;
	S1outFl = NULL;
	S2outFl = NULL;

	createTable(iI+1, jI+1);
	outStart =iI+jI+1;
}
*/
template <class T>  void dynaprog<T>::createTable( int sizeI, int sizeJ )
{

	tableV = new double[sizeI*sizeJ];

	int i,j;
	// init with -1;
	for( i=0; i < sizeI; i++ )
	{
		for( j = 0; j < sizeJ; j++ )
		{
			V(i,j,0);
		}
	}
}

template <class T>dynaprog<T>::~dynaprog()
{

	delete [] tableV;
//	if( S1outStr )
//		delete [] S1outStr;
//	if( S2outStr )
//		delete [] S2outStr;
	if( S1outFl )
		delete [] S1outFl;
	if( S2outFl )
		delete [] S2outFl;
}

template <class T>
double  dynaprog<T>::V(int i, int j)
{
	if(i <= iI && j <= jI)
		return tableV[i*(jI+1) + j];
	else
	{
		printf("range error!\n");
		return 0;
	}
}


template <class T>  
void dynaprog<T>::V(int i, int j, double val)
{
	if(i <= iI && j <= jI)
	{
		tableV[i*(jI+1) + j] = val;
	}

}

template <class T>
int  dynaprog<T>::makeAlignment(double limit )
{
#ifdef complete_table
	int k;
	// init V(0,j)
	for( int j = 1; j <= jI; j++)
	{
		V(0,j,0);
		for( k = 1; k <= j; k++ )
		{
			V(0,j,  V(0,j) + simDyna(gapVal, S2[k-1]) );
		}
	}

	// init V(i,0)
	for( int i = 1; i <= iI; i++)
	{
		V(i,0,0);
		for( k = 1; k <= i; k++ )
		{
			V(i, 0, V(i,0) + simDyna(S1[k-1], gapVal) );
		}
	}

	debug();
	// optimise
	double bestVal;

//	V(0,0, simDyna(S1[0],S2[0]));
	V(0,0,0 );
	for( j = 1; j <= jI; j++)
	{
		for( i = 1; i <= iI; i++)
		{
			bestVal = max3(V(i-1,j-1) + simDyna(S1[i-1], va(S2[j-1])),
						  V(i-1,j) + simDyna(S1[i-1],gapVal),
						  V(i, j-1) + simDyna(gapVal,S2[j-1]) );

						  
			V(i, j,  bestVal );
		}
	}
#else
// do only diagonals -> early abort if costs get to high is possible



	int iMax = -1,
		jMax = -1;

	int i = 0;
	int j = 0;
	double bestVal,
		  startVal = 0,
		  sumVal = 0;

	char end = 0;

	maxPathX = 0;
	maxPathY = 0;

	V(0,0,0);
	int k,maxDiag;
	maxDiag = iI;
	if( jI > maxDiag )
		maxDiag = jI;


	maxPathX = 0;
	maxPathY = 0;

	for( k = 0; k <= maxDiag; k++)
	{		
		startVal = -500;


		iMax = k;
		if( k > iI )
		{
			iMax = iI+1;
		}

		// do row?
		if( k <= jI)
		{

			for( i = 0; i < iMax; i++ )
			{
				bestVal = calcV(i,k);

				printf("%d, %d\n",i,k);
				if(bestVal > startVal)
					startVal = bestVal;

				V(i,k,bestVal);
			}
		}

		
		jMax = k;
		if( k > jI )
		{
			jMax = jI+1;
		}

		if( k <= iI ) // do column
		{


			for( j = 0; j < jMax; j++ )
			{
				printf("%d, %d\n",k,j);
				bestVal = calcV(k,j);
				if(bestVal > startVal)
					startVal = bestVal;

				V(k,j,bestVal);
			}			
		}

		// do diagonal elemnt

		if( k <= iI &&
			k <= jI )
		{
				bestVal = calcV(k,k);

				printf("%d, %d\n",k,k);
				if(bestVal > startVal)
					startVal = bestVal;

				V(k,k,bestVal);

		}



		debug();
		sumVal = startVal;

		if( sumVal < limit )
		{
			printf("Sum of path = %f > limit %f\n",
				sumVal,
				limit);
			return 0;
		}
		maxPathX = k;
		maxPathY = k;

		if( maxPathX > iI )
			maxPathX = iI;
		if( maxPathY > jI )
			maxPathY = jI;
	
	}
	
	debug();
#endif


	return 1;
}

template <class T> 
void dynaprog<T>::debug(FILE *out)
{

	char mustClose = 0;
	if( !out )
	{
		out = fopen("_dynaprog.txt","wt");
		mustClose = 1;
	}
	int i, j;
	for( j = 0; j <= jI; j++ )
	{
		for( i = 0; i <= iI; i++ )
		{
			fprintf(out, "%2.3f ",V(i,j) );
		}
		fprintf(out, "\n");
	}
	fprintf(out, "\n");

	writeS1( out );
	fprintf(out, "\n");
	writeS2( out );
	fprintf(out, "\n");
	writeS1out( out );
	fprintf(out, "\n");
	writeS2out( out );
	fprintf(out, "\n");


	fprintf(out, "maxX/Y= %d/%d\n",maxPathX, maxPathY);
	if( mustClose )
		fclose(out);
}

template <class T>
 void dynaprog<T>::retrieveAlignment(int x, int y)
{
	// Total costs are  in V(iI, jI);
	double sum;

//	if( S1str )
//	{
//		S1outStr = new char [iI+jI+1];
//		S2outStr = new char [iI+jI+1];
//	}
//	else
	{
		S1outFl = new T [iI+jI+1];
		S2outFl = new T [iI+jI+1];
	}


	sum = V(iI, jI);

	int steps = iI+jI;


	// go "steps" back
	int k;

	int i,j;

	int sel; // -1 = (-1,0), 0 = (-1,-1), 1 = (0,-1)
/*
	
	for(i=0; i < steps; i++)
	{
		setS1out(i, '.');
		setS2out(i, '.');

	}
	setS1out(i, 0);
	setS2out(i, 0);
*/

	/*
	i = iI;

	j = jI;
*/
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



	k = 0;
	while( i > 0 || j > 0 )
	{
		sum = gestBestNeighbour(i,j, sel);
//		printf("%f, %i, %i, %i, %f\n",V(i,j) ,i,j,sel, sum);

		switch(sel) 
		{
		case -1 : if( i-1 > -1 )
					setS1out(steps-k-1, e1(i-1));
					setS2out(steps-k-1, gapVal);
				  i--;
			break;
		case 1 :  setS1out(steps-k-1, gapVal);
				  if( j-1 > -1 )
					setS2out(steps-k-1, e2(j-1));
				  j--;
				  break;
		case 0: if( i-1 > -1 )
					setS1out(steps-k-1, e1(i-1));
				if( j-1 > -1 )
					setS2out(steps-k-1, e2(j-1));
				i--;
				j--;
				break;
		debug();

	
		}
		k++;
		debug();
	}
//	if(S1outStr)
//	{
//		setS1out(steps, 0);
//		setS2out(steps, 0);
//	}
	

	debug();
}

 template <class T>
double dynaprog<T>::gestBestNeighbour(int i, int j, int &sel)
{
	double best = 0;

	sel = 0;

	

	if( j > 0 && i > 0 )
	{
		best = V(i-1,j-1);
		sel = 0; // diagonal step
	}

	if( (V(i-1,j) > best) ||
		j == 0 )
	{
		best = V(i-1,j);
		sel = -1;
	}

	if( (V(i,j-1) > best) ||
		i == 0)
	{
		best = V(i,j-1);
		sel = 1;
	}

	return best;
}

template <class T>
dynaprog<T>::dynaprog(const T *s1, 
				   const T *s2, 
				   int l1,  /// length of s1
				   int l2,
				   T gapv )
{
	gapVal = gapv;

	S1fl = s1;
	S2fl = s2;
	stiI = 0 ;
	stjI = 0;
	iI   = l1;
	jI   = l2;


	outStart =iI+jI+1;

//	S1str = NULL;
//	S2str = NULL;
	
	S1outFl = NULL;
	S2outFl = NULL;
//	S1outStr = NULL;
//	S2outStr = NULL;

	createTable( iI+1, jI+1);
}

template <class T>
T dynaprog<T>::e1(int i)
{
//	if( S1str )
//		return S1str[i+stiI];
//	else
		return S1fl[i+stiI];

}

template <class T>
T dynaprog<T>::e2(int j)
{
//	if( S2str )
//		return S2str[j+stjI];
//	else
		return S2fl[j+stjI];
}


template <class T> 
 void dynaprog<T>::setS1out(int id, T val)
{
	if( id < outStart )
		outStart = id;

//	if( S1outStr )
//		S1outStr[id] = val;
//	else
		S1outFl[id] = val;
}

template <class T> 
void dynaprog<T>::setS2out(int id, T val)
{
	if( id < outStart )
		outStart = id;

//	if( S2outStr )
//		S2outStr[id] = val;
//	else
		S2outFl[id] = val;

}

template <class T> 
void dynaprog<T>::writeS1(FILE *out)
{
	int i;
	char c;
	for( i = 0; i < iI; i++ )
	{
//		if(S1str)
//		{
//			c = e1(i);
//			fprintf(out, "%c", c);
//		}
//		else
		{
			fprintf(out, "%3.3f, ", valDyna(e1(i))) ;
		}
	}
}
template <class T>
void dynaprog<T>::writeS2(FILE *out)
{
	int i;
	char c;
	for( i = 0; i < jI; i++ )
	{
//		if(S1str)
//		{
//			c = e2(i);
//			fprintf(out, "%c", c);
//		}
//		else
		{
			fprintf(out, "%3.3f, ", valDyna(e2(i))) ;
		}
	}

}
template <class T> 
void dynaprog<T>::writeS1out(FILE *out)
{
	int i;
	for( i = 0; i < outStart; i++)
		fprintf(out, ".");
	
//	if(S1outStr)
//	{
//		fprintf(out, "%s", S1outStr+outStart);
//	}

//	else if( S1outFl )
	{
		double temp;
		for( i = outStart; i < iI+jI; i++ )
		{
			temp = valDyna(S1outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	}

}
template <class T> 
void dynaprog<T>::writeS2out(FILE *out)
{
	int i;
	for( i = 0; i < outStart; i++)
		fprintf(out, ".");

//	if(S2outStr)
//	{
//		fprintf(out, "%s", S2outStr+outStart);
//	}

//	else if( S2outFl )
	{
		double temp;
		for( i = outStart; i < iI+jI; i++ )
		{
			temp = valDyna(S2outFl[i]);
			fprintf(out, "%3.3f, ", temp) ;
		}
	}

}

/*
0    1     2     3      4      iI

1   1/1    

2   2/1

jI  jI/1        jI/jI  


*/

template <class T>
double dynaprog<T>::calcV(int i, int j)
{
	double bestVal;

			if( i == 0 && j == 0 )
			{
				V(0,0,0 );
				bestVal = 0;
			}
			else if( i == 0 )
			{
				bestVal = V(0,j-1) + simDyna(gapVal, e2(j-1), gapVal);
				// V(0,j, bestVal  );
			}
			else if( j == 0 )
			{
				bestVal = V(i-1,0) + simDyna(e1(i-1), gapVal, gapVal);
				// V(i, 0, bestVal );
			}
			else
			{
				bestVal = max3(V(i-1,j-1) + simDyna(e1(i-1), e2(j-1), gapVal),
					V(i-1,j) + simDyna(e1(i-1),gapVal, gapVal),
					V(i, j-1) + simDyna(gapVal,e2(j-1), gapVal) );
				
				
				// V(i, j,  bestVal );
				
			}

			return bestVal;
}
