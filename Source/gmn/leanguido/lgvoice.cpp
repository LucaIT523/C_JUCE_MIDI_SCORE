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
*  lgvoice.cpp
*
*/
#include <sstream>
using namespace std;

#include <iostream>
#include <typeinfo>

#include <string.h>

#include "../parser/GuidoDefs.h"

#include "lgvoice.h"
#include "lgsegment.h"
/// split all explicit tag ranges (using "(....") ) into
/// \...Begin and \...End
int lgVoice::splitTagRanges( void  )
{
    int res = 0;
    // split the tags in the tag list
    lgTag *temp = firstTag();
    while( temp )
    {
        res += temp->splitRange( );
        temp = dynamic_cast<lgTag *>(temp->next());
    }
    // now split the tags inside chords!
    lgEvent *curNote = firstEvent();
    while( curNote )
    {
        lgChord *tempChord = dynamic_cast<lgChord *>(curNote);
        if( tempChord )
            res += tempChord->splitTagRanges();
        curNote = dynamic_cast<lgEvent *>(curNote->next());
    } // while
    return res;
}

void lgVoice::setNext(lgVoice *seq)

{
    lgObject::setNext( seq );	
}

/// return number of notes  in list
long int lgVoice::cNotes( void )
{
    long int res = 0;
    lgObject *temp = firstEvent();
    while( temp )
    {
        if( dynamic_cast<lgNote *>(temp) )
            res++;
        temp = temp->next();
    } // while
    return res;
}

void lgVoice::init(){
	  events = NULL;
	    tags = NULL;
	    eventsTail = NULL;
	    tagsTail = NULL;
		parent = NULL;

		mCurChord = NULL;
		mCurEvent = NULL;
		mCurOctave = 1;
		mCurEnum = 1;
		mCurDenom = 1;
		mCurDots = 0;
		mEnumSet = false;
}
void lgVoice::getValuesFrom(const lgVoice  *src ){
	mCurDenom = src->mCurDenom;
	mCurEnum = src->mCurEnum;
	mCurOctave = src->mCurOctave;
}
lgVoice::lgVoice(const lgVoice  *src,
				 long int posNum,
				 long int posDenom) : lgEvent(0,0,0,  //duration
				 posNum,
				 posDenom)

{
	init();
	getValuesFrom(src);
}


lgVoice::lgVoice( long int posNum,
				 long int posDenom) : lgEvent(0,0,0,  //duration
				 posNum,
				 posDenom)
				 
{
	init();
}



lgVoice::~lgVoice( void )
{
	/// delete all events
    lgObject *cur = events;	
    while( cur )		
    {		
        lgObject *n = cur->next() ;		
        delete cur;		
        cur = n;		
    } // while
	
	/// delete all tags
    cur = tags;	
    while( cur )		
    {
        lgObject *n = cur->next();
        delete cur;
        cur = n;
    }
}


/// append an event at end of list
lgEvent *lgVoice::appendEvent(lgEvent *ev)
{
	
    if( eventsTail )
    {
        eventsTail->setNext( ev );
    }
    else
    {
        events = ev;
    }
    ev->setPos(durationI);
#ifdef _DEBUG
    std::cout << "ar " << this->durationI.toString() << ":"
    		<< ev->pos().toString( )<< ":" << ev->toString(this) << endl;
#endif
    durationI += ev->duration();
    std::cout.flush();
    eventsTail = ev;
    return ev;
}


int lgVoice::deleteEvent( lgEvent *ev)
{

    if( !events )
        return 0;
    
	
    lgEvent *pre = NULL;
    /// search for event
    lgEvent *cur = firstEvent();
    while( cur && cur != ev )
    {
        pre = cur;
        cur = dynamic_cast<lgEvent *>(cur->next());
    }

    if( cur )
    {
        if( pre )
            pre->setNext( dynamic_cast<lgEvent *>(cur->next()) );
        else // ev == head
        {
            events = dynamic_cast<lgEvent*>(events->next());
        }
        if( eventsTail == ev )
        {
            eventsTail = pre;
            if( !eventsTail )
                eventsTail = events;
        }
        delete ev;
        return 1;
    }
    else
    {
	    cur = firstEvent();
	    int res = 0;
    	while( cur )
    	{
    		lgChord *chord = dynamic_cast<lgChord *>(cur);
    		if( chord )
    		{
    			lgVoice *cVoice = chord->firstVoice();
    			while( cVoice &&
    					!res )
    			{
    				   res = cVoice->deleteEvent( ev );
    				   cVoice = dynamic_cast<lgVoice *>(cVoice->next());
    			}
    			if( res )
    				return 1;
    		} // if chord
        	cur = dynamic_cast<lgEvent *>(cur->next());
    	} // while
    }// else
    return 0; // not found
}

