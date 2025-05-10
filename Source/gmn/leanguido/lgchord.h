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
 *  lgchord.h
 *
 */

#ifndef __lgchord_h__
#define __lgchord_h__

#include "lgevent.h"
#include "lgfactory.h"
//#include "lgvoice.h"


//! a chord based on lgNote, containing a list of lgVoice
class lgChord : public lgEvent
{

protected:
    lgVoice *voicesI,
            *voicesTail;
public: 
	/// replace  in all tags with prevEvI==oldEv, prevEv by newEv
	void replaceRangePtr( lgEvent *old, lgEvent *newEv);
//	    virtual void transpose( int deltaPC );
    lgChord(long int posNum,
            long int posDenom);
    virtual ~lgChord( void );

	/// append an event to voicesTail
    virtual lgEvent *appendEvent(lgEvent *ev);
    void appendVoice( lgVoice *voice   /// sequence or voice
                      );
    
    lgVoice *firstVoice( void )
    { return voicesI; };

    /// reset the closeRange stack for events/tags of all voices
//    virtual void resetStack( void );

	virtual string toString( lgVoice *callSeq );

    virtual void write( FILE *out,
                        lgVoice *callSeq);
                        
	/// search for a tag in all voices
	lgTag * findTag( const char *tagName );
		/// search for a tag in all voices
    virtual lgTag *findTag( long int id );
    
    /// split all explicit tag ranges (using "(....") ) into \...Begin and \...End and return number of changed ranges
    int splitTagRanges( void );
};


#endif
