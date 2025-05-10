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
|	Date	  	 : 17.10.1996-98-04,2011
------------------------------------------------------------------*/

#include <string>
#include <iostream>
#include <sstream>
#include <stack>

using namespace std;

#include <ctype.h>

//#include <io.h>
#include "../lib_src/similarity/dynaprog.h"

#include "pattern.h"
#include "funcs.h"
#include <stdlib.h>

#include "c_track.h"
#include "c_note.h"


#include "liste.h"
#include "statist.h"
#include "h_midi.h"
#include "q_funcs.h"
//---------------------------------------------------------
TPATTERN::TPATTERN( void ) :TSPATTERN(), typicalDistance(patDistanceLimit )
{
	binclass = NULL;
	cUsedCurrent = 0;
}  // TPATTERN
//-----------------------------------------------------------------
TPATTERN::~TPATTERN( void )
{
} //~TPATTERN
//-----------------------------------------------------------------
//-----------------------------------------------------------------
/*
TQNOTE *TPATTERN::Compare( TQNOTE *note,		// current note
			  TFrac  *diffLength,	// Sum of differences of duration
			  TFrac  *diffStart,	//  ""      for attackpints
			  char  *pass,			// flag for match
			  TAbsTime *endLastMatch )	// endpoint of last note
{
	return Compare(note,
						diffLength,
						diffStart,
						pass,
						0, // mode
						endLastMatch);
}
*/
//-----------------------------------------------------------------
/*
	Compare pattern to notes
	mode == 0 -> compare
	mode == 1 -> store
	Rückgabe :
		match : next unprocessed note in track, *pass = 1
		not match : note, *pass = 0
	remarks:
		-alternatives for attackpoint and duration must be calculated
		before
		-mode=1 should only be called, if compare was successfull
*/
#ifdef skdjhskfjdh
TQNOTE *TPATTERN::Compare( TQNOTE *note,		// current note
			  TFrac  *diffLength,	// Sum of differences of duration
			  TFrac  *diffStart,	//  ""      for attackpints
			  char  *pass,		// flag for match
			  char  mode,    /* mode == 0 => compare
						 mode == 1 => store to notes */
			  TAbsTime *endLastMatch )	// endpoint of last note
{
	TQNOTE  *Now,		// current note of track
			 *Next;		// next note of track
	TPNOTE *PNow;		// current note of pattern
	int    Count = 1;	// #notes

	int voice = -1;
	if(note)
		voice = note->GetVoice();

	TAbsTime   BestT,		// values from current note
			 SecT,
			 BestL,
			 SecL;

	TAbsTime  StartTime,	// attackpoint of note
			 TimeToEnd,	// time from current note to end of pattern
			 PatternStart,	// if > 0 , pattern starts with a rest
			 Temp,
			 BestDiff,	// distance to current pattern note
			 SecDiff,
			 BestStart,	// best attackpoint for 1st note
			 SecStart;

	TAbsTime Length;
	char   Ende = 0;

	PNow = dynamic_cast<TPNOTE *>(firstNote());
	Now  = note;

	*diffLength = 0L;
	*diffStart  = 0L;

	Length = GetLength();

	// get alternatives from note
	BestStart = note->qAttackpoint( &SecStart );

	// Pattern starting time
	PatternStart = PNow->GetAbsTime();
	TimeToEnd    = Length - PatternStart;

	if( PatternStart > 0L ) // start with rest?
	{
		BestStart -= PatternStart;
		SecStart  -= PatternStart;
		if( BestStart < 0L )
			BestStart = SecStart;
		if( SecStart < 0L )	// no match
		{
			*pass = 0;
			return note;
		}
	} // if

	// test if BestStart == n*PatternLength
	if( BestStart == 0L ||
		 (BestStart / Length).natural() )
	{
		StartTime = BestStart;
		BestT     = BestStart;

		// CHeck PatternStart for next note
		Next =QNOTE(note->GetNext( voice ));
		if( Next )
		{
			BestStart = Next->qAttackpoint( &SecStart );
			if( (BestStart == StartTime) &&
				(SecStart == StartTime) )
			{
				// Pattern Starts at next note
				*pass = 0;
				return note;
			}
		} // if Next
	} // if
	else if( SecStart == 0L ||
				( SecStart / Length ).natural() )
	// test if SecStart == n*PatternLength
	{
		StartTime = SecStart;
		BestT     = SecStart;
		// Check for next note in track
		Next =QNOTE(note->GetNext(voice ));
		if( Next )
		{
			BestStart = Next->qAttackpoint( &SecStart );
			if( BestStart == StartTime )
			// Pattern starts at next note
			{
				*pass = 0;
				return note;
			}
		} // if Next
	} // else if
	else	// PatternStart does not match to current note
	{
		*pass = 0;
		return note;
	}// else

	if( mode )	// Set attackpoint for first note
		note->QuantizeAbsTime( StartTime + PatternStart);

	// process following notes
	Ende = 0;
	while( !Ende && Now && PNow )
 	{
		  if( Count > 1 ) // attackpoint, start with note 2
		  {
				BestT = Now->qAttackpoint( &SecT );

				Temp     = PNow->GetAbsTime();
				BestDiff = abs( Temp - (BestT - StartTime));
				SecDiff  = abs( Temp - (SecT  - StartTime));

				if( Temp == ( BestT - StartTime) )
				{
					*diffStart = *diffStart + BestDiff ;
					if( mode ) // store
					{
						Now->QuantizeAbsTime( BestT );
					}
					SecT = BestT;
				}
				else if( Temp == ( SecT - StartTime ))
				{
					*diffStart = *diffStart + SecDiff;
					if( mode )
					{
						Now->QuantizeAbsTime( SecT );
					}
					BestT = SecT;
				}
				else // no matches
				{
					Ende = 1;
				}
		  } // if ( Count > 1 )
		  // process durations , start with note 1
		  BestL     = Now->qDuration( &SecL );
		  Temp      = PNow->GetDuration();
		  TimeToEnd -= Temp;
		  if( !Ende )
		  {
			if( Temp == BestL )
			{
			  *diffLength = *diffLength + Now->GetBestDiffDuration();
			  if( mode ) // store
			  {
				Now->QuantizeDuration( BestL );
			  }
			}
			else if( Temp == SecL )
			{
			  *diffLength = *diffLength + Now->GetSecDiffDuration();
			  if( mode )
			  {
				Now->QuantizeDuration( SecL );
			  }
			}
			else	// lgDuration different to pattern
			{
				// check for distance to next note
				Next = QNOTE (Now->GetNext( voice ));

				// Temp == note-duration of pattern
				// BestT == BestAttackpoint of quantize
				// SecT == SecbestAttackpoint of quantize
				if( Next )
				{
					BestDiff = BestT + Temp;
					BestT = Next->qAttackpoint( &SecT );
				}

				// BestDiff == Attackpoint by pattern-quantize
				// BesT == BestAttack(Next)
				// SecT == SecAttack(Next)
				if( !Next ||
					 ((BestDiff <= BestT) || //old: &&
					 (BestDiff <= SecT )) )
				{	// duration of pattern is possible for note

					BestDiff = abs( Temp - Now->GetDuration() );

					*diffLength = *diffLength + BestDiff;
					 if( mode && !Ende )
					 {
						Now->QuantizeDuration( Temp );
					 }
				}
				else	// not enough space between notes -> no match
				{
					Ende = 1;
				}

			} // else
		  } // if !Ende

		  Count++;
		  *endLastMatch = Now->qDuration() + Now->qAttackpoint();

		  PNow = PNow->GetNext();
		  Now  = QNOTE((Now->GetNext( Now->GetVoice() )));
		  
	}; // while

	if( !Ende ) // end of pattern reached?
	{
		// rest at end of pattern?
		if( TimeToEnd > 0L && Now )
		{
			BestT = Now->qAttackpoint( &SecT );
			if( (StartTime + Length ) <= BestT )
			{
				 *pass = 1;
				 return Now;
			}
			else if( (StartTime + Length ) <= SecT )
			{
				 *pass = 1;
				 return Now;
			}
			else
			{
				 *pass = 0;
				 return note;
			}
		}
		if( ! PNow )
		{
			if( Next && Now) // CHeck for overlapping to following note
			{
				StartTime += Length;
				BestT = Now->qAttackpoint( &SecT );
				if( StartTime > BestT &&
					 StartTime > SecT )
				{
					*pass = 0;
					return Now;     // Patttern has more notes than piece
				}
				else  // ok, match possible
				{
					*pass = 1;  // pattern end reached, no rest
					return Now;
				}

			}
			else
			{
				*pass = 1;  // pattern end reached, no rest
				return Now;
			}
		}
		else
		{
			*pass = 0;	// pattern end not reached
		}
		return Now;
	}
	else // Ende == 1
	{
		*pass = 0;
//		return note; old, changed 9/00
		return Now;
	}
}

