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
#include "notestat.h"
#include <math.h>

#include "statist.h"
//////////////////////////////////////////////////////////////////////
// TFracBinClass Klasse
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
#define LOG_DISTANCE

TFracBinClass::TFracBinClass(TFrac duration,
                     int id,
                     double b) : TDoubleBinClass( (double) duration.toDouble(),
                                           id,
                                           b)
{

	durationI = duration;

}

TFracBinClass::~TFracBinClass()
{

}


double TFracBinClass::Value( void )
{
	return durationI.toDouble();
}

double TFracBinClass::FirstV()
{
	return durationI.toDouble();
}

/*
	remarks:
		- val will be used from base classes
*/
double TFracBinClass::distance( double * /*val*/, 
						    TFrac *duration,
						    double, // avWeight, // uniform aaverage weight in list
							double, // lAlpha,
							double, // rAlpha,
							int cClasses)
{
	double nSigma = 0.5 + 0.5 * weight();

	double d1 = log((*duration).toDouble());
	double d2 = log(durationI.toDouble());
	double res = 1 - GaussWindow(d1,
								 d2,
								 nSigma );
	return res;

} // distance



TFrac TFracBinClass::duration()
{
	return durationI;
}


void TFracBinClass::addValue(double val, double norm)
{

#ifdef LOG_DISTANCE
//	val = log(val);
#endif
	TDoubleBinClass::addValue( val, norm );
}


void TFracBinClass::debug(FILE *out)
{
	
	fprintf(out,"dur: ");
	stringstream outStr;
	duration().Write( outStr );
	TDoubleBinClass::debug(&outStr);
	fprintf(out, "%s", outStr.str().c_str());
}


string TFracBinClass::toString()
{
	ostringstream res;

	res << "{" << duration().numerator() << ", " <<
		    duration().denominator() << ", " << weight() << "}";

	return res.str();
}
