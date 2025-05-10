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
*  lgsequence.cpp
*
*/


#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif

#include "lgsequence.h"
#include "lgsegment.h"
#include <sstream>

//-----------------------------------------------------------------------------------------
lgSequence::lgSequence(long int posNum,
					   long int posDenom ) :lgVoice( posNum, posDenom ) 
{
//	setID(0);
}

/*
void lgSequence:: setID(int id)
{
	ID=id;
}
*/
/*
int lgSequence:: getID(void)
{
	return ID;
}
*/
string lgSequence::toString( lgVoice * )
{
    ostringstream res;
	res << "[";
	res << lgVoice::toString();
	res << "]";
	return res.str();
}


