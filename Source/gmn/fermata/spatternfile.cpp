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
|	Filename : PATTERNFILE.CPP
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	  	 : 17.10.1996-98-00-03,2011
------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>

//typedef long int TAbsTime;
#include "funcs.h"   
#include <string>

#include "spatternfile.h"
#include "pat_factory.h"

//-----------------------------------------------------------------
TSPFILE::TSPFILE( const char *na ) : lgSegment( new TPatFactory() )
{
	name = "";
	if( na )
	{
		name = na;
	}
	CPatternP  = 0;
	cLengthI = 0;
} // TSPFILE
//-----------------------------------------------------------------
TSPFILE::~TSPFILE( void )
{
/* this will be done by ~lgSegment
    // delete pattern
    TSPATTERN *Ptr,
				 *NextTemp;

	Ptr = Head;
	while( Ptr )
	{
		NextTemp = Ptr->Next();
		delete Ptr;
		Ptr = NextTemp;
	}
	Head = NULL;
	Current = NULL;
  */  
    CPatternP = 0;
} //~TSPFILE
//-----------------------------------------------------------------

char TSPFILE::Read(  void )
{

	if( name == string("") )
		return 0;

	char res = 1;
	cout << "Parsing patternfile " << name << ",,,";
    if( parseFile( name ) < 0 )
	{
		res = 0;
		// parse error in last pattern		
		// delete last pattern!
		Printf("Last read pattern will be deleted!\n");
		TSPATTERN *prev = NULL, *cur;
		cur = firstPattern();
		while( cur && 
			   cur->next() )
		{
			prev = cur;
			cur = dynamic_cast<TSPATTERN *>(cur->next() );
		}
		if( prev )
			prev->setNext( NULL );
		else
			voicesI = NULL;

		delete cur;
		cout << "Parser error in patternfile " << name;
	}
	// set all prevI Ptrs
	TSPATTERN *temp = firstPattern();
	while(temp)
	{
		temp->setPrevPtrs();
		temp = dynamic_cast<TSPATTERN *>(temp->next());
	}
    Printf("finished\n");

		// check Length, todo this function is blabla!!
		if( firstPattern() )
		{
			LengthI = firstPattern()->GetLength();
			TSPATTERN *temp = firstPattern();
			cLengthI++;
			int id = 1;
			while( temp )
			{
				temp->initStatvals();
				temp->setID( id++ );

				// count the pattern
				CPatternP++;
				if( LengthI != temp->GetLength() )
					cLengthI++;

				temp = nextPattern();
			} // for all pattern
		}
		cout << "Read "<<  CPattern() << " pattern,\n";
/*
    } // if open
	else
	{
		Printf("Warning: Can't open patternfile: %s\n",name );
	}
*/

	// write changes back to file!!
	/*
	FILE *out;
	out = fopen("_mirtest.txt","wt");
	write(out);
	fclose(out);
*/
	return res;
} // TSPFILE::Read
//-----------------------------------------------------------------
//-----------------------------------------------------------------

