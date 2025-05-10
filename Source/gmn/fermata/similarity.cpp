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
#include <iostream>
#include <stack>

#include "../lib_src/similarity/simMatrix.h" /// do this first for min
#include "../lib_src/similarity/dynaprog.h"

#include "alignment.h"
#include "alignment.cpp"

#include "similarity.h"
#include "patternfile.h"
#include <math.h> 


#include "q_chord.h"
#include "note.h"
#include "funcs.h" // dtime2ms
#include "track.h" //TTRACK
#include "../leanguido/lgsegment.h"
#include "../lib_src/ini/ini.h"
#include "statist.h"

#include "notestatlist.h"

/// ptr to current MIDI file
// extern TMIDIFILE *theMidifile;
/// ptr to current control track
// TTRACK *CurControlTrack_;

#define melodySim

// for alouette #define sigmaMelody 2
#define sigmaMelody 2
// positive val for dynaprog, everything <= 
// for alouette #define zeroCostInterval 3
#define zeroCostInterval 3
#define dynPVal (GaussWindow(zeroCostInterval, sigmaMelody))

// interval equivalent to a gap
// for alouette #define gapInterval (zeroCostInterval+2)
#define gapInterval (zeroCostInterval+2)
/// used for eval
//#define retrievalDelta -6
// brack at 1:x positions #define retrievalDelta -5

// for alouette #define retrievalDelta -6
#define retrievalDelta -6
#define selfSimDelta -6

int MIDIPitch( lgNote *x)
{
	if( x )
		return	(x->octave()+4) *12 + x->pitchClass() + x->accidentals();
	else
		return 0;
}
#ifdef _DEBUG

double *createDebugMatrix( long entriesX,
						   long entriesY )
{
	if( entriesX <= 0 )
		entriesX = 1;
	if( entriesY <= 0 )
		entriesY = entriesX;
		
		long matrixSize = (entriesX+1)*(entriesY+1);
		double *alDebugMatrix = new double[matrixSize];
		// init 
		for( long _i = 0; _i < matrixSize; _i++ )
		{
			alDebugMatrix[_i] = 1;
		}
		return alDebugMatrix;
}


void debugMatrix( ostream &out,
				  double *alDebugMatrix,
				  long entriesX,
				  long entriesY )
{					  	
	char diag = 0;
	if( entriesY <= 0 )
	{
		diag = 1;
		entriesY = entriesX;
	}
	out << "Alignment Debug: \n";
	for( int y = 0; y < entriesY; y++ )
	{
		out << "0.5 "; // border
		for( int x = 0; x < entriesX; x++ )
		{
			if( x == y 
				&& diag)
			{
				out << "0.5 "; // diagonal
			}
			else
			{
				out << alDebugMatrix[entriesX*y+x] << " ";
			}
		} // for
		out <<"\n";
	} // for
	for( int x = 0; x < entriesX+1; x++ )
		out << "0.5 "; // border
	out << "\n";
}		
#endif


double _stdcall max3(double f1, double f2, double f3)
{
    double res = f1;
	if( f2 > res )
	{
		res = f2;
	}
	if( f3 > res )
	{
		res = f3;
	}
	return res;
}


string SIMMATRIX_FNAME( const char *midifileName, const char *postfix  )
{
	string res;
	// res = theMidifile->filename();
	res = midifileName;
	if( !postfix )
		postfix = ".txt";
	res += postfix;
	return res;
}

/// 
/// get rel IOI of ptr1
/// IOIratio = IOI(this)/IOI(this->next)

double IOIratio( lgNote *ptr1)
{
	if( !ptr1 ) // error
		return 0;
	
	TPNOTE *p2 = dynamic_cast<TPNOTE *>(ptr1);
	if( !p2 )
		return 0;
	if( p2->isRest() )
		p2 = p2->GetNext();
	
	TFrac ioiPC = p2->IOI();
	
	p2 = p2->GetNext();
	if( !p2 )
		return 0;
	
    TFrac ioiCN = p2->IOI();
	
	double res = IOIratio( ioiPC.toDouble(), ioiCN.toDouble());
    return res;
}

/// because IOIratio(lgNote) retrive rIOI of lgNote->next
/// we write here pitch of ptr->next!
void writePitch( ostream &out, lgNote *ptr)
{
if( !ptr )
	return;
#ifdef melodySim
        out << ptr->pitch() << " ";
#else
    if( ptr->nextNote( ) )   
        out << "%s ",
		ptr->nextNote()->pitch().c_str() );
#endif
};

void writePitch( ostream &out, TNOTE *ptr)
{
    ptr->WritePitch(out);
}

// instantiations
// MCP doesn't like that
//template CSimMatrix<TNOTE *, lgNote *>;
//template CSimMatrix<TNOTE *, TNOTE *>;

//template CSimMatrix<TNOTE *, TPNOTE *>;


//! compare v1[0...windowSize) and v2[0...windowSizw)
//! return [-1...0] (no match... total similarity)
double _stdcall simNoteNote(TNOTE *v1,
                    TNOTE *v2,
                    int windowSize);

double _stdcall simlg(TNOTE *v1,
                      lgNote *v2,
                      int windowSize);


