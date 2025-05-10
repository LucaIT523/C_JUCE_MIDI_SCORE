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
 *  lgrest.h
 *
 */



#ifndef __lgrest_h__
#define __lgrest_h__

 
#include "lgempty.h"



//! a GUIDO rest based on lgEmpty with Duration
class lgRest : public lgEmpty
{

//    virtual void writeV( FILE *out ); // write "_"



public:
	/// write "_*"Duration
	virtual string toString( lgVoice *callingSeq = NULL );
	lgRest();
    lgRest( long int durNnum,
            long int durDenom,
            int cdots,
            // position needs to be update for complete list,
            // when any events are beeing insereted/deleted inside list!
            long int posNum,	
            long int posDenom);
};


#endif
