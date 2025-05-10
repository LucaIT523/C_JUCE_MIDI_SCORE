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
 *  lgfactory.h
 *
 */

#ifndef __lgfactory_h__
#define __lgfactory_h__

class lgSequence;
class lgVoice;
class lgChord;
class lgNote;
class lgRest;
class lgEmpty;
class lgTag;
class lgEvent;

/// a factory class for  lgSequence, lgVoice, lgNote, lgEvent, lgChord, lgEmpty, lgRest, lgTag . 
class lgFactory{
public: 
	lgFactory( void ){};
    virtual ~lgFactory( void ){};

    /// overwrite this function for replacing lgVoice
	virtual lgVoice *newVoice(long int posNum,
		              long int posDenom);

    /// overwrite this function for replacing lgSequence
	virtual lgSequence *newSequence(long int posNum,
		                    long int posDenom);
		                    

	/// overwrite this for replacing lgChord
	virtual lgChord *newChord(long int posNum,
		              long int posDenom);
    
    /// overwrite this function for replacing lgNote
	virtual lgNote *newNote(int pc,
                    int oct,
                    int acc,
                    long int durN,
                    long int durD,
                    int dots,
                    long int durPosN,
		            long int durPosD );
	   /// overwrite this function for replacing lgNote
	virtual lgNote *newNote(int pc );
	/// overwrite this function for replacing lgEmpty
	virtual lgEmpty *newEmpty(long int durN,
                      long int durD,
                      int dots,
                      long int durPosN,
		              long int durPosD );

    /// overwrite this function for replacing lgRest
	virtual lgRest *newRest(long int durN,
                    long int durD,
	                     int dots,
                        long int durPosN,
                long int durPosD );

	/// overwrite this function for replacing lgTag
	virtual lgTag *newTag(long int tagno,
              lgEvent *pEv,
              const char *tagName);
};
#endif
