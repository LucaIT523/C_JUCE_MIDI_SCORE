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

/*
	Analyse voice classes
	-determine #voice profile of track
*/
#if !defined( __ANVOICE_H__ )
#define __ANVOICE_H__

// for afx defines!
#include "note.h"


#include <stdio.h>
#include "defs.h"
#include "q_midi.h"

#include "statist.h"
#include "c_track.h"

#include "../leanguido/lgsegment.h"

typedef int TPitch;

/// helper struct for pointer to delta voice times
typedef struct
{
	TAbsTime absTime;
	int deltaVoice;
	TNOTE *ptr;
} TVoiceEvent;



/// helper class for voice separation 
class TVoiceCountEntry
{
	friend class TVoiceCountList;
	TVoiceCountEntry *nextI;
    /// ptr to start note of segment
    TNOTE *noteI;
    /// # of voice in segment
    int  voicesI;
    /// # of notes in segment
    int  notesI; // # of notes
public:
	void recalcEntries( void );
    int cEntries( void );
    /// count events of a single voice
    int cVoiceEvents(int num = -1 /* / voice number*/
					);
    int maxVoice(void);
    void SetNotes( int cNotes )
    { notesI = cNotes; };
    int Notes( void ) // #of notes
    { return notesI; };
    TNOTE *notePtr( int pos = 0); // noteI of element[pos]
    TVoiceCountEntry( int voices,
                 TNOTE *note );
    virtual ~TVoiceCountEntry( void ) 
    { 	noteI = NULL; };
    TVoiceCountEntry *Next( void );
    TAbsTime Time( void );
    int Voices(void);
    void Write( FILE *out );
    int cVoicesAt(TNOTE *ptr);
protected: 
	virtual int append( TVoiceCountEntry *next );
};


// helper class for voice profile
class TVoiceCountList : public TVoiceCountEntry
{
	/// head of list
	TVoiceCountEntry *head;
	/// last element in list
	TVoiceCountEntry *tail;
public:
	TVoiceCountList( int voices,
		TNOTE *note ) : TVoiceCountEntry( voices, note )
	{
		head = this;
		tail = this;
	}
	virtual ~TVoiceCountList( void ){/* everything should be done in TVoiceCountEntry */};
    virtual int append( TVoiceCountEntry *next )
	{
		if( this == tail )
		{
			TVoiceCountEntry::append( next );
		}
		else
		{
			tail->append( next );
		}
		tail = next;
		return 1;
	}
	
};

void Debug( TVoiceCountEntry *start,
			FILE *out );
void Delete( TVoiceCountEntry *cur);

/// sorted list of time-stamped NOTE* 
class TPitchTime
{
	TPitchTime *nextI;
	TAbsTime timeI;
	TNOTE *ptrI;
public:
	TPitchTime( TNOTE *ptr,
             TAbsTime time );
	void SetNext( TPitchTime *next );
	TPitchTime *Next( void );
	TAbsTime Time( void );
	TPitch pitch(void);
	TNOTE *notePtr( void )
	{ return ptrI; };
};

class TQTRACK;
class TNOTE;

/*
void analyseVoices( TQTRACK *track,
                    FILE *log );
*/

TVoiceCountList *GetVoiceProfile( TQTRACK *track );
TVoiceCountList *GetVoiceProfile( TNOTE *from,
                              TNOTE *to);

// void ReCalc( TVoiceCount *start );
TVoiceEvent GetNextEvent( TNOTE *note,
                          TPitchTime *onsetList);

TNOTE *CheckEqualWindow( TNOTE *start,
                         int recTempo,
                         TAbsTime &res,
                         int EQUAL_TIME);

/// tool struct for storing a set statistical values
typedef struct{
	double BestAttDev,
			 SecondAttDev,
			 BestAttSSQ,
			 SecondAttSSQ,
			 BestDurDev,
			 SecondDurDev,
			 BestDurSSQ,
			 SecondDurSSQ;
	int    Count;
	} DevValues;
	
class TQNOTE;
/*
DevValues GetDeviations( TQNOTE *start,
                         TQNOTE *end );
                         */
//void markChords( TQTRACK *treck, TVoiceCount *voiceProfile );
//void analyseIOI(TQMIDIFILE *file);
//void analyseIOI(TQTRACK *track, FILE *out);

/// list of floats
class TfloatList 
{
private:
	double *entriesI;
protected:
	int cEntriesI;
	void init(int entries);
	void deleteList( void );

public:
	TfloatList(int entries);
	virtual ~TfloatList(void);
	virtual double entry(int i);
        /// write complete list, separated by 'space'
        virtual void write( FILE *out ); 
	virtual void writeEntry(int i, FILE *out);
        /// pattern syntax
        virtual void write(int from, int to, FILE *out); 
	virtual void writePattern( FILE *out);

	virtual int isAnchor( int i );
	int cEntries( void )
        { return cEntriesI; };

	virtual double weight( int i);
	void setEntry( int i, double val);
	double sum(void);
	double absSum(void);
};
typedef TNOTE *PTNOTE;
typedef TCLICKNOTE *PTCLICKNOTE;


/// tool class for TIOIList
typedef struct{
	int cValues;
	double sum;
	double sumOfDev;
} TstatVals;

#ifdef use_IOIList

/// array of IOIs
class TIOIList : public TfloatList
{
public :
	void recalc(TCLICKNOTE *from, TCLICKNOTE *to);
	TIOIList(TCLICKNOTE *from, TCLICKNOTE *to);
	TIOIList(lgEvent *from, lgEvent *to);
	TIOIList(THMIDIFILE *file);
	TIOIList( TNOTE *from, TNOTE *to );
	TIOIList( TNOTE *from, int c );
	~TIOIList( void )
        {
		if( notes)
			delete [] notes;
		if( cnotes )
			delete [] cnotes;
        };
	virtual void write( FILE *out );
	virtual void writeEntry(int i, FILE *out);
	virtual double entry(int i);
	virtual int isAnchor( int i );
    /// quantize entries to main values
    TstatVals preQuantize( TCLICKTRACK *track); 
	PTCLICKNOTE *cnotes;
	PTNOTE *notes;
};

/// IOIratio list
class TIOIratioList : public TfloatList
{
public :
	void preQuantize( void );
	TIOIratioList( TIOIList *IOIList );
	~TIOIratioList( void ){};
	virtual void write( FILE *out );
	virtual void writeEntry(int i, FILE *out);
	virtual int isAnchor( int i );
	TIOIList *IOIListI;
};
#endif
/*
double distance(TIOIList *list1, 
               TIOIList *list2,
               int segStart, /// index in list2
               int segEnd  /// index in list2
               );

double distance(TIOIratioList *list1, 
               TIOIratioList *list2,
               int segStart,	/// index in list2
               int segEnd		/// index in list2
               );
*/
void addValues(TFloatingAverage *floatingAverage,
               TVoiceCount *voiceProfile );

double GetIOIRel(double current, double next);

#endif
