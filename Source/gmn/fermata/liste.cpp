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
|	Filename : LISTE.CPP
|	Author     : Juergen Kilian
|	Date	    : 17.10.1996-03
|	Implementation of TDoubleBinClass and TDoubleBinList
|	lists for statistical tempo detection
------------------------------------------------------------------*/
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif
#include <fstream>
using namespace std;
#include <string.h>
#include <math.h>

#include "liste.h"
#include "../lib_src/ini/ini.h"
// #include "funcs.h"
// #include "portable.h"
#include "patternfile.h"

#include "statist.h"
//------------------------------ -----------------------------------
/*!
	if   (lowerbound*TDoubleBinClass::value)/100 <= newvalue <= 
					(upperbound*TDoubleBinClass::value)/100
	then newvalue can be associated with that TDoubleBinClass
	i.e. upperbound == 142 -> newvalue can be 100-142% of value
*/				
TDoubleBinList::TDoubleBinList( double upperbound,	 // 100..
			    double lowerbound,  // x..100
				double al /* alhpa value for class distance */): avErrorSumI(0), 
	errorSigmaSumI(0), cValsI(0)
{

	// init
	prefix = "IOIr";
	CountI      = 0;		
        if( upperbound > 2 )
            UpperboundI = upperbound;
        else
            UpperboundI = upperbound * 100;
        if( lowerbound > 2 )
            LowerboundI = lowerbound;
        else
            LowerboundI = lowerbound * 100;
        Head        = NULL;			
	Mult  	    = 1;
	firstValue = 1;

	setMinMaxRel(30);		  
	setMinweightDeltaRel( 30.0 ); // local/global evaluation

	MinWeightI = 1;
	deltaI = 1;

	alpha = al;
	if( alpha <= 0 )
		alpha = 1;
	valueSigma = 0.5;
} // TDoubleBinList

TDoubleBinList::~TDoubleBinList( void )
{
	TDoubleBinClass *temp = Head;
	while(temp)
	{
		TDoubleBinClass *tempNext = temp->Next();
		delete temp;
		temp = tempNext;
	}
}


/*!
	return ID and distance of closest class,
	if duration > 0 -> use val double(duration)
	else use val;

 */
TDistanceStruct TDoubleBinList::closestClass( double *val,  
									 TFrac *duration )
{
	double dVal;
	if( duration  )
            dVal = (double) toDouble(*duration); 
	else if( val )
		dVal = *val;
	else
	{
                TDistanceStruct res;
		res.distance = -1;
		res.classPtr = NULL;
		return res;
	}




	// search in complete list for min distance!
	double minDistance = 2;
	TDoubleBinClass *temp = Head,
                        *minPtr = NULL;
	while( temp )
	{
		double dist = distance(temp,
						val,
						duration);
		if( dist < minDistance ) // new minimum
		{
			minDistance = dist;
			minPtr = temp;
		}
		temp = temp->Next();
	}
        TDistanceStruct res;
	res.classPtr = minPtr;
	res.distance = minDistance;
	return res;
}

