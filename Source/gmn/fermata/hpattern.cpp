/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*-------------------------------------------------------------------
	filename: hpattern.cpp
	author:   juergen kilian
	date:     1998-2001
	Implementation of THPattern
---------------------------------------------------------------------*/

#ifdef skdjfhksjdfh
#include "hpattern.h"

THPattern::THPattern( void)
	: TPATTERN()
{
	IOIList = NULL;
	TIOIratioList = NULL;
}

THPattern::~THPattern( void )
{
	if( IOIList )
		delete IOIList;
	if( TIOIratioList )
		delete TIOIratioList;
}


TQNOTE  *THPattern::Set( TQNOTE *ptr,
					  TFrac  *diff,
					  int   ppq,
					  TFrac  *endLastMatch )

{
// todo purpose of THPattern::Set?
	ptr->GetNext(ptr->GetVoice()); 
	return TPATTERN::Set(ptr,diff,ppq,endLastMatch);
}

TFeatures THPattern::Features( void )
{
	return GetFeatures( dynamic_cast<TNOTE*>(firstNote()),NULL);
}

void THPattern::initIOI( void )
{
	IOIList = new TIOIList(FirstNote(),(lgEvent *)NULL);
	TIOIratioList = new TIOIratioList(IOIList);
}

/*
	scan buffer for pattern definition
*/
/*
TAbsTime   THPattern::Read( char *buffer )
{
	TAbsTime res;

	res = TPATTERN::Read(buffer);
	initIOI();
   return res;
}
*/
double THPattern::distance(TIOIList *list1,
						  int segStart,
						  int segEnd)
{
	return ::distance(IOIList,  // of current pattern
					  list1,
					  segStart,
					  segEnd);
}
double THPattern::distance(TIOIratioList *list1,
						  int segStart,
						  int segEnd)
{
	return ::distance(TIOIratioList, 
					  list1,
					  segStart,
					  segEnd);
}

double THPattern::distance(TIOIList *ioiList,
			   TIOIratioList *TIOIratioList,
					TCLICKTRACK * /*clicktrack*/,
					int segStart, // index in IOIlist
					int segEnd)  // index in IOIlist
/*
	compare pattern to segment of ioiList
*/
{ 
	double ioiDistance,
		  movementDistance;
	ioiDistance = distance(ioiList,
					       segStart,
						   segEnd );
	movementDistance = distance(TIOIratioList,
								segStart,
								segEnd-1); //???
//ToDo  evaluate clicktrack information ???
	return (ioiDistance+movementDistance) / 2;
} // distance



#endif
