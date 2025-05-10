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
 *  lgfactory.cpp
 *
 */

#include "lgfactory.h"
#include "lgsegment.h" // all lg types


lgSequence *lgFactory::newSequence(long int posNum,
                        long int posDenom)
{
    return new lgSequence( posNum, posDenom);
}
lgVoice *lgFactory::newVoice(long int posNum,
                              long int posDenom)
{
        return new lgVoice( posNum,
                            posDenom );
};

/// overwrite this for replacing lgChord
lgChord *lgFactory::newChord(long int posNum,
                              long int posDenom)
{
        return new lgChord( posNum,
                            posDenom );
    };
    
    /// overwrite this function for replacing lgNote
lgNote *lgFactory::newNote(int pc,
                    int oct,
                    int acc,
                    long int durN,
                    long int durD,
                    int dots,
                    long int durPosN,
                    long int durPosD )
    {
        return new lgNote(pc,
                          oct,
                          acc,
                          durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
    };
lgNote *lgFactory::newNote(int pc )
    {
        return new lgNote(pc );
    };

    /// overwrite this function for replacing lgEmpty
lgEmpty *lgFactory::newEmpty(long int durN,
                             long int durD,
                             int dots,
                             long int durPosN,
                             long int durPosD )
    {
        return new lgEmpty(durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
    };

    /// overwrite this function for replacing lgRest
lgRest *lgFactory::newRest(long int durN,
                             long int durD,
                             int dots,
                             long int durPosN,
                             long int durPosD )
    {
        return new lgRest(durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
        
    };

lgTag *lgFactory::newTag(long int tagno,
			  lgEvent *pEv,
               const char *tagName)

{
    return new lgTag(tagno, pEv, tagName);
}
