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

/*--------------------------------------------------------------
	Implementation of TFrac
	Author: Juergen Kilian
	Date:   1998-2001
----------------------------------------------------------------*/
#include <sstream>
using namespace std;
#include <string.h>

#include "fragm.h"
#include "timesig.h"
#include <stdlib.h>
#include "../lib_src/ini/ini.h" // for saveAtoX
//#include <string.h>



//#define CDENOM 10
//int denomList[CDENOM] = {2,3,5,7,11,13,17,19,23,29};

/*!
   return 1 for all non binary denominars
*/
char TFrac::nonBinary( void )
{
	long bin = 2048;
	while( denomI > bin )
		bin *= 2;
	if( bin % denomI != 0)
		return 1;
	else
		return 0;
	
    if( denomI == 1  )
        return 0;
    int i = 256;

    while( i > 1 )
    {
        if( denomI == i )
            return 0;
        i /= 2;
    }
    
    return 1; // non "binary" denominator
}

void TFrac::Write(ostream &gmnOut)
{
	if( denomI != 1 )
		gmnOut <<numI <<"/"<< denomI <<"("<<toDouble() <<") ";
	else
		gmnOut <<numI <<"/"<< denomI ;
}

TFrac::TFrac(const char *str)
{
	init(0,1);
	
	parseStr(str);
}

TFrac::TFrac(  int num, int denom )
{
		init(num, denom);
}


void TFrac::parseStr(const char *strC)
{
	long int num,
				denom;

	const char *numS;
	char *denomS;

	char *str;

	if( !strC ||
		*strC == 0)
		return;

	str = new char[ strlen(strC) +1 ];
	strcpy( str, strC );

	numS = str;
	denomS = strstr(str, "/");
	if(denomS  )  // slash found?
	{
		*denomS = 0;        // str end of numerator
		denomS++;			// skip denom;
	}
	else	// no slash
	{
		denomS = "1";
	}

	num = saveAtol(numS);
	denom = saveAtol(denomS);

	init(num, denom);
	delete [] str;
}

void TFrac::init(long int num, long int denom)
{
	numI = num;
	denomI = denom;
	if( denom == 0 )
	{
		numI = 0;
		valid = 0;
	}
	else
		valid = 1;
	if( numI == 0 )
		denomI = 1;
	normalize();
}

TFrac::TFrac( long int num, long int denom )
{
	numI = num;
	denomI = denom;
	if( denom == 0 )
	{
		numI = 0;
		valid = 0;
	}
	else
		valid = 1;
	normalize();
}
/*
TFrac::operator double ( void )
{
	double res;
	if( valid )
	{
		res = (double)numI / (double)denomI;
	}
	else
	{
		res = 0;
	}
	normalize();
	return res;
}
*/

long  TFrac::normalizeTo( long int denom )
/*
	res:
	 >0 : error; denom not multiple of denomI
	 0 : ok
*/
{
		if( !denom )
		{
			return 1;
		}
		if( !denomI )
		{
      	denomI = denom;
			return 0;
		}

		long int multiply;
		multiply = (denom / denomI);
		numI *= multiply;
		denomI = denom;

		return( denom % denomI );
}



double TFrac::toDouble (void)
{
return ::toDouble(*this);
}

/*
	convert return n*d
*/
double toDouble(const TFrac &f2 )
{
	double res;
	if( f2.denomI != 0)
		res = (double)f2.numI / (double)f2.denomI;
	else
		res = 0;

	return res;
}



long TFrac::toLong (void)
/*
	convert return n*d
*/
{
	long int res;
	if( denomI )
		res = numI / denomI;
	else
		res = 0;

	// make "if( TFrac )" save
	if( res == 0 &&
		 numI > 0 )
	{
		res = 1;
	}
	return res;
}


long int TFrac::castLong (int ppq, long int *error)
/*
	res = (n/d)*4*ppq
*/
{
	long int tempError;

	long int tempNum,
				res;

	tempNum = numI * 4;
	tempNum *= ppq;

	res = tempNum / denomI;
	if( error ) // calculate error
	{
		tempError = tempNum - (res * denomI);
		*error = tempError;
	}
	return res;

}


TFrac& TFrac::operator += ( TFrac f2)
{


	long int minMult;
	if( numI == 0 )
	{
		// copy f2
		denomI = f2.denomI;
		numI = f2.numI;
		// normalize();
	}
	else if( f2.numI == 0)
	{
		// DO nothing
	}
	else if( denomI != f2.denomI )
	// find min denom
	{

		minMult = getMinMultiple( denomI, f2.denomI );

		// normalize to shared denominator
		normalizeTo( minMult );
		f2.normalizeTo( minMult );

		// add
		numI += f2.numI;
		normalize();
	}
	else if( denomI == f2.denomI )
	{
		// add
		numI += f2.numI;
		normalize();
	}
    return (*this);    
}

