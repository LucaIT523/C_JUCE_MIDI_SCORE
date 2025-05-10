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
 *  lgVisitor.h
 *
 */

#ifndef __lgVisitor_h__
#define __lgVisitor_h__
#include <string>
using namespace  std;

class lgObject;
class lgEvent;
class lgVoice;
class lgTag;
class lgSequence;
class lgSegement;
class lgNote;
class lgChord;
class lgRest;
class lgEmpty;
/// prototype for a string visitor, not really used at the moment
class stringVisitor
{
public:
    string visit( lgObject *o );
    string visit( lgEvent *e );
    string visit( lgTag *t);

};



#endif
