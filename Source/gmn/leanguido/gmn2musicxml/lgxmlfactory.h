/*
 *  lgxmlfactory.h
 *  MusicXMLleanguido
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#ifndef __lgxmlfactory_h__
#define __lgxmlfactory_h__

#include "../lgfactory.h"

/// factory class for MusicXMLlgTypes
class xmlFactory : public lgFactory {
public: 
	xmlFactory( void ){};


	virtual lgSequence *newSequence(long int posNum,
		                    long int posDenom);
	virtual lgVoice *newVoice(long int posNum,
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

	virtual lgTag *newTag(long int tagno,
              lgEvent *pEv,
              const char *tagName);
};
#endif