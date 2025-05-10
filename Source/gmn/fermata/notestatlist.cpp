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

#include "notestatlist.h"

#include <stdlib.h>
#include "../lib_src/ini/ini.h"
#include "statist.h"
#include "../leanguido/lgsegment.h"
#include "../leanguido/lgtagarg.h"
//////////////////////////////////////////////////////////////////////
// TFracBinList Klasse
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
/*
 sort high->low
 by renumbering the class id's 
 */
void TFracBinList::sort( void )
{
    char success;
    TFracBinClass *e1,
        *e2;
    do // bubble sort
    {
        success = 1;
        for( int i = 1; i < Count(); i++ )
        {
            e1 = dynamic_cast<TFracBinClass *>(Find(i-1));
            e2 = dynamic_cast<TFracBinClass *>(Find(i));
            if( e1->duration() < e2->duration() )
            {
                e1->setID( i );
                e2->setID( i-1 );
                success = 0;
            }
        }
        
    }
    while( !success );
}

TFrac TFracBinList::operator [] (int i)
{
    TFrac res;
    if( i >= 0 &&
        i < CountI )
    {
        TFracBinClass *temp;
        temp = dynamic_cast<TFracBinClass *>(Find( i ));
        if( temp )
            res = temp->duration();
    }
    return res;
}
TFracBinList::TFracBinList(double al /* alpha value for distance to classes */
                             ) : TDoubleBinList( 100,100, al), 
	typicalQuarterIOI(0)
{
	prefix = "Dur";
	valueSigma = 0.5;
}

TFracBinList::~TFracBinList()
{

}

TDoubleBinClass * TFracBinList::factory(TFrac duration,
                                  int id,
                                  double bias)
{
	return new TFracBinClass(duration, id, bias );
}

 

TFracBinClass * TFracBinList::addClass(TFrac duration,
                                    double bias)
{

	TFracBinClass *temp,
			 *prev,
		     *cur;

	CountI++;



	// reset minWeight of classes
	setMinWeight(MinWeightI);



	temp = (TFracBinClass *)factory( duration,
                                 CountI - 1, /* id */
                                 bias);

//	temp = new TDoubleBinClass( val, 0, minWeight );

	
	cur = (TFracBinClass *)Head;
	prev = NULL;
	while( cur &&
		   cur->Value() < duration.toDouble() )
	{
		prev = cur;
		cur = (TFracBinClass *)cur->Next();
	}

	if( prev )
		prev->SetNext( temp );  // temp->setPrev will be called insid
	else
	{
		Head = temp;
		createDistribution();
	}

	temp->SetNext(cur);			// cur->setPrev will be called insid
	return temp;

} // addClass


TDoubleBinClass  *TFracBinList::addValue(TFrac duration,
							 TDoubleBinClass *classPtr)
{

	TDoubleBinClass *temp;
	temp = classPtr;
	if( !temp )
	{
		TDistanceStruct dist;
		dist = closestClass(NULL, 
							&duration);
		temp = dist.classPtr;
	}
	if( temp )
	{
		return TDoubleBinList::addValue(duration.toDouble(), temp );
	}
	return NULL;
	
} // addValue


void TFracBinList::addValue(TFrac duration1,
						  TDoubleBinClass *classPtr1,
						  double w1,
						  TFrac duration2,
						  TDoubleBinClass *classPtr2,
						  double w2,
						  TFrac duration3,
						  TDoubleBinClass *classPtr3,
						  double w3,
						  TFrac duration4,
						  TDoubleBinClass *classPtr4
						  )
{
	TDoubleBinList::addValue(duration1.toDouble(),
					classPtr1,
					 w1,
					 duration2.toDouble(),
					classPtr2,
					w2,
					duration3.toDouble(),
					classPtr3,
					w3,
					duration4.toDouble(),
					classPtr4
						  );
}



/*
	probability for this duration
*/
double TFracBinList::minDist(TFrac pDuration)
{
	TDoubleBinClass *temp;
	double res = -1,
		  dist;

	temp = Head;

	double mEW;
	mEW = maxWeightOfElements();

	if( temp )
		res = distance(temp, NULL, &pDuration);

	if( temp )
		temp = temp->Next();
	while( temp )
	{
		dist = distance(temp, NULL, &pDuration);
		if( dist < res )
			res = dist;
		temp = temp->Next();

	}
	return res;
} // prob


