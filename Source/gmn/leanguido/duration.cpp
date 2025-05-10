/*
	leanGUIDO class library
	Copyright (C) 2003-04  Juergen Kilian, Hogler H. Hoos, SALIERI Project

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string>
using namespace std;
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "duration.h"



long saveAtol(const char *str)
{
//   char *stringPtr = "                             ";
   long d = strtol( str, NULL,10 );
   return d;
}
int saveAtoi(const char *str)
{
	return (int)(saveAtol(str));
}
double saveAtof(const char *str)
{
   char *stringPtr;
   double d = strtod( str, &stringPtr );
   return d;
}



string lgDuration::toString( void )
{
	ostringstream res;

	res << durN;
	res << "/";
	res << durD;
	return res.str();
}


lgDuration::lgDuration( const string &str )
{
    const char *cstr = str.c_str();
    const char *denom = strstr(cstr,"/");
    if( denom )
        denom++;
    durN = saveAtol(cstr);
    if( denom )
        durD = saveAtol(denom);
    rationalise();
}
lgDuration::lgDuration(  long int num,  long int denom ) 
				: durN(num), durD(denom)
{
	// don't allow zero denominators!
	if( durN == 0 )
		durD = 1;
    rationalise();
}


void lgDuration::setDenom(long int denom)
{
	durD = denom;

}
void lgDuration::setEnum(long int enumerator)
{
	durN = enumerator;

}


lgDuration::lgDuration( const lgDuration& d )
{
    durN = d.durN;
    durD = d.durD;
}

lgDuration lgDuration::operator +( const lgDuration &dur )
{
    lgDuration d = lgDuration( durN * dur.durD + dur.durN * durD, durD * dur.durD );
    d.rationalise();
    return d;
}

lgDuration lgDuration::operator -( const lgDuration &dur )
{
    lgDuration d = lgDuration( durN * dur.durD - dur.durN * durD, durD * dur.durD );
    d.rationalise();
    return d;
}

lgDuration lgDuration::operator *( const lgDuration &dur )
{
    lgDuration d = lgDuration( durN * dur.durN, durD * dur.durD );
    d.rationalise();
    return d;
}

lgDuration lgDuration::operator /( const lgDuration &dur )
{
    lgDuration d = lgDuration(durN * dur.durD, durD * dur.durN);
    d.rationalise();
    return d;
}

lgDuration& lgDuration::operator +=( const lgDuration &dur )
{
    durN = durN * dur.durD + dur.durN * durD;
    durD = durD * dur.durD;
    rationalise();
    return (*this);
}

lgDuration& lgDuration::operator -=( const lgDuration &dur )
{
    durN = durN * dur.durD - dur.durN * durD;
    durD = durD * dur.durD;
    rationalise();
    return (*this);
}

lgDuration& lgDuration::operator *=( const lgDuration &dur )
{
    durN = durN * dur.durN;
    durD = durD * dur.durD;
    rationalise();
    return (*this);
}

lgDuration& lgDuration::operator /=( const lgDuration &dur )
{
    durN = durN * dur.durD;
    durD = durD * dur.durN;
    rationalise();
    return (*this);
}

bool lgDuration::operator >( const lgDuration &dur )
{
    // a/b > c/d if and only if a * d > b * c.
    return ((durN * dur.durD) > (durD * dur.durN));
}

bool lgDuration::operator <( const lgDuration &dur )
{
    // a/b < c/d if and only if a * d < b * c.
    return ((durN * dur.durD) < (durD * dur.durN));
}

bool lgDuration::operator ==( const lgDuration &dur )
{
    // a/b < c/d if and only if a * d < b * c.
    return ((durN * dur.durD) == (durD * dur.durN));
}

bool lgDuration::operator !=( const lgDuration &dur )
{
    // a/b < c/d if and only if a * d < b * c.
    return ((durN * dur.durD) != (durD * dur.durN));
}

// gcd(a, b) calculates the gcd of a and b using Euclid's algorithm.
 long int lgDuration::gcd(  long int a1,  long int b1 )
{
	long int a = abs(a1);
	long int b = abs(b1);

    if (!(a == 0) || (b == 0))
    {
        while (b > 0)
        {
            long int r = a % b;
            a = b;
            b = r;
        }
        return a;
    }
    return 1;
}

void lgDuration::rationalise( void )
{
	if( durN == 0 )
	{
		durD = 1;
		return;
	}
    long int g = gcd( durN, durD );
	if( g != 0 )
	{
		durN /= g;
		durD /= g;
	}
    if( durN == 0 )
        durD = 1;
	else if( durD == 0 )
	{
		durD = 1;
		durN = 0;
	}
}


void lgDuration::write(FILE *out )
{
	fprintf(out, "%s", toString().c_str());
}


double lgDuration::toDouble( void )
{
    double res = 0;
    if(durD != 0)
    {
        res = (double) durN / (double) durD;
    }
    return res;
}
