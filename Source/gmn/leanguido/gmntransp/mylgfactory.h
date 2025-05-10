/*
 *  mylgfactory.h
 *  gmntransp
 *
 *	new lgFactory class for classes used in gmntransp demo application
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

/// factory class for myLgSegment, myLgChords,
class myLgFactory : public lgFactory{
public: 
	myLgFactory( void ){};

	/// creates and returns a myLgChord object
	virtual lgChord *newChord(long int posNum,
		              long int posDenom);
    
    /// creates and returns a myLgNote object
	virtual lgNote *newNote(int pc,
                    int oct,
                    int acc,
                    long int durN,
                    long int durD,
                    int dots,
                    long int durPosN,
		            long int durPosD );
};
#endif
