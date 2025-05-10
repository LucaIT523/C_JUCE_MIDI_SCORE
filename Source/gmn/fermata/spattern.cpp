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
|	Filename : PATTERN.CPP
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	  	 : 17.10.1996-98-00-03
------------------------------------------------------------------*/
#include "debug.h"
#include <string>
using namespace std; 
#include <sstream>
#include <stack>


#include "spattern.h"
#include "funcs.h"
#include "../leanguido/lgtag.h"
#include "q_note.h"

#include "anvoice.h"
#include "q_funcs.h"
//-----------------------------------------------------------------

//-----------------------------------------------------------------
 TPNOTE::TPNOTE( int pc,
				int oct, 
				int acc,
			long int durN,
			long int durD,
			int dots,
			long int posN,
			long int posD,
			char ty,
			double m,
			double s) : 
						lgNote( pc, /// pitch
								oct, /// oct
								acc, /// acc
								durN,
								durD,
								dots, 
								posN,
								posD)
{
	typeID   = ty;
	sigma2I   = s;
   	myI = m;
	devTag = NULL;
	prevI = NULL;
	IOIratioI = 0;
	this->IOIratioStdDevI2 = 0;
} // TPNOTE

						TPNOTE::TPNOTE( int pc,
			char ty,
			double m,
			double s) : 
						lgNote( pc)
{
	typeID   = ty;
	sigma2I   = s;
   	myI = m;
	devTag = NULL;
	prevI = NULL;
	IOIratioI = 0;
	this->IOIratioStdDevI2 = 0;
} // TPNOTE

//---------------------------------------------------------
TPNOTE  *TPNOTE::GetNext( void )			
{
	TPNOTE *temp = dynamic_cast<TPNOTE *>(nextNote());
	while( temp &&
		   temp->isRest() )
	{
        temp = dynamic_cast<TPNOTE *>(temp->nextNote());
	}
	return temp;
}

void TPNOTE::calcStatistics( double newN, TFrac attack )
{
	double mult = (newN-1)/newN;
	double delta = pos().toDouble() - attack.toDouble();
	
	myI = myI * mult + (delta/newN);
	
	sigma2I =   sigma2I * mult + (pow(delta - myI,2)/newN);
	// call set tag functions
	setAvOffset( myI );
	setStdDev( sigma2I );
}

//---------------------------------------------------------
//! write note into string
//! if attackpoint pattern > PrevEnd, a rest will be added
//! 	result:
//!	length [ticks]

string TPNOTE::toString( lgVoice *voice )
{
	if( !isRest() )
	{
		return lgNote::toString(voice);
	}
	else
	{
		string s = "_";
		s += lgEvent::toString(voice);
		return s;
//		fprintf(out,"_");
	}
}



#ifdef skjdfhksjdhf
string TPNOTE::toStr( TAbsTime prevEnd )
{
    /// numerator
    long int  Numerator;
    /// denominator
    long int	  Denominator;

    TAbsTime DiffToPrev;

	ostringstream res;

	DiffToPrev = AbsTime - prevEnd;
	if( DiffToPrev > 0L ) // insert rest
	{
			Numerator = DiffToPrev.numerator();
			Denominator = DiffToPrev.denominator();

			res<< "_*"; 

			res<< Numerator;
			res<< '/';
			res<< Denominator;
			res<< ' ';
	} // if

	Numerator = lgDuration.numerator();
	Denominator = lgDuration.denominator();

	res<< Numerator;
	res<< '/';
	res<< Denominator;
	res<< '(';
	res<< delta << ", ";
	res<< standardabweichung;
	res<< ')';
	res<< ' ';

	return res.str();
} // GetString
#endif
//---------------------------------------------------------
/*
	change the resolution
 */
/*
void TPNOTE::ChangePpq( int oldppq, int newppq )
{
	double mult;

	mult = (double)newppq / (double)oldppq;
	AbsTime  *= mult;
	lgDuration *= mult;
} // Change PPq
*/
//---------------------------------------------------------
//---------------------------------------------------------
TSPATTERN::TSPATTERN( void ) : // Length(0,1),
lgSequence(0,0)

