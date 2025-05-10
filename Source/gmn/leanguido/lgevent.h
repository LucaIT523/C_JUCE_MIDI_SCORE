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
 *  lgevent.h
 *  declarations of the lgEvent  class 
 */



#ifndef __lgevent_h__
#define __lgevent_h__

#include <string>
using namespace std;

#include "lgobject.h"

#define lgFrac lgDuration

class lgNote;
//! A lgObject with duration and time-position
class lgEvent : public lgObject
{

	/// time position of event, events of a voice must not overlap each other!
    lgDuration posI;
    //! the "written" duration excluding dots (= duration of notehead)!
    lgDuration durationI;

    bool mIsAbsDur;
    /// the number of dots
    long int cDotsI;


    //! needs access to setNext
    friend class lgVoice;
    //! needs access to setNext
    friend class lgChord;	

protected:
    //! should be overwritten in derived classes
    //! write class spec values (pitch, _, ..)
    virtual void writeV(FILE *out)
	{
		if( out )
			fprintf( out, "%s", toString().c_str() );
	};	


    /** close rangeStack gives the number of ")" which should be
	    placed after "this" in the .gmn out.
        If any tag or note is added/removed to/from the tag/note list,
        closeRange needs to be updated -> see lgVoice
    */        
    // int closeRangeStack;	


public :
    lgEvent( long int durNum, long int durDenom, int cdots,
             long int posNum, long int posDenom );


	/// lgEvent returns only the duration as a string
	virtual string toString( lgVoice *callingSeq = NULL);

    //! duration of event including cDot info!
    virtual lgFrac duration( void );
    
    //! return only explicite written duration without dots, is zero for lgVoice!
    virtual lgFrac headDuration( void ) const
    { return durationI; };
    
    /// number of written dots
    int cDots( void ) const
    { return cDotsI; };
    void setDots(int n)
    { cDotsI = n; };
    /// set the duration
    virtual void setDuration( lgDuration dur, int dots = 0 )
    { durationI = dur, cDotsI = dots; };

    //! absolute time position.  needs to be updated if previous events were inserted or deleted!
    virtual lgFrac pos( void ) const
    { return posI; };
    
    /// be carefull that list is consistent!
    virtual void setPos( const lgFrac &pos )
    { posI = pos; }
    
    /// reset the ")" counter
    // virtual void resetStack( void )
    //{closeRangeStack = 0;};
    /// incrfement the ")" counter
    //virtual void pushRange( void )
    //{closeRangeStack++;};    
    /// return content of closeRangeStack
    //int rangeStack( void )
    //{return closeRangeStack; };
    
    /// write own data AND all tags starting in callSeq in (this->pos...next->pos]
    virtual void write( FILE *out,
                        lgVoice *callSeq );

    /// skip all rests, etc. 
    lgNote *nextNote( void );
    void setAbsDur(long int ms);
}; // lgEvent



#endif
