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
|	filename  : NOTE.CPP
|	author    : Juergen Kilian
|	date	  : 17.10.1996-2000-03,2011
|	contents  : implementation of TQNOTE
------------------------------------------------------------------*/
#include <iostream>
using namespace std;
#include <sstream>

#include <stdlib.h>
#include "q_funcs.h"
#include "q_note.h"
#include "q_chord.h"
#include "funcs.h"
#include "track.h"
#include "portable.h"

#include "meta_key.h"
#include "meta_meter.h"
#include "meta_text.h"

#include "../lib_src/ini/ini.h"
#include "notestatlist.h"

#include "statist.h"
//----------------------------------------------------------------------

#undef UNUSED
//----------------------------------------------------------------------

const char *getColorString( int color )
{
	const char *colorStr; 
		switch( color )
		{
			case 1 : colorStr = "red"; break;
			case 2 : colorStr = "green"; break;
			case 3 : colorStr = "blue"; break;
			case 4 : colorStr = "yellow"; break;
			default : colorStr = "black";
		}
		return colorStr;
}


char TQNOTE::durationIsIOI( void )
{
    TQNOTE *tNext = dynamic_cast<TQNOTE *>(GetNext(GetVoice()));
    if( tNext )
    {
        TFrac delta = GetAbsTime() + GetDuration() - tNext->GetAbsTime();
		
		double dDelta = fabs(delta.toDouble());
        if( dDelta < 0.0625 ) // 1/16
        {
            return 1;
        }
    }
    return 0;
}

TQNOTE *  QNOTE(TMusicalObject *s)
{
	return dynamic_cast<TQNOTE *>(s);
}


TQNOTE::TQNOTE( 	TAbsTime abstime,
			   TAbsTime duration,
			   unsigned char pitch,
			   double intens ) :
TNOTE( abstime, duration, pitch, intens), color(0)
{
	init ();
};

TQNOTE::TQNOTE(lgNote *note) : TNOTE( TFrac(0,0),
									 TFrac(1,4),
									 0,0 ), color(0)
{
	init();
	fill( note );
}



TQNOTE::TQNOTE(const TQNOTE &ptr) : TNOTE( ptr ), color(0)
{
	BestDiffL = ptr.BestDiffL;
	SecDiffL = ptr.SecDiffL;
	dSelection = ptr.dSelection;
	
}


void TQNOTE::Debug( ostream &gmnOut )
{
	TNOTE::Debug(gmnOut);
	gmnOut << "  BT: ";
	(AbsTime+BestDiffT).Write(gmnOut);
    if( atSelection == firstSel )
        gmnOut << "*";
    gmnOut << "ST: ";
	(AbsTime+SecDiffT).Write(gmnOut);
    if( atSelection == secondSel )
        gmnOut <<  "*";
	gmnOut <<  "BD: ";
	(mDuration+BestDiffL).Write(gmnOut);
    if( dSelection == firstSel )
        gmnOut << "*";
	gmnOut << "SD: ";
	(mDuration+SecDiffL).Write(gmnOut);
    if( dSelection == secondSel )
        gmnOut <<  "*";
	gmnOut << "\n";
}


//----------------------------------------------------------------------
/*!
set BestDiffT and SecDiffT
*/
TAbsTime TQNOTE::QuantizeAbsTime( TAbsTime newtime )
{
	BestDiffT = newtime - AbsTime;
	SecDiffT  = BestDiffT;
	return AbsTime + BestDiffT;
} // Quantize Abstime
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
set BestDiffL an SecDiffL
*/
TAbsTime TQNOTE::QuantizeDuration( TAbsTime duration )

{
	if( duration <= 0 )
	{
		Printf("ERROR: Can't quantize duration to <= 0!\n");
		return GetDuration();
	}
	BestDiffL = duration - GetDuration();
	SecDiffL  = BestDiffL;
	return GetDuration() + BestDiffL;
} // Quantize lgDuration
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
check if BestDiffT == SecDiffT
result
1 : equal
0 : else
*/
//----------------------------------------------------------------------
/*
check if BestDiffL == SecDiffL
result
1 : equal
0 : else
*/
char TQNOTE::LOK( void )
{
	if( BestDiffL == SecDiffL )
		return 1;
	else
		return 0;
} // TOK
//----------------------------------------------------------------------