{
}  // TSPATTERN
//-----------------------------------------------------------------
TSPATTERN::~TSPATTERN( void )
{
} //~TSPATTERN
//-----------------------------------------------------------------
/// return duration of lgSegment
TAbsTime TSPATTERN::GetLength( void  )
{
	return TAbsTime(duration().durN, 
					duration().durD);
}

/// return duration of lgSegment
void TSPATTERN::GetLength( long int &Num, long int &Denom)
{
    Num = lgSequence::duration().durN;
    Denom = lgSequence::duration().durD;
}

//-----------------------------------------------------------------

void 	TSPATTERN::setID( int id )
{
	lgTag *temp = findTag("\\statist");
	long int idVal = id;
	string idStr = string("id");
	if( temp )
	{
		lgIntTagArg *tagArg = dynamic_cast<lgIntTagArg *>(temp->findArg( "id" ));
		if( tagArg )
		{
			tagArg->setValInt( id );
		}
		else
		{
			tagArg = new lgIntTagArg( idStr, idVal );
			temp->addArg(tagArg);
		}
	}
	else
	{
		temp = new lgTag(0, this, "\\statist");
		lgIntTagArg *tagArg = new lgIntTagArg(idStr, idVal);
		temp->addArg(tagArg);
		this->insertTag(temp);
	}
}