/// insert a tag in sorted taglist, if tag->prevI == NULL -> append at end of taglist!
void lgVoice::insertTag(lgTag *tag)
{
	//! set prevEvI for tag
	if( !tag->pEvent() )
	{
		if( eventsTail )
			tag->prevEvI = eventsTail;
		else // if no events use me as prevI
			tag->prevEvI = this;
	}

	/// there are already tags
    if( tagsTail &&
		tagsTail->pos() < tag->pos() ) 
    {
        tagsTail->setNext( tag );
	    tagsTail = tag;	
    }
	else if( tagsTail ) // search for insert position
	{
		lgTag *prevTag = NULL;
		lgTag *temp = firstTag();
		while( temp &&
			   temp->pos() < tag->pos() )
		{
			prevTag = temp;
			temp = dynamic_cast<lgTag *>(temp->next());
		}
		// temp->pos >= tag->pos()
		while( temp && 
			   !temp->hasRange() &&	
			   temp->pos() == tag->pos() )
		{
			prevTag = temp;
			temp = dynamic_cast<lgTag *>(temp->next());
		}

		if( prevTag )
		{
			prevTag->setNext( tag );
		}
		else // insert as head of list
		{
			tags = tag;
		}
		tag->setNext( temp );
		// a new tail?
		if( !temp )
			tagsTail = tag;
	}
    else /// first tag
    {
        tags = tag;
	    tagsTail = tag;	
    }
}

string lgVoice::toString( lgVoice * /*callSeq*/)
{
    ostringstream s;
	//! tags at beginn of the voice		
    s << tagsToString( this );
	
    lgEvent *curEv = firstEvent();
	int evInLine = 0; 
    while( curEv )		
    {	
		if( !(evInLine % lgMaxEvInLine) )
		{
			s << "\n";
			evInLine = 0;
		}
		/// write event
		if( dynamic_cast<lgChord *>(curEv) )
			s << curEv->toString(this);
		else 
			s << curEv->toString();

        s << " ";
        /// write all  tags after event
		s << tagsToString( curEv );
/*
		if(callSeq )
			s << callSeq->tagsToString(curEv);
*/
        curEv = nextEvent(curEv);		
		evInLine++;
    }   
	return s.str();
}


void lgVoice::write( FILE *out, lgVoice * )
{	
	
	/// could be replaced by 
	 fprintf(out, "%s", toString().c_str() );
	
	/*
	//! tags at beginn of the voice		
    writeTags(out, this);
    lgEvent *curEv;
    curEv = firstEvent();
    while( curEv )		
    {	
        curEv->write(out, this);		
        curEv = nextEvent(curEv);		
    }   
    */
}



/// write all tags between ev and ev->next
string lgVoice::tagsToString( lgEvent *ev )
{
    ostringstream res;
	
	lgDuration evPos;
	if( ev )
		evPos = ev->pos() + ev->duration();
	
    lgTag *ptr = firstTag();	
    while( ptr &&
		!(ptr->pos() > evPos ))		
    {		
        if( ptr->pEvent() == ev )			
			res << ptr->toString() /* << "\n"*/;
        ptr = nextTag( ptr );		
    }	
	return res.str();
}