//-----------------------------------------------------------------
/*
	Test if value1 fits to in lower/upperbound of value2
		0  wenn Lowerbound(value2) <= value1 <= Upperbound(value2)
		-1 wenn value1     < Lowerbound(value2)
		1  wenn Upperbound(value2) < value1
*/
int TDoubleBinList::CompareValue(  double value1,    // new value
                                   double value2 )	// average of class k
{
	// calculate limits
	double AbsUpper = ( value2 * UpperboundI) / 100;
	double AbsLower = ( value2 * LowerboundI) / 100;
	if( (value1 < AbsLower) )
		return -1;
	else if( AbsUpper < value1 )
		return 1;
	else return 0;
} // CompareValue
//-----------------------------------------------------------------
/*! 
	insert value in closest class, 
	remark:
		value will be normalised (with Mult) before inserting it
		Insert will be called
	return:
		id of selected class
*/
int TDoubleBinList::Add( double value )
{
	lpDirty = 1;
	rpDirty = 1;

	if( value == 0)
	{
		// Printf("Warning TDoubleBinList::Add duration == 0!\n");
		// Printf("ClickTrack may be not correct!\n");
		return -1;

	}
	int Id;
	if( !Head )	// empty list -> create new class
	{
		Mult = 1;
		Head = factory( value,
                  0 /* id */,
                  1 /*bias*/);
		// myAssert( Head != NULL );

		Id = 0;
		CountI ++; 
	}
	else
	{
		if( Mult == 0 )
		{
			// Printf("TListe::Add Mult == 0!\n");
			Mult = 1;
		}
		double NormValue = value / Mult;	// normalise value
		Insert( NormValue,			// insert normalised value
				 &Id );
	} // else
	return Id; 	// id

} // Add
//-----------------------------------------------------------------
// search for class(id)
TDoubleBinClass *TDoubleBinList::Find( int id )
{
        char  end = 0;
	TDoubleBinClass *Temp = Head;
	while( Temp && !end)
	{
		if( Temp->ID() == id )
			end = 1;
		else
			Temp = Temp->Next();
	} // while
	return Temp;
} // Find
//-----------------------------------------------------------------
/*
 insert value in best fitting class
 return:
 1
 */
double TDoubleBinList::Insert( double value,		// normalised value
					  int  *id )	// class id
{
        // TDoubleBinClass *Ptr;

	lpDirty = 1;
	rpDirty = 1;

	int closestID = -1;
	double smallestDistance = -1;

	TDoubleBinClass *Temp = Head;
        TDoubleBinClass  *Prev = NULL;
	char     end = 0;
	while( Temp && !end)	// complete list
	{
		// check if value can be inserted into Temp
		double Comp = CompareValue(value,	// normalised value
							Temp->Value());
		if( Comp == 0 )	// value fits in limits
		{
			if( closestID < 0 )
			{
				closestID = Temp->ID();
				smallestDistance = distance(Temp,
                                                            &value, 
                                                            NULL);
			}
			else // compare
			{
				double tDistance;
				tDistance = distance(Temp,
                                                    &value, 
                                                    NULL);
				if( tDistance < smallestDistance )
				{
					smallestDistance = tDistance;
					closestID = Temp->ID();
				}
			} // else

		} // if == 0
		Prev = Temp;
		Temp = Temp->Next();
	}  // while
	double	 NowValue  = 0;
	TFrac 	 PartsPerNote;	// can be used for return
        

        
	if( closestID > -1 )
	{
		*id = closestID;

		Temp = Find( closestID );
		// de-normalise value, insert value and get new normalise factor
		double NewMult  = Temp->CountUp( value * Mult);
		Mult =  NewMult;

		NowValue  = Temp->Value();

			// check if classes needs to be merged
			/*
			if( Prev )
			{
				PrevValue = Prev->Value();
				Comp = CompareValue( PrevValue,
							  NowValue );
				if( !Comp )
					Comp = CompareValue( NowValue,
								  PrevValue );
				{
					 if( !Comp ) // Elemente zusammenfassen
					 {
						 Prev->AddNext();
						 Temp     = Prev;
						 NowValue = Temp->Value();
					 } // if( !Comp
				} // if( !Comp
			} // if( Prev )
			Next = Temp->Next();
			if( Next )
			{
				NextValue = Next->Value();
				Comp = CompareValue( NextValue,
							  NowValue );
				if( Comp == 0 )
				{
					Comp = CompareValue( NowValue,
							NextValue );
					if( !Comp ) // Elemente zusammenfassen
						Temp->AddNext();
				} // if(!Comp

			} // if( Next
			*/

			end = 1;
			PartsPerNote = Temp->PartsPerNote();
	}// if( Comp == 0
	else // create new class
	{
			TDoubleBinClass *Ptr = factory( value,
						   					CountI /* id */,
    				                       1 /* bias */);
			/*
			Ptr = new TDoubleBinClass( value,
						 CountI,
						 -1); // dummy range
						 */
			// myAssert( Ptr != 0 );
			CountI++;	// 1 Element mehr in der Liste

			*id = Ptr->ID();

			insert(Ptr);
			PartsPerNote = Ptr->PartsPerNote();

	} // else



//	debug();
//	getch();


	return 1;

} // Insert
//-----------------------------------------------------------------
// debugging output function
void TDoubleBinList::write( ostream *out,
				    double *val,
					TFrac  *duration)
{

	char needClose = 0;
	if( !out )
	{
		out = new ofstream();
		((ofstream*)out)->open("_liste_lst.txt");
		needClose = 1;
	}

	if( !out )
	{
		printf("Can't open _liste_lst.txt!\n");
		return;
	}



	(*out) << "Start<List>\n";
	if( val )
		(*out) << "val = " << *val << endl;
	else if( duration )
	{
		(*out) << "val = ";
		(*duration).Write(*out);
		(*out) <<"\n";
	}
	(*out) << "#" << CountI << ", d:"<< deltaI << "\n";

//	TFrac dummy(-1,1);
	double wSum = 0;
	int i = 1;
	double variance = 0;
	TDoubleBinClass *Ptr = Head;
	while( Ptr )
	{
		variance += Ptr->variance();
		wSum += Ptr->weight();
		Ptr->debug(out);
		if( val || duration )
		{
			(*out)<<  " d=" << this->distance(Ptr,
										  val,
										  duration);
		}

		Ptr = Ptr->Next();
		(*out) << "\n";
		i++;
	} // while
	

	(*out) << "Variance: " << variance << endl;
	(*out) << "Sum weights: " << wSum << endl;

	(*out) << "End<List>\n";
	if( needClose )
	{
		((ofstream*)out)->close();
		delete out;
	}

} // debug
//-----------------------------------------------------------------
void TDoubleBinList::MakeTempo( int filePpq,   	// PPQ at recording
						int fileTempo )	// Tempo at recording