/// return pattern ID
/// if no id set result = -1
int TSPATTERN::GetID( void )
{
	lgTag *temp = findTag("\\statist");
	if( temp )
	{
		lgIntTagArg *tagArg = dynamic_cast<lgIntTagArg *>(temp->findArg( "id" ));
		if( tagArg )
		{
			return tagArg->valInt();
		}
	} // if temp
	return -1;
}
#ifdef skldfkjshdf
TAbsTime TSPATTERN::Read( char *buffer ) // ,  int /* ppq */ 
{
	TPNOTE *Note,
			 *Prev = NULL;
	char
        /// numerator
        numerat[10],
        /// denominator
        denominat[10],
        /*! nb==0 -> read numerator
								nb=1 -> read denominator
         */
        nb = 0,
        /// Flag for rest
        Rest = 0;	

	char RetValue,
		  Ende = 0;

	int c,
        /// string position
        i = 0,	
		 z,
		 n;

	TAbsTime lgDuration,
		  AbsTime = 0L;


	numerat[0] = '1';
	numerat[1] = '0';

	/// regular sscanf
	RetValue = -1;
	CNote = 0;

	{
		int num, denom;
		double m,s;

		/// pattern specific data first
		if( sscanf(buffer, "< %f , %f >", &m, &s) == 2 ||
			sscanf(buffer, "< %f >", &m ) == 1 )
		{
			aPriori = m;
		}
		else
		{
			Printf("Parse error in patternfile!\n" );
			return -1;
		}
		// skip whitespace until first note
		while( *buffer != 0 &&
			   *buffer != '>' )
		{
			buffer++;
		}
		if( buffer != 0 )
			buffer++;
		
	
		// read all notes
		while( *buffer != 0 &&
			   *buffer != ';' && // remark
			   *buffer != '\n' )
		{
			// try to read a note
			if ( sscanf(buffer, "%d/%d ( %f , %f )", &num, &denom,
				 &m,&s) == 4 )
			{
				lgDuration = TAbsTime( num, denom);
				Note = new TPNOTE( AbsTime, lgDuration, m , s);
				CNote++;
				// insert into list
				if( !Head )
				{
					Head = Note;
					Prev = Head;
				}
				else if(Prev)
				{
					Prev->SetNext( Note );
					Prev = Prev->GetNext();
				}
				else
				{
					Printf("spattern.cpp: Prev == NULL!\n");
				}
				AbsTime = AbsTime + lgDuration;

			}
			else if ( sscanf(buffer, "_*%d/%d ", &num, &denom ) == 2 )
			{
				// ok, add rest
				lgDuration = TAbsTime( num, denom);
				AbsTime = AbsTime + lgDuration;
			}
			else // parse error
			{
				Printf("Parse error in patternfile!\n" );
				return -1;
			}

			// search for end of current note
			while( *buffer != 0 &&
				   *buffer != ')' &&
				   *buffer != '\n' )
			{
				buffer++;
			}
			if( *buffer == ')' )
			{
				// skip whitespace until next note
				buffer++;
				while( *buffer != 0 &&
					   *buffer != '\n' &&
					   (*buffer == ' ' ||
					    *buffer == '\t') )
				{
					buffer++;
				}
			}
		} // while buffer
		Length = AbsTime;

		return AbsTime; 	
	} // block

	do // read one line from file and parse into pattern
	{
		c = *buffer;
		exit(1);

		if( c != 0 ) // inc begin of buffer
			buffer++;

		if( c == ';' )	 // remark -> skip rest of line
		{
			while( (c != '\n') && (c != 0 ) )
			{
				c = *buffer;
				buffer++;
			} // while
			Ende = 1;
		} // if ( remark
		else if( isdigit( c ) ) // read numerator or denominator
		{
			RetValue = -1;
			if( !nb )
				numerat[i] = ( char )c;
			else
				denominat[i] = ( char )c;

			i++;
			if( i > 9 )   // string to long
				ErrorMsg( 8 );
		} // else if( isdigit )
		else if( (char)c == '/' )	// switch to denominator
		{
			if( nb )						// 2 times switched
				ErrorMsg( 8 );
			else
			{
				if( i )
					numerat[i] = 0;
				nb = 1;
				i  = 0;
			} // else
		} // else if
		else if( nb == 1 ) // first char after  denominator
		{
			// convert to int
			denominat[i] = 0;
			RetValue	= 1;

			z = saveAtoi( numerat );
			n = saveAtoi( denominat );

//			lgDuration = Frac2Duration( z, n, ppq );
			lgDuration = TAbsTime( z,n);

			if( !Rest )
			{
				Note = new TPNOTE( AbsTime, lgDuration);

				CNote++;

				// insert into list
				if( !Head )
				{
					Head = Note;
					Prev = Head;
				}
				else if(Prev)
				{
					Prev->SetNext( Note );
					Prev = Prev->GetNext();
				}
				else
				{
					Printf("spattern.cpp: Prev == NULL!\n");
				}
			} //if(!Rest )
			// reset again
			AbsTime = AbsTime + lgDuration;
			i     = 0;
			nb    = 0;
			Rest = 0;
			numerat[0] = '1';
			numerat[1] = '0';
		} // if( Rest )
		else if( c == '_' ) // rest detected
		{
			Rest = 1;
		}
		else	if( c == ' ' ||
					 c == '\t' ||
					 c == '*' ||
					 c == 0 ||
					 c == 13 ||
					 c == 10 ) // skip
		{

		}
		else
		{
			Printf("Syntax error in patternfile: %s", buffer);
		}
	} while( (c != '\n') && ( c != 0) && !Ende );
	if( i )	// note was not complete
		ErrorMsg( 8 );
	Length = AbsTime;

	if( RetValue == 1 )
		return AbsTime;  // == Length
	else
		return -1;	// error
}// TSPATTERN::Read
#endif
//-----------------------------------------------------------------
// returns Pattern as GUIDO* string,
#ifdef skjjhdfhkjshdfkj
string TSPATTERN::ToString( void )
{
    ostringstream res;
		TPNOTE *Temp;
		TAbsTime   PrevEnd = 0L;
		long Denominator;		// denominator

		long int Numerator;	// numerator
		TAbsTime TimeToEnd;

		// calculate buffer size
		Temp = FirstNote();
		while( Temp )
		{
			res<<  Temp->toStr(PrevEnd);
			PrevEnd = Temp->GetAbsTime() +
						 Temp->GetDuration();
			Temp    = NextNote();
		} // while

		TimeToEnd = Length - PrevEnd;
		if( TimeToEnd > 0L)	// rest at end of pattern
		{
			Numerator = TimeToEnd.numerator();
			Denominator = TimeToEnd.denominator();
            res<< '_';
            res<< '*';
			res<< Numerator;
			res<< '/';
			res<< Denominator;
			res<< ' ';
		} // if( rest at end
		return res.str();
} // GetString
//-----------------------------------------------------------------

