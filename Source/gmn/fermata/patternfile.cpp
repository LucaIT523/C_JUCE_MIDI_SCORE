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
|	Date	  	 : 17.10.1996-98-03
|	2003/06	similarity functions
------------------------------------------------------------------*/

#include <ctype.h>
#include <string>
#include "patternfile.h"
#include "funcs.h"
#include <stdlib.h>
#include "c_track.h"
#include "liste.h"
#include "h_midi.h"
//-----------------------------------------------------------------
TPFILE::TPFILE( const char *name ) : TSPFILE( name ), 
	typicalMatchDistance(patDistanceLimit)
{
	cUsed = 0;
	cUsedCur = 0;
	weightList = new TDoubleBinList(99,101, 0.05);
	weightList->setMinMaxRel(30.0);
	weightList->setMinweightDeltaRel(30.0);
	weightList->createDistribution();

} // TPFILE

//-----------------------------------------------------------------
TPFILE::~TPFILE( void )
{
	delete weightList;
} //~TPFILE
//-----------------------------------------------------------------
//-----------------------------------------------------------------
/*!
	Compare note with all patterns
	result:
 ptr : best matching pattern
 1. criteria : # mathcing notes
 2. criteria : sum of distances
 0 : no matching pattern

 */
/* not used anymore
TPATTERN *TPFILE::Compare( TQNOTE *note,
						  TFrac length)
{
	TQNOTE    *Now;
	TPATTERN
        /// current pattern
        *Pattern,
        /// best matching pattern
        *MinPattern;	
	int i,
		 MaxNotes = 0,
		 Temp3;

	TFrac
        /// sum of distances for duration
        DiffLength,
        /// sum of distances for attackpoints
        DiffStart;		
	TFrac
        /// distances of best pattern
        MinLength = 2000,	
		  MinStart  = 2000;
	 TFrac	  Temp2,
		  Temp1;
	TFrac	  dummy;
	char /// Flag
        Pass;		


	Now        = note;
	Pattern    = (TPATTERN*)firstPattern();
	MinPattern = NULL;

	i = 0;
    /// compare all patterns
    while( Pattern )	
	{
		if( Pattern->GetLength() == length )
		{
			DiffLength = 0L;
			DiffStart  = 0L;
			i++;
			Pattern->Compare( Now,
				  &DiffLength,
				  &DiffStart,
				  &Pass,
				  &dummy );
			if( Pass )	// matching pattern
			{
				Temp1 = DiffLength + DiffStart;
				Temp2 = MinLength  + MinStart;
				Temp3 = Pattern->cNotes();

// 1. critera: nr of matching notes
				if( Temp3 > MaxNotes )
				{
					MaxNotes   = Temp3;
					MinLength  = DiffLength;
					MinStart   = DiffStart;
					MinPattern = Pattern;
				}
				else if( Temp3 == MaxNotes )
				// 2. criteria: sum of distances
				{
					if( abs( Temp1 ) < abs( Temp2 ) )
					{
						MaxNotes   = Temp3;
						MinLength  = DiffLength;
						MinStart   = DiffStart;
						MinPattern = Pattern;
					}
				} // else if
			} // if( Pass)
		}
		Pattern = (TPATTERN *)nextPattern();	//next pattern
	} // for( i...
	return MinPattern;
}// TPFILE::Compare
*/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
/// Used for pattern based tempo detection
//! now unused
//! todo english remarks in pattern.pp
/*!
	Parameter :
		firstdiff = Abstand der Attackpoints der ersten beiden
				 Noten, die quantisiert werden sollen
		firstlength = Tondauer der ersten Note, die quantisiert
				 werden soll
			    Einheit = Ticks
		tempo     = Das bei der Aufnahme eingestellte Tempo der Datei
		fileppq   = Aufl”sung mit der die Datei eingespielt wurde
*/

double TPFILE::updateTypicalMatchDistance( double newDistance)
{
	#define alpha 0.8
	typicalMatchDistance = typicalMatchDistance * alpha + 
							newDistance *(1-alpha);
	#undef alpha 
	return typicalMatchDistance;
}

