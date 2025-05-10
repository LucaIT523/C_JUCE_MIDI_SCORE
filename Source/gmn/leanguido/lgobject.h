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
 *  lgobject.h
 *
 */

#ifndef __lgobject_h__
#define __lgobject_h__

#include <stdio.h>
#include <string>
using namespace std;

#include "duration.h"

/* Version History
	0.5beta	6/2003
	0.6beta	7/2003
	1.0beta 10/2003 nseparate ")" endrange tags
	1.1 2/2004 new tagArg functions, minor improvements for parsing
	1.2 4/2004 beautifying
*/

#define LG_VERSION_STR "1.2beta"

const char * lgGetVersion( void );
class lgVoice;
class stringVisitor;

//! a  GUIDO object with no additional specific info
class lgObject
{
    friend class lgEvent;
    friend class lgVoice;
    friend class lgSequence;
	friend class lgSegment;
    friend class lgTag;
	friend class lgChord;

protected:
    lgObject *nextI;
    virtual void setNext( lgObject *ptr );
    
public :
        void accept( stringVisitor v );
    
        lgObject *next( void )
		{ return nextI; };

    lgObject( void );
	virtual ~lgObject(){
	};

	/// get GUIDO string
	/// should be redefined in all derived classes
	virtual string toString( lgVoice *callingSeq = NULL )
	{	string s;
		callingSeq = NULL; // supress warning
		return s;};

    //! write in GUIDO Syntax
	virtual void write( FILE *out,
				        lgVoice * = NULL/*dummy sequence */ )
	{
		if( out )
			fprintf( out, "%s", toString().c_str() );
	};

	virtual lgDuration pos( void )
	{
		return lgDuration(0,1);
	};
};

#endif