//void TSPATTERN::write( FILE *out )
{
	string Buffer;
	Buffer = ToString();


	ostringstream t;
	t<<  '<';
	t<< aPriori;
	t<< "> ";
	fprintf(out, t.str().c_str() );

	fprintf(out, Buffer.c_str() );
}
#endif 
//-----------------------------------------------------------------
//-----------------------------------------------------------------

TAbsTime TPNOTE::GetAbsTime()
{
	return TAbsTime( pos().durN, pos().durD );
}

TAbsTime TPNOTE::GetDuration()
{
	return TAbsTime( duration().durN, pos().durD );
}


TPNOTE  *TPNOTE::GetPrev( void )
{
	TPNOTE *temp = prevI;
	while( temp &&
		    temp->isRest() )
	{
		temp = temp->prevI;
	}
	return(temp);
}

/*
void TPNOTE::setNext( lgObject *ptr )
{
	lgObject::setNext(ptr);
	TPNOTE *temp;
	temp = dynamic_cast<TPNOTE *>(ptr);
	if( temp )
	{
		temp->prev = this;
	}
}
*/

/// get IOI, skip rests
TAbsTime TPNOTE::IOI()
{

	/// Am I a rest?
	if( typeID == 0 )
	{
		Printf("ERROR: TPNOTE::ioi for rest\n!");
		return -1;
	}
	else
	{
		lgDuration p1 = pos();
		lgObject *pNext = NULL;
		lgObject *temp = next();
		while( temp &&
 				!dynamic_cast<TPNOTE *>(temp) )
		{
			pNext = temp;
			temp = temp->next();
		}
		if( temp )
		{
			lgDuration p2 = temp->pos();
			p1 = p2-p1;
			return TAbsTime(p1.durN, p1.durD );
		}
		else if( pNext &&
				 dynamic_cast<lgRest *>(pNext) )
		{
			// last pattern note was a rest!
			lgRest * rest =dynamic_cast<lgRest *>(pNext);
			lgDuration p2 = rest->pos() + rest->duration();
			p1 = p2-p1;
			return TAbsTime(p1.durN, p1.durD );
		}
				 
	}
	// this is the last pattern note -> use duration

	// Printf("ERROR: TPNOTE::ioi for rest\n!");
	return TAbsTime(duration().durN, duration().durD);
}

void TSPATTERN::initStatvals()
{
#define cParams 4
	const char *paramList[cParams] ={"stdDev", "avOffset", "IOIratio", "IOIrStdDev"};
	// look through all tags
	lgTag *temp = firstTag();
	while( temp )
	{
		if( temp->name() == "\\nv" )
		{
			for( int i = 0; i < cParams; i++ )
			{
				double val = -1;
				lgFloatTagArg *tagArg = dynamic_cast<lgFloatTagArg *>(temp->findArg(paramList[i]));
				lgIntTagArg *intTagArg = dynamic_cast<lgIntTagArg *>(temp->findArg(paramList[i]));
				{
					if( tagArg )
						val = tagArg->valFloat();
					else if( intTagArg )
					{
						val = intTagArg->valInt();
						temp->removeArg( intTagArg );
						string name = string(paramList[i]);
						tagArg = new lgFloatTagArg(name,val);
						temp->addArg(tagArg);
					}
					else // does not exist
					{
						switch(i)
						{
						case 0:
						case 3: val = 0; // stddev
							break;
						case 1: val = 0; // offset
							break;
						case 2: val = 0; // IOIRel
						} // switch
						string name = string(paramList[i]);
						tagArg = new lgFloatTagArg(name,val);
						temp->addArg(tagArg);
					}					
				} // block
				/// copy to all notes in range
				if( !temp->emptyRange() )
				{
					lgObject *curNote = temp->firstInRange();
					while( curNote &&
						   curNote != temp->lastInRange() )
					{
						TPNOTE *curPNote = dynamic_cast<TPNOTE *>(curNote);
						if( curPNote ) // don't applay to rests
						{	
							switch( i )
							{
							case 1 : curPNote->setAvOffset(val); break;
							case 0 : curPNote->setStdDev(val); 
								curPNote->setTag( temp ); break;
							case 2 : if( val == 0 )										
										curPNote->setIOIratio( curPNote->IOIratio() ); 
									  else
										curPNote->setIOIratio( val ); 
									break;

							case 3 : curPNote->setIOIratioStdDev( val ); break; 
							}
						}
						curNote = curNote->next();
					} // while notes
				} // if empty range
			} // for all params
		} // if \\nv
		temp = dynamic_cast<lgTag *>(temp->next());
	} // for all tags
	/// chek if all notes have entries!!


	/* store implicite by cUsed counter
	if( !findTag("\\aPriori" ) )
	{
		lgTag *newTag;
		newTag = new lgTag(0, prev, "\\aPriori");
		/// avOffset
		tagArg = new lgFloatTagArg("",0);
		newTag->addArg(tagArg);
		appendTag(newTag);
	}
	*/
	lgEvent *prev = this;
	lgEvent *cur = FirstNote();
	while( cur )
	{
		TPNOTE *curPNote = dynamic_cast<TPNOTE *>(cur);
		if( curPNote && // only for notess
			curPNote->stdDev() < 0 &&
			curPNote->avOffset() < 0 )
		{
			curPNote->setStdDev(0);
			curPNote->setAvOffset(0);

			lgTag *newTag;
			newTag = new lgTag(0, prev, "\\nv");

			/// avOffset
			string avOffst = string("avOffset");
			string un = string("");
			double zero = 0.0;
			lgFloatTagArg *tagArg = new lgFloatTagArg(avOffst, zero, un);
			newTag->addArg(tagArg);
			/// stdDev
			string sDev = string("stdDev");
			tagArg = new lgFloatTagArg(sDev, zero, un);
			newTag->addArg(tagArg);

			lgTag *endTag;
			endTag = new lgTag(0, curPNote, ")" );
			
			newTag->setRange(endTag);

			insertTag(newTag);
			insertTag(endTag);
		}
		prev = cur;
		cur = dynamic_cast<lgEvent*>(cur->next());
	} // while PNotes
}

