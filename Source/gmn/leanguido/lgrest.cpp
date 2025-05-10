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
 *  lgrest.cpp
 *
 */

#include <string>
#include <sstream>

#include "lgrest.h"
#include "lgsequence.h"

lgRest::lgRest( long int durNum,
                long int durDenom,
                int cdots,
                long int posNum,
                long int posDenom) : lgEmpty(durNum, durDenom, cdots,
                                             posNum, posDenom)
{
}

string lgRest::toString( lgVoice *)

{
	string s = "_";
	s += lgEvent::toString();
	return s;
}