/// test if duration can be put at attackpoint, check also for the resulting rest!
double testDurPosRel( TFrac &attackpoint1,
				   TFrac &duration1,
				   TFrac &attackpoint2 )
{
	if( !duration1.denominator() ||
		!attackpoint1.denominator() )
	{
		return 1; // ok
	}
	double res =1 - primeDenomDistance(attackpoint1.denominator(),
							           duration1.denominator() );
	
	TFrac restAttack = (attackpoint1 + duration1);
	if( res > 0 &&
		attackpoint2 > restAttack)
	{
		TFrac restDur = attackpoint2 - restAttack;
		res = 1 - primeDenomDistance(restAttack.denominator(),
							 		 restDur.denominator() );		 
	}
	
	return res;
}				   
				   
int binaryPos( TFrac &abstime )
{
	if( abstime.denominator() &&
		!(2048 % abstime.denominator()) )
		return 1;
	else
		return 0; 
}
				   
/*
select a new attackpoint and duration from the
calculated alternatives.

  return:
		ok : 0
		overlap error : -1
		remarks:
		- the function is used if no pattern quantization is possible
		- alternatives for attackpoint and duration must be calculated before!
		- voice separation must be done before!!
*/
TAbsTime TQNOTE::SetQData(  TQNOTE *ptr, 	// previous note
						  TFracBinList *bestDurations )
{
    // calculate values-------------------------------
	TFrac BestDuration = GetDuration() + BestDiffL;
	TFrac SecDuration  = GetDuration() + SecDiffL;

	TFrac PrevDuration1,
		  PrevDuration2;	
	TFrac endPrevious;
	if( ptr )
	{
		endPrevious = ptr->qAttackpoint() + ptr->qDuration();
		PrevDuration1 = ptr->qDuration( &PrevDuration2 );
	}

	
	TQNOTE *NextInVoice = QNOTE(GetNext( GetVoice() ));
	
	TFrac BestStart = AbsTime  + BestDiffT;
	TFrac SecStart  = AbsTime  + SecDiffT;
		
		//		this->Debug(stdout);
		// swap attackpoint solutions?
		if( ptr &&
			BestStart != SecStart )
		{
			TFrac IOIBest = BestStart - ptr->qAttackpoint();
			TFrac IOISecond = SecStart - ptr->qAttackpoint();
			if( IOIBest > 0 &&
				IOISecond > 0 )
			{
				//				bestDurations->write(stdout);
				double pBest = bestDurations->probability( IOIBest );
				double pSecond = bestDurations->probability( IOISecond );
				if( pSecond > pBest )
				{
					TFrac tempDur = SecStart;
					SecStart = BestStart;
					BestStart = tempDur;
					
					tempDur = SecDiffT;
					SecDiffT = BestDiffT;
					BestDiffT = tempDur;
				}
			}			
		} // if prev
		
		TFrac tStart = qAttackpoint();	
		TFrac nStart;
		if( NextInVoice )
			nStart = NextInVoice->qAttackpoint();
		double bestDurPosRel = testDurPosRel(tStart,
											  BestDuration,
											  nStart);
											  
		double secDurPosRel = testDurPosRel( tStart,
											  SecDuration,
											  nStart);
			    
		
		// swap best and second duration?
		if( (BestDuration != SecDuration || 
			!(secDurPosRel * bestDurPosRel) ) &&
			dSelection == undefSel )
		{

			// bestDurations->write();
			double pBest = bestDurations->probability( BestDuration );
			if( pBest == 0.0 )
			{
				pBest = bestDurations->probability( TFrac(1L, BestDuration.denominator() ));
			}
			double pSecond = bestDurations->probability( SecDuration );
			if( pSecond == 0.0 )
			{
				pSecond = bestDurations->probability( TFrac(1L, SecDuration.denominator() ));
			}
			
			// use binary position as best
			int canSwap = 1;
			int mustSwap = 0;
			if( secDurPosRel && bestDurPosRel ) // both possible
			{
				if( binaryPos( BestDuration ) &&
				    !binaryPos( SecDuration ) )
				{
				    canSwap = 0;
				}
				else if( !binaryPos( BestDuration ) &&
				    	  binaryPos( SecDuration ) )
				{
				    mustSwap = 1;
				}
			}
			else if( secDurPosRel && !bestDurPosRel )
			{
				mustSwap = 1;
			}
			else if( !secDurPosRel && bestDurPosRel )
			{
				canSwap = 0;
			}
			
			
			if( (canSwap && pBest < pSecond && secDurPosRel > 0) || 
				 (canSwap && secDurPosRel > bestDurPosRel) ||
				 mustSwap ) // swap
			{
				TFrac tempDur = SecDuration;
				SecDuration = BestDuration;
				BestDuration = tempDur;
				
				tempDur = SecDiffL;
				SecDiffL = BestDiffL;
				BestDiffL = tempDur;
				
				int temp = bestDurPosRel;
				bestDurPosRel = secDurPosRel;
				secDurPosRel = temp;
			}
			// test here again the distance to the selected classes!
			TDistanceStruct ds;
			ds = bestDurations->closestClass( NULL, &mDuration );
			TFracBinClass *classPtr;
			classPtr = dynamic_cast<TFracBinClass *>(ds.classPtr);
			TFrac dur = classPtr->duration();

			int durPosRel = testDurPosRel( tStart,
											   dur,
											   nStart );;

			if( BestDuration == classPtr->duration() )
			{
				// do nothing 
			}
			else if( canSwap &&
			         SecDuration == classPtr->duration() 
				     && secDurPosRel ) // swap
				// if( pSecond > pBest )
			{
				TFrac tempDur = SecDuration;
				SecDuration = BestDuration;
				BestDuration = tempDur;
				
				tempDur = SecDiffL;
				SecDiffL = BestDiffL;
				BestDiffL = tempDur;
			} // if
			else if( ds.distance < 0.5 && NextInVoice )  // forget best duration and use closest class
			{
				double pClosest;
				pClosest = bestDurations->probability( TFrac(1L, classPtr->duration().denominator() ));

				
				if( canSwap &&
				    (pClosest > pBest) && durPosRel)
				{
					// swap

					TFrac tempDur = SecDuration;
					SecDuration = BestDuration;
					BestDuration = tempDur;
				
					tempDur = SecDiffL;
					SecDiffL = BestDiffL;
					BestDiffL = tempDur;
				

					// store
					BestDuration = classPtr->duration();
					BestDiffL =  BestDuration - mDuration;
				}
				else if( durPosRel )// store as second best
				{
					SecDuration = classPtr->duration();
					SecDiffL =  SecDuration - mDuration;
				}
			}
			else if( durPosRel )// forget sec duration and use closest class
			{
				SecDuration = classPtr->duration();
				SecDiffL = classPtr->duration() - mDuration;
			}
		} // if conflict situation		
	
		
	TFrac NextLatestStart;
	TFrac NextSecStart;
	TFrac NextBestStart;
	if( NextInVoice )
	{
		 NextBestStart = NextInVoice->qAttackpoint( &NextSecStart );
	
		if( NextBestStart >= NextSecStart )
			NextLatestStart = NextBestStart;
		else
			NextLatestStart = NextSecStart;
	} // if
	
	
	TQNOTE *PrevInVoice = QNOTE(GetPrev( GetVoice() ));
	if( PrevInVoice )
		PrevDuration1 = PrevInVoice->qDuration(&PrevDuration2);
	else
		PrevDuration1 = 0L;
	
	
	if(  NextInVoice &&
		(NextLatestStart <= endPrevious) ) // no gap for this
	{
		// make pre note shorter?
		//		if( GetPrev(1)->CutOrMove( lgDuration + BestDiffL ) )
        
        return -1;	// error, create extra voice for this
		//			ErrorMsg( 28 );	// can't solve overlapping
	}
		
	// compare alternatives for attackpoint abd select
	if( atSelection == undefSel )
	{
		if( !TOK() )	// both alternatives not equal?
		{
			// try to avoid rest between note and pre-note (Vorgaenger)
			if( BestStart == endPrevious )
			{
				atSelection = firstSel;
			}
			else if( SecStart == endPrevious )
			{
				atSelection = secondSel;
			}
			// overlapping between prev and this
			else if( BestStart >= endPrevious )
			{
				atSelection = firstSel;
				if( binaryPos(BestStart) &&
					!binaryPos(PrevDuration1) &&
					binaryPos(PrevDuration2) )
				{
					// is not correct of gap is too large : | c/8 _/8 _/12 c_/6 | 
				}
			}
			else if( SecStart	>= endPrevious )// rest between prev and this
			{
				atSelection = secondSel;
			}
			else // Best && Sec < endPrev
			{
				// ToDo: needs to be checked
				// printf("warning: SetQData: ignored possibilities!\n");
				BestDiffT = endPrevious - AbsTime;
				SecDiffT = endPrevious - AbsTime;
                atSelection = firstSel;
            }
		} // if( !TOK
        else // both possibilities are already equal
        {
            atSelection = firstSel;
        }
        
        // BestDiffT and SecDiffT are now equal
		// check for overlapping with  next-note
		if( 	  NextInVoice  &&
			(BestStart >= NextLatestStart) &&
			(SecStart  >= NextLatestStart) )
		{
			// shift note to avoid overlapping
			// does this.duration fit between prev and next?
			if( (NextLatestStart - endPrevious) >= (GetDuration() + BestDiffL) )
			{
				BestDiffT = (NextLatestStart - (GetDuration() + BestDiffL)) - AbsTime;
			}
			else if ( (NextLatestStart - endPrevious) >= (GetDuration() + SecDiffL) )
			{
				BestDiffT = (NextLatestStart - (GetDuration() + SecDiffL)) - AbsTime;
			}
			else // BestDuration doesn't fit into gap
			{
				BestDiffT = endPrevious - AbsTime;
			}
			atSelection = firstSel;
			printf("warning: SetQData: ignored possibilities!\n");
			SecDiffT = BestDiffT;
		} // if NewTime >= Next..Start
    } // sele == undef

	// evaluate duration value -----------------------------------
	if( qDuration() <= 0 )
	{
		Printf("ERROR: SetQData to duration <= 0!\n");
	}
	
	// check if at least one duration is invalid
	if( BestDuration <= 0L )
	{
		dSelection = secondSel;
	}
	else if( SecDuration <= 0L )
	{
		dSelection = firstSel;
	}
	
	if( dSelection == undefSel )
	{
		// compare alternatives for duration
		if( !LOK() ||    // both alternatives equal?
			qDuration() <= 0 )
		{
			if( NextInVoice )	// next note in same voice ?
			{


				// 1. try to avoid rest between note and next-note
				if( (qAttackpoint() + BestDuration) == NextBestStart )
				{
					dSelection = firstSel;
				}
				else if( (qAttackpoint() + SecDuration) == NextBestStart )
				{
					dSelection = secondSel;
				}
				else if( (qAttackpoint() + BestDuration) == NextSecStart )
				{
					dSelection = firstSel;
				}
				else if( (qAttackpoint() + SecDuration) == NextSecStart )
				{
					dSelection = secondSel;
				}							
				else if( (qAttackpoint() + BestDuration) <= NextLatestStart &&
						(qAttackpoint() + SecDuration) <= NextLatestStart	)
				{
					// in some cases this might be in correct!
					// a) NextLatestStart == wrong and not binary
					// b) | _/12 c/12 _/12 c/4
					TFrac qAt = qAttackpoint();
					if( primeDenomDistance(qAt.denominator(),
										   BestDuration.denominator()) <=
						primeDenomDistance(qAt.denominator(),
									   SecDuration.denominator()) )
					{
						dSelection = firstSel;
					}
					else
					{
						dSelection = secondSel;
					}
				}
				// if rest to next try to keep same duration as prev
				else if( (qAttackpoint() + BestDuration) <= NextLatestStart )
				{
					// SecDiffL = BestDiffL;
					dSelection = firstSel;
				}
				else if( (qAttackpoint() + SecDuration) <= NextLatestStart ) 
				{
					// BestDiffL = SecDiffL;
					dSelection = secondSel;
				}
				else // lgDuration does not fit into gap
				{
					//						printf("WARNING: SetQData ignored DiffT\n");
					//						this->Debug( stdout);
					BestDiffL = (NextLatestStart - qAttackpoint()) - GetDuration();
					SecDiffL = BestDiffL;
					dSelection = firstSel;
					//						this->Debug( stdout);
				}
			} // if( NextInVoice )
			else	// no next note, bestDiffL should be longer duration
			{
				TFrac qAt = qAttackpoint();
				if( primeDenomDistance(qAt.denominator(),
									   BestDuration.denominator()) <=
					primeDenomDistance(qAt.denominator(),
									   SecDuration.denominator()) )
				{
					dSelection = firstSel;
				}
				else
				{
					dSelection = secondSel;
				}
			}
		} // LOK
		else // both version are equal
		{
			dSelection = firstSel;
		}
	} // undef sel
	
	
	
	if( qDuration() <= 0 )
	{
		cout << "WARNING: unresolved setqdata <= 0 at " << qAttackpoint().toDouble() << endl;
		BestDiffT = AbsTime - endPrevious;
		SecDiffT = BestDiffT;		
		BestDiffL = (NextLatestStart - qAttackpoint()) - GetDuration();
		SecDiffL = BestDiffL;
		dSelection = firstSel;
	}
	
	if( qDuration() <= 0 )
	{
		Printf("ERROR: SetQData to duration <= 0!\n");
	}
	
	
	/*
	Because the voice separation is executed before quantization,
	there must not be any overlappings after the quantization
	*/
	TFrac NoteEnd = qAttackpoint() + qDuration();
	
	// CHeck for overlapping
	if( NextInVoice &&
		(NoteEnd > NextBestStart) &&
		(NoteEnd > NextSecStart) )
	{
		// make note shorter to avoid overlapping
		if( qAttackpoint() < NextBestStart )
		{
			BestDiffL = NextBestStart - qAttackpoint() - GetDuration();
			SecDiffL = NextSecStart - qAttackpoint() - GetDuration();
			if( SecDiffL <= 0 )
				SecDiffL = BestDiffL;
			// printf("WARNING: SetQData ignored DiffT!\n");
		}
		else if( qAttackpoint() < NextSecStart )
		{
			BestDiffL = NextSecStart - qAttackpoint() - GetDuration();
			SecDiffL = BestDiffL;
			//				printf("WARNING: SetQData ignored DiffT!\n");
		}
		dSelection = firstSel;
	} // if NoteEnd >=
	else if( TOK()&&			// unique attackpoint solutions
			NextInVoice &&
			NextInVoice->TOK() )
	{
		TFrac qIOI = NextInVoice->qAttackpoint() - qAttackpoint();
		TFrac restSize = NextInVoice->qAttackpoint() - NoteEnd;
		TFrac minRest(1,16);
		if( restSize <= minRest &&
			primeDenomDistance( qAttackpoint().denominator(),
							qIOI.denominator() ) < 1)
		{
			expandTo(qIOI,1);
		}
	}
			
	if( qDuration() <= 0 )
	{		
		Printf("ERROR: SetQData to duration <= 0!\n");
	}
	
	// return best endpoint of note
	return qDuration() + qAttackpoint();
}
// SetQData
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*!      Check Tempo
Check for tempochanges during played notes. If tempo changes are found,
the played not will be splitted.
*/
void TQNOTE::CheckTempo( void )