#ifdef sfjksdjhksjdhf
char TPFILE::FindPpq( TQNOTE *firstN,
                      int  tempo,
                      int  fileppq )
{

	TAbsTime FirstDiffP,
		  FirstDiff = 0L,
		  FirstLength;

	TFrac  NewPpq;
	int	  RealTempo;	   // Tempo in dem das Stück wirklich gespielt wurde

	TQNOTE *SecondN;

	TPATTERN *Ptr;
	TPNOTE   *FirstPN,
		 *SecondPN;


	SecondN = QNOTE(firstN->GetNext(-1/* all voices */));
	if( SecondN )
	{
		FirstDiff = SecondN->GetAbsTime() -
			    firstN->GetAbsTime();
	}
	FirstLength = firstN->GetDuration();

	Ptr = (TPATTERN*)firstPattern();
	while( Ptr )	// Mit allen Pattern vergleichen
	{

		FirstPN  = Ptr->FirstNote();
		SecondPN = Ptr->NextNote();

		if( SecondPN )
		{
			FirstDiffP = SecondPN->GetAbsTime() -
					 FirstPN->GetAbsTime();
			TFrac temp = (TFrac)Ppq *
					(FirstDiff/FirstDiffP);

			NewPpq  = temp;
		}
		else	// Pattern besteht nur aus einer Note
		{
			FirstDiffP = FirstPN->GetDuration();
			TFrac temp	 = (TFrac)Ppq *
					(FirstLength/FirstDiffP);
			NewPpq  = temp;

		}

		RealTempo = (double)tempo * ((double)fileppq/NewPpq.toDouble());
		if( (RealTempo < MAXTEMPO) && (RealTempo>MINTEMPO) )
		{
			 ChangePpq( NewPpq.toLong() );
		}
		Ptr = (TPATTERN *)nextPattern();
	} // while

	return 0;
} // FindPpq
#endif
//-----------------------------------------------------------------

//-----------------------------------------------------------------
/// read .gmn file and init counters
char TPFILE::Read( void )
{
	char res = TSPFILE::Read( /* ppq*/ );
	// check for low level notes

	TPATTERN *temp = dynamic_cast<TPATTERN *>(firstPattern());
	while( temp )
	{
	  lgTag *noteTag = temp->firstTag( "\\note");
	  int maxVoice = 0;
	  if( noteTag )
	  {
		for( int i = 0; i <= maxVoice; i++ )
		{
			int firstInVoice = 1;
			lgNote *prevNote = NULL;
			noteTag = temp->firstTag( "\\note");
			while( noteTag )
			{
				// fill with low level notes
				double perftime = 0,
					   perfduration = 0,
					   susduration = 0;
				long int st;
				int p,
					v,
					vel,
					bar,
					beat;
			
				if( readLNote( noteTag,
							   perftime,
							   perfduration,
							   susduration,
						  	   st,
						 	   p,
						       vel,
						       v,
						       bar,
						       beat) )
				{
					if( v == i )
					{
						if( firstInVoice )
						{
							appendSequence();
							firstInVoice = 0;
							CPatternP++;
						}
						lgNote *curNote;
						if( st >= 0 )
						{
							// p -= 4;
							int octave = p / 12;
							p %= 12;
							octave -= 4;

							curNote = factory->newNote( p,
									  octave, // dummy
									  0, // acc dummy
									  1, // durNdummy
									  64, // durDdummy
									  0, // dots
									  st, // posN
									  96); // pos D
							if( prevNote )
							{
								lgDuration dur = curNote->pos() - prevNote->pos();
								if( dur > 0 )
								{
									if( bar )
									{
										// create a barline
										appendTag(0L, "bar");
									}
									prevNote->setDuration( dur, 0 );
								    prevNote = curNote;
									appendEvent( curNote );
								 }
								 else // overlap
								 {
									string tagtext = "\\text<\"";
									// todo remove this again tagtext += curNote->toString();
									tagtext += "\">";
									 appendTag(0L, tagtext.c_str() );
								 	// todo: create chord
								 	delete curNote;
								 }
							} // if prev Note
							else
							{
								if( st > 0 )
								{
									lgRest *emp  = factory->newRest( 
											  st, // durNdummy
											  96, // durDdummy
									  			0, // dots
											  0, // posN
											  0); // pos D
									appendEvent( emp );								
								}
								prevNote = curNote;
								appendEvent( curNote );
							} // else (no prev note)
						} // if valid st
					} // if v==i
					else if( v > maxVoice )
					{
						maxVoice = v;
					}
				} // if readLNote
				noteTag = temp->nextTag( noteTag );		
			} // while
			if( prevNote )
			{
				lgDuration dur(1,16);
				prevNote->setDuration(dur,0);
			}
		} // for
		} // if
		cUsedCur++;
		cUsed += (temp->cUsed()+1);

		// create distribution
		int id = temp->GetID();	
		TDoubleBinClass *curClass = weightList->addClass(id, temp->cUsed() );
		temp->binclass = curClass;
		temp = dynamic_cast<TPATTERN *>(nextPattern());
	} // while
	if( CPattern() )
	{
		weightList->setMinMaxRel(30);
		/// minWeight = 1/(max/min + k -1)
		weightList->setMinweightDeltaRel(30);
	}
	weightList->createDistribution();
//	weightList->write(stdout);
	return res;
}
//-----------------------------------------------------------------


