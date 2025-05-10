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

/* !!! this file must be included together with alignment.h in .cpp using
	the TAlignmentArray template!
	It MUST NOT be linked as regular obj/cpp file to a cpp project!
*/
#include <math.h>



#include "../lib_src/similarity/dynaprog.h"
#include "../lib_src/similarity/simMatrix.h" /// do this first for min

#include "alignment.h"

/*
#include "patternfile.h"

#include "similarity.h"


#include "note.h"
#include "funcs.h" // dtime2ms
#include "track.h" //TTRACK
#include "../leanguido/lgsegment.h"
#include "../lib_src/ini/ini.h"
*/



// instantiations
// MCP does not like this
// template CDynaprog<TNOTE*, lgNote *>;



#ifdef _DEBUG
template<class T, class U>
void addToDebugMatrix( double *alDebugMatrix,
					   TAlignmentArray<T, U> *align,
						int x,
						int y,
						long entriesX,
						long entriesY )
{
	if( entriesX <= 0 )
		return;
	if( entriesY <= 0 ) // diagonal
		entriesY = entriesX;
				// mark alignment
	int _x = x, 
		_y = y;
					
	// mark start position
	int j;				



	// mark right alignment
	int alen = min(align->lenV1R - align->startV1R,
					align->lenV2R - align->startV2R);
	for( j = 0; j < alen; j++ )
	{
		if( _x >= 0 && _x <= entriesX &&
		    _y >= 0 && _y <= entriesY )
		{
			if( j > 0 )
				alDebugMatrix[entriesX*_y+_x] *= 0.8;
			else
				alDebugMatrix[entriesX*_y+_x] *= 0;
		}
		else   
		{
			std::cout << "addDebugMatrix range error " <<
				_x << "," <<_y << endl;
			cout << "(" <<
				   entriesX << "," << entriesY << ")" << endl;
		} // else
		if( align->v1R[j+align->startV1R] )					
//					if( validIOIratio(val(align->v1R[j+align->startV1R])) )
			_x++;
		if (align->v2R[j+align->startV2R])
//					if( validIOIratio(val(align->v2R[j+align->startV2R])) )
			_y++;
	} // for
	
	_x = x-1; 
	_y = y-1;
	// makr left alignment
//				printf("start %d,%d\n",x,y);
	 alen = min(align->lenV1L - align->startV1L,
	          align->lenV2L - align->startV2L);				
	for( j = 0; j < alen; j++ )
	{
	//printf("%d,%d,%d\n",j,_x,_y);
		if( _x >= 0 && _x <= entriesX &&
		    _y >= 0 && _y <= entriesY )
		{
			if( j > 0 )
				alDebugMatrix[entriesX*_y+_x] *= 0.8;
			else
				alDebugMatrix[entriesX*_y+_x] *= 0;
		}
		else
		{
			std::cout << "addDebugMatrix range error " <<
				_x << "," <<_y << endl;
			cout << "(" <<
				   entriesX << "," << entriesY << ")" << endl;
		}

		if(align->v1L[j+align->startV1L])
//					if( validIOIratio(val(align->v1L[j+align->startV1L])) )
			_x--;
		if(align->v2L[j+align->startV2L])	
//					 if( validIOIratio(val(align->v2L[j+align->startV2L])) )
			_y--;
	} // for

}
#endif

