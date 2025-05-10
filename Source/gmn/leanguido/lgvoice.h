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
*  lgvoice.h
*
*/

#ifndef __lgvoice_h__
#define __lgvoice_h__
#include "lgevent.h"
#include "lgnote.h"
#include "lgtag.h"

class lgSegment;

/// # of max events in one line of output .gmn file
#define lgMaxEvInLine 10

/// a GUIDO voice with list(s) for events and tags
/*! 
lgVoice includes no curEvent, curTag pointers for memory saving reasons!
if a lgSeqeunce includes a large number of chords which then
inlcude lgVoice data the memory size would be unneccessary blown up.
*/
class lgVoice : public lgEvent
{

    //! type can be Sequence or voice (= chord voice )
    friend  class lgChord; 	//! allow to call setNext
    friend class lgSegment;
    /// event list
    lgEvent *events,
        *eventsTail;	// should point to end of list
     /// tag list
    lgTag   *tags,
        *tagsTail;		// should point to end of list
    //! usually this is a segment or a chord
    lgChord *parent;	

   lgEvent *mCurEvent;
   lgChord *mCurChord;
   lgDuration mCurPos;
protected:

   bool mEnumSet;
   long mCurEnum;
   long mCurDenom;
   int mCurDots;
   int mCurOctave;

   void init();
   void getValuesFrom(const lgVoice *);
public:
	/// replace in all tags with prevEvI==oldEv with newEv
	void replaceRangePtr( lgEvent *old, lgEvent *newEv);
	/// use also lgChord::appendVoice as public function!
	virtual  void setNext(lgVoice *seq);

    /// search for a tag starting at beginning of the list
    virtual lgTag * findTag(const char *tagName );

    //    virtual lgTag * findTagByPrefix(char *tagPrefix);
    
    /// get tag which is valid at position pos
    /// a tag is valid if pos is inside range or no range is specified
    /// this function does not look inside chords!
    virtual lgTag *findTagAt( const char *name,
                      		  lgDuration pos );

	/// return 1 if found
    int deleteTag( lgTag *tag );
    /// return 1 if found
    int deleteEvent( lgEvent *event );
    lgVoice( long int posNum,
		long int posDenom);
    lgVoice( const lgVoice *src,
    		 long int posNum,
		     long int posDenom);
    virtual ~lgVoice( void );
	//! append at xxTail
    lgEvent* appendEvent(lgEvent *ev); 

	//! insert tag in tag list keep tag normal form uptodate!
	//! if tag1->pos == tag2->pos unranged tags will sorted first
    void insertTag(lgTag *tag); 

	//! this might be needed for nested scores in Extended GUIDO
	//! void appendSegment(lgSegment *seg);	//! append at xxTail

	/// reset all stack entries
//	virtual void resetStack( void );

	//! update closeRangeStack of all events
  //  virtual void pushRangeStack( void );	

	//! set end range of tag no
    void closeTag( long int no,
		lgEvent *ev,
        lgFactory *factory);	

	//! write all tags, located between ev and ev->next
	//! if ev == lgVoice write all tags at beginning of voice
	/*
    void writeTags(FILE *outl,
		lgEvent *ev);	
	*/
	//! all tags, located between ev and ev->next
	//! if ev == lgVoice get all tags at beginning of voice
    string tagsToString( lgEvent *ev);	

		
	/// this function doesn't look inside chords!
    virtual lgEvent *firstEvent( void );
	/// this function doesn't look inside chords!
    virtual lgEvent *nextEvent( lgEvent *ev  // NULL at end of list
		);
	
	virtual lgNote *firstNote( void )
	{
		lgEvent *ev;
		ev = firstEvent();
		if( !ev )
			return NULL;
		if( !(dynamic_cast<lgNote *>(ev)) )
			ev = ev->nextNote();
		return dynamic_cast<lgNote *>(ev);
	};

	virtual lgNote *nextNote( lgEvent *ev )
	{
		if( ev )
			ev = ev->nextNote();
		return dynamic_cast<lgNote *>(ev);
	};
	/// return number of existing notes. Might be different to #events!
	virtual long int cNotes( void );
	virtual lgTag   *firstTag( const char *n = NULL );
	virtual lgTag   *nextTag( lgTag *tag	// NULL at end of list
		);
    /*! insert a tag at tag->pos()
	*/
	// virtual void insertTag( lgTag *tag );


    /// insert a event at event->pos
    /*!
	pos of succeeding events must be recalced!
	return 1: ok
	0 : can not be inserted because of collision with
	existing events
	*/
	virtual char insertEvent( lgEvent *event );

	virtual string toString( lgVoice *callingSeq = NULL );
	//! write to .gmn
	virtual void write( FILE *out, lgVoice *v = NULL ); 
					
	/*
									 void setPrev( lgEvent *ev )
									 {
									 prevI = ev;
									 }
	*/
	//! return duration of voice, eventsTail must be uptodate!
	virtual lgFrac duration( void );
	virtual lgTag *findTag( long int id );
	
	/// search for event with event->pos == atPos, return NULL if not found
	virtual lgEvent *findEvent( lgDuration atPos );
	/// check if an event is holding at pos
	lgEvent * eventHoldAt( lgDuration pos );
	
	/// get latest event ending <= pos
	lgEvent * latestEventbefore( lgDuration pos );
	
	/// slit an event and tie if needed
	/// return 0 if no event needed to be split
	char splitEvent( lgDuration pos );
	
	/// insert a tag and set range pointers of tag
	/// holding events will be splitted
	/// if startRange==endRange no range will be set
	void insertTag( lgTag *tag,
		lgDuration startRange,
		lgDuration endRange );


    /// split all explicit tag ranges (using "(....") ) into \...Begin and \...End,
    /// return number of splittedt ranges
    int splitTagRanges( void );    

   lgEvent *gd_noteInit(lgFactory *factory, int pc);
   lgEvent* gd_seqAppendNote(void);

   void gd_tagRange();
   lgEvent* gd_seqAppendChord();
   lgVoice* gd_chordInitNote();
   lgEvent* gd_chordAppendNote();
   void gd_chordInit();

   void setAbsDur(long &ms);
   void setOctave(int octave);
   void setEnum(long &n);
   void setDenom( long &n);
   void setDots(int n);
};

#endif
