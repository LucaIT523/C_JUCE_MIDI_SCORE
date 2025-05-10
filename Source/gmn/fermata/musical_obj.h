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
|	filename : musical_obj.H
|	Autor     : Juergen Kilian
|	Datum	    : 2003
|	classdeclaration for TMusicalObject
------------------------------------------------------------------*/
#ifndef __musical_obj_h__
#define __musical_obj_h__

#include "debug.h"

//-----------------------------------------------------------------
#include <stdlib.h>
#include "portable.h"
#include <stdio.h>

#include "fragm.h"


//------------- Bitpositions of Flag Lock ---------------------------------------
#define	ABSTIME  1
#define  DURATION 2

//-----------------------------------------------------------------
//typedef long int TAbsTime;
#include "defs.h"



// bitflags for meta types
//#define META_TEMPO 1
#define META_GRACE_AT_ONSET 2
#define META_GRACE_PRE_ONSET 4
#define META_TRILL 8
#define META_GLISSANDO 16
#define META_TURN 32
#define META_MORDENT_UP 64
#define META_MORDENT_DOWN 128
//#define META_METER 256
//#define META_KEY 512

/*
#define NOTE_CLASS "TNOTE"
#define MUSICAL_CLASS "TMUSICALOBJECT"
#define TEMPO_CLASS "TTEMPO"
#define KEY_CLASS "TKEY"
#define METER_CLASS "TMETER"
#define TEXT_CLASS "TTEXT"
*/

class TNOTE;
class TQNOTE;
class TQuantizedObject;
class TMetaTempo;
class TMetaMeter;
class TMetaKey;
class TMetaText;
class TCLICKNOTE;
class TMIDIFILE;

// for directional values
enum { backward = -1,
    stay,
    dirforward
};

class TTRACK;
/// class TMusicalObject, voice, AbsTime + links
class TMusicalObject
{
private:
    TMusicalObject
    /// ptr to next note in list
            *NextPtr,
    /// ptr to previous note in list
            *Prev;		

protected:
    /// Attackpoint if denominator == 1 -> clicktiming else score-timing
    TAbsTime AbsTime;
    /// -1 == undef, -2 == every voice, 0...n = voice-1
    int Voice;

 	/// generic id when parsed from gln
 	int mId;

public:
	void setId( int id )
	{ mId = id; };
	int getId( void)
	{ return mId; };

	double AbsTimeMS( TMIDIFILE *theMidifile );
	TMusicalObject( const TMusicalObject &ptr );
    /// write GUIDO output must be implemented in derived classes 
    /// return current duration denominator (for output beautifying)   
    virtual TAbsTime Convert( ostream &gmnOut/*! gmn output file */,
                                    	
                              TAbsTime preEndtime  /*! endpoint of pre-note */,
        TAbsTime prev /*! denominator of pre-note */,
        TTRACK          * /* Track0  control track (tempo, key, meter, ...) */
                )
    {
        gmnOut << "Error: Convert for unknown musical object!\n";
        preEndtime = prev; // supress warning
		return prev;
    } 
    /// get relational IOI, notes in both direction can be skipped!
    double IOIratio( int prevPos, int nextPos, int voice = -1 );
    /// inter onset interval
    TAbsTime ioi( int nSkip,
                  int voice = -1 );


    TMusicalObject(TFrac absTime)
    {
        AbsTime = absTime;
        NextPtr = NULL;
        Prev = NULL;
        Voice = -1;
        mId = -1;
    };

    virtual ~TMusicalObject(void){};
    /// convert tick timing into score timing, no quantisation
    virtual void ticksToNoteface( int ppq );


    /// -1 == undef, -2 == every, 0..n == voice-1
    void SetVoice( int voice ) 
    { Voice = voice; };
    /// -1 == undef, -2 == every, 0..n == voice-1
    int GetVoice( void )
    { return Voice; };

    virtual void Debug( FILE *out );

    TAbsTime GetAbsTime( void );
    virtual TAbsTime SetAbsTime( TAbsTime absTime )
    {
        AbsTime = absTime;
        return AbsTime;
    };

    virtual void SetNext( TMusicalObject *next );
    virtual void SetPrev( TMusicalObject *prev );

    virtual TMusicalObject *GetNext(int voice);
    virtual TMusicalObject *GetPrev(int voice);


};


#include "chunklist.h"

/// TChunkList with TMusicalObject ptrs
class TMusObjList : public TChunkList
{

	TDoubleChunkList *offsetList,
			   *oldWeightList,
			   *newWeightList;


public:
	void append(TMusicalObject *ptr );
	int getId( TMusicalObject *ptr );
	void sort( TMusObjList *stopList );
	TMusObjList( /// chunk size
				 int cS,
				 /// start id
				 int os = 0 ) : TChunkList( cS, os )
	{
		oldWeightList = new TDoubleChunkList( cS );
		newWeightList = new TDoubleChunkList( cS );
		offsetList = new TDoubleChunkList( cS );
	};
	virtual ~TMusObjList( void )
	{
		delete offsetList;
		delete oldWeightList;
		delete newWeightList;
	}
	/// return ptr at position id or NULL
	TMusicalObject *get( int id )
	{
		return (TMusicalObject *)(getP(id));
	}
	/// set ptr at pos id, return = 1 if ok, 0 if range error
	char set( int id, 
			  TMusicalObject *ptr )
	{
		return setP(id, ptr );
	}

	void setData( int i,
				  double oldWeight,
		          double newWeight,
				  TFrac &offset )
	{
		oldWeightList->set(i, oldWeight );
		newWeightList->set(i, newWeight );
		double temp = offset.toDouble();
		offsetList->set(i,  temp );
	}
	double offset( int i )
	{
		double temp;
		temp = offsetList->get( i );
		return temp;
	}
	double oldWeight( int i )
	{
		double temp;
		temp = oldWeightList->get( i );
		return temp;
	}
	double newWeight( int i )
	{
		double temp;
		temp = newWeightList->get( i );
		return temp;
	}
				  
	

	void Debug( FILE *out = NULL );

};


/// compare 1. abstimes and 2. pitch
int operator > (  TMusicalObject &n1,  TMusicalObject &n2 );
/// compare 1. abstimes and 2. pitch
int operator >= (  TMusicalObject &n1,  TMusicalObject &n2 );


TFrac GetIOI( TMusicalObject *ptr);
/// return 1 if IOI1 seems to equal to IOI2
char sameIOI( const TFrac &IOI1, const TFrac &IOI2 );

double GetIOIRatio(TMusicalObject *current);
/// get relational IOI for two IOI
double IOIratio( double ioi1, double ioi2);

/// todo: check usage
double IOIDistance(double e1,double e2); // compare two IOIratios

/*! return velocityRatio for TNOTE and TCLICKNOTE
	result 
		error : < 0 
		0 velocity : 0
		else ptr->Intens/next->Intens = (0..127]
*/
double velocityRatio( TMusicalObject *ptr);


/// ptr to TMusicalObject
typedef TMusicalObject *PTMusicObject;

#endif