TFrac TFrac::operator + (const TFrac &f1)
{
	TFrac res( numI, denomI);
	res += f1;
	return res;
}

TFrac TFrac::incNum( int i)
// increment numerator by i 
{
	TFrac res( (long)(numI + i),
					denomI );

	normalize();
	return res;
}
/*
TFrac TFrac::decNum( int i )
{
	TFrac res( numI - i,
					denomI );

	normalize();
	return res;
}
*/

TFrac TFrac::operator - (const TFrac &f1)
{
	TFrac res( numI, denomI);
	res -= f1;
	return res;

}
TFrac& TFrac::operator *= (double f2)
{
	if( numI < 10 &&
		numI > -10 )
	{
		numI = (long)((double)numI * f2 * 1000);
		denomI *= 1000;
	}
	else if( numI < 100 &&
		numI > -100 )
	{
		numI = (long)((double)numI * f2 * 100);
		denomI *= 100;
	}
	else
	{
		numI = (long)((double)numI * f2);
	}
	normalize();
    return (*this);
}

TFrac& TFrac::operator *= (const TFrac &f2)
{
    long int f2denomI = f2.denomI;
    long int f2numI = f2.numI;
	::normalize(numI, f2denomI);
	::normalize(f2numI, denomI);

	numI *= f2numI;
	denomI *= f2denomI;
	normalize();
    return (*this);
}
TFrac& TFrac::operator /= (const TFrac &f2)
{
    long int f2denomI = f2.denomI;
    long int f2numI = f2.numI;
	::normalize(numI, f2numI);
	::normalize(f2denomI, denomI);

	numI *= f2denomI;
	denomI *= f2numI;
	normalize();
    return (*this);
}
TFrac& TFrac::operator /= (int f2)
{
	if( f2 != 0 )
		denomI *= f2;
	normalize();
    return (*this);
}
TFrac TFrac::operator / (int f2)
{
	TFrac res(numI, denomI);
	res /= f2;
	return res;
}
TFrac TFrac::operator / (const TFrac &f2)
{
	TFrac res(numI, denomI);
	res /= f2;
	return res;
}
TFrac TFrac::operator * (double f2)
{
	TFrac res(numI, denomI);
	res *= f2;
	return res;
}

TFrac TFrac::operator * (const TFrac &f2)
{
	TFrac res(numI, denomI);
	res *= f2;
	return res;
}


TFrac& TFrac::operator -= (const TFrac &f2)
{
    // a2 = a - b -> -a2 = -a + b 
    numI *=-1;
//	f2.numI *= -1;
	*this +=(f2);
    numI *= -1;
    return (*this);
}
char TFrac::operator != (const TFrac &f2)
{
	return !(*this == f2 );
}

char TFrac::operator == (const TFrac &f2)
{
	// because fractions are always normalized, only compare
	// needs to be done


	if( numI == f2.numI &&
		 denomI == f2.denomI )
	{
		return 1;
	}
	else if( numI == 0 &&
		 f2.numI == 0 )
	{
		return 1;
	}
	return 0;
}

char TFrac::operator < (long f2)
{
	return toDouble() < f2;
}
char TFrac::operator > (long f2)
{
	return toDouble() > f2;
}
char TFrac::operator <= (long f2)
{
	return toDouble() <= f2;
}
char TFrac::operator >= (long f2)
{
	return toDouble() >= f2;
}

char TFrac::operator < (const TFrac &f2)
{
//	TFrac f1( numI, denomI );
//	long int minMult;

    return toDouble() <  ::toDouble(f2);
    
    /*
	minMult = getMinMultiple( f1.denomI, f2.denomI );

	f1.normalizeTo( minMult );
	f2.normalizeTo( minMult );
	if( f1.numI < f2.numI )
	{
		return 1;
	}
	return 0;
     */
}
char TFrac::operator > ( const TFrac &f2)
{
/*
	TFrac f1( numI, denomI );
	long int minMult;

	minMult = getMinMultiple( f1.denomI, f2.denomI );

	f1.normalizeTo( minMult );
	f2.normalizeTo( minMult );
	if( f1.numI > f2.numI )
	{
		return 1;
	} 
	*/
    if( toDouble() > ::toDouble(f2) )
		return 1;
	return 0;

}
char TFrac::operator >= ( const TFrac &f2)
{
	if( *this == f2 ||
		 *this > f2 )
	{
		return 1;
	}
	return 0;
}

char TFrac::operator <= (const  TFrac &f2)
{
	if( *this == f2 ||
		 *this < f2 )
	{
		return 1;
	}
	return 0;

}