//--------------------
TQNOTE *TPATTERN::Set( TQNOTE *ptr,
				TFrac  *diff,
				int   ppq,
				TFrac  *endLastMatch )
/*
	Copy pattern values to notes.
	call Compare with mode = 1;
*/
{
	TQNOTE *Note;
	TFrac  dummy1,
			dummy2;
	char  dummy3;

	Note = Compare( ptr,
			&dummy1,
			&dummy2,
			&dummy3,
			1 /*mode*/,
			endLastMatch);
	*diff = dummy1 + dummy2;
	return Note;
} //TPATTERN::Set
#endif
//-----------------------------------------------------------------


/// val func for dynamic programming
double _stdcall valTQNote ( TQNOTE *note )
{
	if( note )
		return note->GetAbsTime().toDouble();
	else
		return -1;
}

/// val func for dynamic programming
double _stdcall  valTPNote( TPNOTE *note )
{
	if( note )
		return note->pos().toDouble();
	else
		return -1;
}

/// similarity  func for dynamic programming
/// res = [-1...0]
double _stdcall simQPNote( TQNOTE *qNote,
				  TPNOTE *pNote,
				  TQNOTE *gapVal,
				  int cGaps,
				  int cLocalGaps,
				  int alLength )
{
	if( qNote == gapVal ) // skip a pattern note
		return -0.2;

	if(	pNote == NULL ) // dont' skip a qnote!
		return -0.999;

	double at1 = qNote->GetAbsTime().toDouble();
	double at2 = pNote->GetAbsTime().toDouble() /* + pNote->avOffset() */;

	double ad = fabs(at1-at2); // attackpoint distance
			
	//d1 = qNote->GetDuration().toDouble();
	


	// duration has only minor distance
	double res = 0.8 * exp( - 2* ad) + 0.2  * exp( - 2* ad);
	/// 1..0

	/// 0..-1
	return 0 - (1-res);
}
typedef TQNOTE *PTQNOTE;
typedef TPNOTE *PTPNOTE;
/// retrieve similarity of best possible alignment
CDynaprog<TQNOTE *, TPNOTE *> *TPATTERN::similarity( TQNOTE *start, 
													double &res,
													TFrac &minStart,
													double /* atAccuracy */,
													double /* durAccuracy */)
{	
	//double 
	res = -1;

	// todo: if 1:1 sim is too low -> try a m:n sim with skipped pattern notes!


	// get number of pattern notes
	int cPNotes = cNotes();

	// check available qNotes
	int cQNotes = cPNotes;

	int i = cQNotes;
	TQNOTE *temp = start;
	while( i > 0 &&
		   temp )
	{
		temp = dynamic_cast<TQNOTE *>(temp->GetNext(temp->GetVoice()));
		i--;
	}
	
	if( i ) // not enough qNotes
	{
		cQNotes -= i;
	}

	if( cPNotes <= 0 ||
		cQNotes <= 0)
		return 0;

	if( cPNotes <= 3 )
		return 0;

	// shift pattern to clostest n*1/8 >= minStart
	TFrac eigth(1,8);

	TFrac qNotePos = start->GetAbsTime();
	if( qNotePos < minStart )
		qNotePos = minStart;

	TFrac dEarly = qNotePos % eigth;
	TFrac dLate = eigth - dEarly;

	TFrac shiftPos;
	if( qNotePos - dEarly < minStart )
		shiftPos = qNotePos + dLate;
	else if( dEarly == 0L) 
		shiftPos = qNotePos;
	else if( dLate == 0L )
		shiftPos = qNotePos;
	else if( dEarly < dLate )
		shiftPos = qNotePos - dEarly;
	else
		shiftPos = qNotePos + dLate;
	

	shiftTo( shiftPos );


	// copy pointers
	PTQNOTE *qnotes = new PTQNOTE[cQNotes];
	temp = start;
	for( i = 0; i < cQNotes; i++ )
	{
		qnotes[i] = temp;
		temp = dynamic_cast<TQNOTE *>(temp->GetNext(temp->GetVoice()));
	}
	TPNOTE *temp2 = FirstNote();
	PTPNOTE *pnotes = new PTPNOTE[cPNotes];
	for( i = 0; i < cPNotes; i++ )
	{
		pnotes[i] = temp2;
		temp2 = NextNote();
	}




	// do alignment
    CDynaprog<TQNOTE *, TPNOTE *> *dynaprog;	
	dynaprog = new CDynaprog<TQNOTE *, TPNOTE *>(qnotes, pnotes, 
									cQNotes,
									cPNotes,
									NULL,
									1 /* forward dir*/,
 									valTQNote,
									valTPNote,
									simQPNote );


	// create alignment
	double processTime;
	res = dynaprog->makeTable( -1 /* no delta limit */,
								-1 /* no absLimit */,
								&processTime);
	// normalize to length
	res /= cPNotes;
	res = 1+res;

	//! read alignment from table
	dynaprog->retrieveAlignment();


	// get path similarity
	{
		res = 1;
		int i = dynaprog->outStart;
		TQNOTE *qnote;
		TPNOTE *pnote;
		// get cosine distance
		double Pa = 0,
			   Qa = 0,
			   Pd = 0,
			   Qd = 0,
			   PQa = 0,
			   PQd = 0;
			   
		initNDimAngle( Qa,Pa,PQa);
		initNDimAngle( Qd,Pd,PQd);
			   
		while( i < dynaprog->iI + dynaprog->jI )
		{
			qnote = dynaprog->S1outFl[i];
			pnote = dynaprog->S2outFl[i];

			// calculate the cosine distance
			double qVal;
			if( qnote )
				qVal = qnote->GetAbsTime().toDouble();
			else
				qVal = pnote->GetAbsTime().toDouble() * 0.8;

			double pVal;
			if( pnote )
				pVal = pnote->GetAbsTime().toDouble();
			else
				pVal = qnote->GetAbsTime().toDouble() * 0.8;

			updateNDimAngle( qVal, 
							pVal,
							Qa,
							Pa,
							PQa);

			if( qnote )
				qVal = qnote->GetDuration().toDouble();
			else
				qVal = pnote->GetDuration().toDouble() * 0.8;


			if( pnote )
				pVal = pnote->GetDuration().toDouble();
			else
				pVal = qnote->GetDuration().toDouble() * 0.8;
			updateNDimAngle( qVal, 
							pVal,
							Qd,
							Pd,
							PQd);

			// res *= (1 + simQPNote(qnote, pnote, NULL ));
			i++;
		} // blick		

		double resA =  nDimAngle(Qa,Pa,PQa);
		double resD =  nDimAngle(Qa,Pa,PQd);
		// normalise to length
		
		res /= pow(0.98, cPNotes );
		if( res > 1 )
			res = 1;
#ifdef _DEBUG
	Printf("Debug: TPATTERN::similarity!\n");	
#endif
		res = resA *0.8 +resD *0.2;
	}
	
	
	
//	printf("sim %f\n", res );
//	this->write( stdout );
	

	// reset Pattern abstimes

	dynaprog->debug();
	// don't reset shift, keep for quantisation!
 	// shiftTo( TFrac(0,1) );

	/*
	dynaprog->delOutVal();
	delete dynaprog;
	*/
	delete [] qnotes;
	delete [] pnotes;
	// return 1+res;
	return dynaprog;

}


