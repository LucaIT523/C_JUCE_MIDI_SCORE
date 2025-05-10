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


#include <sstream>
using namespace std;
#include <stack>
#include "similarity.h"

#include "../lib_src/similarity/simMatrix.h" /// do this first for min
#include "../lib_src/similarity/dynaprog.h"

#include "alignment.h"
#include "alignment.cpp"

#include "click_similarity.h"
#include "patternfile.h"
#include <math.h> 

#include "c_track.h"

#include "q_chord.h"
#include "note.h"
#include "funcs.h" // dtime2ms
#include "track.h" //TTRACK
#include "../lib_src/ini/ini.h"
#include "statist.h"

#include "notestatlist.h"
#include "import.h"

// positive val for dynaprog
#define dynPVal 0

/// because IOIratio(lgNote) retrive rIOI of lgNote->next
/// we write here pitch of ptr->next!
void writePitch( ostream &out /* out*/
				, TCLICKNOTE * /*ptr*/)
{
};


//! compare v1[0...windowSize) and v2[0...windowSizw)
//! return [-1...0] (no match... total similarity)
template<class U,class V>
double _stdcall simClickNote(U *v1,
                    V *v2,
                    int windowSize);



//! return a double value derived from v1
double _stdcall val( TCLICKNOTE *v1)
{
    if( v1 )
        return v1->IOIratio(-1,1);
    else
        return 0;
}

//! compare x and y, 
//! return [-1...0] (no match... total similarity)
double  _stdcall simDyna(TCLICKNOTE *x, 
						 TCLICKNOTE *y, 
						 TCLICKNOTE *gap,
						 int cGaps,
						 int cLocalGaps,
						 int alLength); /// score matrix / distance function
/*!
	  calculate the similarity of two notes.
	  The similarity function must be context free (dynamic programming)! 
	  res = (-1..0]
*/
double  _stdcall simDyna(TCLICKNOTE *x, 
						 TCLICKNOTE *y, 
						 TCLICKNOTE *gap,
						 int cGaps,
						 int cLocalGaps,
						 int alLength) /// score matrix / distance function
{
	double res = 0;
	if( x == y ) // don't allow self similar alignment!
	{
		res = -1;
	}
	else if( x == gap || y == gap ) // is one NOTE a gap?
	{
		//! prefer gaps aligned to small notes
		double duration;
		if( x == gap )
		{
			ShowErrorMsg("similarity is currently broken!");
			// use [ms] IOI for penalty 
			/*
			duration = DTIMEtoMS( ctrlTrack->Parent()->Ppq(),
					ctrlTrack->GetTempo( y->GetAbsTime()),
					y->ioi(1, y->GetVoice()).toLong()
				);
				*/
		}
		else if( y == gap )
		{
			ShowErrorMsg("similarity is currently broken!");
			// use [ms] IOI for penalty 
			/*
			duration = DTIMEtoMS( ctrlTrack->Parent()->Ppq(),
					ctrlTrack->GetTempo( x->GetAbsTime()),
					x->ioi(1, x->GetVoice()).toLong()
				);

				*/
		}
		// 50ms / duration
		/*
		if( duration > 50 )
		res = 50/duration; // penalty if duration gets longer
		else
		res = 1;	// no penalty if duration is smaller than "equal" distance
		// transform into  output range
		// res = (0...1]
		res = 1/res - 1;
		// res = 0...oo
		if( res < 0 )
		res = -1/res;
		// (0..oo)
		*/
        // TODO, should never be called in current version!
		if( duration < 50 )
		{
			res = 0;
		}
		else
		{
			res = 1 - GaussWindow(duration-50, 500);
			// res = 1-exp(-res/100);
		}
		res *= -1;		
		// now res = (-1..0] == no similarity ... equal
	}
	else // both notes are no gaps
	{
		// test: use simMatrix values
		
		// res = relSimilarity(x,y) * -1;
		res = simClickNote(x,y,
			1 //! windowSize == 1 -> only a single note
			);
		if( res < 0 )
			res *= -1;
		res = (1-res) * -1;
	}
	// normalise to range -1..0 -> -1 ..k to prefer very good matches
	{
		double k = dynPVal;
		res *= (1+k);
		res += k;
	}	
	return res; 
}




