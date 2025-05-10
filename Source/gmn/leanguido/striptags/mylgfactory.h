/*
 *  mylgfactory.h
 *  striptags
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#ifndef __mylgfactory_h__
#define __mylgfactory_h__

#include "../lgfactory.h"

class lgSequence;
class lgVoice;
class lgChord;
class lgNote;
class lgRest;
class lgEmpty;
class lgTag;
class lgEvent;

/// a factory class for myLgChord
class myLgFactory : public lgFactory{
public: 
	myLgFactory( void ){};



	/// creates and returns a myLgChord object
	virtual lgChord *newChord(long int posNum,
		              long int posDenom);
    
};
#endif