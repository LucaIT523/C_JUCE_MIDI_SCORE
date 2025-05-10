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

	24.10.2002 Juergen Kilian



*/

#if !defined ( __similariy_h__ )

#define __similarity_h__



#include <string>
using namespace std;
#include <sstream>


// #define SIMMATRIX_FNAME "_simmatrix.txt"
string SIMMATRIX_FNAME(const char *midifileName, const   char *postfix );

string SIMDIAGFNAME( const char *postfix );
#define RELIOIHISTOGRAM "_ioiHistogram.txt"



class TMusicalObject; 
class TTRACK;
class lgNote;
class TNOTE;

/// calc similarity between two MusicalObjects
/// return: [-1...0]
double similarity(  TMusicalObject *current1,
                    TMusicalObject *current2);

double similarity(  TMusicalObject *current1,
                    int windowSize);


/// similarity of relIOIs
/// return [-1...0]
double relSimilarity( TMusicalObject *current1,
					  TMusicalObject *current2);



double relSimilarity( TMusicalObject *current1,
					  int windowSize);



/// create similarity Matrix and alignment 
void selfSimilarity( TMusicalObject *from, 
						    TMusicalObject *to,
							int windowSize,
							int stepSize,
							TTRACK *controlTrack,
							const char *midiFilename
							);

/// search for all seqeunces stored in gmnFilename
void retrieveSequence( TMusicalObject *from,  
                       TMusicalObject *to,
                       int windowSize,
                       int stepSize,
                       TTRACK *controlTrack,
                       const char *gmnFilename,
						const char *midiFilename
                       );

void createSimilarityDiag( TMusicalObject *from, 
						   TMusicalObject *to,
						   int windowSize,	/// 1..n
						   int stepSize, ///  1..n
							TTRACK *controlTrack
							); 

#ifndef _stdcall
#define _stdcall
#endif

double _stdcall val(lgNote *v1);
double _stdcall val(TNOTE *v1);
void writePitch( ostream &out, lgNote *ptr);
void writePitch( ostream &out, TNOTE *ptr);

#ifdef _DEBUG
double *createDebugMatrix( long entriesX,
						   long entriesY );
void debugMatrix( ostream &out,
				  double *alDebugMatrix,
				  long entriesX,
				  long entriesY );
				
#endif

#endif