/*!
	compare IOI's
	result = (0..1]
*/
//! retrieve alignment between v1,v2, start at bestHi1/bestHit2 in both directions
template<class T, class U>
TAlignmentArray<T,U> makeLRAlignment( T **v1,	// array 1
                                U **v2,	// array 2
						 int lenV1,
						 int lenV2,
						 int bestHit1,	// id of best hit position
						 int bestHit2,
						 double deltaLimit,	// delta search limit
						 double, // absLimit,
						 double *processTime,
						  double (_stdcall  *valDynaT)( T *v1),
						  double (_stdcall  *valDynaU)( U *v1),
						  double (_stdcall * simDyna)( T *f1, U *f2, T *gap,
						    							  int cGaps,
						    							  int cLocalGaps,
						    							  int alLength)
						)
{
/*
	int d1 = 0,	// current delta
		bD = 0; // bestdelta
	double bestSim = LC;
	/// search for the longest best matching note in range
	for( d1 = 0; d1 <  windowSize; d1++ )
	{
		double cSim = simDyna(v1[bestHit1 + d1],
			       v2[bestHit2 + d1],
				   NULL);
		if( cSim > bestSim )
		{
			bestSim = cSim;
			bD = d1;
		}
   		else if( cSim == bestSim &&
			     valDyna(v1[bestHit1 + d1]) > valDyna(v1[bestHit1 + bD])  )
		{
			bestSim = cSim;
			bD = d1;
		}
	} // for
*/

	int length1 = lenV1 - bestHit1;
	int length2 = lenV2 - bestHit2;
	if( (void *)v1 == (void *)v2 )
	{
		// we ar in a lower trianguilar matrix
		length1 = bestHit2-bestHit1;
	}
	// Problem: dynamic prog tends to insert gaps until a 1:1 match can be found!
	// start in middle of direct hit window
	CDynaprog<T *, U *> *dynaprogR = new CDynaprog<T *, U *>(v1+bestHit1,
										v2+bestHit2,
							length1, //lenV1-bestHit1,	// length of array = [bestHit...len]
							length2, //lenV2-bestHit2,
							NULL,		// gap val
							1,			// direction
							valDynaT,
			                valDynaU,
            			    simDyna);  
	// seed note is not part of bith alignments
	length1 = bestHit1;
	length2 = bestHit2;
	if( (void *)v1 == (void *)v2 )
	{
		// we ar in a lower trianguilar matrix
		length2 = bestHit2-bestHit1;
	}
	// start also with seed note
	CDynaprog<T *, U *> *dynaprogL = new CDynaprog<T *, U *>(v1+bestHit1-1,
									v2+bestHit2-1,
									length1,	// length of array = [0...bestHit]
									length2, // bestHit2,
									NULL,		// gap val
									-1,			// direction
									valDynaT,
							        valDynaU,
									simDyna);
	

	/// create the tables, stop if min distance >= limit
	double time = 0;
/*
	dynaprogR->makeTable(deltaLimit,
						 absLimit,
						 &time);

	*processTime = time;
	dynaprogL->makeTable(deltaLimit,
						 absLimit,
						 &time);
	(*processTime) += time;
*/
	dynaprogR->initTable();
	dynaprogL->initTable();
	*processTime = 0;
	int endR = 0,
		endL = 0;
	double bestOfPath = -100000000;  
	while(!endR || !endL)
	{
		if( !endR )
		{
			endR =  dynaprogR->incrementTable(bestOfPath,	// call by reference
										deltaLimit,
										&time);
			*processTime += time;
		}
		if( !endL )
		{
			endL = dynaprogL->incrementTable(bestOfPath,	// call by reference
										deltaLimit,
										&time);
			*processTime += time;
		}
	} // while
	
	dynaprogR->deleteTable();
	dynaprogL->deleteTable();

	

	/// retireve the alignment
	double quality = dynaprogR->retrieveAlignment(0);
	quality = dynaprogL->retrieveAlignment(0);
	#ifdef _DEBUG
	FILE *out = fopen("_dynaprog.txt","wt");
   dynaprogR->debug(out);
	dynaprogL->debug(out);
	fclose(out);
	#endif

	TAlignmentArray<T,U> res;
	/// copy result values
	res.lenV1L = dynaprogL->iI + dynaprogL->jI; //- dynaprogL->outStart;
	res.lenV2L = dynaprogL->iI + dynaprogL->jI;// - dynaprogL->outStart;
	
	res.lenV1R = dynaprogR->iI + dynaprogR->jI;// - dynaprogR->outStart;
	res.lenV2R = dynaprogR->iI + dynaprogR->jI;// - dynaprogR->outStart;
	
	res.startV1L = dynaprogL->outStart;
	res.startV2L = dynaprogL->outStart;
	res.startV1R = dynaprogR->outStart;
	res.startV2R = dynaprogR->outStart;
	
	res.v1L = dynaprogL->S1outFl;
	res.v2L = dynaprogL->S2outFl;
	res.v1R = dynaprogR->S1outFl;
	res.v2R = dynaprogR->S2outFl;
	
	// cut gaps at left and right end
	int i = res.lenV1L-1;
	while( i >= res.startV1L &&
          (res.v1L[i] == NULL ||
		   res.v2L[i] == NULL) )
	{
		i--;
		res.lenV1L--;
		res.lenV2L--;
	} // while

	i = res.lenV1R-1;
	while( i >= res.startV1R &&
           (res.v1R[i] == NULL ||
		    res.v2R[i] == NULL) )
	{
		i--;
		res.lenV1R--;
		res.lenV2R--;
	} // while

	delete dynaprogL;
	delete dynaprogR;
	return res;
}