void TPATTERN::shiftTo(TAbsTime pos)
{
	TPNOTE *temp = FirstNote();
	if( temp ){
		lgDuration offset = FirstNote()->pos() - lgDuration(pos.numerator(),
						  pos.denominator() ) ;
		while( temp )
		{
			temp->setPos( temp->pos() - offset );
			temp = NextNote();
		}
	}
	else
	{
		cout << "ERROR: Empty TPattern!";
	}
}

double TPATTERN::updateTypicalDistance(double newDistance)
{
	typicalDistance += newDistance;
	typicalDistance /=2; 
	return typicalDistance;
}

double TPATTERN::distancePenalty(double newDistance, double sigma)
{
	if( newDistance <= typicalDistance )
		return 0;
	double res = 1 - GaussWindow(typicalDistance, 
							newDistance,
							sigma);		
	return res;
}
                  

/// how often was this pattern used?
double TPATTERN::cUsed( void )
{
	lgTag *temp = findTag("\\statist");
	if( temp )
	{
		lgIntTagArg *tagArg = dynamic_cast<lgIntTagArg *>(temp->findArg( "cUsed" ));
		if( tagArg )
		{
			return tagArg->valInt();
		}
	}
	return 0;
}
/// increment used counter
void TPATTERN::incCUsed( int delta )
{
	cUsedCurrent += delta;
}