/*
void lgVoice::writeTags( FILE *out ,						
						lgEvent *ev )						
{	
	/// could be replaced by
	// fprintf(out, tagsToString( ev );
    lgTag *ptr;
	lgDuration evPos;
	if( ev )
		evPos = ev->pos() + ev->duration();
	
    ptr = firstTag();	
    while( ptr &&
		!(ptr->pos() > evPos ))		
    {		
        if( ptr->pEvent() == ev )			
            ptr->write( out );		
        ptr = nextTag( ptr );		
    }	
} 
*/

/// add a close range tag to tag range started by tag->id == id
void lgVoice::closeTag(long int id,					   
					   lgEvent *ev,
                       lgFactory *factory)
					   
{	
    lgTag *cur = firstTag();
	
    while( cur &&
		cur->id() != id )
    {
        cur = nextTag( cur );
    }
    if( cur )
    {
        lgTag *endTag = factory->newTag(-id,
                                 ev,
                                 ")" );
        cur->setRange(endTag);
        insertTag(endTag);
    } // if
}



lgEvent *lgVoice::firstEvent( void )

{		
    return events;
}

lgEvent *lgVoice::nextEvent( lgEvent *curEvent )
{
	
	if( curEvent )
		return dynamic_cast<lgEvent *>(curEvent->next());
	else
		return NULL;
}




/// get first tag with name == n
lgTag   *lgVoice::firstTag( const char *n )
{
    if( !n )
        return tags;
    else
    {
        lgTag *temp = tags;
        while( temp &&
               strcmp(temp->name().c_str(), n ) )
        {
            temp = dynamic_cast<lgTag *>(temp->next() );
        }
        return temp;
    }
}

lgTag   *lgVoice::nextTag( lgTag *cur )
{
	if( cur )
		return dynamic_cast<lgTag *>(cur->next());
	else
		return NULL;
}





/*! insert a tag after *ev
if ev = NULL, the tag will be inserted 
at the beginning of the sequence

*/
/* removed does the same than appedn tag
void lgVoice::insertTag( lgTag *tag  )
{
	lgObject *temp, 
		*prev = NULL;
	
	
	stackDirty = 1;	
	temp = firstTag();
	
	if( !temp ) 
	{
		tags = tag;
		tagsTail = tag;		
	}
	else // search for position
	{
		
		// stop at first event/tag with position >=  tag
		while( temp &&
			temp->pos() > tag->pos() )
		{
			prev = temp;
			temp = temp->next();
		}
		
		if( prev )
			prev->setNext( tag );
		else
		{			
			// insert as new head of list			
			tags = tag;
			
			if( !tagsTail )				
				tagsTail = tag;					
		}
		
		// temp might == objects || temp == tags		
		tag->setNext( temp );
	}	
}
*/
/*! insert a event at event->pos
pos of succeeding events must be recalced!  
return 1: ok
0 : can not be inserted because of collision with
existing events		
*/		  
char lgVoice::insertEvent( lgEvent *event )			  
{			  
	char res = 1;
	// search for position			  
	lgEvent *prev = NULL;			  
	lgEvent *temp = firstEvent();			  
	if( !temp ) // first note				  
	{				  				  
		events = event;				  
		eventsTail = event;						  				  
	}			  
	else				  
	{				  
		// stop at first note with position >=  event				  
		while( temp &&					  
			temp->pos() < event->pos() )					  
		{					  
			prev = temp;					  
			temp = nextEvent(temp);					  
		}				  
		
		lgDuration prevEnd;				  
		
		if( prev )					  
		{					  
			prevEnd = prev->pos() + prev->duration();					  
			if( ! (prevEnd < event->pos() ) )						  
			{						  
				res = 0;						  
			}					  
			else if( temp && 						  
				(temp->pos() < event->pos() + event->duration()) )						  
			{						  
				res = 0;						  
			}					  
			else  // insert						  
			{						  
				// skip all possible tags after prev						  
				lgObject *tag = prev;						  
				while( tag &&							  
					tag->next() != temp )							  
				{							  
					tag = tag->next();							  
				}						  
				event->setNext( tag->next() );						  
				tag->setNext(event);						  
				// update tail						  						  
				if( eventsTail == tag )							  
					eventsTail = event;						  						  
			} // else					  
		} // if temp				  
	} // else			  
	return res;			  
}