/*!
similarity function for similarity matrix
sim = f(lgDuration, IOI, rIOI, weight, patternLength)
result = (0..1] = no match ... perfect match
this function might be context sensitive!
*/
template<class U,class V>
double _stdcall simClickNote( U *v1,
		V *v2,
					int windowSize)
{
	TCLICKNOTE *p1 = dynamic_cast<TCLICKNOTE *>(v1->GetPrev(-1)); 
	TCLICKNOTE *p2 = dynamic_cast<TCLICKNOTE *>(v2->GetPrev(-1));
		double res = 1;
		int i;
		for( i = 0; i < windowSize; i++ )
		{
			/// IOIratio sim
			double i1 = v1->IOIratio(-1,1);
			double i2 = v2->IOIratio(-1,1);
			
			if( validIOIratio(i1) )
				i1 = normIOIratio(i1);
			if( validIOIratio(i2) )
				i2 = normIOIratio(i2);

			double c = GaussWindow(i1,i2,0.7);			

			res *= c;
			p1 = v1;
			p2 = v1;
			v1 = v1->getNext(1,-1); 
			v2 = v2->getNext(1,-1); 
		} // for 
	return res;
}





/*!
This is actaully a gapped BLAST implementation!
1. create window-sied simMatrix 
2. look for very best entries
3. do alignments for best entries in BOTH directions
3a) stop alignment at threshold

  
	Issues:
	- find window size
	- find seed note for alignment inside window
	- find threshold for abort of dynaprog path
	
*/
void selfSimilarity( TCLICKNOTE *from, 
					TCLICKNOTE *to,
					int windowSize,
					int stepSize,
					TTRACK *cTrack,
					const char *midiFilename)
{
	if( !from )
		return;
		
	
	//printf("Start self-similarity analysis\n");
	// create vector
	int cnotes = 0;
	TMusicalObject  *temp = from;
	// count notes
	while( temp &&
		temp != to )
	{
		cnotes++;
		temp = temp->GetNext(temp->GetVoice());
	}
	
	int entries = cnotes-2; // skip first and last because IOIratio is undefined
	if( entries > 0 )
	{
	
	TInifile *alIni = new TInifile(SIMMATRIX_FNAME(midiFilename,".al.ini").c_str());
	// create array of pointers
	PTCLICKNOTE *xVector = new PTCLICKNOTE [entries];
	
	// fill ptr-vector
	temp = from;
	int i;
	for( i = 0; i < entries; i++ )
	{
		xVector[i] = dynamic_cast <TCLICKNOTE *> (temp);
		temp = temp->GetNext(temp->GetVoice());
	}
	
	/// create the window based similarity matrix
	CSimMatrix<PTCLICKNOTE, PTCLICKNOTE> simMatrix(entries,	/// |x-vector|
		entries,	/// |y-vector|
		simClickNote,		/// sim function
		val,		/// val function
		val
		);
	simMatrix.setXVector(xVector);
	double mean, var;
	double processTime;
	simMatrix.calcTable(windowSize, 
						stepSize,
						&mean,
						&var,
						&processTime);
	
	FILE *out = fopen(SIMMATRIX_FNAME(midiFilename, "csim.txt").c_str(),"at");
	if( out )
	{
		fprintf(out,"clicknote selfsimilarity\n"); 
		fprintf(out,"#Entries = %d (%f)\n",entries, (double)entries/(double)stepSize);
		fprintf(out, "WindowSize = %d\n",windowSize);
		fprintf(out,"SetpSize = %d\n",stepSize);
	}
		
	double  my,
			sigma,
			sigmaLess,
			sigmaGreater;
	simMatrix.calcDistributionParams( &my,
									  &sigma,
									  &sigmaLess,
									  &sigmaGreater);
	/// start alignment search
	//! get list of best matches 
//	double limit = my + sqrt(sigmaGreater);
	int lSize;
	double limit = my + sigmaGreater;
	Cxy *bestList = simMatrix.getBestIDs(limit,		/// limit
										windowSize,	/// min distance between two hits
										&processTime,
										&lSize);
	
	stringstream outStr;
	simMatrix.debug(limit, &outStr);
	fprintf(out, "%s", outStr.str().c_str());
	// count number of high scoring positions
		int e2 = (double)entries/(double)stepSize;
		printf(" %d (%f%%) high scoring windows in %d x %d found!\n",
			lSize,
			(double)lSize/(double)(e2*e2/2),
			e2,e2);
	i = 0;
	
	FILE *pdatabase = fopen("_pdatabase.gmn","wt");
	fprintf( pdatabase, "(* Aligned patterns from similarity analysis! *)\n");
	fprintf( pdatabase, "(* Only the relational IOIs of these pattern should be evaluated! *)\n");
	fprintf( pdatabase, "{");
	char firstPat = 1;
	
#ifdef _DEBUG
		double *alDebugMatrix = createDebugMatrix( entries, entries );
#endif				
	
	// Try alignment for all entries in best lists
	fprintf(out,"Best matches (window=%d):\n", windowSize);
	while( bestList[i].x > -1 )
	{
		if( bestList[i].x < entries+1 )
		{
			//! debug matrix match
			fprintf(out, "(%d>%d:%d %f)<",i, bestList[i].x, bestList[i].y,
				simMatrix.V(bestList[i].x, bestList[i].y) );
			//print x pattern
			int j;
			for( j = 0; j < windowSize; j++ )
			{
				fprintf(out, "%3.3f", val(xVector[bestList[i].x+j]));
				// xVector[bestList[i].x+j]->WritePitch(out);
				fprintf(out, " ");
			}
			//print y pattern
			fprintf(out,">\n                  <");
			for( j = 0; j < windowSize; j++ )
			{
				fprintf(out, "%3.3f", val(xVector[bestList[i].y+j]));
//				xVector[bestList[i].y+j]->WritePitch(out);
				fprintf(out, " ");
			}
			fprintf(out,">\n");
		}		
		
		
		
		/// make left-right alignment for this entry
		
		
		int x = bestList[i].x;
		int y = bestList[i].y;
		
		if( x < entries+1 && 
			y < entries+1 )
		{
			//			printf("start at %d,%d\n",x,y);
			double processTime;
			TAlignmentArray<TCLICKNOTE, TCLICKNOTE>  align = makeLRAlignment( xVector,
				xVector,
				entries,
				entries,
				x,
				y,
				-0.6,		///  delta similarity threshold
				-1.5,			/// abs limit
				&processTime,
				val,
				val,
				simDyna
				);
			
			
			// debug alignment
			fprintf(out,"\nDynaprog table (%d,%d):\n",
				x,y);
			stringstream outStr;
			align.debug( outStr );
			fprintf(out, "%s", outStr.str().c_str());
#ifdef _DEBUG
			addToDebugMatrix( alDebugMatrix,
					   		  &align,
							  x,
							  y,
						      entries,
						      -1 );

#endif			


			
			/// delete all bestList entries in range of alignment in same diagonal
			{
				int i2 = i+1,
					delta = y-x;
				int len = align.rightAl1Len();
				while( bestList[i2].x > - 1 &&
					//					   i2 < i+windowSize)
					i2 < i+len)
				{
					
					if( abs(bestList[i2].x + delta - bestList[i2].y) < 3  )
						bestList[i2].x = entries+1; // make invalid					
					i2++;
				} // while
			}
			
			/// calc siginificance and complexity of alignment (# of reps, length, total similarity)
			
			
			int alLength = align.alignmentLen();
			
			
			TDoubleBinList *IOIList1 = new TDoubleBinList(0.99,1.01,20);
			TDoubleBinList *IOIList2 = new TDoubleBinList(0.99,1.01,20);
			int alSim = 0;
			/// get total similarity
			int j;
			int cGaps = 0;	// count the gaps
		
			for( j = 0; j < alLength; j++ )
			{
				TCLICKNOTE *ptr1 = align.v1(j);
				TCLICKNOTE *ptr2 = align.v2(j);
			
				double rIOI1,
						rIOI2;
							
				double localSim = 0;
				if( ptr1 == NULL )	// gap
				{
					rIOI2 = ptr2->IOIratio(-1,1,ptr2->GetVoice());
					rIOI1 = 0;
					localSim  = rIOI2;
					// todo add penalty of neighbours
					cGaps++;
				}
				else if (ptr2 == NULL )
				{
					rIOI2 = 0;
					rIOI1 = ptr1->IOIratio(-1,1,ptr1->GetVoice());
					localSim  = rIOI1;
					// todo add penalty of neighbours
					cGaps++;
				}
				else
				{
					rIOI1 = ptr1->IOIratio(-1,1,ptr1->GetVoice());
					rIOI2 = ptr2->IOIratio(-1,1,ptr2->GetVoice());
					if( rIOI1 != 0 &&
						rIOI2 != 0)
					{
						localSim  = rIOI1 -rIOI2; 
					}
				}
				/// if no rIOI could be calculated, localsim == 0 
				localSim = exp( - fabs(localSim) /* /5 */);
				/// todo evaluate weight/significance of notes?
				
				//				alSim *= (localSim / 0.8 );
				alSim += localSim;
				
				/// calc complexity
				if( rIOI1 == 0) 
				{
					rIOI1 = rIOI2;
				}
				if( rIOI2 == 0 )
				{
					rIOI2 = rIOI1;
				}
				/// add to closest noteStatList
				TDistanceStruct dist;
				
				dist = IOIList1->closestClass(&rIOI1);
				if( dist.distance > 0.1 )
				{
					dist.classPtr = IOIList1->addClass(rIOI1);
				}
				IOIList1->addValue(rIOI1, dist.classPtr );
				
				dist = IOIList2->closestClass(&rIOI2);
				if( dist.distance > 0.1 )
				{
					dist.classPtr = IOIList2->addClass(rIOI2);
				}
				IOIList2->addValue(rIOI2, dist.classPtr );
				
				
			} // for complete alignment
			outStr.clear();
			IOIList1->write(&outStr);
			fprintf(out, "%s", outStr.str().c_str());
			fprintf(out, "c1: %d\n",IOIList1->Count() );
			outStr.clear();
			IOIList2->write(&outStr);
			fprintf(out, "c2: %d\n",IOIList2->Count() );
			fprintf(out, "%s", outStr.str().c_str());
			
			/// write to pattern-database -----------------------------------
			/// todo: remove duplicated patterns!!!
			if( IOIList1->Count() > 2 ||
				IOIList2->Count() > 2 )
			{
				
				if( !firstPat ) 
					fprintf(pdatabase, ",\n[ ");
				else
				{
					fprintf(pdatabase, "\n[ ");
					firstPat = 0;
				}
				
				TFrac curDur = TFrac(1,4);
				for( j = 0; j < alLength; j++ )
				{
					
					TCLICKNOTE *ptr1 = align.v1(j);
					if( !ptr1 )
						ptr1 = align.v2(j);
					if( ptr1 )
					{
						double rIOI;
				
						rIOI = ptr1->IOIratio(-1,1,ptr1->GetVoice() );
						if( rIOI > 0 && j > 0)
						{
							double durD = curDur.toDouble();
							durD *= rIOI;
							curDur = TFrac((long)(durD*960+0.5),960L);
						}
						else if ( rIOI < 0 && j > 0 )
						{
							double durD = curDur.toDouble();
							durD /= -rIOI;
                            curDur = TFrac((long)(durD*960+0.5),960L);
						}
						else
						{
							j = j;
						}
						fprintf(pdatabase, "\\i<\"\",%3.3f> ", (double)ptr1->Intens() / 127 );
						TQChord *qChord = dynamic_cast<TQChord *>(ptr1);
						if( qChord )
							fprintf(pdatabase, "{");
						// ptr1->WritePitch( pdatabase );
						fprintf(pdatabase, "*%ld/%ld ",
							curDur.numerator(), 
							curDur.denominator() );
						if( qChord )
						{
							TQNOTE *asPlayed;
							asPlayed = qChord->playedNotes();
							while( asPlayed )
							{
								fprintf(pdatabase,", ");
								stringstream pdStr;
								asPlayed->WritePitch(pdStr);
								fprintf(pdatabase, "%s", pdStr.str().c_str());
								fprintf(pdatabase, "*%ld/%ld ",
													curDur.numerator(), 
													curDur.denominator() );
								asPlayed = dynamic_cast<TQNOTE *>(asPlayed->GetNext(-1) );
							}
							fprintf(pdatabase, "} ");
						} // if qChord
					} // if !=
				} // if complexity
				fprintf(pdatabase, "\n]\n");
				
				if( IOIList1->Count() != IOIList2->Count()  )
				{					
					fprintf(pdatabase,",(* alignment 2*) [\n ");
					curDur = TFrac(1,4);
					for( j = 0; j < alLength; j++ )
					{
						TCLICKNOTE *ptr2 = align.v2(j);
						if( !ptr2 )
							ptr2 = align.v1(j);
						if( ptr2 )
						{
							double rIOI = ptr2->IOIratio(-1,1,ptr2->GetVoice() );
							if( rIOI > 0 && j > 0)
							{
								double cDur = curDur.toDouble();
								cDur *= rIOI;
								curDur = TFrac((long)(960*cDur+0.5),
                                                       960L);
							}
							else if ( rIOI < 0 && j > 0 )
							{
								double cDur = curDur.toDouble();
								cDur /= -rIOI;

                                 curDur =  TFrac((long)(-960*cDur+0.5),
                                                          960L);
							}
							else
							{
							}
//							ptr2->WritePitch( pdatabase );
							fprintf(pdatabase, "*%ld/%ld ",
								curDur.numerator(), 
								curDur.denominator() );
						} // if
					} // for alLength
					fprintf(pdatabase, "\n]\n");
				} // if != 
			} // if complexity
			double cp1 = IOIList1->Count();
			double cp2 = IOIList2->Count();
			
			if( alLength != 0 )
			{
				cp1 /= alLength;
				cp2 /= alLength;
			
				alSim /= alLength;
			}
			
			/// length penalty
			fprintf(out, "alSim=%d, #gaps= %d, cp1 = %f, cp2 = %f, ",
				alSim, 
				cGaps,
				cp1,
				cp2);
			// IOIList1->write(stdout);
			double alpha = 0.8,
				gamma = 0.8,
				beta = 0.1;			
			alSim = pow(alSim, alpha) * pow(alLength, beta) * 
				(1-pow((double)cGaps/(double)alLength, gamma));
			
			
			/// Todo: build list of pattern ranges for counting the repetitions,
			/// compare also to entries found in best list.
						
			fprintf(out, "significance=%d\n", alSim);
			
			
			/// awrite to _align.ini
			ostringstream iniVal;
			iniVal.clear();
			
			iniVal << bestList[i].x;
			iniVal << ":";
			iniVal << bestList[i].y;
			iniVal << ":";
			iniVal << alSim;
			iniVal << align.toString();
			
			alIni->addEntryN("PATTERN", iniVal.str().c_str(),NULL );
			
			
			fprintf(out,"----------------------------\n");
			
			
			delete [] align.v1L;
			delete [] align.v1R;
			delete [] align.v2L;
			delete [] align.v2R;
			
		} // if
		i++;
	}// while
#ifdef _DEBUG
	outStr.clear();
	debugMatrix( outStr,
			     alDebugMatrix,
			     entries,
			     -1 /*diagonal */ );
	fprintf(out, outStr.str().c_str());
	delete [] alDebugMatrix;
#endif	
	
	fprintf( pdatabase, "}");
	fclose( pdatabase );
	delete [] bestList;
	delete [] xVector;
	
	delete alIni;
	fclose( out );

	} // if entries

	
	
	
}