{
	// jk replaced Next by NextPtr!
	/*
	Correct?
	1. build correct track/sequence
	2. insert tempo notes
	3. Check for overlappings between tempo notes and original notes
	4. split original if needed
	*/
	TFrac 	TimeNext;
	TQNOTE *Next = dynamic_cast<TQNOTE*>(GetNext(-1));
	TimeNext =  Next->qAttackpoint();
	if( TimeNext < ( qAttackpoint() + qDuration() ) ) // split note
	{
		Printf("checktempo : note split!\n");
		// copy values to new note
		Next->SetAll( Next->GetAbsTime(),
			GetDuration() - ( Next->AbsTime - AbsTime ),
			GetMIDIPitch(), //Pitch,
			mIntens,
			mId);
		// make duration shorter
		BestDiffL = Next->qAttackpoint() - qAttackpoint();
		SecDiffL  = BestDiffL;
	} // if( TimeNext ...
} // CheckTempo
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/*
Convert into gmn format and write to file
return:
new numerator
*/
TFrac TQNOTE::Convert( ostream &gmnOut,	// gmn output file
					  TAbsTime preEndtime,	// enpoint of pre-note
					  TFrac  defFrac,		// denominator of pre-note
					  TTRACK *Track0 ) // controll track (tempo, key, meter, ...)
					  
