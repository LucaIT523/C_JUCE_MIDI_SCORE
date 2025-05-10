/*
 *  lgfactory.cpp
 *  leanguido
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#include "mylgfactory.h"

#include "../lgsegment.h" // all lg types

#include "gmntransp.h"



/// overwrite this for replacing lgChord
lgChord * myLgFactory::newChord(long int posNum,
                          long int posDenom)
{
    return new myLgChord( posNum,
                          posDenom );
};
/// overwrite this function for replacing lgNote
lgNote * myLgFactory::newNote(int pc,
                        int oct,
                        int acc,
                        long int durN,
                        long int durD,
                        int dots,
                        long int durPosN,
                        long int durPosD )
{
    return new myLgNote(pc,
                        oct,
                        acc,
                        durN,
                        durD,
                        dots,
                        durPosN,
                        durPosD );
};


