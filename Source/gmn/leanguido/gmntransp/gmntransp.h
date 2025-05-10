/*
 *  gmntransp.h
 *  gmntransp
 *
 * Demo application for leanGUIDO
 *
 *  Created by Juergen Kilian on Fri Apr 04 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __gmntransp_h__
#define __gmntransp_h__

#include "../lgchord.h"
#include "../lgsegment.h"
#include "mylgfactory.h"

/// new chord class
class myLgChord : public lgChord
{
public:
    myLgChord(   long int posNum,
                 long int posDenom) : lgChord( posNum, posDenom ){};
    /// transpose function
    virtual void transpose( /// transpose interval
    						int deltaPC,
    						/// voice == 1...n;  -1,0 == all voices
    						int voice = -1);
};

/// new lgNote class
class myLgNote : public lgNote
{
public:
    myLgNote( //! pitchclass 0..11
              int pc,
              //!octave	-oo..+oo, 0 = c'
              int oct,
              //! # of accidentals -oo .. +oo
              int acc, 
              long int durNum,
              long int durDenom,
              int cdots,
              long int posNum,
              long int posDenom) : lgNote( pc,	
                                           oct, 
                                           acc, 
                                           durNum,
                                           durDenom,
                                           cdots,
                                           posNum,
                                           posDenom)
    {};
    /// chromatic transpose function
    virtual void transpose( int deltaPC );
};

/// a new lgSegment class, transpose function will be inherit from myLgChord
class myLgSegment :  public lgSegment,  public myLgChord
{
public:
    myLgSegment( void ) :  lgSegment( new myLgFactory() ), myLgChord(0,0)
{};

    
};


#endif