{
	if( atSelection == undefSel ||
        dSelection == undefSel )
    {
        printf("\nWarning: Undefined selection: ");
        Debug(cout);
    }
	TAbsTime duration = qDuration();
	TAbsTime attackpoint = qAttackpoint();
	
	if( preEndtime > attackpoint ) // should not happen
	{
		attackpoint = preEndtime;
		Printf("Quantisation error: Attackpoint shifted! at ");
		Printf(attackpoint);
		Printf("\n");
		gmnOut << "\n(* Quantisation error: Attackpoint shifted!*) ";
	}
	

	//---------------------------------------------------------------
	// search and write all control events >= preEndTime <= absTime
	defFrac = TQuantizedObject::Convert(gmnOut,
		preEndtime,
		defFrac,
		Track0);
	
	{
		
		/// convert all text tags between preEnd and this
		TMusicalObject *textObject = TMusicalObject::GetPrev( GetVoice() ); 
		while( textObject  &&
			dynamic_cast<TMetaText *>(textObject) &&
			textObject->GetAbsTime() >= preEndtime )
		{
			textObject->Convert( gmnOut, 
				0l,
				0l,
				Track0 );
			textObject = textObject->GetPrev(GetVoice()); // returns only text!
		}
		

		// convert all text tags during this
		textObject = TMusicalObject::GetNext( GetVoice() ); 
		while( textObject  &&
			dynamic_cast<TMetaText *>(textObject) &&
			//			!strcmp( textObject->ClassName(), TEXT_CLASS ) &&
			textObject->GetAbsTime() < qAttackpoint()+qDuration() )
		{
			textObject->Convert( gmnOut, 
				0l,
				0l,
				Track0 );
			textObject = textObject->GetNext(GetVoice()); // returns only text!
		}
		
		
		// write duration
		defFrac = TFrac( -1,16 );
		
		// checl for end of ornament
		
		
		// search Track 0 for control events <= endPoint
		// split note
		// write control event
		// write splitted duration
		if( color > 0 )
		{
			const char *colorStr = getColorString( color );
			gmnOut << " \\noteFormat<color=\""<< colorStr <<"\">(";
		}
		
		if( duration <= 0 )
		{
			cout << "ERROR: note duration <= 0 at " << qAttackpoint().toDouble() <<endl;
			gmnOut << "\n(*Error: duration < 0!!  ";
		}
		if( getIntens() > 0 && !dynamic_cast<TQChord *>(this)) // don't write intens inside chords
		{
			writeIntens( gmnOut, getIntens() );
		}
		if( getId() > 0 )
		{
			gmnOut << "\\id<"<< getId() <<">(";
		}
		WritePitch( gmnOut );
		defFrac = WriteDuration( gmnOut,
			duration,
			defFrac );
		if( getId() > 0 )
		{
			gmnOut <<  ") ";
		}
		if( duration <= 0 )
		{
			gmnOut << "*)\n ";
		}
		if( color > 0 )
		{
			gmnOut << " )";
		}
	} // if( duration
	
	gmnOut << ' ';
	// close tag
	
	return TFrac( -1,8);
} // Convert
//----------------------------------------------------------------------
/*
check for legato
result
1 :	if played attackpoint == played endpoind of pre-note
0 : else
*/
int TQNOTE::Legato( void )
{	
	TNOTE *Temp = NOTE(GetPrev( GetVoice() ));
	if( Temp )
	{
		TFrac PrevEnd = Temp->GetAbsTime() + Temp->GetDuration();
		if( PrevEnd == AbsTime )
			return 1;
	}
	return 0;
} // CheckLegato
//----------------------------------------------------
// not tested, not used !!!
/*
avoid overlapping to pre-note by shifting note or
cutting pre-note
result:
1 : overlapping can't be avoided
0 : overlapping is avoided
*/
int TQNOTE::CutOrMove( TAbsTime duration )
{
	if( BestDiffL >=  duration  )	// make note shorter
	{
		BestDiffL -= duration;
		SecDiffL  = BestDiffL;
		return 0;
	}
	else if( Legato() )	// pre-note must be shiftet too
	{
		TQNOTE *TempPrev = QNOTE(GetPrev( 1 ));
		if( TempPrev )
		{
			return 1;
			// must be evaluated
			if( !TempPrev->CutOrMove( duration ) )	// pre not can be moved
			{
				BestDiffT -= duration;
				SecDiffT  = BestDiffT;
				return 0;
			}
			else
			{
				return 1;	// no chance to resolve
			}
			
		}
		else if( (AbsTime + BestDiffT) > duration ) // no pre-note
		{
			BestDiffT -= duration;
			SecDiffT  = BestDiffT;
			return 0;
		}
		else	// no chance
		{
			return 1;
		}
	} // if Legato
	else // no Legato
	{
		TFrac QDuration = GetDuration() + BestDiffL;
		TFrac PrevEnd = 0L;
		TQNOTE *TempPrev = QNOTE(GetPrev( 1 /* search in same voice */));
		if( TempPrev )
			PrevEnd = TempPrev->qAttackpoint() + TempPrev->qDuration();
		if( (AbsTime - duration) >= PrevEnd )	// shift note
		{
			BestDiffT -= duration;
			SecDiffT = BestDiffT;
			return 0;
		}
		else if( QDuration >= ( (TFrac)2* duration ) )	// make note shorter
		{
			BestDiffL = QDuration - duration - GetDuration();
			SecDiffL = BestDiffL;
			return 0;
		}
	} // else
	return 1;// no chance
	
} // CurOtMove


