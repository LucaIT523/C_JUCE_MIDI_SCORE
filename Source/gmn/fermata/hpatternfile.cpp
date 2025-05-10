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

/*-------------------------------------------------------------------
	filename: hpatternfile.cpp
	author:   juergen kilian
	date:     1998-2001
	Implementation of THPattern
---------------------------------------------------------------------*/

#include "hpatternfile.h"
#include "hpattern.h"


#ifdef skdjhksjhfkdj

TSPATTERN *THPFile::factory( void )
{
	return new THPattern();
}


/*!
	find closest pattern for note and following notes
 */
TPATTERN *THPFile::Compare( TQNOTE *note, TFrac )
{
	double //minIOIDist = 50,
		//	minAbleitungDist = 50,
			IOIDist,
			AbleitungDist;

	THPattern *temp,
				 *closestPattern = NULL;

	TIOIList *nIOI;
	TIOIratioList *nAbleitung;

	double absSum1, absSum2;

	FILE *out = NULL;
	#ifdef _DEBUG
		out = fopen("pcomp.txt","at");
	#endif

	temp = (THPattern *) firstPattern();
	while(temp)
	{
		nIOI = new TIOIList(note, temp->cNotes() );
		nAbleitung = new TIOIratioList(nIOI);

		AbleitungDist = temp->distance(nAbleitung);
		IOIDist = temp->distance(nIOI);
		// new minima?

		absSum1 = nIOI->absSum();
		absSum2 = temp->IOI()->absSum();
		if( absSum1 < absSum2 * 1.1 &&
			 absSum1 > absSum2 * 0.9 )
		{
				if( out )
				{
					fprintf(out, "%f,",nIOI->absSum() );
					nIOI->write(out);
					fprintf(out,"; ");
					fprintf(out, "%f,",nAbleitung->absSum() );
					nAbleitung->write(out);
					fprintf(out,"; ");
					temp->write(out);
					fprintf(out, "\n%f; ",temp->IOI()->absSum());
					fprintf(out,"ioiDist %f, ablDist %f\n ", IOIDist, AbleitungDist);
			}
			closestPattern = temp;
		}
		#ifdef jhjhghjgj
		if((!closestPattern && AbleitungDist < 0.1)||
			(AbleitungDist <= minAbleitungDist &&
			AbleitungDist < 0.1 )) // < 10$
		{

			if(!closestPattern ||
				IOIDist <= minIOIDist )
			{
				minAbleitungDist = AbleitungDist;
				minIOIDist = IOIDist;
				closestPattern = temp;
			}
		}
		#endif
		delete nIOI;
		delete nAbleitung;
		temp = (THPattern *) next();
	}
	if(out)
		fclose(out);
	return closestPattern;
}


/*!
	compare clicktrack to pattern pool
	return: list of matching pattern
 */
TPatternList * THPFile::compare(TIOIList *ioiList, 
								TIOIratioList *TIOIratioList, 
								TCLICKTRACK *clicktrack, 
								int *listSize)
{
	if( !ioiList ||
		!TIOIratioList ||
		!clicktrack )
		return NULL;

	int i = 0;

	int segStart = 0, // index
		segEnd   = 0;


	int bestPattern;

	*listSize = 0;

	TPatternList *res;
	TCLICKNOTE *startPtr;

	res = new TPatternList[ioiList->cEntries()+1];
	// do segmentation of ioiList/clicktrack (find anchors)
	
	startPtr = clicktrack->FirstNote();
	while( segEnd < ioiList->cEntries() )
	{
		// try pattern match for each segment
		while( segEnd < ioiList->cEntries() &&
			   !ioiList->isAnchor(segEnd) &&
			   segEnd - segStart < 3)
		{
			segEnd++;
		}
		// select best pattern and write into list
		bestPattern = match(ioiList,
						    TIOIratioList,
							clicktrack,
							segStart,
							segEnd);

		res[i].index = bestPattern;
		res[i].click = startPtr;

		segStart = segEnd+1;
		i++;
	} // while
	
	*listSize = i;
	return res;
}

/*!
	find best matching pattern in [segStart .. segEnd]
	return index of pattern, -1 if no match
 */
int THPFile::match(TIOIList *ioiList,
                   TIOIratioList *TIOIratioList, 
				   TCLICKTRACK *clicktrack, 
				   int segStart, 
				   int segEnd)
{
	double distance,
		  minDistance = 0;

	int res = -1; // defualt for no pattern

	THPattern *current,
			  *minPattern = NULL;

	current = (THPattern *)firstPattern();
	if( !current )
		return -1;
	
	
	while( current &&	// get first min Distance
		! minPattern )
	{
		if( current->cNotes() > 2 ) 	//dont'allow pattern size < 3
		{
			minDistance = current->distance(ioiList,
				TIOIratioList,
				clicktrack,
				segStart,
				segEnd);
			minPattern = current;
		}

        current = (THPattern *) nextPattern();
	} // while


	while(current) // search in complete list
	{

		if( current->cNotes() > 2 ) 	//dont'allow pattern size < 3
		{
///ToDo check distance functions,
			distance = current->distance(ioiList,
				TIOIratioList,
				clicktrack,
				segStart,
				segEnd);
			if( distance < minDistance )
			{
				minPattern = current;
				minDistance = distance;
			}
		}
        current = (THPattern *) nextPattern();
	} // while
#define DistanceLimit 1.5
/// ToDo check Limitsize
	if( minDistance > DistanceLimit )
		minPattern = NULL;
	if( minPattern )
		res = minPattern->GetID();
	return res;
} // match

#endif
