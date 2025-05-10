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
 *  lgfactory.h
 *  leanguido
 *
 *  Created by JŸrgen Kilian on Thu Feb 06 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#ifndef __pat_factory_h__
#define __pat_factory_h__

#include "../leanguido/lgfactory.h"
class TPFILE;

/// factory class for TPATTERN
class TPatFactory : public lgFactory{
public: 
	TPatFactory( void )
	{
	};


	virtual lgSequence *newSequence(long int posNum,
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
	virtual lgNote *newNote(int pc);
/*
	virtual lgVoice *newVoice(long int posNum,
		              long int posDenom);

	/// overwrite this for replacing lgChord
	virtual lgChord *newChord(long int posNum,
		              long int posDenom);
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
              */
};
#endif