void TQNOTE::ticksToNoteface( int ppq )
{
	TFrac resolution = TFrac( 1, ppq*4);
	TQuantizedObject::ticksToNoteface(ppq);
	mDuration *= resolution;
	BestDiffL *= resolution;
	SecDiffL *= resolution;
}


// write duration as frac to file
TFrac WriteDuration( ostream &gmnOut,
					TAbsTime duration,
					TFrac prev )
{
	long int durNumerator = duration.numerator();
	long int durDenominator = duration.denominator();
	if( durDenominator > 64 )
	{
		durDenominator = durDenominator;
	}
	
	ostringstream num;
	num << durNumerator;	
	string strNumerator = num.str();
	
	ostringstream denom;
	denom << durDenominator;	
	string strDenominator = denom.str();
	
	if( durNumerator != 1 )
	{
		// if duration has changed -> write numerator
		if(	durNumerator != prev.numerator() ||
			durDenominator != prev.denominator() )
		{
			gmnOut << '*' << strNumerator;
		}
	}
	// if duration has changed -> write denominator
	if( durNumerator != prev.numerator() ||
		durDenominator != prev.denominator() )
	{			  
		gmnOut << '/' << strDenominator;
	}
	prev = TFrac(durNumerator, durDenominator);
	gmnOut << ' ';
	
	// return TFrac
	return prev;
}