long int gcd(  long int a1,  long int b1 )
{
    long int r;

	long int a = abs(a1);
	long int b = abs(b1);

    if (!(a == 0) || (b == 0))
    {
        while (b != 0)
        {
            r = a % b;
            a = b;
            b = r;
        }
		return a;
    }
    return 1;
}


void normalize( long &numI, long &denomI)
{

	if( !denomI )
	{
//		valid = 0;
		return;
	}
	if( !numI )
	{
		denomI = 1;
		return;
	}
	// signature is carried in numI
	if( denomI < 0 )
	{
		denomI *= -1;
		numI *= -1;
	}

	// numI == n*denomI;
	if( numI &&
		 denomI &&
		 !(numI % denomI ) )
	// numI = n * denomI;
	{
		numI /= denomI;
		denomI = 1;
		return;
	}
	// denomI == n*numI
	if( numI &&
		 denomI &&
		 !(denomI % numI ) )
	{
		denomI = (long)denomI / numI;
		numI = 1;
		// check signature
		if( denomI < 0 )
		{
			numI *= -1;
			denomI *= -1;
		}
		return;
	}
/*
	// check signature
	if( denomI < 0 )
	{
		numI *= -1;
		denomI *= -1;
	}
*/



	long int ggt;
	ggt = gcd( numI, denomI);
	numI /= ggt;
	denomI /= ggt;
	return;

	// try to normalize
    /*
	long int kgT;


	while( !ende )
	{

		kgT = smallestDiv( numI, denomI );
		if( kgT > 0 )
		{
			numI /= kgT;
			denomI /= kgT;
		}
		else
		{
			ende = 1;
		}
	} // while ! ende
	while( denomI > 50000L )
	{
		denomI /= 2;
		numI /= 2;
	};
     */
}

long int getMinMultiple( long int d1, long int d2 )
{
	long int maxD,
		 minD;

	if( !d1 ||
		 !d2 )
	{
   	return 0;
	}
	if( d1 >= d2 )
	{
		maxD =  d1;
		minD =  d2;
	}
	else
	{
		maxD = d2;
		minD = d1;
	}

	// maxD is multiple of minD
	if( !(maxD % minD) )
		return maxD;

	int i;
	i = 2;
	char ende = 0;
	long int tempD = maxD;
	while( !ende &&
			 i <= minD )
	{
		tempD += maxD;
		if( !(tempD % minD) )
		{
			ende = 1;
		}
		else
		{
			i++;
		}
	}
	return tempD;
}
/*
long int smallestDiv( long int num, long int denom )
{
	char ende = 0;
	int denomPos = 0;
	long	 curDenom = 0;

	if( labs(denom) == 1 )
		return 0;
	if( num == 0 )
		return 0;
	if( denom == 0 )
		return 0;

	while( !ende )
	{
		// check for primeNumbers
		if( denomPos < CDENOM )
		{
			curDenom = denomList[denomPos];
			if( curDenom > labs(num) ||
				 curDenom > labs(denom) )
			{
				curDenom = 0;
				ende = 1; // stop normalizing
			}
			else
			{
				if( !(num % curDenom) &&
					 !(denom % curDenom) )
				{
					ende = 1; // divident found
				}
				else // test next denom
				{
					denomPos++;
				}
			} // curDenom > denomI
		}
		else // denomPos > CDENOM
		{
      	curDenom = 0; // invalid
			ende = 1;
		}
	}
	return curDenom;
}
*/
/*
	result:
		1 : fragm == natural (denom==1)
		0 : else
*/
char TFrac::natural(void )
{
	if( denomI == 1 )
		return 1;
	return 0;
}

TFrac abs( const  TFrac &F1 )
{
	return TFrac(labs(F1.numI), labs(F1.denomI));
};

TFrac TFrac::operator % ( int f2)
{
	return *this % TFrac(f2,1);
}

TFrac TFrac::operator % ( const TFrac &f2)
{
	long mult;
	if( f2.numI == 0L )
		return 0L;

	if(*this < f2 )
		return *this;
	if(*this == f2 )
   	return 0L;

	mult = (*this / f2).toLong();
	TFrac res(f2);

    
	res = *this - (res * mult);
	return res;
}



TFrac::TFrac(const TTimeSignature &sig)
{
	init( sig.numeratorI, sig.denominatorI);
}


string TFrac::toString( void )
{
	ostringstream res;
	res << numerator();
	res << "/";
	res << denominator();
	return res.str();
}


char operator == (const TFrac &frac, const lgDuration &dur)
{
	TFrac temp(dur.durN, dur.durD);	
	return(  temp == frac );

}
char operator == (const lgDuration &dur, const TFrac &frac)
{
	return  (frac == dur);
}
