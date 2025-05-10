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

/*
 *  lgfactory.cpp
 *  leanguido
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#include "pat_factory.h"

#include "hpatternfile.h"

lgSequence *TPatFactory::newSequence(long int /* posNum*/,
                        long int /*posDenom*/)
{
    return new TPATTERN();
}
    
    /// overwrite this function for replacing lgNote
lgNote *TPatFactory::newNote(int pc,
                    int oct,
                    int acc,
                    long int durN,
                    long int durD,
                    int dots,
                    long int durPosN,
                    long int durPosD )
    {
        return new TPNOTE(pc,
                          oct,
                          acc,
                          durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
    };
lgNote *TPatFactory::newNote(int pc )
    {
        return new TPNOTE(pc );
    };
/*
lgVoice *TPatFactory::newVoice(long int posNum,
                              long int posDenom)
{
        return new lgVoice( posNum,
                            posDenom );
};

/// overwrite this for replacing lgChord
lgChord *TPatFactory::newChord(long int posNum,
                              long int posDenom)
{
        return new lgChord( posNum,
                            posDenom );
    };

    /// overwrite this function for replacing lgEmpty
lgEmpty *TPatFactory::newEmpty(long int durN,
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
lgRest *TPatFactory::newRest(long int durN,
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

lgTag *TPatFactory::newTag(long int tagno,
			  lgEvent *pEv,
               const char *tagName)

{
    return new lgTag(tagno, pEv, tagName);
}
*/