// call MakeTempo for all classes (elements)
{
	TDoubleBinClass *Ptr = Head;
	while( Ptr )
	{
		Ptr->MakeTempo( filePpq,
						fileTempo );
		Ptr = Ptr->Next();
	} // while
} // MakeTempo
//-----------------------------------------------------------------
void TDoubleBinList::Reset( void )
{
    createDistribution();
    return;
    
    TDoubleBinClass *Ptr;

	lpDirty = 1;
	rpDirty = 1;

	Ptr = Head;

	Mult = 1;

	double av = avWeight();
	while( Ptr )
	{
//		Ptr->Reset();
		Ptr->setWeight( av );
		Ptr = Ptr->Next();
	} // while
	
} // Reset
//-----------------------------------------------------------------
/*
	calculate for each class a score duration

	Search in the list for class with most entries
	The score duration of this class will be set to "mostduration"
	The score duration for the other classes wil be 
	calculated from the relation between their duration
	and mostduration
*/
char TDoubleBinList::FindQuarter( double PlayPPQ,
								  int /*ppq*/ )
{


	/*
	#if !defined( Windows )
	printf("Viertelnote ?");
	cscanf("%s",Str);

	j = saveAtoi( Str );

	Ptr = Head;

	while( Ptr && ( i!=j) )
	{
		i++;
		Ptr = Ptr->Next();
	}

	if( i == j )	// Ptr zeigt jetzt auf die in mostduration angegebene Notenklasse
	{
	#endif
	*/
/*
	// Die Klasse mit den meisten Werten suchen
	Ptr  = 0;
	Temp = Head;
	Max  = 0;
	while( Temp )
	{
		if( (Temp->Number() > Max) &&
			 (Temp->Tempo() > 50) &&
			 (Temp->Tempo() < 240)  )
		{

			Max = Temp->Count();
			Ptr = Temp;
		} // if
		Temp = Temp->Next();
	}// while
*/
	// Ptr zeigt jetzt auf die Klasse mit den meisten Werten
/*
	if( Ptr )
	{
		OldValue = Ptr->Value();
		Ptr->SetPartsPerNote( mostduration );
//		QuarterTempo = Ptr->Tempo( ppq );
*/
		// Jetzt den anderen Noten Längen zuordnen


	TDoubleBinClass *Temp = Head;
		while( Temp )
		{
		/*
			NewValue = (mostduration * Temp->Value()) / OldValue;


			diff =  (double)ppq / (double)NewValue;
			diff *= 100;
		 */
			double diff;
			if( Temp->Value() == 0 )
			{
				printf("TListe::FindQuarter Temp->Value()== 0");
				diff = 32000;
			}
			else
			{
				diff = PlayPPQ / Temp->Value();
//				diff =  Temp->Value() / PlayPPQ;
			}
			diff *= 100;
			/*
				diff bigt jetzt das Verhältniss zwischen der ungerundeten
				Länge von Temp und der Länge einer Viertelnote an
				Nach diesem Wert wird jetzt die Länge von Temp gerundet.
			*/
			TFrac NewValue = 0L;
			if( (diff>20) && (diff<28) )	// Ganze Noten
			{
				NewValue = TFrac(1,1);
			}
			else if( (diff>40) && (diff<60) )	// Halbe
			{
				NewValue = TFrac(1,2);
			}
			else if( (diff>80) && (diff<120) )  // Viertel
			{
				NewValue = TFrac(1,4);
			}
			else if( (diff>160) && (diff<240) )	// Achtel
			{
				NewValue = TFrac(1,8);
			}
			else if( (diff>320) && (diff<480) )	// Sechzehntel
			{
				NewValue = TFrac(1,16);
			}
			else if( (diff>680) && (diff<920) )	// 32tel
			{
				 NewValue = TFrac(1,32);
			}
			else
			{
			}
			Temp->SetPartsPerNote( NewValue );
			Temp = Temp->Next();
		} // while( Temp
  //	} // if( Ptr )

	return 1;
} // FindQuarter
//-----------------------------------------------------------------
double TDoubleBinList::Average( void )
{
	double res = 0;
	TDoubleBinClass *Temp = Head;
	while( Temp )
	{
		res += Temp->Value();
		Temp = Temp->Next();
	}
	if( Head )
		res /= (double)Count();

	return res;
}
double TDoubleBinList::SSQ( void )
{
	double av = Average();
	double res = 0;
	TDoubleBinClass *Temp = Head;
	while( Temp )
	{
		double sq = av - Temp->Value();
		sq *= sq;
		res += sq;
		Temp = Temp->Next();
	}
	if( Head ) 
		res /= (double)Count();
	return res;
}


