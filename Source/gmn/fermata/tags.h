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
	Juergen Kilian
	9/1998, 2011
	Beat/Tag/Ornament detection Functions
*/

#if !defined(__TAGS_H__)
#define __TAGS_H__

#include <stdio.h>

//typedef long int TAbsTime;
#include "defs.h"



class TNOTE;
class TQNOTE;
class THTRACK;
class TTRACK;
class TQTRACK;
class THMIDIFILE;
class TInifile;

typedef  int TTagType;

#define TAG_UNDEF 0
#define TAG_SLUR  1
#define TAG_FERMATA 2
#define TAG_STACCATO 4
#define TAG_STRESS 8
#define SLUR_OFF 16
//#define TAG_GRACE 32

/// generic GUIDO tag list
class TTagList
{
	private:
		TTagList *nextI;
		/// TagType
		TTagType   TypeI;
		TNOTE *StartPtr,
				  *EndPtr;
		void PrintSingle( FILE *out );
	public:
		TTagList( int type,
					 TNOTE *start );
		TTagList( int type,
					 TNOTE *start,
					 TNOTE *end );
		void Print( FILE *out );
		void SetEnd( TNOTE *end );
		TTagType Type( void )
			{ return TypeI; };
		TNOTE *Start( void );
		TNOTE *End( void );
		TTagList *Next( void )
				{ return nextI; };
		void SetNext( TTagList *next );
		virtual double weight( void ){return 0;};
};

/// generic GUIDO tag list with weight feature
class TWeightedTag : public TTagList  
{
	double WeightI;
public:
	TWeightedTag( int type,
					 TNOTE *start,
					 double weight);
	TWeightedTag( int type,
					 TNOTE *start,
					 TNOTE *end,
					 double weight);
	virtual ~TWeightedTag( void );
	virtual double weight(void);

};

//------------------------------------------
typedef TTagList *TagPtr;

int ListSize( TTagList *tags);
TagPtr *CreateOnsetIndex( TTagList *tags );
TagPtr *CreateOffsetIndex( TTagList *tags);

TTagList *Merge( TTagList *temp1, TTagList *temp2 );

typedef TTagList *(* PPROC)(TQNOTE *start, TInifile *tempInifile);


TTagList *DoForAllVoices( TQTRACK *track,
							PPROC fptr);

TTagList *DoForAllTracks( THMIDIFILE *file,
							PPROC fptr);


TTagList *MarkStressTrack(TQTRACK *track );


TTagList *MarkStressByDuration( TQNOTE *start, TInifile *tempInifile );
TTagList *MarkStressByDuration( TQTRACK *track );

TTagList *MarkStressByIntens( TQNOTE *start, TInifile *tempInifile );
TTagList *MarkStressByIntens( TQTRACK *track );

TTagList *MarkStressByNumber( TQNOTE *start, TInifile *tempInifile );
TTagList *MarkStressByNumber( TQTRACK *track );

TTagList *MarkSlur(THMIDIFILE *file );
TTagList *MarkSlur( TQTRACK *track );
TTagList *MarkSlur( TQNOTE *track, TInifile *inifile );

TTagList *MarkStaccato(THMIDIFILE *file );
TTagList *MarkStaccato( TQTRACK *track );
TTagList *MarkStaccato( TQNOTE *track, TInifile *inifile  );

void DeleteTags( TTagList *start );
TTagType GetTags( TTagList *tags,
						TAbsTime start,
						int voice);
void PrintTags(ostream &gmnOut,
					 TTagType PrevTags,
					 TTagType CurTags);



#endif

