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
|	filename : PATTERN.H
|	Author     : Juergen Kilian (kilian@noteserver.org)
|	Date	    : 17.10.1996-98-03
-----------------------------------------------------------*/

#ifndef __S_PATTERN_H__
#define __S_PATTERN_H__



//---------------------------------------------------------------

#include <string>
using namespace std; 
#include <sstream>



#include "note.h"
#include "../leanguido/lgsequence.h"


class TQNOTE;

//---------------------------------------------------------------

/// simple pattern note
class TPNOTE :  public lgNote
{

	///1 = n ote, 0 = rest
	char typeID; 

protected:

	lgTag * devTag;
   //! sigma^2 for delta to attackpoint
    double sigma2I;

    //! average delta of attackpoint
    double myI;

	/// performance av IOIratio
	double IOIratioI;

	/// performance av IOIratioStdDev
	double IOIratioStdDevI2;


public:
	double durationalAccent( void );
	TPNOTE *prevI;
	virtual void setNext( lgObject *ptr );
	TPNOTE * GetPrev( void );
	double IOIratio();
	TAbsTime IOI( void );
	void setTag( lgTag * tag );
	/// copy Abstiem etc. to qnote->Best/Sec, call shiftTo before!
	void copyTo( TQNOTE *qnote );

	virtual string toString( lgVoice *voice = NULL);
	TAbsTime GetDuration ( void );
	TAbsTime GetAbsTime( void );

	TPNOTE( int pc,
			int oct,
			int acc,
			long int posN,
			long int posD,
			int dots,
			long int durN,
			long int durD,
			char ty = 1, /// standard type = note
			double m = 0, /// 
			double s = 0); /// sigma == invalid
	TPNOTE( int pc,
			char ty = 1, /// standard type = note
			double m = 0, /// 
			double s = 0); /// sigma == invalid

	/// skip rests! for historical reasons!
	virtual TPNOTE  *GetNext( void );	

	char isRest( void )
	{
		return (typeID == 0);
	}

	void setStdDev( double s )
	{
		sigma2I = s;
		lgFloatTagArg *tagArg;
		if( devTag )
		{
			tagArg = dynamic_cast<lgFloatTagArg *>(devTag->findArg("stdDev"));
			tagArg->setValFloat( sigma2I );
		}
	}
	void setAvOffset( double m )
	{
		myI = m;
		lgFloatTagArg *tagArg;
		if( devTag )
		{
			tagArg = dynamic_cast<lgFloatTagArg *>(devTag->findArg("avOffset"));
			tagArg ->setValFloat( myI );
		}
	}

	void setIOIratio( double m )
	{
		IOIratioI = m;
		lgFloatTagArg *tagArg;
		if( devTag )
		{
			tagArg = dynamic_cast<lgFloatTagArg *>(devTag->findArg("IOIratio"));
			tagArg ->setValFloat( IOIratioI );
		}
	}
	void setIOIratioStdDev( double m )
	{
		IOIratioStdDevI2 = m;
		lgFloatTagArg *tagArg;
		if( devTag )
		{
			tagArg = dynamic_cast<lgFloatTagArg *>(devTag->findArg("IOIrStdDev"));
			tagArg ->setValFloat( IOIratioStdDevI2 );
		}
	}

	double stdDev( void )
	{
		return sqrt(sigma2I);
	};
	double IOIRatioStdDev( void )
	{
		return IOIratioStdDevI2;
	}
	double perfIOIratio( void )
	{
		return IOIratioI;
	}
	double avOffset( void )
	{
		return myI;
	};
	void calcStatistics( double n, TFrac attack );
}; // TPNOTE

//-----------------------------------------------------------------

class TPFILE;
/// simple pattern class
class TSPATTERN : public lgSequence
{

	friend class TPFILE;
protected:

	lgEvent   *Current;	
    /// aPriori probability of pattern, should be called only from TPFILE
    virtual double aPriori( double );

public:
	void setPrevPtrs( void );

	/// copy statitical values from tag list not PNOTES
	void initStatvals( void );
	TSPATTERN( void );
	virtual ~TSPATTERN( void );   

	/// return number of TPNOTE (!= lgSequence cNotes!)
	virtual long int  cNotes( void );



    //!Duration of pattern
    TAbsTime GetLength( void );	

	void GetLength( long int &Num, long int &Denom);

    /// get pattern id
	int 	GetID( void );			
    /// set pattern id

	void 	setID( int nr );

	TPNOTE *FirstNote( void )		
	{
		  Current = firstNote();
		  return dynamic_cast<TPNOTE *>(Current);
	};

	TPNOTE *NextNote( void )		
	{
		if( Current )
			Current = nextNote(Current);
		return dynamic_cast<TPNOTE *>(Current);
	}

}; // TSPATTERN

//-----------------------------------------------------------------

//-----------------------------------------------------------------

#endif
