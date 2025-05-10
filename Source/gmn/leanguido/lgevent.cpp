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
 *  lgevent.cpp
 */

#include <string>
#include <sstream>
using namespace std;

#include "lgevent.h"
#include "lgsequence.h"

lgEvent::lgEvent( long int durNum, long int durDenom,
                  int cdots,
                  long int posNum, long int posDenom) : lgObject(),
				  posI(posNum, posDenom),
				  durationI( durNum, durDenom)
{
    cDotsI = cdots;
    this->mIsAbsDur = false;
}

/// skip all rests, etc.
lgNote *lgEvent::nextNote( void )
{
    lgObject *temp = next();
    while( temp &&
           !dynamic_cast<lgNote *>(temp) )
    {
        temp = temp->next();
    }
    return dynamic_cast<lgNote *>(temp);
}

/// base event class returns only duration, derived classes return also pitch, ...
string lgEvent::toString( lgVoice * )
{
    ostringstream res;
	res<<"*";
	res<<durationI.toString();
	int i;
    for( i = 0; i < cDotsI; i++)
		res<<".";
	return res.str();
}


void lgEvent::write( FILE *out,
                    lgVoice *callSeq)
{
	fprintf(out, "%s", toString().c_str() );

    fprintf(out, " " );
    if( callSeq )
    	fprintf(out, "%s", callSeq->tagsToString(this).c_str() );
    	
}


//! return real duration, including dots!
lgDuration lgEvent::duration( void )
{
	lgDuration res(durationI),
			 dotDur( durationI ),
			 half(1,2);

	int i = cDotsI;
	while( i )
	{
		dotDur *= half;
		res += dotDur;
		i--;
	}
	return res;
}

void lgEvent::setAbsDur(long int ms)
{
	mIsAbsDur = true;
	setDuration(lgDuration(ms,1),0);
}
