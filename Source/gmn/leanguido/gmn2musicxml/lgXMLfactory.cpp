/*
 *  lgXMLfactory.cpp
 *  factory for MusicXMLleanGUIDO
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#include "gmn2musicxml.h"
#include "../lgsegment.h" // all lg types
#include "lgxmlfactory.h"

lgSequence * xmlFactory::newSequence(long int posNum,
                        long int posDenom)
{
    return new mXMLLgSequence( posNum, posDenom, this);
}
lgVoice *xmlFactory::newVoice(long int posNum,
                              long int posDenom)
{
        return new lgVoice( posNum,
                            posDenom );
};

/// overwrite this for replacing lgChord
lgChord *xmlFactory::newChord(long int posNum,
                              long int posDenom)
{
        return new mXMLLgChord( posNum,
                            posDenom );
    };
    
    /// overwrite this function for replacing lgNote
lgNote *xmlFactory::newNote(int pc,
                    int oct,
                    int acc,
                    long int durN,
                    long int durD,
                    int dots,
                    long int durPosN,
                    long int durPosD )
    {
        return new mXMLLgNote(pc,
                          oct,
                          acc,
                          durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
    };

    /// overwrite this function for replacing lgEmpty
lgEmpty *xmlFactory::newEmpty(long int durN,
                             long int durD,
                             int dots,
                             long int durPosN,
                             long int durPosD )
    {
        return new mXMLLgRest(durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
    };

    /// overwrite this function for replacing lgRest
lgRest *xmlFactory::newRest(long int durN,
                             long int durD,
                             int dots,
                             long int durPosN,
                             long int durPosD )
    {
        return new mXMLLgRest(durN,
                           durD,
                           dots,
                           durPosN,
                           durPosD );
        
    };

lgTag *xmlFactory::newTag(long int tagno,
              lgEvent *pEv,
                const char *tagName)

{
    return new mXMLLgTag(tagno, pEv, tagName);
}