/*
	create new class
*/
TDoubleBinClass * TDoubleBinList::addClass(double val,
                            double bias)
{

	// reset minWeight of classes
	setMinWeight( MinWeightI);
	TDoubleBinClass *temp = factory( val,CountI, bias );
	CountI++;
	// insert into sorted list
	insert( temp );

	// reset minWeight of classes
	setMinWeight( MinWeightI );
	return temp;
}


TDoubleBinClass *TDoubleBinList::addValue(double val, TDoubleBinClass *ptr)
{
	if( firstValue ) // first in List
	{
		setMinWeight(MinWeightI);
		normClasses(); // set average weight
//		write(stdout);
//		normSum = 1-(CountI*minWeight) ;
		firstValue = 0;
		// write(stdout);
	}

	if( !ptr )
	{
		TDistanceStruct ds = closestClass(&val);
		ptr = ds.classPtr;
		if( ds.distance != 0 )
			ptr = ptr;
	}
	else
	{
		avErrorSumI += fabs(ptr->FirstV() - val);
		cValsI++;
		errorSigmaSumI += pow(fabs(ptr->FirstV() - val) - avError(),2);
	}

	// normalize values;
	// subtract delta from all classes
	double delta = deltaI;
    double wSum = 0; // sum of weights, used for error correction
    TDoubleBinClass *temp = Head;
	double normSum = 0;
	while( temp )
	{
		normSum += temp->normalize(delta,  // countI == #of classes
								   MinWeightI);
        if( ptr != temp )
        	wSum += temp->weight();            
    	temp = temp->Next();
	} // while
	
    ptr->addValue( val, normSum);

    //	printf("vla: %f\n",val);
//	write(stdout);

	lpDirty = 1; 
	rpDirty = 1;

	return ptr;
}


