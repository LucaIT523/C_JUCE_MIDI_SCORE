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
 *  lgchord.cpp
 *
 */

#include "lgchord.h"
#include "lgsequence.h"



//--------------------- lgChord ----------------------------------
lgChord::lgChord(   long int posNum,
                  long int posDenom) : lgEvent(0,0,0,posNum, posDenom)
{
    voicesI = NULL;
	voicesTail = NULL;
}


lgChord::~lgChord( void )
{
    // delete all voices
    lgObject *cur = voicesI;
    while( cur )
    {
        lgObject *n = cur->next();
        delete cur;
        cur = n;
    } 
}

int lgChord::splitTagRanges( void )
{
	int res = 0;
    lgVoice *temp = firstVoice();
    while(temp)
    {
        res += temp->splitTagRanges(  );
        temp = dynamic_cast<lgVoice *>(temp->next());
    }
    return res;
}

string lgChord::toString( lgVoice *callSeq)
{

	string res;
    if( !firstVoice() ||
        (!firstVoice()->firstEvent() &
         !firstVoice()->firstTag()) )
        return res;
    
    res ="{";


    lgVoice *cur = firstVoice();
    int c = 0;
    while( cur )
    {
        // write "," before each real note
		if( c )
		{
            if( cur->firstEvent() ||
                cur->firstTag() )
                res += ", ";
			// new line for each sequence
			if( dynamic_cast<lgSequence *>(cur) )
				res += "\n";
		} // if
		res += cur->toString(callSeq);
        c++;
        cur = dynamic_cast<lgVoice *>(cur->next());
    } // while

    res += "}";
	res += " ";
	//! write all tags between this and next
	//! callSeq might be NULL for Segements
	/*
	if( callSeq )
		res += callSeq->tagsToString( this );
	*/
	return res;
}
void lgChord::write( FILE *out,
                    lgVoice *callSeq)
{
	fprintf( out, "%s", toString(callSeq).c_str() );
}


//! append voice to chord, 
//! voicesTail must be up to date!
void lgChord::appendVoice(lgVoice *v)
{
	// no head -> not tail
    if( !voicesI )
	{
		voicesI = v;
	}
	else
	{
		voicesTail->setNext( v );
    }
	voicesTail = v;
    v->parent = this;
	v->posI = posI;
}


//! search for tag in tag list of all voices
lgTag * lgChord::findTag( long int id )
{
	lgTag *res = NULL;

	lgVoice *temp = firstVoice();
	while( temp && 
			!res)
	{
		res = temp->findTag( id );
		temp = dynamic_cast<lgVoice *>(temp ->next());
	}
	return res;
}

lgTag * lgChord::findTag( const char *tagName)
{
	lgTag *res = NULL;

	lgVoice *temp = firstVoice();
	while( temp && 
			!res)
	{
		res = temp->findTag( tagName );
		temp = dynamic_cast<lgVoice *>(temp ->next());
	}
	return res;
}


//! append event in last opened voice
lgEvent  *lgChord::appendEvent( lgEvent *ev )
{
	lgVoice *temp = voicesTail;
	if( temp )
	{
		return temp->appendEvent(ev);
	}
	else
	{
		// error, because initVoice was not called before!
	}
	return ev;
}


void lgChord::replaceRangePtr(lgEvent *oldEv, lgEvent *newEv)
{
    lgVoice *temp = firstVoice();
    while(temp)
    {
		temp->replaceRangePtr(oldEv, newEv );
        temp = dynamic_cast<lgVoice *>(temp->next());
    } // while
}
