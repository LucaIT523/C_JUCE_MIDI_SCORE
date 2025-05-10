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

#if !defined (__hpattern_h__)
#define __hpattern_h__

#include <sstream>
#include "pattern.h"

#include "q_note.h"
#include "anvoice.h"
#include "k_array.h"

#ifdef skjfhj
class TK_Array
{
public:
	TK_Array( void );
	int CNotes;
	TAbsTime OnsetSum;
	double AvOnset;
	double SSQ;
	TAbsTime IOTSum;
	double AvIOT;
	double IOTSSQ;
};
#endif
#ifdef sdkfjhskdjfh
/// advanced version of TPATTERN
class THPattern : public TPATTERN
{
protected:
	TIOIList *IOIList;
	TIOIratioList *TIOIratioList;
	void initIOI( void );
public:
	THPattern( void );
	TFeatures Features( void );
	virtual ~THPattern( void );
/*
    virtual TAbsTime   Read( char *buffer);
*/
	virtual TQNOTE  *Set( TQNOTE *ptr,
			  TFrac  *diff,
			  int   ppq,
			  TFrac  *endLastMatch );
	double distance(TIOIList *list1,
				   int Start = 0, // index in list
				   int End = -1); // index in list
	double distance(TIOIratioList *list1,
				    int Start = 0, // index in list
					int End = -1); // index in list
	double distance(TIOIList *ioiList,
				   TIOIratioList *TIOIratioList,
					TCLICKTRACK *clicktrack,
					int segStart, // index in IOIlist
					int segEnd);  // index in IOIlist

   TIOIList *IOI( void ){return IOIList; };

};

class TCLICKTRACK;


#endif

/// toolclass for pattern based tempo detection
typedef struct{ int index;
TCLICKNOTE *click; } /// toolclass for pattern based tempo detection
TPatternList;

#endif