void TPFILE::debug( FILE *out   )
{
	if( !out )
	{
		out = stdout;
	}
	lgSegment::write( out );
}

/*!
	calc similarity of best fitting pattern
	bias exact similarity with atAccuracy and dur Accuracy
*/
CDynaprog<TQNOTE *, TPNOTE *> *TPFILE::similarity( TQNOTE *note,
													double &sim,
													TFrac &minStart,
													int &patternID,
													double atAccuracy,
													double durAccuracy)
{
	sim = -1;
	double curSim = 0;
	double realSim;	// unbiased similarity == result for sim	

	TPATTERN *res = NULL;

	CDynaprog<TQNOTE *, TPNOTE *> *curDynaprog,
								  *dynaprog = NULL;
	TPATTERN *cur    = (TPATTERN*)firstPattern();
	patternID = -1;
    /// compare all patterns

    while( cur )	// for all pattern
	{
		curDynaprog = cur->similarity(note, 
									  curSim, 
									  minStart,
									  atAccuracy,
									  durAccuracy);
		// do accuracy biasing
		double tempSim = curSim;
		// Todo do pUsed biasing
		/*
		curSim = avAccuracy*pUsed(cur) +
							  (1-avAccuracy)*curSim;
		*/					   
		if( curSim > sim )
		{
			sim = curSim;
			res = cur;
			realSim = tempSim;	// keep the unbiased version
			if( dynaprog )
			{
				dynaprog->delOutVal();
				delete dynaprog;
			}
			dynaprog = curDynaprog;
			patternID = cur->GetID();
		}
		else if( curDynaprog )
		{
			curDynaprog->delOutVal();
			delete curDynaprog;
		}
		cur = (TPATTERN *)nextPattern();	//next pattern
	}
	sim = realSim;
	return dynaprog;
}


void TPFILE::incCUsed(TPATTERN *pat,
					  int delta)
{
	weightList->addExact(pat->GetID());
/*
	fprintf(stdout, "%d\n", pat->GetID());
	weightList->write(stdout);
	fflush(stdout);
*/
	/*
	cUsedCur += delta;
	pat->incCUsed(delta);
	*/
}

double TPFILE::pUsedBefore(TPATTERN *pat)
{
//	return pat->aPriori( cUsed );
	return (pat->cUsed() + 1) / (cUsed + CPattern());

}

double TPFILE::pUsedCur(TPATTERN *pat)
{
	return pat->curWeight();
	// return (pat->cUsedCur()+1) / (cUsedCur + CPattern());
}

void TPFILE::finaliseCUsed()
{
	TPATTERN *temp;
	temp = dynamic_cast<TPATTERN *>(firstPattern());
	while( temp )
	{
		temp->finaliseCUsed();
		temp = dynamic_cast<TPATTERN *>(nextPattern());
		
	}
}

double TPFILE::pUsed(TPATTERN *pat)
{
	return pat->curWeight();
//	return (pat->cUsedCur() + pat->cUsed() + 1) / (cUsedCur + cUsed + CPattern());
}

 int comparePDistance( const void *p1,
                       const void *p2 )
 {
     if( ((TPatternDistance *)p1)->distance > ((TPatternDistance *)p2)->distance )
         return 1;
     else if ( ((TPatternDistance *)p1)->distance < ((TPatternDistance *)p2)->distance )
         return -1;
     return 0;
 }