//----------------------------------------------------------


// count # of attackpoints at attack of now
int countAttacks( TQNOTE *now,
				 TNOTE  * to )
{
	
	
	if(!now )
		return 0;
	
	//#define GLOBAL_SPLITV
	
#ifdef GLOBAL_SPLITV
	// count all attacks of track
	TAbsTime attack = now->GetAbsTime();
	int res = 1;
	TQNOTE *temp = QNOTE(now->GetNext(-1));
	while( temp )
	{
		res++;
		temp = QNOTE(temp->GetNext(-1));
	}
	return res;	
#endif
	
#ifdef LOCAL_SPLITV
	/*  count attacks until next global rest !!! leads to errors in chord distance*/
	/*
	attack = now->GetAbsTime();
	earliestNoteOff = attack + now->GetDuration();
	// count # of attacks in first chord during now
	temp = QNOTE(now->GetNext(-1));
	while( temp && 
	temp != to &&	
	temp->GetAbsTime() < earliestNoteOff)
	{
	res++;
	if( temp->GetAbsTime() + temp->GetDuration() > earliestNoteOff )
	earliestNoteOff = temp->GetAbsTime() + temp->GetDuration();
	temp = QNOTE(temp->GetNext(-1));
	
	  }
	*/
	/* old version with all-overlap slices */
	int res = 1;
	TAbsTime attack = now->GetAbsTime();
	TAbsTime earliestNoteOff = attack + now->GetDuration();
	// count # of attacks in first chord during now
	TQNOTE *temp = dynamic_cast<TQNOTE*>(now->GetNext(-1));
	while( temp && 
		temp != to &&	
		temp->GetAbsTime() >= attack &&
		temp->GetAbsTime() < earliestNoteOff)
	{
		// new, ealier noteOff?
		if( temp->GetAbsTime() + temp->GetDuration() < earliestNoteOff )
		{
			earliestNoteOff = temp->GetAbsTime() + temp->GetDuration();
			// start again
			res = 1;
			temp = now;
		}
		else
		{
			res++;
		}
		temp = dynamic_cast<TQNOTE*>(temp->GetNext(-1));
	} // while
	return res;
#endif
} // counAttacks

  /*
  remove all overlappings to succeeding notes in sVoice
  Todo check if this is still used?
*/
void TQNOTE::removeLegato(int sVoice)
{
	TFrac minAttack = GetAbsTime() + GetDuration();
	TQNOTE *temp = QNOTE(GetNext(sVoice));
	if( temp ) // do nothing if no successor
	{
		while( temp )
		{
			if( temp->GetAbsTime() < minAttack &&
				temp->GetAbsTime() > GetAbsTime() )
				minAttack = temp->GetAbsTime();
			temp = QNOTE(temp->GetNext(sVoice));
		}
		SetDuration(minAttack - GetAbsTime());	
	} // for all notes in voice
}


