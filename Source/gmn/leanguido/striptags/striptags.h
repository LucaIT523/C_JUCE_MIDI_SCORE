/*
 *  striptags.h
 *  striptags
 *
 * gmn striptag demo application using leanGUIDO
 *
 *  Created by JŸrgen Kilian on Fri Apr 04 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __striptags_h__
#define __striptags_h__

#include "../lgchord.h"
#include "../lgsegment.h"

#include "mylgfactory.h"

/// a new chord class with removeTags function
class myLgChord : public lgChord
{
public:
    myLgChord(   long int posNum,
                 long int posDenom) : lgChord( posNum, posDenom ){};
    /// remove a specific tag or all tags if tagName == NULL, if tagName == \tag all occurencies of \\tag AND \\tagBegin, \\tagEnd will be removed!
	int removeTags( char *tagName );
};

/// a new segment class based on myLgChord
class myLgSegment :  public lgSegment,  public myLgChord
{
public:
    myLgSegment( void ) :  lgSegment( new myLgFactory() ), myLgChord(0,0)
{};
        
};


#endif