/// usedBefore?
double TPATTERN::curWeight( void )
{
	if( binclass )
		return binclass->weight();
	return 0;
}

void TPATTERN::finaliseCUsed()
{
	long int tmpVal = cUsedCurrent;
	lgTag *temp = findTag("\\statist");
	if( temp )
	{
		lgIntTagArg *tagArg = dynamic_cast<lgIntTagArg *>(temp->findArg( "cUsed" ));
		if( tagArg )
		{
			int use = tagArg->valInt(  );		
			tagArg->setValInt( use + cUsedCurrent );
		}
		else
		{
			string cUsed = std::string("cUsed");
			string un = string("");
			tagArg = new lgIntTagArg(cUsed, tmpVal, un);
			temp->addArg(tagArg);
		}
	}
	else
	{
		temp = new lgTag(0, this, "\\statList");
		string cUsed = std::string("cUsed");
		string un = string("");

		lgIntTagArg *tagArg =  new lgIntTagArg(cUsed, tmpVal, un);
		temp->addArg(tagArg);
		this->insertTag(temp);
	}
}

void TPATTERN::write(FILE *out,
					lgVoice *)
{

	finaliseCUsed();
//	fprintf(out, "%%pattern \n");
	lgSequence::write(out);
}

char mySemaphore = 0;
 string TPATTERN::toString(lgVoice *v)
{
	if( !mySemaphore )
	{
		mySemaphore++;
		finaliseCUsed();
	}
	string res = TSPATTERN::toString(v);
	mySemaphore--;
	return res;
	
}

