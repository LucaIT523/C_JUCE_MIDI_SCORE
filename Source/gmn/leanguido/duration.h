/*
	leanGUIDO class library
	Copyright (C) 2003  SALIERI Project

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

/*!
   Fraction class implemented at UBC/Vancouver
   adapted and improved 2003 for leanGUIDO by Juergen Kilian (kilian@noteserver.org)
*/

#ifndef __duration_h__
#define __duration_h__

#include <stdio.h>
#include <string>
using namespace std;
#include <string>

//! generic fraction class. lgDuration is used to store the information pertaining to the duration/attacktime of a note.
#ifdef lgDuration
alksdjlk
#endif

class lgDuration
{

public:
	/// numerator
    long int durN;
    /// denominator
    long int durD;
    lgDuration(  long int num = 0,  long int denom = 1 );
    lgDuration( const lgDuration& d );
    lgDuration( const string &str );

    void setDenom(long int denom);
    void setEnum(long int enumerator);


    
    lgDuration operator +( const lgDuration &dur );
    lgDuration operator -( const lgDuration &dur );
     //! Useful for notes with dots.
    lgDuration operator *( const lgDuration &dur ); 
    lgDuration operator /( const lgDuration &dur );
    // (i.e. dur * 3/2 or dur * 7/4)

    lgDuration& operator +=( const lgDuration &dur );
    lgDuration& operator -=( const lgDuration &dur );
     //! Useful for notes with dots.
    lgDuration& operator *=( const lgDuration &dur ); 
    lgDuration& operator /=( const lgDuration &dur );
    // (i.e. dur * 3/2 or dur * 7/4)

    bool operator >( const lgDuration &dur );
    bool operator >= ( const lgDuration &dur )
    {return ! (*this < dur); };
        bool operator <( const lgDuration &dur );
    bool operator <=( const lgDuration &dur )
    {return ! (*this > dur); };
        
    bool operator ==( const lgDuration &dur );
    bool operator !=( const lgDuration &dur );
    /// Used to "rationalise" duration.
    void rationalise( void ); 

    /// Used by rationalise(). greatest common divisor
    long int gcd(  long int a,  long int b ); 

    virtual string toString( void );
	void write( FILE *out);
    double toDouble( void );
};


long saveAtol(const char *str);
double saveAtof(const char *str);
int saveAtoi(const char *str);

#endif











