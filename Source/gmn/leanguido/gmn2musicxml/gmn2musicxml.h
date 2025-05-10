/*!
 *  gmn2musicxml.h
 *  GUIDO to Musicxml converter
 *
 *  Created by JŸrgen Kilian on Tue Apr 15 2003 during
 *  a never ending train ride from Paris to Lyon
 */

#ifndef __gmn2musicxml_h__
#define __gmn2musicxml_h__

#include "../lgchord.h"
#include "../lgsegment.h"
#include "lgxmlfactory.h"

#include <sstream>

/// tag types
enum{ 
		tEventTag /* tag belongs to events */,		
	    tMeasureTag /* tag belongs to measure */,
      	tProcessed /* tag has already be processed */
};


/// a MusixXML Tag 
class mXMLLgTag : public lgTag
{
public:
    mXMLLgTag( long int id /*! unique id */,
           	   lgEvent *pEv /*! prev event */,
               const char *nameI /*! tag name including \ */) : lgTag(id, pEv, nameI )
    {
    		// which tag type
          if( !strcmp(nameI, "\\key") ||
              !strcmp(nameI, "\\meter") )
          {
              tagType = tMeasureTag;
          }
          else
          {
              tagType = tEventTag;
          }
    };

    virtual string toString( void  );
    /// type == event | measure | processed
    int tagType;
};

/// a MusicXML chord
class mXMLLgChord : public lgChord
{
public:
    mXMLLgChord(   long int posNum,
                 long int posDenom) : lgChord( posNum, posDenom ){};

    virtual string toString( lgVoice *callingSeq = NULL );
};


/// a MusicXMLSequence
class mXMLLgSequence : public lgSequence
{
    int idI;
	xmlFactory *factory;
public:
    mXMLLgSequence(  long int posNum,
                     long int posDenom,
					 xmlFactory *fy) : lgSequence( posNum, posDenom )
	{ 
		idI = -1; 
		factory = fy;
	  
	};
	void setId( int id )
	{
		idI = id;
	}
//    virtual int cMeasures( void );

    /// start position of measure
//    virtual lgDuration measureStart( int id );
    /// split note (and tie) which are sounding at pos
    /// needs to be done for measure based MusicXML output
//    void splitAt( lgDuration pos);
    /// pre-process tags etc.
    virtual string toString( lgVoice *callingSeq = NULL );
    int getStaff( lgEvent *ev ){return idI; };
    int id( void )
    {return idI; };
    mXMLLgTag *first(char *tagName);

    /// write all measrue tags in [from, to)
    string measureAttributes( lgDuration from,
                              lgDuration to );
    /// get position of next barline
    lgDuration nextBarStart( lgDuration barStart,
                           lgDuration curMeterSig,
                           lgDuration &nextMeterSig);
    /// if needed, split event at pos and tie again
    /// return if split operation has been proceed
    char splitNextEvent( lgEvent *ev, 
						lgDuration pos,
						xmlFactory *factory);

};

/// a MusicXML rest
class mXMLLgRest : public lgRest
{
public:
    mXMLLgRest( long int durNum,
                long int durDenom,
                int cdots,
                long int posNum,
                long int posDenom) : lgRest( durNum,
                                             durDenom,
                                             cdots,
                                             posNum,
                                             posDenom)
	{};
    virtual string toString( lgVoice *callingSeq = NULL );
};

/// a MusicXML note
class mXMLLgNote : public lgNote
{
public:
    mXMLLgNote( int pc	/*! pitchclass 0..11 */,
              int oct /*!!octave	-oo..+oo, 0 = c' */,
              int acc /*! # of accidentals -oo .. +oo */,
              long int durNum,
              long int durDenom,
              int cdots,
              long int posNum,
              long int posDenom) : lgNote( pc,	
                                           oct, 
                                           acc, 
                                           durNum,
                                           durDenom,
                                           cdots,
                                           posNum,
                                           posDenom)
    {};
    virtual string toString( lgVoice *callingSeq = NULL );
};

//! a MusicXML segment
class mXMLLgSegment :  public  lgSegment
{
	/// number of voices
    int cVoiceI;
public:
    mXMLLgSegment( void ) :   lgSegment(new xmlFactory() )
    {
        cVoiceI = 0;
    };

    virtual lgSequence *appendSequence( lgSequence *seq )
	{
		seq = lgSegment::appendSequence(seq);
		mXMLLgSequence *temp;
		cVoiceI++;
		temp = dynamic_cast<mXMLLgSequence *>(seq);
		{
			if( temp )
				temp->setId( cVoiceI );
		}
		return seq;
	}

    virtual string toString( lgVoice *callingSeq = NULL );
    /*    virtual void write( FILE *out );
    {
        mXMLLgChord::write( out, NULL );
    }
    */
    void convert( void )
    {
        splitTagRanges();
    };
};


#endif
