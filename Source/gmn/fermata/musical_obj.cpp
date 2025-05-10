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

/*------------------------------------------------------------------
|	filename : musical_obj.CPP
|	author     : Juergen Kilian
|	date	  : 2003
|	implementation of TMusicalObject
------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>

#include "musical_obj.h"
#include "note.h"
#include "funcs.h"
#include <string>

#include "meta_meter.h"

#include "midi.h"

#include <iostream>
using namespace std;

int operator > (  TMusicalObject &n1,  TMusicalObject &n2 )
{
   	TNOTE *tn1 = dynamic_cast< TNOTE *>(&n1);
	TNOTE *tn2 = dynamic_cast< TNOTE *>(&n2);
	if( tn1 && tn2 )
		return (*tn1) > (*tn2);
     
	// 1. criteria AbsTime
	if ( n1.GetAbsTime() > n2.GetAbsTime() )
		return 1;
	return 0;
}
int operator >= (  TMusicalObject &n1, TMusicalObject &n2 )
{
	TNOTE *tn1 = dynamic_cast<TNOTE *>(&n1);
	TNOTE *tn2 = dynamic_cast< TNOTE *>(&n2);
	if( tn1 && tn2)
		return (*tn1) >= (*tn2);
	// 1. criteria AbsTime
	if ( n1.GetAbsTime() >= n2.GetAbsTime() )
		return 1;
	return 0;
}

void TMusicalObject::Debug( FILE *out )
{
	fprintf(out,"AP: ");
	stringstream outStr;
	AbsTime.Write(outStr);
	fprintf(out, "%s", outStr.str().c_str());
	fprintf(out,"V: %d ",Voice);
}
//----------------------------------------------------------------------
TAbsTime TMusicalObject::GetAbsTime( void )
{
	return AbsTime;
} // GetAbsTime
//----------------------------------------------------------------------


/*
	result
		voice >=0 : Next note with Voice == voice
		voice < 0 : Next note
*/
TMusicalObject *TMusicalObject::GetNext( int voice )
{
	TMusicalObject *Temp = NextPtr;
	if( voice >= 0 ) // search in Voice
	{
		while( Temp &&
//	todo check:			 Temp->Voice >= 0 && // skip all voice notes
				 Temp->Voice != voice )
        {
            Temp = Temp->NextPtr;
        } // if
    }
    return Temp;
} // GetNext
//----------------------------------------------------------------------
/*
	result
	result
		voice >=0 : Prev note with Voice == voice
		voice < 0 : Prev note
*/
TMusicalObject *TMusicalObject::GetPrev( int voice )
{
	TMusicalObject *Temp = Prev;
	if( voice >= 0 )
	{
		while( Temp &&
				 Temp->Voice >= 0 && // skip all voice notes
				 Temp->Voice != Voice )
        {
            Temp = Temp->Prev;
            // printf(".");fflush(stdout);
        }
    } // if
	return Temp;
} // GetNext
//----------------------------------------------------------------------
/*
	insert next in list
*/
void TMusicalObject::SetNext( TMusicalObject *next )
{
	NextPtr = next;
	if( NextPtr )
		NextPtr->SetPrev( this );

	// check the sort
	if( NextPtr &&
		 (AbsTime > NextPtr->AbsTime) )
	{
//		Printf( "SortError at %ld\n", AbsTime );
	}
} // SetNext
//-------------------------------------------------
/*
	insert prev in list
*/
void TMusicalObject::SetPrev( TMusicalObject *prev )
{
    if( prev == this )
    {
        std::cout << "ERROR: Circular linking!" << endl;
        exit(1);
    }
    Prev = prev;
	// check sort
	if( Prev &&
		 (AbsTime < Prev->AbsTime) )
	{
//		Printf( "SortError %ld\n", AbsTime );
	}
} // SetPrev
//----------------------------------------------------------------------

/*
	Convert all tick-timing information into noteface information
*/
void TMusicalObject::ticksToNoteface( int ppq )
{
	TFrac resolution = TFrac( 1, ppq*4);
	// std::cout << "ticktime " << ppq << " " << AbsTime.toString() << endl;
	AbsTime *= resolution;

}