#define PITCHDECAY 0.8
/*
return interval between this->pitch and pitch2	
*/
double TQNOTE::deltaPC(int pitch2, int lookBack)
{
	
	// printf("<qn "); fflush(stdout);
	int p = GetMIDIPitch();
	double av = p;
	
	// printf("2:%d, ",p); fflush(stdout);
	int count = 1;
	//printf("<do "); fflush(stdout);
	TQNOTE  *prev = QNOTE(GetPrev(GetVoice()));
	while( lookBack > 0 && prev )
		// get average pitchdistance to previous notes
	{
		//printf("<av "); fflush(stdout);
		
		int pPrev = prev->avMIDIPitch();
		//printf("av>"); fflush(stdout);
		{
			av *= PITCHDECAY;		
			av += (double)pPrev * (1-PITCHDECAY);
			lookBack--;
			count++;
		}
		prev = QNOTE(prev->GetPrev(GetVoice()));
	} // while
	//printf("do>"); fflush(stdout);
	
	// todo : make weighted pitch 
	int res = pitch2 - av;
	if( res < 0 )
		res *= -1;
	//printf("qn>\n"); fflush(stdout);
	return res;
}


double TQNOTE::overlapDistance(TAbsTime attack2,
							   TAbsTime duration2,
							   TMIDIFILE *theMidifile)
{
	
	// overlap = min(offset(), attack2+duration2) - max(GetAbsTime(),attack2);
	TFrac overlap;
	if( GetAbsTime() <= attack2 )
	{
		overlap = offset() - attack2;
	}
	else
	{
		overlap = attack2+duration2 - GetAbsTime();
	}
	
	double res = 0;
	/// calc relation between old length and cut-length
	if( overlap.toDouble() > 0L )
	{
		// overlap <= duration because of sort
		/// penalty for ms duration
		double ovlDurMS = DTIMEtoMS(theMidifile->Ppq(),
			theMidifile->Tempo(),
			overlap.toDouble() );
		/// overlap 2s -> penalty 0.8 -> 0.80472
		

		double absPenalty = 1- exp(-ovlDurMS/1000 * 0.80472 );
//		pRel = 0;
		// prop to overlap
		absPenalty = 1 - GaussWindow(ovlDurMS/1000.0, 0.5); 
//		absPenalty = 1 - GaussWindow(ovlDurMS/1000.0, 0.5); 
		double newDur = GetDuration().toDouble() - overlap.toDouble();
		double newDurMS = DTIMEtoMS(theMidifile->Ppq(),
									theMidifile->Tempo(),
									newDur );
		// propto 1/newDur
		double absPenalty2 = GaussWindow(newDurMS/1000.0, 0.5);		
		
		// res = res + (1-res)*absPenalty;
		// res = max(pRel, absPenalty);
		res = absPenalty * absPenalty2;

		// short note -> high penalty for short overlap,
		// long note -> 
	}
	return res;
}

