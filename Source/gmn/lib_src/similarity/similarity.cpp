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

// similarity.cpp : Definiert den Einsprungpunkt für die Konsolenanwendung.
//

// #include "stdafx.h"
#include "dynaprog.h" 
#include "simMatrix.h" // templates

template CSimMatrix<double, double>;
template CDynaprog<double>;

double _stdcall sim(double v1,
                    double v2,
					int windowSize);


double _stdcall val( double v1);


// template CSimMatrix<double,double>;



double _stdcall val(double v)
{
	return v;	
}

double _stdcall sim(double tv1,
		   double tv2,
		   int windowSize)
{
	double res = 0,
		   temp;
	int i;

	double *v1, *v2;
	v1 = &tv1;
	v2 = &tv1;

	for( i = 0; i < windowSize; i++ )
	{

		temp = v1[i] - v2[i];
		temp *= temp;

		res += temp;
	}
	return res;
}

/*
double _stdcall s(char x, char y, char gapv) // score matrix
{
	int dist;
	dist = x - y;
	if( dist > 0 )
		dist *= -1;

	if(x == gapv || y == gapv )
		dist = -1;
	return (double)dist;
}
*/
double _stdcall s(double x, double y, double gapv); // score matrix
double _stdcall s(double x, double y, double gapv) // score matrix
{
	double dist;
	dist = x - y;
	dist *= dist;

	dist *= -1;

	if(x == gapv || y == gapv )
		dist = -0.1;

	return dist;
}

/*
double max3(double f1, double f2, double f3)
{
	double res = f1;

	if( f2 > res )
	{
		res = f2;
	}
	if( f3 > res )
	{
		res = f3;
	}

	return res;
}
*/

int main(int argc, char* argv[])
{

	CDynaprog<double> *myDynaProg;



	/*
		Problem with first character: add an equal first character,
		Problem strlength: add dummy stringlength
	*/

	// myDynaProg = new dynaprog("zaxxxxyu", "aubcd");

#define l1 10
#define l2 5
	double a1[l1] = {3.5, 1.2, 2, -2, 1, 1, -3, 2, 4, 1},
		a2[l2] = { 3.5, 1.2, 2, 1, -3};
        double gapVal = 0;
        myDynaProg = new CDynaprog<double>(&(a1[0]),&(a2[0]),l1,l2, gapVal,1,val, s);

	myDynaProg->makeTable(-0.2);
	myDynaProg->retrieveAlignment();

	myDynaProg->debug();

	delete myDynaProg;

	CSimMatrix<double, double> *myMatrix;
	myMatrix = new CSimMatrix<double, double>(l1,l1, sim, val);


	myMatrix->setXVector(a1);
	myMatrix->calcTable(1,1);
	myMatrix->debug();

	delete myMatrix;

	return 0;
}