void TDoubleBinList::addValue(double duration1,
						  TDoubleBinClass *classPtr1,
						  double w1,
						  double duration2,
						  TDoubleBinClass *classPtr2,
						  double w2,
						  double duration3,
						  TDoubleBinClass *classPtr3,
						  double w3,
						  double duration4,
						  TDoubleBinClass *classPtr4
						  )
{
	if( firstValue ) // first in List
	{
		setMinWeight(MinWeightI);
		normClasses(); // set average weight
		firstValue = 0;
		// write(stdout);
	}
	//	write(stdout);
	
		double delta = deltaI;

		// subtract delta from all classes
		double normSum = 0;
		TDoubleBinClass *temp = Head;
		while( temp )
		{
			normSum += temp->normalize(delta,  // countI == #of classes
									   MinWeightI);
			temp = temp->Next();
		}
		// add sum to 
		temp = Head;
	


	/// get used weight
	double lastPart = 1 - w1 - w2- w3;
	if( lastPart <= 0 )
		printf("ERROR: weight sum must be < 1!\n");

	double usedW = 0;
	int cClass = 0;

	if( classPtr1 )
	{
		usedW += w1;
		cClass++;
	}
	else
		w1 = 0;

	if( classPtr2 )
	{
		usedW += w2;
		cClass++;
	}
	else
		w2 = 0;
	if( classPtr3 )
	{
		usedW += w3;
		cClass++;
	}
	else
		w3 = 0;
	if( classPtr4 )
	{
		usedW += lastPart;
		cClass++;
	}
	else
		lastPart = 0;

	if( cClass == 0 )
	{
		printf("error: cClass == 0!\n");
		return;
	}

	double rel = (1/usedW);

/*
	if( (w1+w2+w3+lastPart)*rel > 1.0 )
		printf("sum %f\n", (w1+w2+w3+lastPart)*rel);
*/

	if( classPtr4 )
		classPtr4->addValue( duration4, normSum * lastPart * rel);

	if( classPtr3 )
		classPtr3->addValue( duration3, normSum * w3 * rel);

	if( classPtr2 )
		classPtr2->addValue( duration2, normSum * w2 * rel);

	if( classPtr1 )
		classPtr1->addValue( duration1, normSum * w1 * rel );


	lpDirty = 1; 
	rpDirty = 1;

}



TDoubleBinClass * TDoubleBinList::factory(double value,
                           int ID,
                           double bias)
{
	return new TDoubleBinClass( value,
						 ID,
                      bias);

}



/*
	reset weight of all classes to 1/countI;
*/
void TDoubleBinList::normClasses()
{

	createDistribution();
	return;

	double weight = avWeight();

	TDoubleBinClass *temp = Head;
	while( temp )
	{
		temp->setWeight( weight );
		temp = temp->Next( );
	}

//	resetMinWeight();

}

double TDoubleBinList::setMinWeight(double minWeight)
{

	// check if possible
/*
	if( minWeight > avWeight() )
		minWeight = avWeight();
*/
    if( CountI &&
        minWeight > 1.0 / (double)CountI )
    {
        minWeight = 1.0 / (double)CountI;
        minWeight /= 50.0;
    }
	// obsolete, will implicte worked out in addValue
	MinWeightI = minWeight;
	return minWeight;

	char needReset = 0;
	TDoubleBinClass *temp = Head;
    while( temp )
	{
		if( temp->weight() <  minWeight )
		{
			needReset = 1;
		}
		temp = temp->Next( );
	}

	if( needReset )
		Reset();
	return minWeight;
}







