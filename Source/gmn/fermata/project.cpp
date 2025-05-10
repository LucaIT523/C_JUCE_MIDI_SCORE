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

/*---------------------------------------------------------------
	filename: project.cpp
	author    Juergen Kilian
	date      1999-2001
	projection of duration and pitch
	based on ideas for OCR of Ichiro Fujinaga
---------------------------------------------------------------*/
#include "project.h"
#include "debug.h"
//--------------------------------------------------
TDoubleBinList *DurationProjection( TQNOTE *start,
									 TQNOTE *end )
/*
	project all durations in range [start,end) into
	a class list of different durations

	remarks:
		result must be deleted

*/
{
	TDoubleBinList *res;
	res = new TDoubleBinList(120,80,1);
	// Insert all durations into a list
	while( start &&
			 start != end )
	{
//		if( start->GetDuration() > 0)
//		if( !start->Ornament()
//		 &&  !start->Tempo()		)
	// todo use IOI instead of duration?
			res->Add( start->GetDuration().toDouble() );
		start = QNOTE(start->GetNext(start->GetVoice()));
	}
	return res;
}
//--------------------------------------------------
/*
	project all pitches in a class list in range [start,end)
	into a classlist
	remarks:
		-result must be deleted
*/
TDoubleBinList *PitchProjection( TQNOTE *start,
                         TQNOTE *end )
{
	TDoubleBinList *res;
	res = new TDoubleBinList(100,100,1);
	// Insert all pitchclasses into a list
	while( start &&
			 start != end )
	{
		res->Add( (double)start->GetMIDIPitch() );
		start = QNOTE(start->GetNext(start->GetVoice()));
	}
	return res;
}
//--------------------------------------------------
/*
void Projection( TQNOTE *start,
				 TQNOTE *end )
// 
//	testfunction for projection algorithm
//

{
#ifdef _DEBUG
	TDoubleBinList *DurationClasses,
			 *PitchClasses;

	DurationClasses = DurationProjection( start, end );
	PitchClasses    = PitchProjection( start, end );
	FILE *out;
	out = fopen("project.txt","wa");
	fprintf(out,"Durations\n");
	DurationClasses->write(out);
	fprintf(out,"Pitches\n");
	PitchClasses->write(out);
	fclose(out);
	delete DurationClasses;
	delete PitchClasses;
#endif
}
*/
//--------------------------------------------------

