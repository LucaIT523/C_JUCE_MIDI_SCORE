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

/*----------------------------------------------------------------
	filename: k_array.cpp
	author:   Juergen Kilian
	date:     1998-2001, 2011

------------------------------------------------------------------*/
#include "k_array.h"
/*
#include "liste.h"
#include "c_note.h"
#include "q_note.h"
*/
TFeatures::TFeatures( void )
/*
	implements a feature vector
*/  
{


	/*
	CNotes = 0;
	CIOIs = 0;
	IOI_Rest = 0L;
	Length = 0L;
*/
// todo where is CFEATURES defined
	if(CFEATURES < 3)
	{
		printf("Error: CFeatures must be > 1!\n");
		exit(2);
	}
	valI = NULL;
	createValues(CFEATURES);
	int i;
	for(i=0;i<cFeaturesI;i++)
	{
		valI[i] = 0;
	}
}
void TFeatures::createValues( int c)
{
	if( c )
	{
		cFeaturesI = c;
		valI = new FeatureVal[cFeaturesI];
	}

}

TFeatures::TFeatures( TFeatures *ptr)
/*
	create a copy of *ptr
*/
{
	createValues( ptr->cFeatures() );
	int i;
	for(i=0;i<cFeaturesI;i++)
	{
		valI[i] = ptr->valI[i];
	}
}


TFeatures::~TFeatures( void )
{
	if( valI )
		delete [] valI;
	valI = NULL;
}

void TFeatures::Write( FILE *out )
{
	if( !out )
		return;
	/*
	fprintf(out,"#notes: %d, ",CNotes);
	fprintf(out,"#IOI-classes: %d, ",CIOIs);
	fprintf(out,"IOI/Rest: %f, ",(double)IOI_Rest.numerator()/(double)IOI_Rest.denominator());
	fprintf(out,"Length: %ld/%ld, ",Length.numerator(),Length.denominator());
	fprintf(out,"Av: %f, SSQ: %f\n",Average,SSQ);
	*/
	int i;
	for(i=0; i < cFeaturesI; i++)
	{
		fprintf(out, "%f, ",valI[i]);
	}
	fprintf(out,"\n");

}


TFeatures *GetFeatures( TNOTE *from,
						TNOTE *to)
/*
	create an array of feature vectors of note list [from,to)
	remarks:
		from -> to, must be a single voice!!
		return  value must be deleted!!!
*/
{
	TFeatures *result;

	result = new TFeatures();

	TNOTE *now, *next, *temp;
	TAbsTime holdSum(0,1);
	TAbsTime restSum(0,1);
	TAbsTime diff(0,1);

	// calculate CNotes, Rest/IOI
	now = from;
	while( now &&
			 now != to )
	{
		result->IncCNotes();

		holdSum += now->GetDuration();
		next = NOTE(now->GetNext(now->GetVoice()));
		if( next )
		{

			diff =  next->GetAbsTime() -
					  (now->GetAbsTime() + now->GetDuration());
			if( diff > 0L )
			{
				restSum += diff;
			}
		}
		now = next;
	}
	if( holdSum > 0L )
		result->SetIOI_Rest( restSum/holdSum);
	else
		result->SetIOI_Rest(0L);

//	result->SetLength(holdSum + restSum);


	// count # of different IOIs
	// count # of DeltaIOIS
	// works only on single voice lines!!
	if( result->CNotes() )
	{
		// init
		TFrac IOI(0,1), nextIOI(0,1), minIOI(0,1),maxIOI(0,1);
		int i;

		/*
		int *cIOIs
		cIOIs = new int[result.CNotes+1]; // -1: invalid, else = counter
		for( i = 0; i < result.CNotes+1; i++ )
			cIOIs[i] = 0;
		*/

		// calculate
		i = 0;
		now = from;
		temp = from;
		now = NOTE(now->GetNext(now->GetVoice()));

		TDoubleBinList IOIList(110,90,1);
		TDoubleBinList DeltaIOIList(110,90,1);

		minIOI = GetIOI( now );	// IOI | now -> next |
//							  temp);
		maxIOI = minIOI;

		// compare succeding IOIs
		while( i < result->CNotes() && now)
		{
			IOI = GetIOI( now );	// IOI | now -> next |
//							  temp);
			if( temp )
			{
				nextIOI = GetIOI( temp );	// IOI | now -> next |
//										temp->GetNext(temp->GetVoice()));
				DeltaIOIList.Add(IOI.toDouble() / nextIOI.toDouble());
			}
			if(IOI < minIOI)
				minIOI = IOI;

			if(IOI > maxIOI)
				maxIOI = IOI;
			IOIList.Add(IOI.toDouble());
			temp = now;
			now = NOTE(now->GetNext(now->GetVoice()));
			i++;
		} // while i < CNotes

		result->SetAverage( IOIList.Average());
		result->SetSSQ( IOIList.SSQ() );
		result->SetCIOIs( IOIList.Count());
		result->SetCDeltaIOIs(DeltaIOIList.Count());
		result->SetdMinMaxIOI( maxIOI.toDouble()/minIOI.toDouble());
	} // if CNotes
	return result;
}
//-----------------------------------------------------------------