TPATTERN * TPFILE::bestIOIratioMatch(TCLICKNOTE *from, 
							 double &distance,
							 TDoubleBinList *IOIratioList,
							 TFracBinList *durationList,
							 TCLICKTRACK *clicktrack,
							 TPatternDistance *pArray,
							 int lSize,
							 // last perfIOI, scoreIOI relation
							 double normF,
							 double normFSigma
							 )
{


	int i = 0;
	TPATTERN *temp = dynamic_cast<TPATTERN *>(firstPattern());
	// fille the list
	double dSum = 0;
	while( temp )
	{
		#ifdef _DEBUG
		int d = temp->GetID();
	if( from->GetAbsTime().numerator() == 10564)
		printf("");
		#endif
		
		double headDist = temp->headDurationDistance(from,
								       IOIratioList,
								       durationList,
								       clicktrack,
									   normF,
									   normFSigma);
		double ratioDist;
		if( headDist < 0.8 )
		{
			ratioDist = temp->IOIratioDistance(from);
			if( ratioDist > 0.6 )
			{
				ratioDist = 1;
				headDist = 1;
			}
		}
		else
		{
			ratioDist = 1;
			headDist = 1;
		}
									   
		/*
		double dist = (1-ratioDist) * (1-headDist);
		dist = 1-dist;
		*/
		// head Dist must stay original otherwhise bad distance errors
		// headDist *= headDist;
//		headDist *= headDist;
		double normDistance = ratioDist + headDist - ratioDist*headDist;

		/*
		printf("ID:%d, d:%f\n", temp->GetID(),
								dist);
		*/
		if( normDistance < 0 )
			ratioDist = temp->IOIratioDistance(from);

		double p2 = pUsedBefore(temp);
		double p3 = pUsedCur(temp);
		double p = p2*0.2 + p3*0.7;
		// p == 0...1 (bad ... good)
		// p == 1 ...0.7 (bad good)
		// there are problems with p overriding ratioDist and p 
		// changing overall average distance!!
		// increase minWeight of list?
		// p = 1 - p3* 0.5;		
		p = 0.8;
		
		
		// mke distance smaller if pattern was used before	
		double dist = 1;
		if( normDistance < 0.999 //&& p > 0
		)
		{
			p =  0.5 + 0.5 * temp->distancePenalty(normDistance, 
													patDistanceLimit / 3.0 );
			p = 0.8;
			dist = p * normDistance;
		}
	
		
//#define _DEBUG_PATTERN
#ifdef _DEBUG_PATTERN
FILE *out = fopen("_pattern.txt","at");
fprintf(out,"p:%1.3f=>%1.3f ",p, dist);
fclose(out);
#endif

		// bayes adaption by cemgil
		/*
		if( dist >= 1 )
			dist = 0.9999999;

		 dist = -log(1-dist) + log( 1/p);
		*/
		dSum += dist;

// Todo: turn pUsedBefore on
		if( i < lSize )
		{
			pArray[i].ptr = temp;
			pArray[i].distance = dist;
			pArray[i].normDistance = normDistance;
		}
		else if( dist < pArray[lSize-1].distance ) 
		{
			// replace least pattern and resort 
			pArray[lSize-1].ptr = temp;
			//Todo:  evaluate "weight" of pattern!!
			pArray[lSize-1].distance = dist;
			pArray[lSize-1].normDistance = normDistance;
			
			qsort(pArray, lSize, sizeof(TPatternDistance), comparePDistance );	
		}
		i++;
		temp = dynamic_cast<TPATTERN *>(temp->next());
	} // while
	
	// normalise dist's and create a distribution
	// fille the list
	/*
	// normalise and create a dirstribution?
	for( i = 0; i < lSize; i++ ) 
	{
			pArray[i].distance /= dSum;			
	}
	*/
	// printf("-------------------\n");
	// re-sort
	qsort(pArray, lSize, sizeof( TPatternDistance), comparePDistance );
	return pArray[0].ptr;
}

TPATTERN *TPFILE::bestQMatch(TQNOTE *note, 
							TFrac &minStart, 
							double &distance, 
							double atAccuracy, 
							double durAccuracy,
							TFrac patternLength,
							TFrac curMeter)
{


	TPATTERN *cur    = (TPATTERN*)firstPattern();
	// best fit pattern
	distance = -1;
//	double avAccuracy = (atAccuracy+durAccuracy)/2;
	TPATTERN *res = NULL;
    /// compare all patterns
    while( cur )	// for all pattern
	{
		TFrac cDur(cur->duration());
		if( patternLength <= 0  ||
			cDur % patternLength  == 0L ||
			patternLength % cDur == 0L
			)
		{
			double curDistance =  cur->IOIDistance(note,
									          minStart,
											  curMeter,
											 atAccuracy,
											 durAccuracy);


/*
			
			similarity(note, 
									  curSim, 
									  minStart,
									  atAccuracy,
									  durAccuracy);
									  */
		// do accuracy biasing
		// printf("%d, %f: ", cur->GetID(), curSim);

		// Todo do pUsed biasing
		/*
		curSim = avAccuracy*pUsed(cur) +
							  (1-avAccuracy)*curSim;
		*/					   
			if( distance < 0 ||
				curDistance < distance )
			{
				res = cur;
	//			realSim = tempSim;	// keep the unbiased version
				distance = curDistance;
			}
		} // if correct length
		cur = (TPATTERN *)nextPattern();	//next pattern
	}
	return res;
}