double TQNOTE::avMIDIPitch()
{
	return GetMIDIPitch();
}


int TQNOTE::closestPitch(int /* pitch2 */)
{
	return GetMIDIPitch(); // QNOTE includes only 1 pitch info
}



// ------------------------------------
#ifdef PENALTY_CHECK 
// toolfunctions for penalty checker
#include "voiceinfo.h"
double TQNOTE::pPitch( void )
{
	double interval;
	TQNOTE *prev = QNOTE(GetPrev( GetVoice() ));
	if( prev )
	{
		interval = prev->pitchDistance(GetMIDIPitch());
	}
	else
	{
		TInifile *tempIni = GetInifile();
		if(tempIni )
		{
			interval = tempIni->GetValInt("EMPTYVOICEIV","11","start interval (penalty) in empty voice");
		}
		else
		{
			interval = 11;
		}
	} // if prev
	double res = pitchPenalty( interval );
	return res;
}
double TQNOTE::pChord( void )
{
	double res = 0;
	return res;
}

#endif




TFrac TQNOTE::latestStart()
{
	TAbsTime s1 = AbsTime + BestDiffT;
	TAbsTime s2 = AbsTime + SecDiffT;
	if( s1 > s2 )
		return s1;
	return 2;
}




void TQNOTE::init()
{
	BestDiffL = 0L;
	SecDiffL  = 0L;
	BestDiffT = 0L;
	SecDiffT  = 0L;
	dSelection = undefSel;
	sustainOffset = offset().toLong();
}