/* not needed at the moment
void createSimilarityDiag( TNOTE *from, 
TNOTE *to,
int windowSize,
int stepSize,
TTRACK *cTrack)
{
FILE *out;

  if( !from )
  return;
  
    
      // create vector
      
        // create matrix
        
          
            createIOIRatioHistogram( from, to,
            windowSize,
            stepSize,
            cTrack);
            
              
                
                  out = fopen( SIMDIAGFNAME, "wt");
                  
                    int row,
                    column;
                    
                      TNOTE *current1,
                      *current2;
                      
                        
                          int voice;
                          
                            current1 = from;
                            voice = from->GetVoice();
                            row = 1;
                            
                              double sim;
                              
                                while( current1 &&
                                current1 != to )
                                {
                                current2 = from;
                                column = 1;
                                fprintf(out, "%3d: ", row );
                                fprintf(out, "%5.3f %+1.3f: ", GetIOI(current1).toDouble(), GetIOIRatio(current1) );
                                
                                  if( current1->GetNext(voice) ) // no IOIratio at last element!
                                  {
                                  while( column < row )
                                  {
                                  // compare
                                  //              sim = similarity( current1, current2);
                                  sim = relSimilarity( current1, current2);
                                  fprintf(out, "%2.2f ",sim + 0.005);
                                  
                                    // increment
                                    current2 = NOTE(current2->GetNext( voice ));
                                    column++;
                                    } // while 
                                    }
                                    fprintf(out,"\n");
                                    
                                      // increment
                                      current1 = NOTE(current1->GetNext( voice ));
                                      row++;
                                      } // while row
                                      
                                        
                                          fclose( out );
                                          }
                                          */