TK_Array::TK_Array( void )
{
	CNotes = 0;

	OnsetSum = 0L;
	AvOnset = 0;
	SSQ = 0;
	IOTSum = 0L;
	AvIOT = 0;
	IOTSSQ = 0;
}
#ifdef kajshdkjads
TK_Array THPattern::K_Array( void )
{
	TK_Array res;
	TPNOTE *temp,
			  *temp2;

	TAbsTime restLength = 0;

	temp = FirstNote();
	temp2 = NULL;
	if( temp )
		restLength = GetLength() - temp->GetAbsTime();

	while( temp || restLength)
	{
	  if( temp )
	  {
		res.CNotes++;
		res.OnsetSum += temp->GetDuration();
		// calculate IOT values
		if(temp2)
		{
			res.IOTSum += (temp->GetAbsTime() -
								temp2->GetAbsTime());
		}
		restLength = GetLength() - temp->GetAbsTime();
		temp2 = temp;
		temp = NextNote();
	  }
	  else if( restLength ) // last run
	  {
		if( temp2 )
		{
			res.IOTSum += restLength;
		}
		restLength = 0;
	  }
	};

	if( res.CNotes )
	{
		res.AvOnset = res.OnsetSum / (TFrac)res.CNotes;
		res.AvIOT = res.IOTSum / (TFrac)res.CNotes;
	}

	double SQ,
			 SQ2;
	temp = FirstNote();
	temp2 = 0;
	if( temp )
		restLength = GetLength() - temp->GetAbsTime();
	while( temp || restLength )
	{
//		SQ = res.AvOnset - temp->GetDuration();
	  if( temp )
	  {
		SQ = res.AvOnset / temp->GetDuration();
		SQ *= SQ;

		res.SSQ += SQ;

		if( temp2 )
		{
			SQ2 = res.AvIOT /
						(temp->GetAbsTime() - temp2->GetAbsTime());
			SQ2 *= SQ2;
			res.IOTSSQ += SQ2;
		}
		restLength = GetLength() - temp->GetAbsTime();
		temp2 = temp;
		temp = NextNote();
	  }
	  else if( restLength ) // last run
	  {
		if( temp2 )
		{
			SQ2 = res.AvIOT / restLength;
			SQ2 *= SQ2;
			res.IOTSSQ += SQ2;
		}
		restLength = 0;
	  }

	};

	if( res.CNotes )
	{
		res.SSQ /= res.CNotes;
		res.IOTSSQ /= res.CNotes;
	}

	return res;



}
#endif
void Printf( TK_Array K_Array, FILE *out )
{
	if( !out )
		return;
	fprintf(out,"#notes: %d, SOnset: %ld, AvOnset: %f,    SSQ: %f\n",
				K_Array.CNotes,
				K_Array.OnsetSum.toLong(),
				K_Array.AvOnset,
				K_Array.SSQ );
	fprintf(out,"              SIOT: %ld,   AvIOT: %f, SSQIOT: %f\n",
				K_Array.IOTSum.toLong(),
				K_Array.AvIOT,
				K_Array.IOTSSQ );
}

#ifdef OLDCLICKNOTE_kjhkjh
 obsolete because CLICKNOTE->MusicalObject

TFrac GetIOI( TCLICKNOTE *ptr)
//					TNOTE *prev )
/*
	res: IOI : |ptr, ptr->Next|
		if ptr is end of list: compare durations, 
		return duration(ptr)
*/
{
	TFrac res(0,1);
	TCLICKNOTE *next, *prev;
	if( !ptr )
		return res;

	next = CLICKNOTE(ptr->GetNext(-1));
	prev = CLICKNOTE(ptr->GetPrev(-1));
	if( next )
	{
		res = next->Playtime()- ptr->Playtime();
	}

 // ptr is end of list, compare durations ???
	/*
		!!!!
		Clicktrack must include at the offset-time of
		the last note (end of duration)
	*/ 
	return res;
}

#endif