void TDoubleBinList::insert(TDoubleBinClass *ptr)
/*
	insert ptr into sorted list
*/
{
	if( !ptr )
		return;

	TDoubleBinClass	*prev = NULL;
	TDoubleBinClass	*current = Head;
	while( current &&
		   current->Value() < ptr->Value() )
	{
		prev = current;
		current = current->Next();
	}
	ptr->SetNext( current );
	if( prev )
	{
		prev->SetNext( ptr );
	}
	else
	{
		Head = ptr;
	}
}


void TDoubleBinList::debug( FILE *out  )
{
#ifdef _DEBUG
	if( !Head )
		return;

	FILE *out2 = fopen("_TLISTE.txt","wt");

	char needClose = 0;
	if( !out )
	{
		out = fopen("_TLISTE.csv","wt");
		needClose = 1;
	}

	if( !out )
	{
		printf("Can't open _TLISTE.csv!\n");
		return;
	}
	// get min and max of elements
	TDoubleBinClass *temp = Head;
	double min = temp->Value();
	double max = min;
	while( temp )
	{
		stringstream outStr;
		temp->debug( &outStr );
		fprintf(out, outStr.str().c_str());
		fprintf(out2, "\n");
		double cur = temp->Value();
		if(  cur < min )
			min = cur;
		else if ( cur > max ) 
			max = cur;
		temp = temp->Next();
	} // while

	// write output
	if( min < 0 )
		min *= 1.2;
	else
		min *= 0.8;

	if( max > 0 )
		max *= 1.2;
	else
		max *= 0.8;

	TDistanceStruct dTemp;
	for( double cur = min; cur < max; cur += 0.001 )
	{
		dTemp = closestClass(&cur);
		fprintf(out, "%f %f\n",cur, dTemp.distance );
	}

	if( needClose )
		fclose( out );
	fclose( out2 );
#endif
}

double TDoubleBinList::avError(void)
{
	double x = -1;
	if( cValsI )
	{
		x = avErrorSumI / (double)cValsI;
	}
	return x;
}

double TDoubleBinList::errorSigma(void)
{
	double x = 0.05;
	if( cValsI > 1)
	{
		x = sqrt(errorSigmaSumI / (double)(cValsI-1));
	}
	return x;
}

double TDoubleBinList::setMinMaxRel(double minMaxRel)
{
	setMinWeight(1.0/(minMaxRel + Count() ) );
	return MinWeightI;
}

double TDoubleBinList::maxWeight(void)
{
	double x = 1 - Count()*MinWeightI;
	return x;
}
/*
double TDoubleBinList::resetMaxWeight(double maxWeight)
{
	
	TDoubleBinClass *temp;

	temp = Head;
	char needReset = 0;

	// check if possible
	if( maxWeight > 1 )
		maxWeight = 1;

	while( temp )
	{
		if( temp->weight() >  maxWeight )
		{
			needReset = 1;
		}
		temp = temp->Next( );
	}
	MaxWeightI = maxWeight;

	if( needReset )
		Reset();
	return maxWeight;

}
*/
double TDoubleBinList::avWeight()
{
	if( Count() )
		return 1/(double)Count();

	return 0;
}

void TDoubleBinList::setMinweightDeltaRel(double rel)
{
	deltaI = MinWeightI/rel;
}

double TDoubleBinList::maxWeightOfElements()
{
	TDoubleBinClass *temp = Head;
	double res = 0;
	while( temp )
	{
		if( temp->weight() >  res )
		{
			res = temp->weight();
		}
		temp = temp->Next( );
	}
	return res;

}


double TDoubleBinList::preciseL()
{
	if( !lpDirty  )
	{
		return precL;
	}
	lpDirty = 0;

	int cValid = 0;
	double res = 0;

	TDoubleBinClass *temp = Head;
	while( temp )
	{
		if( temp->Count() > 0 )
		{
			cValid++;
			res += temp->preciseL;
		}
		temp = temp->Next();
	} // while
	if( cValid )
		res /= cValid;

	precL = res;

	return res;
}