#ifdef askjsdhkasjdhfk
//DEL TAbsTime TSPFILE::Add( char *buffer,
//DEL 					  long int  num,
//DEL 					  long int denom )
//DEL /*
//DEL 	Parse buffer and add pattern to list
//DEL 	result :
//DEL 		patternindex : ok
//DEL 		-1 : error
//DEL */
//DEL {
//DEL 	TSPATTERN *NewPattern,
//DEL 				*Temp = NULL,
//DEL 				*Next;
//DEL 	TAbsTime length;
//DEL 
//DEL 	int erg,
//DEL 		 i;
//DEL 
//DEL //Printf("TSPFile::Add\n");
//DEL 
//DEL 	TAbsTime	 RetValue;
//DEL 
//DEL 	erg = 0;
//DEL 
//DEL 	if( num < 1 )
//DEL 	{
//DEL 		length = -1;   // don't use
//DEL 	}
//DEL 	else
//DEL 	{
//DEL //			length = Frac2Duration( num, denom, Ppq);
//DEL 			length = TFrac( num, denom );
//DEL 	}
//DEL 
//DEL 	if( CPatternP )
//DEL 	{
//DEL 		Temp = Head;
//DEL 		Next = Temp->Next();
//DEL 		i = 1;
//DEL 		// search for end of list
//DEL 		while( Temp && (i < CPatternP) )
//DEL 		{
//DEL 			Temp = Next;
//DEL 			Next = Temp->Next();
//DEL 			i++;
//DEL 		} // while
//DEL 		// Temp == last pattern of list
//DEL 	} // if Listenicht leer
//DEL 
//DEL //	NewPattern = new TSPATTERN();
//DEL 	NewPattern = factory();
//DEL 	RetValue = NewPattern->Read( buffer  /*, Ppq */);
//DEL 	if(  RetValue.toLong() < 0 )
//DEL 	{
//DEL 		erg = -1;
//DEL 	} // if
//DEL 	else if( (length.toLong() >=0) &&
//DEL 			!(RetValue == length) )	// lentgh ok?
//DEL 	{
//DEL 		erg = -1;
//DEL 	} // else if
//DEL 	else   	// alles OK
//DEL 	{
//DEL 		if( CPatternP )
//DEL 		{
//DEL 			Temp->SetNext( NewPattern );
//DEL 		}
//DEL 		else
//DEL 		{
//DEL 			Head = NewPattern;
//DEL 		}
//DEL 		erg = CPatternP;
//DEL 		NewPattern->SetNr( CPatternP );
//DEL 		CPatternP ++;
//DEL 	} // else
//DEL 
//DEL //	Printf("TSPFile::Add-End\n");
//DEL 	if( erg == -1 )	// Fehler ?
//DEL 	{
//DEL 		delete NewPattern;
//DEL 		return -1;	// error
//DEL 	}
//DEL 	else
//DEL 		return CPatternP;	// new index
//DEL } // Add
#endif
//-----------------------------------------------------------------
#ifdef aslkdjlaksjlks
/*!
	modify an existing pattern(index),  buffer will be parsed.
	if new length != oldLength an error occurs
	result:
		index : ok
		-1 : error
*/
//DEL int TSPFILE::Change( char *buffer,
//DEL                      long int  num,
//DEL                      long int denom,
//DEL                      int  index  )	// Index of Pattern
//DEL {
//DEL 
//DEL 	TSPATTERN *Temp,
//DEL 				*Prev,
//DEL 				*Next,
//DEL 				*NewPattern;
//DEL 	int i;
//DEL 	TAbsTime length;
//DEL 	if( num < 1 )
//DEL 	{
//DEL 		length = -1;   // don't use
//DEL 	}
//DEL 	else
//DEL 	{
//DEL //			length = Frac2Duration( num, denom, Ppq);
//DEL 			length = TFrac( num, denom );
//DEL 	}
//DEL 
//DEL 
//DEL 	// check if pattern exists?
//DEL 	if( (index < 0)  || (index >= CPatternP) )
//DEL 				return -1;
//DEL 	else
//DEL 	{
//DEL //		NewPattern = new TSPATTERN();
//DEL 		NewPattern = factory();
//DEL 		NewPattern->Read( buffer /*, Ppq*/ );
//DEL 		if( (length.toLong() >=0) &&
//DEL 			!(NewPattern->GetLength() == length) )	// length ok?
//DEL 		{
//DEL 			delete NewPattern;
//DEL 			return -1;
//DEL 		}
//DEL 		i = 0;
//DEL 		Prev = 0;
//DEL 		Temp = Head;
//DEL 		// search for pattern
//DEL 		while( i < index )
//DEL 		{
//DEL 			Prev = Temp;
//DEL 			Temp = Temp->Next();
//DEL 			i++;
//DEL 		} // while
//DEL 		Next = Temp->Next();
//DEL 		// insert into list
//DEL 		if( !Prev )
//DEL 		{
//DEL 			Head = NewPattern;
//DEL 		}
//DEL 		else
//DEL 		{
//DEL 			Prev->SetNext( NewPattern );
//DEL 		}
//DEL 		NewPattern->SetNr( index );
//DEL 		NewPattern->SetNext( Next );
//DEL 		// delete old pattern
//DEL 		delete Temp;
//DEL 	} // else
//DEL 	return index;
//DEL } // Set
#endif
//-----------------------------------------------------------------
#ifdef delPatternsmfnbsmdnfb
int TSPFILE::Del( int index )
/*
	delete pattern at position index
	result : 
		nr of Patterns : ok
		-1 : error
*/
{
		TSPATTERN *Prev,
					*Search,
					*Next;

		int i;
		// Range Check
		if( (index < 0)  || (index >= CPatternP) )
				return -1;
		else
		{
			//  init
			Prev   = 0;
			Search = firstPattern();
			Next   = Search->Next();
			i = 0;
			// search in list
			while( i < index )
			{
				Prev   = Search;
				Search = Search->Next();
				i++;
			} // while
			CPatternP--;
			Next   = Search->Next();

			// remove from list
			if( Prev )
				Prev->SetNext( Next );
			else
				Head = Next;

			delete Search;
			// change indexes of following patterns
			while( Next )
			{
				Next->SetNr( i );
				Next = Next->Next();
				i++;
			} // while
		} // else
		return 1;
} // Del
#endif
//-----------------------------------------------------------------
/// get pattern by id
TSPATTERN *TSPFILE::get( int i )
{
		if( i < 0  )
			return NULL;
		TSPATTERN *res;
		res = firstPattern();

		while( res &&
		       res->GetID() != i) 
		{
            res = nextPattern();
		}
		if( res && 
		    res->GetID() != i )
		    res = NULL;
		    
		return res;
}
/// count the pattern or pattern with patternlength = length
int TSPFILE::CPattern( TFrac *length )		
{
	int res = 0;
	if( !length )
		return CPatternP;
	else
	{
		TSPATTERN *temp;
		temp = firstPattern();
		while( temp )
		{
			if( temp->GetLength() == *length )
				res++;
			temp = nextPattern();
		}
	}
	return res; 
}

//-----------------------------------------------------------------
//DEL TSPATTERN *TSPFILE::factory( void )
//DEL {
//DEL 	return new TSPATTERN();
//DEL }
//-----------------------------------------------------------------

void TSPFILE::write(FILE *out,
					lgVoice * )
{
    
    char mustClose = 0;

	if( !out )
	{
		if( name == string("") )
			return;
		out = fopen(name.c_str(), "wt");
		mustClose = 1;
	}
	if( !out )
		return;

	/// copy sigm and my to tags!

	/// call lgSegment write
	lgSegment::write(out);
	if( mustClose )
		fclose( out );
}
/*

lgTag *TSPFILE::newTag(  long int no,
                           char *name )
{
	char ignore = 0;
	if( !name )
		name = "";
	/// check name and see what to do
	if( !strcmp(name, "\\aPriori") )
	{
		/// allow
	}
	else if( !strcmp(name, "\\nv") )
	{
		/// allow
	}	
	else
	{
		/// ignore
	}
	if( !ignore )
	{
		return new lgTag(no, NULL, name);
	}

	return NULL;
}
*/