void TFracBinList::debug( FILE *out )
{
	if( !Head )
		return;

	FILE *out2;

	out2 = fopen("_TNoteStatList.txt","wt");

	char needClose = 0;
	if( !out )
	{
		out = fopen("_TNoteStatList.csv","wt");
		needClose = 1;
	}

	if( !out )
	{
		printf("Can't open _TNoteStatList.txt!\n");
		return;
	}

	// get min and max of elements
	double min, max, cur;
	TFracBinClass *temp;
	temp = (TFracBinClass *)Head;

	
	min = temp->Value();
	max = min;
	while( temp )
	{
		temp->debug(out2);
		fprintf(out2, "\n");
		cur = temp->Value();
		if ( cur > max ) 
			max = cur;

		temp = (TFracBinClass *)(temp->Next());
	}

	// write output
	if( min < 0 )
		min *= 2;
	else
		min /= 2;

	if( max > 0 )
		max *= 2;
	else
		max /= 2;

	TFrac cur2, min2, max2;
	min2 = TFrac( -2 );
	max2 = TFrac( (long)(max * 1000+0.5), 1000L);
	TDistanceStruct dTemp;
	for( cur2 = TFrac(1,1000); cur2 < max2; cur2 += TFrac(1,1000) )
	{
		dTemp = closestClass(NULL, &cur2);
		fprintf(out, "%f %f\n",cur2.toDouble(), dTemp.distance );
	}

	if( needClose )
		fclose( out );
	fclose( out2 );
}

double TFracBinList::probability(const TFrac &duration)
{
	double res = 0;
	TFracBinClass *temp;
	temp = dynamic_cast<TFracBinClass *>(Head);
	while( temp )
	{
		if( temp->duration() == duration )
			return temp->weight();
		temp = dynamic_cast<TFracBinClass *>(temp->Next());
	}
    /*
    if( res < 0 || res > 1 )
    {
        printf("illegal prob %f\n",res);
        exit(20);
    }
     */
    return res;
	
}

// return clsoest (n*Class_dur)
TFracBinClass * TFracBinList::closestTypicalDuration(double IOI,
													 double sigma,
													 int &bestN)
{
	TFracBinClass *res = NULL;
	double bestDist = -1;
	double bestWeight = 0;

	bestN = -1;

	TFracBinClass *temp;
	temp = dynamic_cast<TFracBinClass *>(Head);
	while( temp )
	{
		double delta;

		int n = 1;
		double clTickIOI = tickDuration(temp,1);
		// search for closest multiple
		if( temp->duration().denominator() == 4 )
		{
			while( fabs( n*clTickIOI - IOI ) > fabs( (n+1)* clTickIOI - IOI ))
			{
				n++;
			}
		}

	    delta = fabs( n*clTickIOI - IOI );
		double pThis = 0;
		if( sigma > 0 )
		{
			pThis = GaussWindow( IOI,
								n * clTickIOI,
								sigma );
			// pThis *= GaussWindow(n,1,4.0);
//			double p = pThis + temp->weight() - pThis*temp->weight();
			double p = pThis * temp->weight();
			if( clTickIOI &&
//			  ((pThis*0.7 + temp->weight()*0.3 > bestWeight)
		      ((   pThis > 0.6 &&
//				 pThis*temp->weight() > bestWeight)			
				 p  > bestWeight)			
			  || bestDist < 0) )
			{
				/*
				if( bestDist < 0 ||
					n < bestN ||
					bestWeight > pThis*temp->weight()*1.2 
					) // prefer small n
				*/
				{

					// bestWeight = pThis * temp->weight();
					bestWeight = p; 				
//					bestWeight =  pThis*temp->weight(); 				
					bestDist = delta;
					res = temp;
					bestN = n;
				}
			}
		}
		else
		{
			char useIt = 0;
			if( clTickIOI )
			{
			if( bestDist < 0 )
			{
				useIt = 1;
			}
			else if( delta < bestDist )
			{
				useIt = 1;
			}
			else if(delta < bestDist*1.1 &&
				    temp->weight() > bestWeight)
			{
				useIt = 1;
			}
			}

			
			if( useIt )
			{
				bestWeight = temp->weight();
				bestDist = delta;
				res = temp;
				bestN = n;
			}
		}
		temp = dynamic_cast<TFracBinClass *>(temp->Next() );
	}	
	return res;
}

void TFracBinList::updateTypicalDurations(TFrac &dur,
										  double typDur,
										  double significance)
{

	double quarterIOI;
	double curRel;
	double quarter = 0.25;
	// norm to quarter duration
	
	if( significance > 1 ||
		significance  < 0 )
	{
		printf("Illegal significance!!!\n");
		return;
	}
	curRel = dur.toDouble() / quarter;
	quarterIOI = typDur / curRel;
	
	if( typicalQuarterIOI )
	{
		typicalQuarterIOI = typicalQuarterIOI *(1-significance) + quarterIOI * significance; 
	}
	else
	{
		typicalQuarterIOI = quarterIOI;
	}
}