double TPATTERN::aPriori( double sum )
{
	double res = cUsed()+1; // keep always > 0
	res /= sum;
	return res;
}

/// compare by IOIratio
double TPATTERN::IOIDistance(TQNOTE *start,
							TFrac &minStart,
							TFrac meter,
							double atAccuracy,
							double durAccuracy)
{

	// todo: if 1:1 sim is too low -> try a m:n sim with skipped pattern notes?

	// get score position of performance note
	TFrac qNotePos = start->GetAbsTime();
	
	// might be true because of shifting
	if( qNotePos < minStart )
	{
		// Printf("ERROR: illegal minStart\n");
		qNotePos = minStart;
	}

	// calc length of leading rest in pattern
	TFrac startRestLength;
	lgEvent *tempEvent = firstEvent();
	while( tempEvent &&
			!dynamic_cast<lgNote *>(tempEvent) )
	{
		startRestLength += tempEvent->duration();
		tempEvent = dynamic_cast<lgEvent *>(tempEvent->next());
	}
	if( qNotePos - startRestLength < minStart )
	{
		// not enough space between start and prev note
		return 1;
	}

	// use only n*1/8 as pattern start
	/*
	TFrac eigth(1,8);
	TFrac dEarly = qNotePos % eigth;
	TFrac dLate = eigth - dEarly;
	*/
	/// calc distance between performace attack n*patternLength, (n+1)*patternLength
	/// if pattern length == n*meter -> use meterlength 
	TFrac nDur = duration();
	if( nDur > meter )
		nDur = meter;
	TFrac dEarly = qNotePos % nDur;
	TFrac dLate = nDur - dEarly;
	
	
	
	/// shift complete pattern to closest grid pos to minStart
	TFrac shiftPos;
	if( qNotePos - dEarly  - startRestLength < minStart )
		shiftPos = qNotePos + dLate;
	else if( dEarly == 0L) 
		shiftPos = qNotePos;
	else if( dLate == 0L )
		shiftPos = qNotePos;
	else if( dEarly < dLate )
		shiftPos = qNotePos - dEarly;
	else
		shiftPos = qNotePos + dLate;
	/// shift pattern times
	shiftTo( shiftPos );
	
	// don't use pattern if start at error position
	long atDenom = firstNote()->pos().durD;
	long durDenom = firstNote()->duration().durD;
	if( primeDenomDistance(atDenom, durDenom ) >= 1 )
		return 1;

	/// compare pattern-performance ----------------------------------
	// get number of pattern notes
	int cPNotes = cNotes();
	// don't evaluate very short pattern
	if( cPNotes < 3 )
		return 1;

	// check available qNotes
	int cQNotes = cPNotes;

	int i = cQNotes;
	TQNOTE *temp = start;
	// get number of available performance notes
	while( i > 0 &&
		   temp )
	{
		temp = dynamic_cast<TQNOTE *>(temp->GetNext(temp->GetVoice()));
		i--;
	}
	
	/// compare incomplete pattern
	if( i ) // not enough qNotes
	{
		cQNotes -= i;
	}

	if( cPNotes <= 0 ||
		cQNotes <= 0)
		return 1;

	double Qa = 0,
		   Pa = 0,
		   Qd = 0,
		   Pd = 0,
		   dPCd = 0,
		   dPCa = 0;

	
	// calculate angle between offset and attackpoint vectors	
	initNDimAngle( Qa,Pa,dPCa);
	initNDimAngle( Qd,Pd,dPCd);

	double
		   maxDeltaA = 0;
	cPNotes = 0;
	TPNOTE *cur = this->FirstNote();
	while( cur && start )
	{
		cPNotes++;
		if(  cPNotes > 1 ) // first note has always zero distance!!!
		{
			// attackpoint compare
			updateNDimAngle( 100 * (start->GetAbsTime().toDouble() - shiftPos.toDouble()),
							 100 * (cur->GetAbsTime().toDouble() - shiftPos.toDouble()),
							 Qa, 
							 Pa, 
							 dPCa );
		} 
		// duration  compare
	/*
		updateNDimAngle(100 * start->GetOffset().toDouble(),
					    100 * (cur->GetAbsTime().toDouble() + cur->GetDuration().toDouble()),
					    Qd,
					    Pd,
					    dPCd);
	*/	
		updateNDimAngle(100 * start->GetDuration().toDouble(),
					    100 * cur->GetDuration().toDouble(),
					    Qd,
					    Pd,
					    dPCd);
		double deltaA = fabs(cur->GetAbsTime().toDouble() 
							  - start->GetAbsTime().toDouble());
		if( deltaA > maxDeltaA )
		{
			maxDeltaA = deltaA;
			// don't allow attackpoint shifts > 1/12
			if( maxDeltaA > 1.0/12.0 )
			{
				// early abot, save some time
				return  1;
			}
		} // if

		// check IOI vs duration
		double pIOI = cur->IOI().toDouble();
		double pDur = cur->duration().toDouble();
		double nIOI = start->ioi(1, start->GetVoice() ).toDouble();
		double nDur = start->GetDuration().toDouble();
		double pRel = pDur/pIOI;
		double nRel = nDur/nIOI; 

		// don't accept insertion of rests!
		if( nRel > 0.9 &&
			nRel > pRel )
		{
			return 1;
		}

		start = dynamic_cast<TQNOTE *>(start->GetNext(start->GetVoice() ));
		cur = cur->GetNext();
	} // loop all notes
	if( start ) // add a penalty for the next attackpoint
	{
		double notePos = start->GetAbsTime().toDouble() -
		    			  shiftPos.toDouble();
		double patternEndPos = FirstNote()->GetAbsTime().toDouble() + 
								duration().toDouble() - shiftPos.toDouble();
//								nDur.toDouble() - shiftPos.toDouble();
		if(  notePos < patternEndPos)
		{
			updateNDimAngle( 100 * notePos,
							 100 * patternEndPos,
							 Qa, 
							 Pa, 
							 dPCa );
		} // if next note earlier than pattern end
	} // if

	double resA = nDimAngle(Qa,Pa,dPCa);
	double resD = nDimAngle(Qd,Pd,dPCd);

	// biasing with accuracy values
	double mult = 2 - GaussWindow(atAccuracy, 0.5);
	resA *= mult;
	mult = 2 - GaussWindow(durAccuracy, 0.5);
	resD *= mult;

	double res = 0.8*resA + 0.2*resD;
	
	// range of angle is 0...pi
	if( res > 1 )
		res = 1;
//	Printf("distance = %f\n",res);
	return res;
}