int lgVoice::deleteTag(lgTag *tag)
{
	if( !tag )
		return 0;
		
	// look also to combined tag
	lgTag *endRange = tag->endRange();
	if( endRange )
		endRange->setRange( NULL );	
		
	// search for tag
	lgObject *prev = NULL;
    //! remove head of list?
	if( tag == tags )		
	{		
		tags = dynamic_cast<lgTag *>(tag->next());		
		if( tagsTail == tag )			
			tagsTail = tags;		
		delete tag;		
	}	
	else		
	{		
		prev = tags;		
	}
	
	while( prev &&		
		prev->next() != tag )		
	{		
		prev = prev->next();		
	}	
	int res = 0;
	if( prev ) 		
	{		
		prev->setNext( tag->next() );
		delete tag;		
		res = 1;
	}	
	if( endRange )
		deleteTag( endRange );
	return res;
}

//! return duration of a voice
//! eventsTail must be up to date!
lgDuration lgVoice::duration( void )
{
	
	if( eventsTail )
	{
		lgDuration dur = eventsTail->pos() 
						 + eventsTail->duration() 
						 - events->pos();
		return dur;
	}
	else
		return lgDuration(0);
}

lgTag *lgVoice::findTagAt( const char *name,
                           lgDuration pos )

{
    lgTag *res = NULL;
    //! search in tag list
    lgTag *t = firstTag();
    while( t &&
           t->pos() <= pos )
    {
        if( !strcmp(t->name().c_str(), name)  )
        {
            if( t->hasRange() &&
                t->pos() <= pos &&
                t->endPos() >= pos )
            {
                res = t;                    
            }
            else if( t->pos() <= pos )
            {
                res = t;
            }                
        } // if found
        t = dynamic_cast<lgTag *>(t->next());
    } // while
    return res;   
}


//! search for tag in tag list and all chords 
lgTag * lgVoice::findTag( long int id )
{
	lgTag *res = NULL;
	//! search in tag list
	lgTag *t = firstTag();
	while( t && !res )
	{
		if( t->id() == id )
			res = t;
		t = dynamic_cast<lgTag *>(t->next());
	} // while
	
	
	//! search in  chords!
	lgEvent *temp = firstEvent();
	while( temp && !res )
	{
		lgChord *c = dynamic_cast<lgChord *>(temp) ; 
		if(c)
		{
			res = c->findTag( id );
		}		
		temp = dynamic_cast<lgEvent *>(temp->next());
	} // while
	return res; 
}




lgTag * lgVoice::findTag(const char *tagName)
{
	string name2 = tagName;
	
	lgTag *res = NULL;
	//! search in tag list
	lgTag *t = firstTag();
	while( t && !res )
	{
		if( t->name() == name2 )
			res = t;
		t = dynamic_cast<lgTag *>(t->next());
	} // while
	
	
	//! search in  chords!
	lgEvent *temp = firstEvent();
	while( temp && !res )
	{
		lgChord *c = dynamic_cast<lgChord *>(temp) ; 
		if(c)
		{
			res = c->findTag( tagName );
		}		
		temp = dynamic_cast<lgEvent *>(temp->next());
	} // while
	return res; 
}

lgEvent *lgVoice::findEvent( lgDuration atPos )
{
	lgEvent *cur = firstEvent();
	while( cur && 
		   cur->pos() < atPos )
	{
		cur = dynamic_cast<lgEvent *>(cur->next());
		
	}
	if( cur &&
		cur->pos() == atPos )
		{
			return cur;
		}
	return NULL;
}


/// replace in all tags with prevEvI==oldEv with newEv
void lgVoice::replaceRangePtr( lgEvent *oldEv, 
							   lgEvent *newEv)
{
	// find corresponding taglist
	lgTag *temp = firstTag();
	while( temp )
	{
		if( temp->pEvent() == oldEv )
			temp->prevEvI = newEv;
		temp = dynamic_cast<lgTag *>(temp->next());
	}

	// look into chords
	lgEvent *ev = firstEvent();
	while( ev )
	{
		lgChord *chord = dynamic_cast<lgChord *>(ev);
		if( chord )
		{
			chord->replaceRangePtr(oldEv, newEv);
		}
		ev = dynamic_cast<lgEvent *>(ev->next());
	} // while
}