/// should be called only from TPFILE!
double TSPATTERN::aPriori( double /* sum */)
{
	lgTag *temp = findTag("\\aPriori");
	if( temp )
	{
		lgFloatTagArg *tagArg;
		tagArg = dynamic_cast<lgFloatTagArg *>(temp->getArg(1));
		if( tagArg )
		{
			return tagArg->valFloat();
		}

	}
	return -1;
}


long int TSPATTERN::cNotes( void )
{

	lgObject *cur;
	long int res = 0;
	cur = this->FirstNote();
	while( cur )
	{
		res++;
		cur = NextNote();
	}
 	return res;
}

void TPNOTE::copyTo(TQNOTE *qnote)
{
	if( !qnote )
		return;

	qnote->shiftQAbstimeTo( TFrac(pos()), TFrac(pos()),1 );
	qnote->expandTo( TFrac(duration()),1 );

}

void TPNOTE::setTag(lgTag *tag)
{
	devTag = tag;
}


double TPNOTE::IOIratio()
{
	TPNOTE *temp;
	temp = GetPrev();

	double ioi1 = IOI().toDouble();
	if( temp )
	{
		double ioi2 = temp->IOI().toDouble();
return ::IOIratio(ioi2, ioi1);
	}
	return 0; // not available
}



void TPNOTE::setNext(lgObject *ptr)
{
	lgObject::setNext( ptr );
	TPNOTE *temp;
	temp = dynamic_cast<TPNOTE *>(ptr);
	if( temp )
	{
		temp->prevI = this;
	}
}

void TSPATTERN::setPrevPtrs()
{
	TPNOTE *cur, *prev;
	prev = FirstNote();
	while( prev )
	{
		cur = prev->GetNext();
		if( cur )
		{
			cur->prevI = prev;
		}
		prev = cur;
	}
}

double TPNOTE::durationalAccent()
{
	double  prevIOI = 0,
		   curIOI = IOI().toDouble(),
		   nextIOI = 0;

	TPNOTE *p,*n;
	p = GetPrev();

	n = GetNext();
	if( p )
		prevIOI = p->IOI().toDouble();
	if( n ) 
		nextIOI = n->IOI().toDouble();
	return ::durationalAccent(prevIOI, curIOI, nextIOI);

}

