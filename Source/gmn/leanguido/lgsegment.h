/*
	leanGUIDO class library
	Copyright (C) 2003  Juergen Kilian, SALIERI Project

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 *  lgsegment.h
 *
 */

/// Todo: add parse-error function to lgSegment?
/// Todo: add visitor concept
/// Todo: replace lgObject, lgEvent and lgVoice by containers ?


#ifndef __lgsegment_h__
#define __lgsegment_h__


#include <stack>
using namespace std;

#include "lgtag.h"

#include "lgchord.h"
#include "lgsequence.h"
#include "lgfactory.h"



/// if USE_GUIDO_PARSER is undefined a lgVersion without parser functionality can be compiled
#define USE_GUIDO_PARSER

//! GUIDO Segment based on lgChord with parsing support
class lgSegment : public lgChord
{
	//! curPointers for parsing
    lgSequence *curSequence;
    lgChord    *curChord;
    lgEvent    *curEvent;
    lgTag      *curTag;
	lgVoice	   *curChordVoice;

	stack<lgTag *> mTagStack;
	bool mCurRangeOpened;
protected:
	lgTag *popTag();
	void pushTag(const lgTag *tag);

public:
	/// ptr to the factory object
	lgFactory  *factory;
    lgSegment( 	lgFactory *fy );

#ifdef USE_GUIDO_PARSER
	/// result: 0 == ok, 1 == parse error
	int parseFile(FILE *file);
	/// result: 0 == ok, 1 == parse error
    int parseFile(std::string &filename);
	/// result: 0 == ok, 1 == parse error
    int parseBuffer(std::string *buffer);
    	/// result: 0 == ok, 1 == parse error
	int parseString( char *str );
	/// result: 0 == ok, 1 == parse error
	int parseString( string str )
    {
        return parseString(str.c_str() );
    };
#endif

    virtual ~lgSegment( void );
    /// append event in curSequence OR curChord!!!
    virtual lgEvent  *appendEvent( lgEvent *ev);

    /// create a new virtual lgRest event and append
    lgRest  *appendRest( long int durN,
                      long int durD,
                     int dots,
                      long int durPosN,
                      long int durPosD )
    {
        lgRest *temp;
        temp = factory->newRest(durN,
                              durD,
                              dots,
                              durPosN,
                       durPosD);

        appendEvent(temp );
        return temp;
    };

    /// create a new virtual lgEmpty and append
    lgEmpty *appendEmpty( long int durN,
                 long int durD,
                 int dots,
                 long int durPosN,
                 long int durPosD )
    {
    	lgEmpty *newEmpty = factory->newEmpty(durN,
                              durD,
                              dots,
                              durPosN,
                              durPosD);
        appendEvent( newEmpty );
        return newEmpty;
    };

    /// create a new virtual lgNote and append
    lgNote *appendNote( int pc,
                     int oct,
                     int acc,
                     long int durN,
                     long int durD,
                     int dots,
                     long int durPosN,
                     long int durPosD )
    {
        lgNote *temp;
        temp =  factory->newNote( pc,
                              oct,
                              acc,
                              durN,
                              durD,
                              dots,
                              durPosN,
                         durPosD);

            appendEvent(temp);
            return temp;
    };

    
    
    /// append chord in curSequence, if ch == NULL -> close current chord, return == appended chord!
    lgChord *appendChord( lgChord *ch);

    /// append tag in curSequence
    void insertTag( lgTag *tag);
    /// create a new tag and append
    /// overwrite this function for tag semantics check!
    virtual lgTag *appendTag(long int tagno,
                             const char *tagname );
                             
    /// create a new chord in curSequence
    lgChord *initChord( long int posNum, long int posDenom );
    /// create a new voice in curChord
    void initChordVoice( void );
    /// close curVoice
    void exitChordVoice( void );
    

    lgEvent* gd_noteInit (const char *id);
    void gd_noteAcc (int n);
    void gd_noteOct (int n);
    void gd_noteEnum (long int n);
    void gd_noteDenom (long int n);
    void gd_noteDot(void);
    void gd_noteDdot(void);
    void gd_noteTdot(void);
    void gd_noteAbsDur (long int n);

    void gd_seqAppendNote();

    /** called at "(" and at ")" */
    void gd_tagRange();
    void gd_addTag();
    void gd_seqAppendChord();
    void gd_chordInitNote();
    void gd_chordAppendNote();
    void gd_chordInit();

    void gd_tagEnd(void);



    virtual lgSequence *appendSequence( lgSequence *seq = NULL);

    void exitSequence( void );
    
    void closeTag(void);
    lgTag* getCurTag(void)
	{ return curTag; };
    
    lgSequence *firstSequence( void )
    {
        curSequence = dynamic_cast<lgSequence *>(firstVoice());
        return curSequence;
    };
    lgSequence *nextSequence( void )
    {
        if( curSequence )
            curSequence = dynamic_cast<lgSequence *>(curSequence->next());
        return curSequence;
    };

    virtual string toString( lgVoice *callingSeq = NULL )
    {
        return lgChord::toString( callingSeq );
    }
	
    virtual void write( FILE *out, lgVoice *v = NULL )
	{
		v = NULL;
		lgChord::write( out, NULL );
	}
	
	virtual void writeFile( const char *fname )
	{
		if( !fname )
			return;
		FILE *out;
		out = fopen(fname,"wt");
		if( !out )
			return;
		write(out);
		fclose(out);
		return;
	}
	// ls: einzeln Segment kann mehre Sequencen besitzen, die duch ID durchnummeriert werden. 
	lgSequence* searchByID(int n)
	{
		if(!n) return NULL;
		else
		{
			lgSequence *tmp=firstSequence();
			while(tmp && tmp->getID()<n) tmp=nextSequence();
			if(!tmp) return NULL;
			else if(tmp->getID()==n) return tmp;
			else return NULL;
		}
	};

}; 

int gd_info(const char *msg);
class lgSegment;
int gd_parseGMNFileMode(lgSegment *seg, const char* filename, int mode);
int gd_parseGMNFile(lgSegment *seg, const char* filename);
int gd_parseGMNBuffer(lgSegment *seg, const char* filename);

#endif