/*!
	res: IOI : |ptr, ptr->Next|
		if ptr is end of list: compare durations, 
		return duration(ptr)
*/
TFrac GetIOI( TMusicalObject *ptr)
{
	TFrac res(0,1);
	if( !ptr )
		return res;

	TMusicalObject *next = ptr->GetNext(ptr->GetVoice());
	TNOTE *notePtr = NOTE(ptr);
//	prev = ptr->GetPrev(ptr->GetVoice());
	if( next )
	{
		res = next->GetAbsTime() - ptr->GetAbsTime();
	}
	else if( notePtr ) // ptr is end of list, compare durations
	{

/*
		TNOTE *notePtr;
		notePtr = NOTE(ptr);

		if( notePtr )
		*/
			return notePtr->GetDuration();
			/*
		else
			return TFrac(-1,1); // invalid IOI
			*/
/* old code
		if( !prev )
			return ptr->GetDuration();
		IOI1 = prev->GetDuration();
		IOI2 = ptr->GetDuration();
		if( sameIOI( IOI1, IOI2 ) )
		{
			return GetIOI(prev);
//			res = ptr->GetAbsTime() - prev->GetAbsTime();
		}
		else	// return lgDuration( prev )
		{
			return ptr->GetDuration();
		}
*/
	} // else
	return res;
}

char sameIOI(const   TFrac &IOI1,
			  const  TFrac &IOI2 )
/*
	check if two IOIs are equal
*/
{
	TFrac upperLimit( 11,10 );
	TFrac lowerLimit( 9,10);
// todo check limits in sameIOI
	TFrac rel = IOI1;
    rel /= IOI2;
	if( rel < upperLimit &&
		 rel > lowerLimit )
	{
		return 1;
	}

	return 0;
}

/*
	return relative IOI current/next
	if |next| < |current| result is negative

	if no next note -> return 0
*/
double GetIOIRatio(TMusicalObject *current)
{
	if(!current)
		return 0;
	double IOI1 = (double)GetIOI(current).toDouble();
	
	
	TMusicalObject *middle = NOTE(current->GetNext(current->GetVoice()));
	if(!middle) // no next note
	{
		return 0; // invalid
	}
	
	double IOI2 = (double)GetIOI(middle).toDouble();
	double res = 0;
	if(IOI1<=IOI2)
		res = IOI2/IOI1;
	else
		res = IOI1/IOI2 * -1;

	return res;
} // GetIOIRatio



/*
	compare two IOIratios
	return: e2/e1
	if e < 0 -> e' = 1/e
*/
double IOIDistance(double e1,double e2)
{
	double res = 1;

	if(e1 > 0 && e2 > 0 )
	{
		res = e2/e1;
	}
	else if(e1 < 0 && e2 < 0) // 1/e1,1/e2
	{
		res = e1/e2;
	}
	else if(e1 < 0) // 1/e1,e2
	{
		res = e1*e2*-1;
	}
	else if( e2 < 0) // e1,1/e2
	{
		res = 1/(e1*e2)*-1;
	}
	return res;
}

/*!
return:
 >= 0: IOI between this and this+pos
 < 0:  error, IOI can't be calculated
	remarks:
 pos = 0 and pos = 1 return the same result
 */
