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

#if !defined (__hpatternfile_h__)
#define __hpatternfile_h__


#include <sstream>
#include <string>
using namespace std;

#include "patternfile.h"
#include "hpattern.h"

#include "q_note.h"
#include "anvoice.h"
#include "k_array.h"

#ifdef skdjfhksjdfhkjd

class TCLICKTRACK;


class THPFile : public TPFILE
{
public:
	int match( TIOIList *ioiList, TIOIratioList *TIOIratioList, TCLICKTRACK *clicktrack, int segStart, int segEnd);
	TPatternList * compare(TIOIList *ioiList, 
							TIOIratioList *TIOIratioList, 
							TCLICKTRACK *clicktrack, 
							int *listSize);
	THPFile( const char *name ) : TPFILE( name ){};

	virtual TPATTERN *Compare( TQNOTE *note, TFrac length );
	virtual TSPATTERN *factory( void );
};

#endif
#endif
