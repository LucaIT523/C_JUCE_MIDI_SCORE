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

 #include "element.h"
 // #include "funcs.h"
 // #include "portable.h"
#include <math.h>
#include "statist.h"
//-----------------------------------------------------------------
// TDoubleBinClass is been used to collect all played durations which might
// belong to the same score duration
TDoubleBinClass::TDoubleBinClass( double value,	// first value added to this element
					int  id,	// class id
                    double b )/// bias
{
    bias = b;
    FirstVI   = value;	// firstvalue
	IDI   = id;	// id of class

	CountI = 0;			// nr of entries
//todo change TDoubleBinClass constructor

	nextI  = NULL;		// 
	PrevI  = NULL;
	PossibleTempoI = 0;	
	PartsPerNoteI  = 0L;  // midi ticks per note
//	minWeightI = -1;	// needs to be reset later
	weightI = 0;    // needs to be reset later
   	preciseL = 0;
	preciseR = 0;

	avI = value; // average

	ex = 0;
	ex2 = 0;
	cLocalMax = 0;
	myI = value;
	sigma2I = 0;
} // TDoubleBinClass;

//-----------------------------------------------------------------
TDoubleBinClass* TDoubleBinClass::SetNext( TDoubleBinClass *ptr )
{
		nextI = ptr;
		if( ptr )
			ptr->SetPrev(this);
		return ptr;
} // SetNext;
//-----------------------------------------------------------------
// reset counter
void TDoubleBinClass::Reset( void )
{
	CountI = 0;
} // Reset
//-----------------------------------------------------------------
// output for debugging
void TDoubleBinClass::debug( ostream *out )
{
//	(*out)<< "Number:%d, ", NumberI );
	(*out)<< "1st :"<< FirstVI <<", ";
	(*out)<< "l: "<< preciseL<<", ";
	(*out)<< "r: " << preciseR<<", ";
	(*out)<< "#:" << CountI <<", ";
	(*out)<< "w: "<< weight()<<", ";
	(*out)<< "l: " << cLocalMax<<", ";
	(*out)<< "my: " <<  myI<<", ";
	(*out)<< "s: " <<  sigma2I <<", ";
//	(*out)<< "E:%f", E() );
//	(*out)<< "V:%f", V () );

//	(*out)<< "T:%d, ", PossibleTempoI );
//	(*out)<< "PPN:%ld/%ld \n", PartsPerNoteI.numerator(),
//											 PartsPerNoteI.denominator() );
} // debug
//-----------------------------------------------------------------
// Insert new element into class.
// value is not normalized right now
double TDoubleBinClass::CountUp( double newvalue )
{
	double Mult = 0;

//	long   Temp;
	CountI++;
	if( FirstVI == 0 )
	{
		// Printf( "TDoubleBinClass::CountUp First VI == 0!\n");
	}
	else if( newvalue == 0 )
	{
		// Printf( "TDoubleBinClass::CountUp newvalue == 0!\n");
		// Printf( "Warning: ClickTrack may be not correct!\n");
		Mult = 1;
	}
	else
	{
		// return relation between first value and current value
		Mult = newvalue / FirstVI;
	}
	return Mult;
} // CountUp
//-----------------------------------------------------------------
/*
	merge to succeding classes
*/
void TDoubleBinClass::AddNext( void )
{
/*
	  TDoubleBinClass  *Temp,
					*NextKlass;

	  NextKlass = Next();

	  if( NextKlass )	// gibt es eine nächste Klasse
	  {
		// Max und Min Daten neu setzen
		if ( ValueLowI > Next()->ValueLowI )
			ValueLowI = Next()->ValueLowI;
		if ( ValueUpI < Next()->ValueUpI )
			ValueUpI = Next()->ValueUpI;
		// Durchschnitt bilden
		ValueI = ( ValueI * CountI ) +
			 ( Next()->ValueI * Next()->CountI );
		ValueI /= ( CountI + nextI->CountI );
			CountI += Next()->CountI;

		FirstVI = (FirstVI + Next()->FirstVI) / 2;

		Temp = this;
		while( Temp->nextI != NextKlass )
			Temp = Temp->nextI;
		Temp->LinkToNext = 1;	// Flag setzen für gelinkte Klasse
	  } // if( NextKlass )
*/
} // AddNext;
//-----------------------------------------------------------------
// return next class
TDoubleBinClass *TDoubleBinClass::Next( void )
{
		return nextI;
} // Next
//-----------------------------------------------------------------
// search for class(id)
char TDoubleBinClass::Find( int id )
{
	TDoubleBinClass *Temp;
	char erg = 0;

	Temp = this;
	if( Temp && (Temp->IDI == id ))
		erg = 1;
	return erg;
} // Find
//-----------------------------------------------------------------
// calculate PossibleTempoI with assumption that the score duration of this class
// is a quarter note
void TDoubleBinClass::MakeTempo( int filePpq,		// PPQ at recording
						  int fileTempo )	// Tempo at recording
{
	if( FirstVI == 0 )
	{
		// Printf("TDoubleBinClass::MakeTempo FirstVI == 0!\n");
		PossibleTempoI = 32000; // dummy value 
	}
	else
	{
		double temp = 0;
		temp  = fileTempo * (filePpq / FirstVI);
		PossibleTempoI = temp;
	}

} // MakeTempo
//-----------------------------------------------------------------