TAbsTime TMusicalObject::ioi(int pos, int voice)
{
    TAbsTime res = -1l;
	int dir = pos;

    TMusicalObject *temp = NULL;
    if( pos >= 0 ) // search in next
    {
        temp = GetNext(voice);
        pos--;
        while(pos > 0 && temp) // skip some notes
        {
            temp = temp->GetNext(voice);
            pos--;
        }
    }
    else if( pos < 0 ) // search in prev
    {
        temp = GetPrev(voice);
        pos++;
        while(pos < 0 && temp) // skip some notes
        {
            temp = temp->GetPrev(voice);
            pos++;
        }
    }
    // caluclate distance
    if( temp && dir >= 0)
    {
        res = temp->GetAbsTime() - GetAbsTime();
    }
    else if( temp && dir < 0 )
    {
        res = GetAbsTime() - temp->GetAbsTime() ;
    }
    return res;
}
/*
	returns relational IOI  between this+prevPos:this:this+nextPos
	result:
 >= 1 -> ioiCN/ioiPC
 < -1 -> -ioiPC/ioiCN
 [-1..1) is not defined!!
*/
double TMusicalObject::IOIratio(int prevPos, int nextPos, int voice)
{
	if( nextPos > 0 && 
		prevPos > 0  )
	{
		if( GetNext(voice ) )
		{
			return GetNext(voice)->IOIratio(prevPos-1, nextPos-1,voice);
		}
		else
		{
			std::cout << "ERROR: Can't calc IOIratio" << endl;
		}
	}
	else if( nextPos < 0 && 
		prevPos < 0 )
	{
		if( GetPrev(voice ) )
		{
			return GetPrev(voice)->IOIratio(prevPos+1, nextPos+1,voice);
		}
		else
		{
			Printf("ERROR: Can't calc IOIratio\n");
		}
	}


    if( nextPos == 0 )
	{
		nextPos = 1;
	}
	if( prevPos == 0 )
	{
		prevPos = -1;
	}

    if( nextPos - prevPos < 2 )
    {
        std::cout << "ERROR: IOIratio: distance between prevPos and nextPos must be >= 2" << endl;
    }
    else
    {
        double ioiPC = ioi(prevPos,voice).toDouble() ;
        double ioiCN = ioi(nextPos, voice).toDouble();
        double res = ::IOIratio( ioiPC, ioiCN);
		return res;
    }
    return 0;
}

/*
 >= 1 -> ioiCN/ioiPC
 < -1 -> -ioiPC/ioiCN
 [-1..1) is not defined!!  == error
*/
double IOIratio( double ioiPC, double ioiCN)
{
        if( ioiPC > 0l &&
            ioiCN > 0l )
        {        	
		    double res = 0;
            if( ioiCN >= ioiPC )
            {
                res = ioiCN/ioiPC;
            }
            else
            {
                res = -ioiPC/ioiCN;
            }
            return res;
        }
        else
        {
//            Printf("WARNING: can't calc IOIratio\n");
        }
        return 0;
}

TMusicalObject::TMusicalObject(const TMusicalObject &ptr)
{
	AbsTime = ptr.AbsTime;
	NextPtr = NULL;
	Prev = NULL;
	Voice = ptr.Voice;
	mId = -1;
}


//------------------------------------------------------------
void TMusObjList::Debug( FILE *out )
{
	char mustClose = 0;
	if( !out )
	{
		mustClose = 1;
		out = fopen("_musObjList.txt","wt");
	}
	for(int  i = 0; i <= maxID; i++ )
	{
		TMusicalObject *temp = get(i);
		if( temp )
			temp->Debug( out );
		else
			fprintf(out, "NULL");

		fprintf(out, "\n");
	} // for

	if( mustClose )
	{
		fclose( out );
	}
}

/// sort list and keep stopList elements aligned
void TMusObjList::sort(TMusObjList *stopList)
{
	char swap = 0;
	do 
	{
		swap = 0;
		for(int  i = this->maxID; i > 0; i-- )
		{
			if( (get(i) &&
				 get(i-1) &&
				 get(i)->GetAbsTime() < get(i-1)->GetAbsTime()) ||
				 (get(i) && !get(i-1)) )
			{
				swap = 1;
				TMusicalObject *temp = get(i-1);
				set(i-1, get(i));
				set(i, temp);

				temp = stopList->get(i-1);
				stopList->set(i-1, stopList->get(i));
				stopList->set(i, temp);
			} // if
		} // for
	}
	while( swap );
}

int TMusObjList::getId(TMusicalObject *ptr)
{
	if( !ptr )
		return -1;
	for(int i = 0; i < count(); i++ )
	{
		if( get(i) == ptr )
			return i;
	} // for
	return -1;
}

void TMusObjList::append(TMusicalObject *ptr)
{
	set(count(), ptr );
}

double TMusicalObject::AbsTimeMS( TMIDIFILE *theMidifile)
{
	if( theMidifile->tickTiming &&
	     theMidifile->RecPPQ)
	{
		return DTIMEtoMS( theMidifile->RecPPQ,
						   theMidifile->Tempo(&AbsTime),
						   GetAbsTime().toDouble() );
	}
	else
	{
		// this is only correct, if there are no tempo changes!
		return DTIMEtoMS( 0,
						   theMidifile->Tempo(&AbsTime),
						   GetAbsTime().toDouble() );

	}
}
