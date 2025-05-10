/*
	leanGUIDO class library
	Copyright (C) 2003  Juergen Kilian, SALIERI Project

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

/*
 *  lgnote.cpp
 *
 */

#include <string>
#include <sstream>

#include "lgnote.h"
#include "lgsequence.h"

//-------------------------------------------------
lgNote::lgNote( int pc,
                int oct,
                int acc,
                long int durNum,
                long int durDenom,
                int cdots,
                long int posNum,
                long int posDenom) : lgRest(durNum, durDenom, cdots,
                                             posNum, posDenom)
{
    pitchClassI = pc;
    octaveI = oct;
    accidentalsI = acc;    
}

lgNote::lgNote( int pc ) : lgRest(0,1,0,0,1){
	 pitchClassI = pc;
	 octaveI = 1;
	 accidentalsI = 0;
}

const char *lgNoteNames[12]={"c","cis","d","dis","e","f","fis","g","gis","a","ais","b"}; 
string lgNote::toString( lgVoice * )
{
	string s = pitch();
	s += lgEvent::toString();
	return s;
}

string lgNote::pitch( void )
{
    ostringstream res;
    
    //const char *name;
    int nameID = pitchClassI;
    while( nameID < 0 )
        nameID += 12;
    while( nameID > 11 )
        nameID -= 12;
    /// copy pitch class
    res<< lgNoteNames[nameID];
    if( accidentalsI > 0 )
    {
        for(int i=0; i < accidentalsI; i++)
            res << "#";
    }
    if( accidentalsI < 0 )
    {
        for(int i=0; i > accidentalsI; i--)
	        res<< "&";
    }
    res<< octaveI;

    return res.str();
}

int lgNote::pitchClass()
{
	return pitchClassI;
}

int lgNote::octave()
{
	return octaveI;
}


