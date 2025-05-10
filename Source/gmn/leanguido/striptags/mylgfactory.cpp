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

#include "striptags.h"


/// overwrite this for replacing lgChord
lgChord * myLgFactory::newChord(long int posNum,
                          long int posDenom)
{
    return new myLgChord( posNum,
                          posDenom );
};