//DEL double TFracBinList::distance(const TFrac &dur)
//DEL {
//DEL 
//DEL }

TFracBinClass * TFracBinList::findExact(TFrac &duration)
{
	TFracBinClass *temp;
	temp = dynamic_cast<TFracBinClass *>(Head);
	while( temp && 
		   temp->duration() != duration )
	{
		temp = dynamic_cast<TFracBinClass *>(temp->Next() );
	}
	return temp;

}


int TFracBinList::parseLine(string line)
{
	TdoublePList *pList;
	pList = new TdoublePList(line.c_str());
	
	if( pList->c() > 0 )
	{
		double num,
			   denom,
			   bias;

		pList->val(0,num);
		pList->val(1,denom);

		pList->val(2,bias);

		TFracBinList::addClass(TFrac((long)num,(long)denom), bias);
		return 1;
	}
	delete pList;
	return 0;
}


string TFracBinList::iniHeader( void )
{
	return string("{ numerator, denominator, bias } ");
}

TFracBinClass * TFracBinList::addIfNew(TFrac frac)
{
	TFracBinClass *res = NULL;
	res = findExact( frac );
	if( !res )
	{
		res = addClass(frac, 0 );
		addValue(frac, res );
	}
	else
		res = NULL;
	return res;
}


/// write in gmn syntax
int TFracBinList::readIni( const char *fname )
{
	lgSegment *seg;
	
	seg = new lgSegment(new lgFactory());
	string filename = string(fname);
	seg->parseFile(  filename);
	int i = 0;
	// convert into list
	lgSequence *cSeq;
	cSeq = seg->firstSequence();
	if( !cSeq ) // empty file
	{
		// parser error, try old file format
		return TDoubleBinList::readIni( fname );
	}
	else // convert
	{
		while( cSeq )
		{
			i++;
			lgNote *cNote;
			cNote = cSeq->firstNote();
			if( cNote )
			{
				lgTag *cTag;
				cTag = cSeq->firstTag("\\statist");
				double bias = 1;
				string dummy;
				if( cTag )
					cTag->getParamFloat("w",1,bias,dummy);
				TFrac dur( cNote->duration() );
				this->addClass(dur, bias);
			}
			cSeq = seg->nextSequence();
		} // while
	}
	createDistribution();
	delete seg;
	return i;
}
/// write in gmn syntax
int TFracBinList::writeIni( const char *fname )
{
	lgSegment *seg = new lgSegment(new lgFactory());
	TFracBinClass *temp;
	int i = 0;
	temp = dynamic_cast<TFracBinClass *>(Head);
	while( temp )
	{
		TFrac cDur;
		cDur = temp->duration();
		double weight;
		weight = temp->weight();
		seg->appendSequence( NULL );
		lgTag *cTag;
		cTag = seg->appendTag(i++,"\\instr");
		lgStrTagArg *iName;
		char tID[10];
		sprintf(tID,"id=%d",temp->ID());
		string u = string("");
		string id = string(tID);
		iName = new lgStrTagArg(u,id);
		cTag->addArg( iName );
		cTag = seg->appendTag(i,"\\statist");
		seg->appendNote(0, // pc
						0, // acc
						0, // octave
						cDur.numerator(),
						cDur.denominator(),
						0, // dots
						0, // pos
						0);
		seg->closeTag();
		lgFloatTagArg *cArg;
		string w = string("w");

		cArg = new lgFloatTagArg(w, weight,u );
		cTag->addArg(cArg);
		i++;
		temp = dynamic_cast<TFracBinClass *>(temp->Next() );
	}
	FILE *out = fopen(fname,"wt");
	fprintf(out, "(* duration distribution for midi2gmn \n");
	fprintf(out, "only the first note of each segment will be used, pitch is ignored!\n");
	fprintf(out, "w = weight of duration classe.*)\n");
	seg->write( out );
	fclose(out);
	delete seg;
	return i;	
}


TFracBinClass * TFracBinList::addExact(TFrac frac)
{
	TFracBinClass *res;
	res = findExact( frac );
	if( res )
		addValue(frac,res);
	else
	{
		res = addClass(frac,0);
		addValue(frac,res);
	}
	return res;
}

double TFracBinList::tickDuration(TFracBinClass *ptr, int bestN)
{
	if( !ptr )
		return 0;
	TFrac sDur = ptr->duration();
	return tickDuration(sDur) * bestN;

}

double TFracBinList::tickDuration(TFrac &sDur)
{
	// normalise to quarter;
	double rel = sDur.toDouble() / 0.25;
	double res;
	res = typicalQuarterIOI * rel;
	return res;
}