double TDoubleBinList::preciseR()
{

	if( !rpDirty  )
	{
		return precR;
	}

	rpDirty = 0;
	
	int cValid = 0;
	double res = 0;
	TDoubleBinClass *temp = Head;
	while( temp )
	{
		if( temp->Count() > 0 )
		{
			cValid++;
			res += temp->preciseR;
		}
		temp = temp->Next();
	} // while
	if( cValid )
		res /= cValid;

	precR = res;

	return res;
}

/*
double TDoubleBinList::precise()
{

	if( lDirty >0 )
	{
		precL = preciseL();
		precR = preciseR();
		lDirty = 0;
	}
	return ( precL + precR) / 2;
}
*/

double TDoubleBinList::lAlpha()
{
	return alpha + (alpha*preciseL());
}

double TDoubleBinList::rAlpha()
{
	return alpha + (alpha*preciseR());

}

int TDoubleBinList::remove(int id)
{
	TDoubleBinClass *temp = Find(id);
	if( temp )
	{
		TDoubleBinClass *prev = temp->Prev();
		TDoubleBinClass *next = temp->Next();
		if( prev )
		{
			prev->SetNext(next);
		}
		else
		{
			Head = next;
		}
		delete temp;

		// recalc data
		CountI--;
		// renumber class id's
		next = Head;
		while( next )
		{
			if( next->ID() > id )
				next->setID( next->ID() - 1);
			next = next->Next();
		}

		setMinWeight( MinWeightI );
		return 1;
	}
	return 0;
}



void TDoubleBinList::createDistribution( void )
{
    // sum bias values, cound classes
    double sBias = 0;
    int count = 0;
    TDoubleBinClass *temp = Head;
    while( temp )
    {
        sBias += temp->bias;
        count++;
		temp = temp->Next();
    }
    // set weights
    temp = Head;
	while( temp )
	{
		if( sBias > 0 )
		{
			temp->setWeight(temp->bias / sBias);
		}
		else
		{
			temp->setWeight(1.0/count);
		}
		temp = temp->Next();
    } // while    
}

/// return best TDoubleBinClass with weight <= ptr->weight
TDoubleBinClass *TDoubleBinList::getBest( TDoubleBinClass *ptr )
{
    double bestWeight = -1,
           ptrWeight,
           ptrCount,
           bestCount = -1;
    /// 1. search neighbours for equals
    if( ptr )
    {
        ptrWeight = ptr->weight();
        ptrCount = Count();
        TDoubleBinClass *temp = ptr->Next();
        while( temp &&
               temp->weight() != ptrWeight &&
               temp->Count() != ptrCount )
        {
            temp = temp->Next();
        }
        if( temp )
            return temp;
    }
    else
    {
        ptrWeight = 2;
        ptrCount = -1;
    }
    // 2. if no eaqual search for smaller from begin
    TDoubleBinClass *best = NULL;
    TDoubleBinClass *temp = Head;
    while( temp )
    {
        if( !ptr ||
            temp->weight() < ptrWeight )
        {
            if( temp->weight() > bestWeight )
            {
                best = temp;
                bestWeight = temp->weight();
                bestCount = temp->Count();
            }
            else if( temp->weight() == bestWeight &&
                     temp->Count() > bestCount )
            {
                best = temp;
                bestWeight = temp->weight();
                bestCount = temp->Count();
            }
        }
        else if( ptr &&
                 temp->weight() == ptr->weight() &&
                 temp->Count() < ptr->Count() )
        {
            if( ptr->Count() > bestCount )
            {
                best = temp;
                bestWeight = temp->weight();
                bestCount = temp->Count();                
            }
        } // else if
		temp = temp->Next();
    } // while temp
    return best;
}

