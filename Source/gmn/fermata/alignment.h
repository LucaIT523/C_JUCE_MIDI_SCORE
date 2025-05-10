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

/*---------------------------------------
	alignment.h
	declaration of TAlignmentArray
	Juergen Kilian
	3/2003
---------------------------------------*/

#if !defined ( __alignment_h__ )
#define __alignment_h__
 
#include "note.h"

#include "../lib_src/similarity/dynaprog.h"
#include "pattern.h"

/*! help class for storing two alignment vectors
	The valid alignment can be found in
	v1L[len-1]...V1L[start]==V1R[start]...V1R[len-1]
	v2L[len-1]...V2L[start]==V2R[start]...V2R[len-1]
		
*/
/// help class for storing two alignment vectors
template<class T, class U>
class TAlignmentArray
{
protected:
	/// return the rhythmical complexity of the left alignment
	double lComplexity( void );
	/// return the rhythmical complexity of the right alignment
	double rComplexity( void );

public:

	//! array of pointer
	T 	  **v1L,
		  **v1R;
    U 	  **v2L,
		  **v2R;

	/// return alignment as two line string
	string toString( void );

	int //! id of first valid entry in V1L
		startV1L,	
		//! id of first valid entry in V1R
		startV1R,	
		//! id of first valid entry in V2L
		startV2L,	
		//! id of first valid entry in V2R
		startV2R;	



	int //! size of array (including unused positions)
		lenV1R,		
		//! size of array (including unused positions)
		lenV2R,		
		//! size of array (including unused positions)
		lenV1L,		
		//! size of array (including unused positions)
		lenV2L;		

	/// return id position in real alignment (endLeft ...startLeft, startRight+1 ... endRight )
	T *v1( int id );
	/// return id position in real alignment (endLeft ...startLeft, startRight+1 ... endRight )
	U *v2( int id );

    int leftAl1Len( void );
    int leftAl2Len( void );

    int rightAl1Len( void );

    int rightAl2Len( void );

	int leftAlLen( void )
	{
        if( leftAl1Len () < leftAl2Len() )
            return leftAl1Len();
        return leftAl2Len();
	};

	int rightAlLen( void )
	{
        if( rightAl1Len () < rightAl2Len() )
            return rightAl1Len();
        return rightAl2Len();
    };


	/// return the real alignment length
	int alignmentLen( void )
	{
		return  leftAlLen() + rightAlLen() - 1;
	};

	
	void debug(ostream &out);
};

/*
template<class X, class Y>
TAlignmentArray<X, Y> getVal( int i );
*/
 
/// retrieve alignment between v1,v2
template<class T, class U>
/// retrieve alignment between v1,v2
TAlignmentArray<T, U > makeLRAlignment( /// array 1
										T **v1,	
										/// array 2
                                      U **v2,	
						 int lenV1,
						 int lenV2,
						 /// id of best hit position in v1
						 int bestHit1,	
						 /// id of best hit position in v2
						 int bestHit2,
						 /// delta search limit 0...-oo
						 double deltalimit,
						 /// absolute limit for stop 0...-oó
						 double absLimit,
						 double *processTime,
						  double ( *valDynaT)( T *v1),
						  double ( *valDynaV)( U *v1),
						  double (_stdcall * simDyna)( T *f1, U *f2, T *gap,
						    							  int cGaps,
						    							  int cLocalGaps,
						    							  int alLength )

						);
// instantiations
//template TAlignmentArray<TNOTE , TNOTE>;
//template TAlignmentArray<TNOTE , lgNote >;

#ifdef _DEBUG
/// add retrieved best path to an alignment matrix for visualisation
template<class T, class U>
void addToDebugMatrix( double *alDebugMatrix,
					   TAlignmentArray<T, U> *align,
						int x,
						int y,
						long entriesX,
						long entriesY );
						
#endif

#endif