//! return a double value derived from v1
double _stdcall valNote( TNOTE *v1);
double _stdcall valLgNote(lgNote *v1);

// instantiations
//template CDynaprog<TNOTE *, TNOTE *>;
//template CDynaprog<TNOTE*, lgNote *>;

//! compare x and y, 
//! return [-1...0] (no match... total similarity)
double  _stdcall simDynaNoteNote(TNOTE *x,
						 TNOTE *y, 
						 TNOTE *gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength ); /// score matrix / distance function

//! compare x and y, 
//! return [-1...0] (no match... total similarity)

double  _stdcall simDynaNoteLgNote(TNOTE *x,
						 lgNote *y, 
						 TNOTE *gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength ); /// score matrix / distance function
 /*!
	  calculate the similarity of two notes.
	  The similarity function must be context free (dynamic programming)! 
	  res = (-1..0]
*/
double  _stdcall simDynaNoteNote(TNOTE *x,
						 TNOTE *y, 
						 TNOTE *gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength ) /// score matrix / distance function
{
	double res = 0;
	if( x == y ) // don't allow self similar alignment!
	{
		res = -1;
	}
	else if( x == gap || y == gap ) // is one NOTE a gap?
	{
		#ifdef melodySim
			double interval;
		#else
		//! prefer gaps aligned to small notes
		double duration;
		#endif
		if( x == gap )
		{
			#ifdef melodySim
				interval = 2;
			#else
			// use [ms] IOI for penalty 
			duration = DTIMEtoMS( CurControlTrack_->Parent()->Ppq(),
				CurControlTrack_->GetTempo( y->GetAbsTime()),
				y->ioi(1, y->GetVoice()).toLong()
				);
			#endif
		}
		else if( y == gap )
		{
			#ifdef melodySim
				interval = 2;
			#else

			// use [ms] IOI for penalty 
			duration = DTIMEtoMS( CurControlTrack_->Parent()->Ppq(),
				CurControlTrack_->GetTempo( x->GetAbsTime()),
				x->ioi(1, x->GetVoice()).toLong()
				);
			#endif
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
		#ifdef melodySim
			res = 1 - GaussWindow(gapInterval, sigmaMelody);
		#else
		if( duration < 50 )
		{
			res = 0;
		}
		else
		{
			res = 1 - GaussWindow(duration-50, 100);
			// res = 1-exp(-res/100);
		}
		#endif
		res *= -1;				
		// now res = (-1..0] == no similarity ... equal
	}
	else // both notes are no gaps
	{
		// test: use simMatrix values
		
		// res = relSimilarity(x,y) * -1;
		res = simNoteNote(x,y,
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
calculate the similarity of two notes.
The similarity function must be context free (dynamic programming)! 
res = (-1..0] or (-1..k], k > 0
*/

double  _stdcall simDynaNoteLgNote(TNOTE *x,
						 lgNote *y, 
						 TNOTE *gap,
    							  int cGaps,
    							  int cLocalGaps,
    							  int alLength ) /// score matrix / distance function
{
	double res;
	if( x == gap || y == (lgNote *)gap ) // is one NOTE a gap?
	{
		//! prefer gaps aligned to small notes
		#ifdef melodySim
			double interval;
		#else
			double duration;
		#endif
		if( x == gap )
		{
		#ifdef melodySim 
			interval = 5;
		#else
			duration = 4000 * y->duration().toDouble();
		#endif
		}
		else if( y == (lgNote *)gap )
		{
		#ifdef melodySim 
			interval = 3;
		#else
			// use [ms] IOI for penalty 			
			duration = x->durationMS();
		#endif
		}
		
	#ifdef melodySim 
		// gap penalty is a little bit worse than a major third
		res = 1 - GaussWindow(gapInterval, sigmaMelody);
	#else

		// 50ms / duration
		if( duration > 50 )
		{
			res = 1 - GaussWindow(duration, 500);
		}
		else
			res = 0;	// no penalty if duration is smaller than "equal" distance
	#endif
		// transform into  output range
		res *= -1;
		// now res = (-1..0] == no similarity ... equal
	}
	else // both notes are no gaps
	{
		// test: use simMatrix values
		
		// res = relSimilarity(x,y) * -1;
		res = simlg(x,
			y,
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

double _stdcall valNote(TNOTE *x)
{
	if( x )
	#ifdef melodySim
		return x->GetMIDIPitch();
	#else
		return GetIOIRatio(x);
	#endif
	else // gap
		return 0;
}

double _stdcall valLgNote(lgNote *x)
{
	if( x )
	#ifdef melodySim
		return MIDIPitch(x);
	#else
		return IOIratio(x);
	#endif
	else // gap
		return 0;
}


/*!
similarity function for similarity matrix
sim = f(lgDuration, IOI, rIOI, weight, patternLength)
result = (0..1] = no match ... perfect match
this function might be context sensitive!
*/
double _stdcall simNoteNote( TNOTE *v1,
					TNOTE *v2,
					int windowSize)
{
		int voice1 = v1->GetVoice();
		int voice2 = v2->GetVoice();

//		TNOTE *p1 = dynamic_cast<TNOTE *>( v1->GetPrev(voice1) );
//		TNOTE *p2 = dynamic_cast<TNOTE *>( v2->GetPrev(voice2) );

		double res = 1;
		int i;
		for( i = 0; i < windowSize; i++ )
		{
		#ifdef melodySim
			int iv = v1->minInterval( v2 );
//			iv %= 12;

			double c = GaussWindow(iv, sigmaMelody);
/*
			int iv1 = 0,
			    iv2 = 0;
			if( p1 && p2 )
			{
			 	iv1 = p1->minInterval( v1 );
			 	iv2 = p2->minInterval( v2 );
			 }
			double c = GaussWindow(iv1, iv2,  3);
*/
		#else
				/// IOIratio sim
			double i1 = v1->IOIratio(-1,1);
			double i2 = v2->IOIratio(-1,1);
			
			if( validIOIratio(i1) )
				i1 = normIOIratio(i1);
			if( validIOIratio(i2) )
				i2 = normIOIratio(i2);

			double c = GaussWindow(i1,i2,0.7);				
			/*
			// ----- rel intensity sim
			if( p1 && p2 )
			{
				i1 = (double)(v1->Intens()+1)/(double)(p1->Intens()+1);
				i2 = (double)(v2->Intens()+1)/(double)(p2->Intens()+1);

				if( i1 > i2 )
					c *=  0.8 + (i2/i1)*0.2;
				else
					c *=  0.8 + (i1/i2)*0.2;
			} // if p1 && p2
			*/
			/*
			// ----- rel cNotes/chord sim
			i1 = 1;
			i2 = 1;
			TQChord *qChord = dynamic_cast<TQChord *>(v1);
			if( qChord )
				i1 = qChord->cNotes();

			qChord = dynamic_cast<TQChord *>(v2);
			if( qChord )
				i2 = qChord->cNotes();

			if( i1 > i2 )
				c *= 0.9 + (i2/i1)*0.1;
			else
				c *= 0.9 + (i1/i2)*0.1;

			*/
			#endif
			res *= c;
//			p1 = v1;
//			p2 = v1;
			v1 = dynamic_cast<TNOTE *>(v1->GetNext(voice1));
			v2 = dynamic_cast<TNOTE *>(v2->GetNext(voice2));
		} // for
//		res /= windowSize;
	return res;
}


/*!
similarity function for similarity matrix
sim = f(lgDuration, IOI, rIOI, weight, patternLength)
result = (0..1] = no match ... perfect match
this function might be context sensitive!
*/
double _stdcall simlg( TNOTE *v1,
					  lgNote *v2,
					  int windowSize)
{
	int i;
	double res = 1;
	#ifdef melodySim
		int pP1 = 0;
		int pP2 = 0;
	#endif
	for( i = 0; i < windowSize; i++ )
	{
		#ifdef melodySim
			pP1 = v1->GetMIDIPitch(); // % 12;
			pP2 = MIDIPitch(v2); // % 12;
			double c = GaussWindow(pP1, pP2, sigmaMelody);
				 
			/*
			if( i == 0 )
			{
				c = 1;
				pP1 = v1->GetMIDIPitch();
				pP2 = MIDIPitch(v2);
			}
			else
			{
				int p1 = v1->GetMIDIPitch();
				int p2 = MIDIPitch(v2);
				c = GaussWindow(pP1 - p1, pP2 - p2, 1);
				pP1 = p1;
				pP2 = p2;				
			}
			*/
		#else
			double i1 = GetIOIRatio(v1);
			double i2 = IOIratio(v2);
			
			if( validIOIratio(i1) )
				i1 = normIOIratio(i1);
			if( validIOIratio(i2) )
				i2 = normIOIratio(i2);

			double c = GaussWindow(i1,i2,0.7);			
		#endif
			res *= c;

			v1 = dynamic_cast<TNOTE *>(v1->GetNext(v1->GetVoice()));
			v2 = v2->nextNote();
		} // for
//		res /= windowSize;
	return res;
}


double _stdcall val(TNOTE *v1)
{
	double res = -1;
	#ifdef melodySim
	if( v1 ) 
		res = v1->GetMIDIPitch();
	#else
		res = GetIOIRatio(v1);
	#endif
	return res;
}


/// keep in mind that IOIratio(v1) is IOIratio of v1->next!

double _stdcall val(lgNote *v1)
{
    double res = -1;
	#ifdef melodySim
		res =  MIDIPitch(v1);
	#else
	    res = IOIratio(v1);
    #endif
    return res;
}

/*!
compare IOI's
result = (0..1]
*/
double similarity( TMusicalObject *current1,
				  TMusicalObject *current2 )
{
	double sim = GetIOI(current1).toDouble() / GetIOI(current2).toDouble();
	
	if( sim > 1 ) 
		sim = 1/sim;
	
	return sim;
}

/*!
similarity for IOIratio
compare IOIratio's
result = (0..1]  = no match ... perfect match
*/
double relSimilarity( TMusicalObject *current1,
					 TMusicalObject *current2)
{
	// todo: compare also  weight of notes
	double i1 = GetIOIRatio(current1);
	double i2 = GetIOIRatio(current2);
		
	/*
	// todo: compare also  weight of notes
	sim = (double)IOIDistance( i1, i2);
	
	  if( sim > 1 )
	  sim = 1/sim;
	*/
	// normalize
	if( i1 > 0 ) 
		i1 -= 1;
	else
		i1 += 1;
	
	// normalize
	if( i2 > 0 ) 
		i2 -= 1;
	else
		i2 += 1;
	
//	sim = exp( - 2 * pow(i1-i2,2) ); 
	double sim = GaussWindow( i1,i2, 0.6 );
	return sim;
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
void selfSimilarity( TMusicalObject *from, 
					TMusicalObject *to,
					int windowSize,
					int stepSize,
					TTRACK *CurControlTrack_,
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
#ifdef melodySim
	int entries = cnotes; 
#else		
	int entries = cnotes-2; // skip first and last because rIOI is undefined
#endif
	
	if( entries > 0 )
	{
		TInifile *alIni = new TInifile("_align.ini");
		// create array of pointers
		PTNOTE *xVector = new PTNOTE [entries];
	
		// fill ptr-vector
		temp = from;
		long i;
		for( i = 0; i < entries; i++ )
		{
			xVector[i] = dynamic_cast <TNOTE *> (temp);
			temp = temp->GetNext(temp->GetVoice());
		}
	
	
	/// create the window based similarity matrix
	CSimMatrix<PTNOTE, PTNOTE> simMatrix(entries,	/// |x-vector|
		entries,	/// |y-vector|
		simNoteNote,		/// sim function
		val,		/// val function
		val
		);
	simMatrix.setXVector(xVector);
	double mean,var;
	double processTime;
	simMatrix.calcTable(windowSize, 
					    stepSize,
						&mean,
						&var,
						&processTime);
	
	ofstream out;
	out.open(SIMMATRIX_FNAME(midiFilename,"sim.txt").c_str(), ios_base::app );
	out << "voice " << from->GetVoice() << " selfsimilarity \n"; 
	out << "#Entries = "<< entries << "^2 ("<< (double)entries/(double)stepSize << ")\n";
	out << "WindowSize = "<< windowSize << "\n";
	out << "StepSize = "<< stepSize << "\n";
	out << "time = "<<processTime << "\n";
	
	
	simMatrix.debug(0.5, &out);
	
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
	double limit = my + sigmaGreater;
	limit = 0.995;

	int lSize;
	/// start alignment search
	//! get list of best matches 
	Cxy *bestList = simMatrix.getBestIDs(limit,		/// limit
		windowSize,	/// min distance between two hits
		&processTime,
		&lSize
		);
	
	// count number of high scoring positions
	int e2 = (double)entries/(double)stepSize;
	printf(" %d (%f%%) high scoring windows in %d x %d found!\n",
			lSize,
			(double)lSize/(double)(e2*e2/2),
			e2,e2);
	ofstream pdatabase;
	pdatabase.open("_pdatabase.gmn");
	pdatabase << "(* Aligned patterns from similarity analysis! *)\n";
	pdatabase << "(* Only the IOI ratios of these pattern should be evaluated! *)\n";
	pdatabase << "{";

		/// make left-right alignment for this entry
#ifdef _DEBUG
		double *alDebugMatrix = createDebugMatrix( entries, entries );
#endif				

	
	char firstPat = 1;	
	// Try alignment for all entries in best lists
	out << "Best matches (window=" << windowSize << "):\n";
	double totalAlignmentTime = 0;
	i = 0;
	while( bestList[i].x > -1 )
	{
		if( bestList[i].x < entries )
		{
			//! debug matrix match
			out << "("<< i << ">"<< bestList[i].x << ":"<<bestList[i].y << " "<<simMatrix.V(bestList[i].x, bestList[i].y)  << ")<";
			//print x pattern
			int j;
			for( j = 0; j < windowSize; j++ )
			{
				out <<val(xVector[bestList[i].x+j]);
				xVector[bestList[i].x+j]->WritePitch(out);
				out << " ";
			}
			//print y pattern
			out << ">\n                  <";
			for( j = 0; j < windowSize; j++ )
			{
				out <<  val(xVector[bestList[i].y+j]);
				xVector[bestList[i].y+j]->WritePitch(out);
				out << " ";
			}
			out << ">\n";
				

		} // if 
		
		double bestWMatchVal = -10000;
		int bestInWindow = 0;
		// search for best pair of matches in window
		int j; 
		for( j = 0; j < windowSize-1; j++ )
		{
			if( bestList[i].x+j < entries )
			{
			double _bestWMatchVal = simNoteNote(xVector[bestList[i].x+j],
									 xVector[bestList[i].y+j],1);
			_bestWMatchVal += simNoteNote(xVector[bestList[i].x+j+1],
									 xVector[bestList[i].y+j+1],1);									 
			_bestWMatchVal /= 2;									 
			if( _bestWMatchVal > bestWMatchVal )
			{
				bestWMatchVal = _bestWMatchVal;
				// seed note for rAlign = betsInWindow, for lAlign = bestInWindow-1
				bestInWindow = j+1;
			}
			}
		}
		#ifdef _DEBUG
		printf("bestInWindow %d,(%f)\n", bestInWindow, bestWMatchVal);
		#endif
		int x = bestList[i].x + bestInWindow;
		int y = bestList[i].y + bestInWindow;
		
		if( x < entries && 
			y < entries )
		{
			//			printf("start at %d,%d\n",x,y);
			double processTime;
			/// this is a lower triangular matrix therefore:
			TAlignmentArray<TNOTE, TNOTE> align = makeLRAlignment( xVector,
														xVector,
														entries,
														entries,
														x,
														y,
														selfSimDelta,		/// delta similarity threshold
														-100000,			/// abslimit
														&processTime,
														valNote,
														valNote,
														simDynaNoteNote);
			totalAlignmentTime += processTime;
			// debug alignment
			out << "\nDynaprog table ("<<x << ","<< y<< ":"<< processTime<< "s):\n";
			
			align.debug( out );
			/// todo cut "no matches" at left and right end, gaps are already removed  
			
#ifdef _DEBUG
			addToDebugMatrix( alDebugMatrix,
					   		  &align,
							  x,
							  y,
						      entries,
						      -1 );

#endif			
			/// delete all bestList entries in range of alignment in same diagonal
			if(0)
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
			} // block
			
			/// calc siginificance and complexity of alignment (# of reps, length, total similarity)			
			int alLength = align.alignmentLen();
			
			TDoubleBinList *IOIList1 = new TDoubleBinList(99,101,20);
			IOIList1->setMinMaxRel(30.0);		  
			IOIList1->setMinweightDeltaRel( 20.0 ); // local/global evaluation
			IOIList1->createDistribution();

			TDoubleBinList *IOIList2 = new TDoubleBinList(99,101,20);
			IOIList2->setMinMaxRel(30.0);		  
			IOIList2->setMinweightDeltaRel( 20.0 ); // local/global evaluation
			IOIList2->createDistribution();
			double alSim = 0;
			/// get total similarity
			int cGaps = 0;	// count the gaps
			
			for( j = 0; j < alLength; j++ )
			{			
				TNOTE *ptr1 = align.v1(j);
				TNOTE *ptr2 = align.v2(j);
		
				double 	rIOI1,
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
				} // else
				/// if no rIOI could be calculated, localsim == 0 
				localSim = exp( - fabs(localSim) /* /5 */);
				rIOI1 = normIOIratio(rIOI1);
				rIOI2 = normIOIratio(rIOI2);
				
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
				TDistanceStruct dist = IOIList1->closestClass(&rIOI1);
				if( dist.distance > 0.1 )
				{
					dist.classPtr = IOIList1->addClass(rIOI1);
					IOIList1->createDistribution();
				}
				IOIList1->addValue(rIOI1, dist.classPtr );
				
				dist = IOIList2->closestClass(&rIOI2);
				if( dist.distance > 0.1 )
				{
					dist.classPtr = IOIList2->addClass(rIOI2);
					IOIList2->createDistribution();
				}
				IOIList2->addValue(rIOI2, dist.classPtr );				
			} // for alignment
			#ifdef _DEBUG
				IOIList1->write(&out);
				out << "c1: " << IOIList1->Count() << endl;
				IOIList2->write(&out);
				out << "c2: " << IOIList2->Count()  << endl;
			#endif
			/// write to pattern-database -----------------------------------
			/// todo: remove duplicated patterns!!!
			if( IOIList1->Count() > 2 ||
				IOIList2->Count() > 2 )
			{				
				if( !firstPat ) 
					pdatabase << ",\n[ ";
				else
				{
					pdatabase << "\n[ ";
					firstPat = 0;
				}
				
				TFrac curDur(1,4);
				double rIOI;
				TQChord *qChord;
				for( j = 0; j < alLength; j++ )
				{
					// quantize curDur, limit denominator
					curDur = TFrac((long)(curDur.toDouble()*960+0.5), 960L );
					
					TNOTE *ptr1 = align.v1(j);
					if( !ptr1 )
						ptr1 = align.v2(j);
					if( ptr1 )
					{
						rIOI = ptr1->IOIratio(-1,1,ptr1->GetVoice() );
						if( rIOI > 0 && j > 0)
						{
							TFrac temp((long)(rIOI*960+0.5),960L);
							curDur *= temp;
						}
						else if ( rIOI < 0 && j > 0 )
						{
                             TFrac temp((long)(-rIOI*960+0.5),960L);
							 curDur /= temp;
						}
						else
						{
							j = j;
						}
						pdatabase << "\\i<\"\"," << (double)ptr1->getIntens() / 127  << "> ";
						qChord = dynamic_cast<TQChord *>(ptr1);
						if( qChord )
							pdatabase <<  "{";
						ptr1->WritePitch( pdatabase );
						pdatabase << "*" <<
							curDur.numerator() << "/" <<
							curDur.denominator() << " ";
						if( qChord )
						{
							TQNOTE *asPlayed;
							asPlayed = qChord->playedNotes();
							while( asPlayed )
							{
								pdatabase << ", ";
								asPlayed->WritePitch(pdatabase);
								pdatabase << "*" << 
													curDur.numerator() << "/" <<
													curDur.denominator() ;
								asPlayed = dynamic_cast<TQNOTE *>(asPlayed->GetNext(-1) );
							}
							pdatabase << "} ";
						}

					} // if !=
				} // for alignment
				pdatabase << "\n]\n";
				
				if( IOIList1->Count() != IOIList2->Count()  )
				{					
					pdatabase << ",(* alignment 2*) [\n ";
					curDur = TFrac(1,4);
					for( j = 0; j < alLength; j++ )
					{
						// quantize curDur, limit denominator
						curDur = TFrac((long)(curDur.toDouble()*960+0.5), 960L );
						TNOTE *ptr2 = align.v2(j);
						if( !ptr2 )
							ptr2 = align.v1(j);
						if( ptr2 )
						{
							rIOI = ptr2->IOIratio(-1,1,ptr2->GetVoice() );
							if( rIOI > 0 && j > 0)
							{
								TFrac temp((long)(960*rIOI+0.5),960L);
								curDur *= temp;
							}
							else if ( rIOI < 0 && j > 0 )
							{
								TFrac temp((long)(-960*rIOI+0.5),960L);
								curDur /= temp;
							}
							else
							{
							}
							ptr2->WritePitch( pdatabase );
							pdatabase << "*" <<
								curDur.numerator()  << "/"  <<
								curDur.denominator() ;
						} // if
					}
					pdatabase << "\n]\n";
				} // if != 
			} // if complexity
			double cp1 = IOIList1->Count();
			double cp2 = IOIList2->Count();
			
			cp1 /= alLength;
			cp2 /= alLength;
			
			// IOIList1->write(stdout);
			alSim /= alLength;
			
			/// length penalty
			out << "alSim=" << alSim << ", #gaps= " << cGaps << ", cp1 = " << cp1 << ", cp2 = " << cp2 << ", ";
			double alpha = 0.8,
				gamma = 0.8,
				beta = 0.1;			
			alSim = pow(alSim, alpha) * pow(alLength, beta) * 
				(1-pow((double)cGaps/(double)alLength, gamma));
			
			
			/// Todo: build list of pattern ranges for counting the repetitions,
			/// compare also to entries found in best list.			
			out << "significance="<< alSim<<endl;
			
			
			/// awrite to _align.ini
			ostringstream	iniVal;			
			iniVal.clear();
			
			iniVal << bestList[i].x;
			iniVal << ":";
			iniVal << bestList[i].y;
			iniVal << ":";
			iniVal << alSim;
			iniVal << align.toString();
			
			alIni->addEntryN("PATTERN", iniVal.str().c_str(),NULL );
			
			
			out <<"----------------------------\n";
			
			
			delete [] align.v1L;
			delete [] align.v1R;
			delete [] align.v2L;
			delete [] align.v2R;
			
		} // if
		i++;
	}// while best matching windows
	out << "Total alignment Time 0 "<<totalAlignmentTime<<endl;
#ifdef _DEBUG
	debugMatrix( out,
			     alDebugMatrix,
			     entries,
			     -1 /*diagonal */ );
	delete [] alDebugMatrix;
#endif	
	
	
	 pdatabase << "}";
	 pdatabase.close();
	delete [] bestList;
	delete [] xVector;
	
	delete alIni;
	out.close();

	} // if entrie
}



/// search for all sequences stored in gmnFilename
void retrieveSequence( TMusicalObject *from,  
					  TMusicalObject *to,
					  int windowSize,
					  int stepSize,
					  TTRACK *cTrack,
					  const char *gmnFilename,
					  const char *midiFilename
					  )
{
	if( !from )
		return;
	cout << "Starting MIR with file " << gmnFilename <<
			" (1st sequence)\n";
	// create vector

	
	int cnotes = 0;
	TMusicalObject  *temp= from;
	// count notes
	while( temp &&
		    temp != to )
	{
		cnotes++;
		temp = temp->GetNext(temp->GetVoice());
	}
	#ifdef melodySim
		int xentries = cnotes;
	#else
		int xentries = cnotes-2; // skip first and last because IOIratio is undefined
	#endif
	if( xentries < 1 )
	{
		Printf("ERROR: Sequence is too short for MIR!\n");
		return;
	}

	/// parse, lgSegment and fill yVector
	TPFILE *patterns = new TPFILE( gmnFilename  );
	patterns->Read( );
	patterns->write();
	
	// todo use more than first pattern for MIR
	lgSequence *curPattern = NULL;	
	if( !curPattern )
		curPattern = patterns->firstSequence();
	else
		curPattern = patterns->nextSequence();

	int yentries;
	if( curPattern )
	{
		#ifdef melodySim
			yentries = curPattern->cNotes();
		#else
			yentries = curPattern->cNotes()-2;
		#endif
	}
	else
	{
		if( !gmnFilename )
			gmnFilename = "NULL";
		cout << "Warning: could not open MIR file "<< gmnFilename <<
				", or file is empty!\n" ;
		delete patterns;
		return;
	}
	if( yentries < 2 )
	{
		cout << "Warning: pattern with len "<< yentries+2 <<
				" are is too short for MIR!\n";
		delete patterns;
		return;
	}
	
	if( yentries < windowSize )
	{
		cout << "Warning: pattern len " << yentries <<
				"is smaller than windowsSize " << windowSize << endl;
		delete patterns;
		return;
	}
	
	
	typedef lgNote *PlgNote;
	/// create and fill vectors
	lgNote **yVector = new PlgNote[yentries]; 
	int i;
	lgNote *curEvent = curPattern->firstNote();
	for( i=0; i < yentries; i++ )
	{
		yVector[i] = curEvent;
		curEvent = curPattern->nextNote(curEvent);        
	}
	
	// create array of pointers
	PTNOTE *xVector = new PTNOTE [xentries];
	// fill ptr-vector
	temp = from;
	for( i = 0; i < xentries; i++ )
	{
		xVector[i] = dynamic_cast <TNOTE *> (temp);
		temp = temp->GetNext(temp->GetVoice());
	}
	
	/// inifile for patterns
	TInifile *alIni = new TInifile("_align.ini");	
	if( curPattern )
		alIni->SetEntry(REMARK_ENTRY,curPattern->toString().c_str(),NULL );
	
	/// create the window based similarity matrix
	CSimMatrix<PTNOTE, lgNote *> simMatrix(xentries,	/// |x-vector|
											yentries,	/// |y-vector|
											simlg,		/// sim function
											val,		/// val TNOTE
											val		/// val lgNote function
											);
	simMatrix.setXVector(xVector);
	simMatrix.setYVector(yVector);
	double mean, var;
	double processTime;
	simMatrix.calcTable(windowSize, 
						stepSize,
						&mean,
						&var,
						&processTime);
	ofstream out;
	out.open(SIMMATRIX_FNAME(midiFilename, ".mirsim.txt").c_str(), ios_base::app);
	if( out.is_open() )
	{
		out << "MIR stepsize = "<< stepSize<< ", wsize="<< windowSize<< "("<<processTime << "s)\n";
		simMatrix.debug(0.5, &out);
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
	double limit = my + sigmaGreater;

	limit = 0.995;
	/// start alignment search
	//! get list of best matches
	int lSize;
	Cxy *bestList = simMatrix.getBestIDs(limit,		/// limit
									     yentries, /// min distance between two hits
										 &processTime,
									     &lSize);
		
#ifdef _DEBUG
		double *alDebugMatrix = createDebugMatrix( xentries, yentries );
#endif				
		
	// Try alignment for all entries in best lists
	out <<"Best matches (window="<< windowSize<<"):\n";
	out.flush();
	i = 0;
	double totalAlignmentTime = 0;
	while( bestList && 
		   bestList[i].x > -1 )
	{
		//! debug matrix match
		out << "(" << i <<  ">" << bestList[i].x << ":" << bestList[i].y<< " " << simMatrix.V(bestList[i].x,bestList[i].y)<< ")<";
		//print x pattern
		int j;
		for( j = 0; j < windowSize; j++ )
		{
			out <<  val(xVector[bestList[i].x+j]);
			xVector[bestList[i].x+j]->WritePitch(out);
			out << " ";
		}
		//print y pattern
		out <<">\n                  <";
		for( j = 0; j < windowSize; j++ )
		{
			out << val(yVector[bestList[i].y+j]);
			writePitch( out, yVector[bestList[i].y+j]);
			out << " ";
		}
		out <<">\n";
		
		// get best matching point in window
		int bestInWindow = 0;
		j = 0;
		
		
		double bestWMatchVal = -10000;
		// search for best pair of matches in window
		for( j = 0; j < windowSize-1; j++ )
		{
			double _bestWMatchVal = simlg(xVector[bestList[i].x+j],
									 yVector[bestList[i].y+j],1);
			_bestWMatchVal += simlg(xVector[bestList[i].x+j+1],
									 yVector[bestList[i].y+j+1],1);									 
			_bestWMatchVal /= 2;									 
			if( _bestWMatchVal > bestWMatchVal )
			{
				bestWMatchVal = _bestWMatchVal;
				// seed note for rAlign = betsInWindow, for lAlign = bestInWindow-1
				bestInWindow = j+1;
			}
		}
		#ifdef _DEBUG
		printf("bestInWindow %d,(%f)\n", bestInWindow, bestWMatchVal);
		#endif
		int x = bestList[i].x + bestInWindow;
		int y = bestList[i].y + bestInWindow;
		/// make left-right alignment for this entry

		TAlignmentArray<TNOTE , lgNote > align;
		double processTime;
		align = makeLRAlignment<TNOTE , lgNote>( xVector,
			yVector,
			xentries,
			yentries,
			x,
			y,
			retrievalDelta,		/// delta similarity limit -> no limit
			-1000,		/// abs limit, no limit search until end
			&processTime,
			valNote,
			valLgNote,
			simDynaNoteLgNote
			);
		totalAlignmentTime += processTime;
		// debug alignment
		out <<"\nDynaprog table ("<<x<<","<<y<<":"<<processTime<<"s):\n";
		out.flush();

		align.debug( out );

		/// calc siginificance of alignment (# of reps, length, total similarity)
		{
			double  alSim = 0;
			int alLength = align.alignmentLen();
			
			// get total similarity
			double _xy = 0,
				   _x = 0,
				   _y = 0;
			initNDimAngle( _xy,_x,_y, 0 );
			
			double norm = 0; // norm > minIOIratio, to prevent zero vectors!
			
		 	int	 cGaps = 0;	// count the gaps			
			for( j = 0; j < alLength; j++ )
			{
			
                TNOTE *ptr1 = align.v1(j);
                lgNote *ptr2 = align.v2(j);
                double localSim = 0;
                if( ptr1 == NULL )  // gap
				{
					localSim  = IOIratio(ptr2);
					// todo add penalty of neighbours
					cGaps++;
				}
				else if (ptr2 == NULL )
				{
					localSim  = ptr1->IOIratio(-1,1,ptr1->GetVoice());
					// todo add penalty of neighbours
					cGaps++;
				}
				else
				{
				
					double xi = ptr1->IOIratio(-1,1,ptr1->GetVoice());
					double yi = IOIratio(ptr2);
					// avoid zero vectors!!
					if( (xi == 0 && yi >= 0) ||
						(xi >= 0 && yi == 0))
					{
						xi += 1;
						yi +=1;
					}
					else if( (xi == 0 && yi <= 0) ||
							 (xi <= 0 && yi == 0) )
					{
						xi -= 1;
						yi -= 1;
					}
					
					// Todo: norm IOIratios
					updateNDimAngle( (xi+norm),
									 (yi+norm),
									 _xy,
									 _x,
									 _y );
					/*
					xy += (xi+norm)*(yi+norm);
					x += pow(xi+norm,2);
					y += pow(yi+norm,2);
					*/
					localSim  = ptr1->IOIratio(-1,1,ptr1->GetVoice()) -
						IOIratio(ptr2) ;
				} // else
				localSim = exp( - fabs(localSim));
				/// todo evaluate weight/significance of notes?
				alSim += localSim;
            } // for

			Printf("Use cost sim in similarity!\n");
			nDimAngle( _xy,_x, _y );
            alSim /= alLength;
            /// length penalty
            out << "alSim="<<alSim <<", #gaps= "<< cGaps;
			/* use this function for self similar analysis with undertermined lengths
			alSim = pow(alSim, alpha) * pow(alLength, beta) *
			(1-pow((double)cGaps/(double)alLength, gamma));
			*/	
			/// Todo: build list of pattern ranges for counting the repetitions,
            double alpha = 0.8,
				gamma = 0.8,
				beta = 0.1;
			alSim = pow(alSim, alpha) * pow((double)alLength/(double)yentries, beta) *
																(1-pow((double)cGaps/(double)alLength, gamma));
			/// compare also to entries found in best list.
			out << "significance=" << alSim << endl;
			/// awrite to _align.ini
			ostringstream	iniVal;			
			iniVal << bestList[i].x;
			iniVal << ":";
			iniVal << bestList[i].y;
			iniVal << ":";
			iniVal << alSim;
			iniVal << align.toString();
//			if( alSim > 0.3 )
			{
					#ifdef _DEBUG
					addToDebugMatrix( alDebugMatrix,
							   		  &align,
									  x,
									  y,
								      xentries,
								      yentries );
					#endif			

					alIni->addEntryN("PATTERN", iniVal.str().c_str(),NULL );			
			} // if high significance
		} // alignement similarity!!
		out <<"----------------------------\n";
		delete [] align.v1L;
		delete [] align.v1R;
		delete [] align.v2L;
		delete [] align.v2R;
		i++;
    }// while
	out << "Total alignment Time 0 "<< totalAlignmentTime << "s\n";

#ifdef _DEBUG
	debugMatrix( out,
			     alDebugMatrix,
			     xentries,
			     yentries);
	delete [] alDebugMatrix;
#endif	
    
	out.close();
    delete [] xVector;
    delete [] bestList;
	delete patterns;
	delete alIni;
    return; //-----------------------------------------------------------------------
	{
	// create matrix
	ofstream out;
	out.open( RELIOIHISTOGRAM );
    TNOTE *current1,
        *current2;	;
	int voice,
		count = 0,
		cClasses = 0,
		id;
	voice = from->GetVoice();
	int histogram[101];
	for( id = 0; id < 101; id++ )
	{
		histogram[id] = 0;
	}
	double sim;
	current1 = NOTE(from);
	current2 = NOTE(current1->GetNext( voice ));
	int row,
		column;
	row = 1;
	while( current1 &&
		current1 != to )
	{
		current2 = NOTE(from);
		column = 1;
		if( current1->GetNext(voice) ) // no IOIratio at last element!
		{
			while( column < row )
			{
				count++;
				sim = relSimilarity( current1, current2);
				id = (int)((sim * 100) + 0.5);
				// count number of used classes
				if( histogram[id] == 0 )
					cClasses++;
				histogram[id]++;
				// increment
				current2 = NOTE(current2->GetNext( voice ));
				column++;
			} // while
		}
        // increment
        current1 = NOTE(current1->GetNext( voice ));
        row++;
    } // while row
    for( id = 0; id < 101; id++ )
    {
        out << id << " : " << histogram[id] << " ("<< (double)histogram[id]/(double)count << ")\n";            
    }
    out <<  "#values: " << count << endl;
    out << "#classes: " << cClasses << endl;
	out.close();
	}
}


