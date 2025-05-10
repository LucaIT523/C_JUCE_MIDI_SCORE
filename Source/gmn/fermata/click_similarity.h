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

	Tool functions for similarity diagram and measurement

	3.12.2003 Juergen Kilian



*/

#if !defined ( __click_similariy_h__ )
#define __click_similarity_h__



#include <string>
#include <iostream>
using namespace std;


// #define SIMMATRIX_FNAME "_simmatrix.txt"


// #define SIMDIAGFNAME "_similarity.txt"



#define RELIOIHISTOGRAM "_ioiHistogram.txt"



class TCLICKNOTE; 
class TTRACK;
class lgNote;
class TNOTE;


/// calc similarity between two MusicalObjects
/// return: [-1...0]
double similarity(  TCLICKNOTE *current1,
                    TCLICKNOTE *current2);

double similarity(  TCLICKNOTE *current1,
                    int windowSize);


/// similarity of IOIratios
/// return [-1...0]
double relSimilarity( TCLICKNOTE *current1,
					  TCLICKNOTE *current2);



double relSimilarity( TCLICKNOTE *current1,
					  int windowSize);



/// create similarity Matrix and alignment 
void selfSimilarity( TCLICKNOTE *from, 
				     TCLICKNOTE *to,
							int windowSize,
							int stepSize,
							TTRACK *controlTrack,
							const char *filename
							);


#ifndef _stdcall
#define _stdcall
#endif

double _stdcall val(TCLICKNOTE *v1);
void writePitch( std::ostream &out, TCLICKNOTE *ptr);


#endif