/*
	Distance to IOIList entry
	remarks:
		- duration will be used from derived classes
*/
double TDoubleBinClass::distance(double *val, 
						 TFrac * /*duration*/,
						 double avWeight  /* uniform average weight in list */,
						 double lAlphaBin /* alpha value for class */,
						 double rAlphaBin,
						 int cClasses)
{
	if( !val )
		return -1;

	double _alphaBin;

	// calculate unbiased distance
	double dist = *val - FirstVI;

	if( dist <= 0 )
		_alphaBin = lAlphaBin;
	else
		_alphaBin = rAlphaBin;

	 dist *= dist;
	// dist = fabs(dist);

	// do biasing

	// version 2: encode weight into slope

//	nWeight = weight()/maxWeight; // (0...1]
	/// normalised weight
	double nWeight = weight();
//	alpha = 1+_alphaBin*(1-nWeight); // (1..1+alphaBin]
	double alpha = 10 * avWeight/weight()*(1-weight()); // (1..1+alphaBin]

	if( alpha <= 0 )
		alpha = 10;

	// low alpha -> high attraction
	// high alpha -> small attraction
	// todo: use normal_pdf for distance!!!
	double dist2;
	{
		// normalise sigma
		double sigma = 0.3;
		nWeight =  cClasses * weight(); 
		nWeight = pow(nWeight,0.5);
		
		double nSigma = nWeight * sigma;
		// hard limit of max sigma
		if( nSigma > 0.7 )
			nSigma = 0.7;
		else if( nSigma < 0.01)
			nSigma = 0.01;
	
		double p1 = GaussWindow(*val, FirstVI, nSigma); // == P(A|B);
		dist2 = 1 - p1;
	}
	dist = 1 - exp(-(alpha *dist));
	
/*
	printf("v:%f, p:%f, a_i:%f, a:%f, w:%f, d:%f\n",
			val,
			FirstVI,
			alpha,
			_alphaBin,
			weight(),
			dist);
*/
	return dist2;
}



/*
	- decrease weightI for delta
	- limit weightI to minWeightI
	- return distance between old and new weightI

*/
double TDoubleBinClass::normalize(double delta,
						  double minWeightI)
{
	double res;
    // adaptive strategie
    // 
    delta *= (weightI-minWeightI)/(1-minWeightI);

    
    res = 0;
    if( weightI < minWeightI )
	{
        // uninitialised value, set to minWeightI
        res = weightI - minWeightI;
		weightI = minWeightI;
	}
	else if( weightI - delta < minWeightI )
	{
        // limit to minweight, should never happen
        res = (weightI - minWeightI);
		weightI = minWeightI;
	}
	else // std processing
	{
		weightI -= delta;
		res = delta;
	}
		
	return res;
}

void TDoubleBinClass::addValue(double val, double delta)
{
	CountI++;
	{
		double mult;
		mult = CountI+1; // use firstVI as first my
		mult = (mult-1)/mult;

		sigma2I = sigma2I * mult + pow(val - myI,2)/((double)CountI+1.0);		
		myI = (myI*CountI + val)/((double)CountI+1.0);
	}

	// printf("Add: %f, %f\n", val, FirstVI );
	weightI += delta;
	double dist;
	dist = FirstVI - val;
	dist *= dist; // variance

	if( val > FirstVI  )
	{
		if( dist > preciseR )
			cLocalMax++;
		preciseR = (preciseR*0.5) + (dist*0.5);
	}
	else
	{
		if( dist > preciseL )
			cLocalMax++;
		preciseL = (preciseL*0.5) + (dist*0.5);
	}

	avI = avI*0.8 + val*0.2; 

	ex += val * phi(val);
	ex2 += (val*val) + phi(val*val);
}

double TDoubleBinClass::weight( void )
{

	return weightI;
}

TDoubleBinClass * TDoubleBinClass::Prev()
{
	return PrevI;
}

void TDoubleBinClass::SetPrev(TDoubleBinClass *prev)
{
	PrevI = prev;
}




void TDoubleBinClass::setWeight(double weight )
{
	if( weight > 0 )
		weightI = weight;

}



double TDoubleBinClass::variance()
{
	double res;

	res = preciseL + preciseR;
	return res;
}


/*
void TDoubleBinClass::debug(FILE *out)
{
	(*out)<< "val: %f, weight:%f, #%d", 
			FirstVI,
			weightI,
			CountI );
}
*/
double TDoubleBinClass::E()
{
	if( Count() )
		return ex/Count();
	else
		return FirstV();
}

double TDoubleBinClass::V()
{
	return ex2-(ex*ex);
}

double TDoubleBinClass::phi(double val)
{
#ifndef M_PI
	#define	M_PI		3.14159265358979323846
#endif
	#define _sigma 0.5
	
	return (1/(_sigma*sqrt(2*M_PI))*exp(-0.5*pow((val-FirstV())/_sigma,2)));
}

double TDoubleBinClass::stdDev( void )
{
	return sqrt(V());
}

string TDoubleBinClass::toString()
{
	ostringstream res;

	double val;
	val = Value();
	res << "{" << val << ", " << weight() << "}";

	return res.str();
}


TFrac getDuration( TFrac cDuration, // score duration of prev note
				   TDoubleBinClass *tempClass) // selected IOI class
{	
//	if( !tempClass )
//		Printf("Error: tempClass == NULL!\n");
	double durationF = (double)cDuration.toDouble();	
	double rIOI = tempClass->FirstV();
	
	// normalize
	if( rIOI > 0 )
		durationF *= (rIOI+1);
	else
		durationF /= ((rIOI-1)*-1);
	
	// convert into Frac
	// include 32nd=x/32. 16tripl=x/48, 5tplets=x/20  
	TFrac sDuration = TFrac((long)(durationF*960+0.5), 960L);
	return sDuration;
}