double normIOIratio( double IOIratio )
{
	if( IOIratio > 0 )
		return IOIratio - 1;
	if( IOIratio < 0 )
		return IOIratio + 1 ;

	return 0;
}

double denormIOIratio( double IOIratio )
{
	if( IOIratio >= 0 )
		return IOIratio + 1;
	if( IOIratio < 0 )
		return IOIratio - 1 ;

	return 0;
}

// #define _DEBUG_PATTERN

/// durational distance between note and first clicknote
double TPATTERN::headDurationDistance( TCLICKNOTE *note,
					         TDoubleBinList * /* IOIratioList */ ,
                             TFracBinList *durationList,
                             TCLICKTRACK *clicktrack,
                             /// previous relation between score and performance
							 double normF,
							 /// variance of relation
							 double normFSigma)
{
	TPNOTE *cur = this->FirstNote();
	if( !cur || !note)
		return 1;

	double /// tempo penalty
		   pTempo = 1,
		   /// normalisation penalty
		   pNorm = 1;

	
	/// perfIOI of note
	double localIOI = note->ioi(1,-1).toDouble();	
	if( normF > 0)
	{
		/// tick duration of pattern note
		double tickDur = 0;
	
		// first pattern note has no valid IOIratio!
		// compare IOI or abs tempo for first notes
		TFracBinClass *cl;
	
		// performace IOI

		 // IOI of first pattern note
		TFrac patIOI = cur->IOI();

		if( 0 ) // version 1
		{
			/// check  the probability of the first pattern note
			/// version 1: performed IOI -> performed score dur -> compare
			TFrac sIOI;
			TDistanceStruct ds;
			
			// convert perf IOI into performerd score duration
			sIOI = TFrac((long)(960*localIOI / normF+0.5),960L);
			// get a quantised score duration
			ds = durationList->closestClass(NULL, &sIOI );
			cl = dynamic_cast<TFracBinClass *>(ds.classPtr);

			// find class of first pattern note
            ds.classPtr = durationList->findExact(patIOI);
			// what would happen, if from->IOI == cur->IOI?
			if( ds.classPtr )
			{
				double patDurDist = durationList->distance( ds.classPtr,
										NULL,
										&sIOI );
				if( patDurDist < 0.6 )
					cl = dynamic_cast<TFracBinClass *>(ds.classPtr);

			}
			tickDur = cur->IOI().toDouble() * normF;			
		} // version1
		else if( 1 ) // we have a normF -> compare like std pattern note
		{
			// convert first pattern note to tick timing
			tickDur = cur->IOI().toDouble() * normF;
			cl = NULL;	
		}
		else if( 0 )			
		/// version 2: 1st pattern note score duration -> score IOI -> compare
		{
			/// tick IOI of first pattern note
			cl = durationList->findExact(patIOI);
			double pScoreDur = 0.001;
			if( cl )
			{
					/// probability for the selected score duration
					pScoreDur = cl->weight();
			}
			else
			{
					// uncommon duration, not in list
			}
			tickDur = durationList->tickDuration(patIOI);
		} // version 2
		if( normFSigma < normF * 0.1 )
		{
			// allow a minimum of 10%
			normFSigma = normF * 0.1;
		}
		else if( normFSigma > normF * 0.4 )
		{
			normFSigma = normF * 0.5;		
		}
		pNorm = GaussWindow(tickDur, localIOI, normFSigma);
	}
	{	
			// res depends only on resulting tempo
			/// check resulting tempo, if note would become click note duration
			// set test values
			TFrac oldCDuration = note->scoreDurationI;
			note->scoreDurationI = cur->IOI();
			double tempo = clicktrack->tempo( note );
			// reset values
			note->scoreDurationI = oldCDuration;
			pTempo =  GaussWindow(tempo - 130, 
								  80 /*sigma*/); 				
	}//
	double res = 1 - (pTempo * pNorm);
	return res; 
}