/// return id position in real alignment (endLeft-1 ...startLeft, startRight+1 ... endRight-1 )
template<class T, class U>
T *TAlignmentArray<T,U>::v1( int id )
{
	int pos = -1;

	if( id < 0 )
	{
		return NULL;
	}
	if( id < leftAlLen() )
	{
		pos = lenV1L - id - 1;
		return v1L[pos];
	}
	else if( id < alignmentLen() )
	{
		pos = id-leftAlLen()+1 + startV1R;
		return v1R[pos];
	}
	else // ut of range
	{
		return NULL;
	}
}


/// return id position in real alignment (endLeft ...startLeft, startRight+1 ... endRight )
template<class T, class U>
U * TAlignmentArray<T,U>::v2( int id )
{
	int pos = -1;
	if( id < 0 )
	{
		return NULL;
	}
	if( id < leftAlLen() )
	{
		pos = lenV2L - id - 1;
		return v2L[pos];
	}
	else if( id < alignmentLen() )
	{
		pos = id-leftAlLen()+1 + startV2R;
		return v2R[pos];
	}
	else // ut of range
	{
		return NULL;
	}
	return v2L[pos];
}




template<class T, class U>
string TAlignmentArray<T,U>::toString( void )
{
	ostringstream	al1,
			al2;

	int j;
	string res;
	/// debug v1L[len-1]...v1L[start] 
	al1 << "<";
	for( j = lenV1L-1; j >= startV1L; j-- )
	{
		if( v1L[j] && v2L[j])
			al1 << "+"; /// entry
		else if( v1L[j] )
			al1 << "_";	/// gap
		else
			al1 << "-";	/// gap
	}
	al1 << ":";
	/// debug v1R[start]...v1R[len-1] 
	for( j = startV1R; j < lenV1R; j++ )
	{
		if( v1R[j] && v2R[j])
			al1 << "+"; /// entry
		else if( v1R[j] )
			al1 << "_";	/// gap
		else
			al1 << "-";	/// gap
	}
	al1 << ">";
/*
	al2 << "<";
	/// debug v2L[len-1]...v1L[start] 
	for( j = lenV2L-1; j >= startV2L; j-- )
	{
		if( v2L[j] )
			al2 << "+"; /// entry
		else
			al2 << "-"; /// entry
	}

	al2 << ":";
	/// debug v1R[start]...v1R[len-1] 
	for( j = startV2R; j < lenV2R; j++ )
	{
		if( v2R[j] )
			al2 << "+"; /// entry
		else
			al2 << "-"; /// entry
	}
	al2 << ">";
	res = al1.str() + al2.str();
*/
	res = al1.str();
	return res;
}

template<class T, class U>
int TAlignmentArray<T,U>::rightAl2Len( void )
{
    if( lenV2R <= 0 )
        return 0;
    return lenV2R - startV2R;
}


template<class T, class U>
int TAlignmentArray<T,U>::rightAl1Len( void )
{
    if( lenV1R  <= 0)
        return 0;
    return lenV1R - startV1R;
}

template<class T, class U>
int TAlignmentArray<T,U>::leftAl2Len( void )
{
    if( !lenV2L )
        return 0;
    return lenV2L -  startV2L;
}

template<class T, class U>
int TAlignmentArray<T,U>::leftAl1Len( void )
{
        if( lenV1L <= 0 )
            return 0;
        return lenV1L - startV1L;
}

template<class T, class U>
void TAlignmentArray<T,U>::debug( ostream &out )
{
	out << "(l="<< lenV1L-startV1L+lenV1R-startV1R <<","<<		
									lenV1L-startV1L <<","<< 
									lenV1R-startV1R<<")\n";
	/// debug v1L[len-1]...v1L[start] 
	int j;
	for( j = lenV1L-1; j >= startV1L; j-- )
	{
		out <<  val(v1L[j]);
		if( v1L[j] )
			writePitch(out, v1L[j]);
		else
			out << "   ";
		out << " ";
	}
	out <<":::";
	/// debug v1R[start]...v1R[len-1] 
	for( j = startV1R; j < lenV1R; j++ )
	{
		out << val(v1R[j]);
		if( v1R[j] )
                    writePitch(out, v1R[j]);
		else
			out << "   ";
		out << " ";
	}
	out <<">\n";
	/// debug v2L[len-1]...v1L[start] 
	for( j = lenV2L-1; j >= startV2L; j-- )
	{
		out <<  val(v2L[j]);
		if( v2L[j] )
                    writePitch(out, v2L[j]);
		else
			out << "   ";
		out << " ";
	}
	out << ":::";
	/// debug v1R[start]...v1R[len-1] 
	for( j = startV2R; j < lenV2R; j++ )
	{
		out << val(v2R[j]);
		if( v2R[j] )
                    writePitch(out, v2R[j]);
		else
			out << "   ";
		out << " ";
	}
	out << ">\n\n";	
}

