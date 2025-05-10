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

#if !defined( __fragm_h__ )
#define __fragm_h__

#include <string>
#include <sstream>
using namespace std;

//#include <stdlib.h>
#include <stdio.h>
#include "../leanguido/duration.h"


void normalize( long &n, long &d);

class TTimeSignature;
/// a fraction class 
class TFrac
{
protected:

	char valid;
	void normalize( void )
	{
		::normalize(numI, denomI );
	}

	void parseStr(const char *str);
	void init(long int num, long int denom);
public:

	
        long int numI;
    long int	 denomI; 
    TFrac(const TTimeSignature &sig);
	long int normalizeTo( long denom );
	TFrac(const char *str);
	TFrac( long int num = 0L, long int denom = 1L);
	TFrac(int num , int denom);
/*	char Valid( void )
		{ return valid; };*/

	string toString( void );
	long toLong( void );
	double toDouble( void );
	long int castLong(int ppq, long int *error = NULL);

//	operator long int ( void );
	TFrac& operator += (TFrac f2);
	TFrac operator + (const TFrac &f1);
	TFrac( lgDuration dur ) 
	{
		init(dur.durN, dur.durD );
	};

	TFrac incNum( int i);
//	TFrac decNum( int i );
	TFrac operator - (const TFrac &f1 );
	TFrac operator * (const TFrac &f2);
	TFrac operator * (double f2);
	TFrac& operator *= (double f2);

	TFrac& operator *= (const TFrac &f2);
	TFrac& operator /= (const TFrac &f2);
	TFrac& operator /= (int  f2);
	TFrac operator / ( const TFrac &f2);
	TFrac operator % (int f2);
	TFrac operator / (int f2);
	TFrac operator % ( const TFrac &f2);
	TFrac& operator -= (const TFrac &f2);
	char operator == (const TFrac &f2);
	char operator != (const TFrac &f2);
	char operator < (const TFrac &f2);
	char operator > ( const TFrac &f2);
	char operator >= (const TFrac &f2);
	char operator <= ( const TFrac &f2);
	char operator <= (long i);
	char operator >= (long i);
	char operator < (long i);
	char operator > (long i);
	long int numerator( void )
		{ return numI; };
	long int denominator( void )
		{ if( numI != 0 )
			return denomI;
		  else 
			return 0;
	};
	void Write( ostream &gmnOut );
    char natural( void );
    /// return 1 for x/3 x/5 x/7 x/13 x/17 etc....
    char nonBinary( void );
};

double toDouble( const TFrac &f2 );

long int getMinMultiple( long int i1, long int i2);
// long int smallestDiv( long int num, long int denom );
TFrac abs(const  TFrac &F1 );


char operator == (const TFrac &frac, const lgDuration &dur);
char operator == (const lgDuration &dur, const TFrac &frac);

#endif
