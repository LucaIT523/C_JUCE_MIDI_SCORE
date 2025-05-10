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
 *  lgempty.h
 *  declaration of lgEmty for GUIDO empty notes
 *
 */



#ifndef __lgempty_h__
#define __lgempty_h__


#include "lgevent.h"

//! empty note based on lgNote see GUIDO spec for description
class lgEmpty : public lgEvent

{

    //! write "empty"
//    virtual void writeV( FILE *out ); 



public:
	virtual string toString( lgVoice *callingSeq = NULL);
    lgEmpty( long int durNum,
             long int durDenom,
             int cdots,
             long int posNum,
             long int posDenom);
};


#endif