/*! 
do a 1:1 compare between 2..n clicknotes and pattern,
durationDistance of head must be added!
 */
 
double TPATTERN::IOIratioDistance(TCLICKNOTE *from)
{

#ifdef _DEBUG_PATTERN
FILE *out = fopen("_pattern.txt","at");
fprintf(out,"\n%d: ",GetID());
#endif
	if( !from )
		return 1;
		
	TPNOTE *cur = this->FirstNote();	
	if( !cur )
		return 1;
		
	double pLenXxX = 0, 
		cLenYxY = 0, 
		pcXxY = 0;

	initNDimAngle( pcXxY, pLenXxX, cLenYxY );

	
	// get normFactor from relation between first notes
	double normF = from->ioi(1,-1).toDouble() / cur->IOI().toDouble();
	
	// skip first notes	
	cur = cur->GetNext();
	from = from->getNext(1,-1);

	// compare all remaining pattern notes
	double avDist = 0;
	double minDist = 1,
		   maxDist = 0;
//		   sigmaDist = 0;
		   
	int abort = 0;
	int i= 0;
	char compound = 0;
	/*
	if( cNotes() < 5 )
		compound = 1; // use also compound notes
	*/
	while( !abort && 
		   cur && 
		   from )
	{
		TFrac pScoreDur; // score duration penalty
		double localIOI; // perfIOI
		if( compound == 2 &&
			cur->GetNext() &&
			from->getNext(1,-1) )
		{
			// build a compound note of current two notes;
			pScoreDur = cur->IOI() + cur->GetNext()->IOI();
			localIOI = from->ioi(2,-1).toDouble();
		}
		else
		{
		// get tick duration of current pattern note
			pScoreDur = cur->IOI();
			localIOI = from->ioi(1,-1).toDouble();
		}
		/*
		double tickDur = pScoreDur.toDouble() * normF;
		*/
		/// normalise perfIOI to a generic score duration,
		/// 1/1 == 1000;
		/// by converting the perfIOI to score*1000 we are
		/// independent froom the ppq of the file!!!		
		double newNormF = localIOI / pScoreDur.toDouble();
		
		localIOI *= (1000/normF);
		double tickDur = pScoreDur.toDouble() * 1000;
		
		double dist = fabs(tickDur - localIOI);
		
		avDist += dist;
	
		// early abort?	
		double rel = localIOI/tickDur;
#ifdef _DEBUG_PATTERN
fprintf(out,"%1.3f:%1.3f",localIOI, tickDur); 
#endif
		if( rel < 1 )
			rel = 1/rel;
		if( rel > 1.4 ) // abort, save time 
		{
			abort = 1;
		}
		else
		{
			updateNDimAngle( localIOI,
							tickDur,
							 pcXxY, 
							 pLenXxX, 
							 cLenYxY );
		
#ifdef _DEBUG_PATTERN
fprintf(out,"(%1.3f)",normF); 
#endif
			// update relation
		 	normF = newNormF;
		
			// evaluate here the "weight" of the pattern?
			if( dist < minDist )
			{
				minDist = dist;
			}
		    if( dist > maxDist )
			{
				maxDist = dist;
			}		
			i++;
			if( compound != 1 )
			{
				cur = cur->GetNext();
				from = from->getNext(1,-1);
				if( compound == 2 )
				{
					compound = 1;
				}
			}
			else
			{
				if( cur->GetNext() &&
					from->getNext(1,-1) )
				{
					// we can build a compound note
					compound = 2;
				}
				else // no more note after cur or from
				{
					compound = 0;
				}
			}
		} // else (dist good)
	} // while
	
	if( i )
		avDist /= i;


	// see Zanon de Poli "Estimation of the Parameters in Rule Systems ...."
	/*
	res = acos( res );  // acos(-1) == pi
	*/
	double res;
	if( !abort )
	{
		res = nDimAngle( pcXxY, 
					 pLenXxX, 
					 cLenYxY );
	}
	else
	{
		res = 0.9999;
	}

	// normalise to [0..1)
	// res /= acos(-1);

	/*
	if( i )
		res /= i;
	// if we have some bad matches, dont' use pattern
//	if( minDist * 1.3 < maxDist )
	
	res = maxDist;
	*/

#ifdef _DEBUG_PATTERN
fprintf(out,"=%1.3f",res); 
fclose(out);
#endif
	
	return res;
}