double TDoubleBinList::distance(TDoubleBinClass *ptr, 
						double *val, 
						TFrac *duration)
{	
	// ptr->distance should be
	/*
		p( val | ptr->[val] ) = Gauss([val] - val], sigma1)
		p( ptr, val  ) = Gauss([val]-val), ptr->weight) =  ptr->distance(val,sigma2)
		p( ptr->[val]  ) = ptr->weight  
		ptr->distance(val, sigma2)*G(val-ptr->av, sigma1)
	*/	
	
	if( ptr )
	{
		// todo: check this
		double mEW = avWeight();
		return ptr->distance(val, 
							duration,
							mEW, 
							this->alpha*(1-preciseL()), 
							this->alpha*(1-preciseR()),
							Count());
	}
	return -1;
}



int TDoubleBinList::readIni( const char *fname )
{
	TInifile *in = new TInifile( fname );
	int c = in->GetValFloat("COUNT","0");

	char *entryName = new char[80];
	for(int i = 1; i <= c; i++ )
	{
		sprintf(entryName,"%s%d",prefix,i);
		string line = in->GetValChar(entryName,"");
		parseLine( line );
	}

	delete [] entryName;
	delete in;
	return 1;
}

string TDoubleBinList::iniHeader( void )
{
	return string("format: { normalised IOIratio, bias } ");
}

int TDoubleBinList::writeIni( const char *fname )
{
	TInifile *in;
	in = new TInifile( fname );
	
	int c;
	c = in->GetValFloat("COUNT","0");

	char *entryName;
	string line;
	entryName = new char[strlen(prefix)+10];
	// remove existing entries
	int i;
	for(i = 1; i <= c; i++ )
	{
		sprintf(entryName,"%s%d",prefix,i);
		in->RemoveEntry(entryName);
	}

	in->SetEntry("FORMAT","SYNTAX",iniHeader().c_str());

	// create new entries

	TDoubleBinClass *current = Head;
	i = 0;
	double wSum = 0;
	while( current )
	{
		line = current->toString();
		in->SetEntryN(prefix,i+1,line.c_str() );
		wSum += current->weight();
		current = current->Next();
		i++;
	} // while

	in->SetEntry("COUNT",i);
	in->SetEntry("WEIGHTSUM",wSum);
	delete [] entryName;
	delete in;
	return 1;
}



int TDoubleBinList::parseLine(string line)
{
	int res = 0;
	
	TdoublePList *pList = new TdoublePList(line.c_str());	
	if( pList->c() > 0 )
	{
		double IOIratio,
			   bias;
		pList->val(0,IOIratio);
		pList->val(1,bias);

		addClass(IOIratio, bias);
		res = 1;
	}
	delete pList;

	return res;
}

void TDoubleBinList::addPattern(TPATTERN *pattern)
{
	if( !pattern )
		return;
		
	TPNOTE *cur = dynamic_cast<TPNOTE *>(pattern->firstNote());
	// no IOIratio for first note
	if( cur )
		cur = cur->GetNext();
	while( cur )
	{
		double IOIratio = cur->IOIratio();
		IOIratio = normIOIratio(IOIratio);
		addValue(IOIratio, NULL);
		cur = cur->GetNext();
	} // while
}

TDoubleBinClass * TDoubleBinList::addIfNew(double val)
{
	TDoubleBinClass *res = NULL;
	TDistanceStruct dStruct = closestClass(&val);
	if( fabs( dStruct.classPtr->FirstV() - val) > 0.001  )
	{
		res = addClass(val,0);
		addValue(val,res);
	}
	else
	{
		res = dStruct.classPtr;
	}
	return res;

}

TDoubleBinClass * TDoubleBinList::addExact(double val)
{
	TDoubleBinClass *res = NULL;
	TDistanceStruct dStruct = closestClass(&val);
	if( fabs( dStruct.classPtr->FirstV() - val) > 0.001 )
	{
		res = addClass(val,0);
	}
	else
	{
		res = dStruct.classPtr;
	}

	addValue(val,res);
	return res;	
}