int gdPitch2lgPitch(int pc )
{
	switch(pc)
	{
	case NOTE_C : return 0;
	case 		NOTE_CIS : return 1;
	case 	NOTE_D  : return 2;
	case 		NOTE_DIS  : return 3;
	case 		NOTE_E  : return 4;
	case 		NOTE_F  : return 5;
	case 		NOTE_FIS  : return 6;
	case 		NOTE_G  : return 7;
	case 		NOTE_GIS : return 8;
	case 		NOTE_A  : return 9;
	case 		NOTE_AIS  : return 10;
	case 		NOTE_H  : return 11;
	}
	return -1;

}
lgEvent *lgVoice::gd_noteInit(lgFactory *factory, int pc)
{
	mEnumSet = false;
	if( mCurEvent != NULL )
	{
		mCurPos = 0l;
		mCurPos += mCurEvent->pos();
		mCurPos += mCurEvent->duration();
	}
	else
		mCurPos = 0l;

	if( pc == REST )
		mCurEvent = factory->newRest(mCurEnum,mCurDenom,mCurDots, mCurPos.durD, mCurPos.durN);
	else if(pc == EMPTY )
		mCurEvent = factory->newEmpty(mCurEnum,mCurDenom,mCurDots, mCurPos.durD, mCurPos.durN);
	else
	{
		  int pitch = gdPitch2lgPitch( pc);
		 lgNote* note = factory->newNote(pitch);
		 note->setPos(mCurPos);
		 note->setOctave(mCurOctave);
		 note->setDuration(lgDuration(mCurEnum, mCurDenom),mCurDots);
		 mCurEvent = note;
	}
	return mCurEvent;

}

lgEvent* lgVoice::gd_seqAppendNote(void){
	appendEvent(mCurEvent);
	lgEvent *tmp = mCurEvent;
	mCurEvent = NULL;
	return tmp;
}
lgEvent* lgVoice::gd_chordAppendNote(){
	mCurChord->appendEvent(mCurEvent);
	lgEvent *tmp = mCurEvent;
	mCurEvent = NULL;
	return tmp;
}


lgEvent* lgVoice::gd_seqAppendChord(){
	appendEvent(mCurChord);
	lgEvent *tmp = mCurChord;
	mCurChord = NULL;
		return tmp;
}
lgVoice* lgVoice::gd_chordInitNote(){
	lgVoice *voice = new lgVoice(this, mCurChord->pos().durD,mCurChord->pos().durD);
	mCurChord->appendVoice(voice);
	return voice;
}
void lgVoice::gd_chordInit(){
	if( mCurEvent != NULL )
		{
			mCurPos = 0l;
			mCurPos += mCurEvent->pos();
			mCurPos += mCurEvent->duration();
		}
		else
			mCurPos = 0l;
	mEnumSet = false;
	mCurChord = new lgChord(mCurPos.durN, mCurPos.durD);
}

void lgVoice::setAbsDur(long &ms){
	mCurEvent->setAbsDur(ms);
}
void lgVoice::setOctave(int octave)
{
	lgNote *n = dynamic_cast<lgNote *>(mCurEvent);
	if( n != NULL )
		n->setOctave(octave);
	mCurOctave = octave;
}


void lgVoice::setEnum(long &n)
{
	mEnumSet = true;
	mCurDenom = 1;
	mCurEnum = n;
	mCurDots = 0;

	mCurEvent->setDuration(lgDuration(mCurEnum, mCurDenom),mCurDots);
}
void lgVoice::setDots(int n)
{
	mCurDots = n;
	mCurEvent->setDots(n);
}
void lgVoice::setDenom( long &n)
{
	mCurDenom = n;
	mCurDots = 0;
	mCurEvent->setDots(0);
	if( mEnumSet )
	{
		mCurEvent->setDuration(lgDuration(mCurEnum, mCurDenom),mCurDots);
	}
	else
	{
		mCurEvent->setDuration(lgDuration(1, mCurDenom),mCurDots);
	}
}