// return best suffix  with end(suffix) > end(this-delta)
int TPATTERN::IOIratioSuffix(TPatternDistance *pArray, int size, int delta)
{
	for( int i = 0; i < size; i++ )
	{
		if( IOIratioSuffix(pArray[i].ptr, delta ) )
			return i;
	}
	return -1;
	
}

char TPATTERN::IOIratioSuffix(TPATTERN *p2, int delta)
{
	TPNOTE *cur1 = FirstNote();
	TPNOTE *cur2 = p2->FirstNote();
	// use ony "real" suffix
	long cn = cNotes();
	if( cn - delta >= p2->cNotes() )
	{
		return -1;
	}
	// skip delta notes
	while( delta && cur1)
	{ 
		cur1 = cur1->GetNext();
		delta--;
	}
	TFrac mult(1,1);
	char res = 1;
	if( cur1 && cur2 )
	{
		mult = cur1->IOI() / cur2->IOI();
		cur1 = cur1->GetNext();
		cur2 = cur2->GetNext();
	}
	TAbsTime norm2;
	while(cur1 && cur2)
	{
		norm2 = cur2->IOI() * mult;
		if( norm2 != cur1->IOI() )
			return 0;

		cur1 = cur1->GetNext();
		cur2 = cur2->GetNext();
	}
	return res;
}

// return best suffix  with end(suffix) > end(this-delta)
int TPATTERN::IOISuffix(TPatternDistance *pArray, int size, int delta)
{
	for( int i = 0; i < size; i++ )
	{
		if( IOISuffix(pArray[i].ptr, delta ) )
			return i;
	}
	return -1;
	
}

char TPATTERN::IOISuffix(TPATTERN *p2, int delta)
{
	TPNOTE *cur1 = FirstNote();
	TPNOTE *cur2 = p2->FirstNote();
	// use ony "real" suffix
	long cn = cNotes();
	if( cn - delta >= p2->cNotes() )
	{
		return 0;
	}
	// skip delta notes
	while( delta && cur1)
	{ 
		cur1 = cur1->GetNext();
		delta--;
	}
	TFrac mult(1,1);
	char res = 1;

	lgRest *cp1 = cur1;
	lgRest *cp2 = cur2;

	while(cp1 && cp2)
	{

		if( cp1->duration() != cp2->duration() )
			return 0;
		lgNote *n1 = dynamic_cast<lgNote *>(cp1);
		lgNote *n2 = dynamic_cast<lgNote *>(cp2);
		// note against rest?
		if( (n1 || n2) &&
			(!n1 || !n2) )
			return 0;

		cp1 = dynamic_cast<lgRest*>(cp1->next());
		cp2 = dynamic_cast<lgRest *>(cp2->next());
	}
	return res;
}



int TPATTERN::IOIratioPrefix(TPatternDistance *pArray, int size, int delta)
{
	for( int i = 0; i < size; i++ )
	{
		if( pArray[i].ptr->IOIratioSuffix(this, delta ) )
			return i;
	}
	return -1;
	
}

