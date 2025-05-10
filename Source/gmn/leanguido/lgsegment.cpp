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
 *  lgsegment.cpp
 *
 */

#include <iostream>

#include "lgsequence.h"
#include "lgsegment.h"
#include "lgfactory.h"


lgSegment * curSegment; // used in parser

#ifndef WIN32
#include <unistd.h>
#define Sleep(t)	usleep(t*10)
#else
#include <windows.h>
#endif

// if parser functions are not needed, undef next line
#ifdef USE_GUIDO_PARSER

#include "../parser/guido.h"
/*! parse a .gmn file and fille the data structure
	result:		0 = ok
				1 = error
*/


static int mutex = 0;
int lgSegment::parseFile(std::string &filename)
{
	while( mutex > 0 )
	{
		Sleep(100);
	}
	mutex++;
    curSegment = dynamic_cast<lgSegment *>(this);
    /// pointers for parsing
    curSequence = NULL;
    curChord = NULL;
    curEvent = NULL;
    curTag	 = NULL;
    curChordVoice = NULL;
    int res = gd_parseGMNFile(this, filename.c_str());
    mutex--;
    return res;
};
int lgSegment::parseBuffer(std::string *buffer)
{
	while( mutex > 0 )
	{
		Sleep(100);
	}
	mutex++;
    curSegment = dynamic_cast<lgSegment *>(this);
    /// pointers for parsing
    curSequence = NULL;
    curChord = NULL;
    curEvent = NULL;
    curTag	 = NULL;
    curChordVoice = NULL;
    int res = gd_parseGMNBuffer(this, buffer->c_str());
    mutex--;
    return res;
};
#endif



lgSegment::lgSegment( lgFactory *fy) : lgChord(0,1 )
{
    curEvent = NULL;
    curSequence = NULL;
    curChord = NULL;
	curChordVoice = NULL;
    curTag = NULL;
	factory = fy;    
}

/// append a new sequence and store in curSequence, if seq == NULL a sequence will be created
lgSequence * lgSegment::appendSequence( lgSequence *seq )
{
	if( !seq )
		seq = factory->newSequence(0,0);

	// use lgChord function
	appendVoice( seq );

	// current ptr for parsing
	curSequence = seq;
	curChord = NULL;
	curEvent = NULL; 
	curChordVoice = NULL;
	curTag = NULL;
	mCurRangeOpened = false;
    return seq;
}


//! append event in current sequence or in current chordVoice! Each chord voice must contain exactly one event!
lgEvent  *lgSegment::appendEvent( lgEvent *ev )
{
	/// we are inside a chord
	if( curChord )
	{
		//! create new voice in chord!
		curChordVoice->appendEvent(ev); 
	}
	else
	{
		curSequence->appendEvent(ev);
	}
	curEvent = ev;
	return ev;
}

/// leave, close the current sequence if parsing a ']'
void lgSegment::exitSequence( void )
{
	curSequence = NULL;
	curEvent = NULL;
	curChord = NULL;
	curTag = NULL;
}

//! create a new voice in curChord, call initChord before using this function!!!
void lgSegment::initChordVoice( void )
{
	if( curChord )
	{
		lgVoice *temp = factory->newVoice(0,0);
		curChord->appendVoice(temp);
		curChordVoice = temp;
	}
	else
	{
		// error, because no chord was initialised before!
	}
}
//! create a new chord
lgChord * lgSegment::initChord(long int posNum, long int posDenom )
{
	lgChord *temp = factory->newChord( posNum, posDenom );
	curChord = temp;
	curChordVoice = NULL;
    return temp;
}

//! if ch =0 NULL chord is completely parsed -> append curChord, else append ch!
lgChord *lgSegment::appendChord( lgChord * ch )
{
	if( !ch ) // close current chord
		ch = curChord;
	if( ch )
	{
		curSequence->appendEvent( ch );
		curChord = NULL;
		curEvent = ch;
		curChordVoice = NULL;
	}
	return ch;
}
//! appand tag in curSequence tag list or curChordVoice tag list!
void lgSegment::insertTag(lgTag *tag )
{
	/// tag might be NULLL if it should be ignored!
	if( !tag )
		return;
	// if tag->pEv == NULL this will be set to the last event of curVoice/curSequence during insertTag
	if( curChordVoice )
		curChordVoice->insertTag(tag );
	else
        curSequence->insertTag(tag);
}

/// append a tag to curSequence->[curChordVoice]
lgTag *lgSegment::appendTag(long int tagno,
                         	const char *tagname )
{

    lgTag *temp = factory->newTag(tagno, NULL, tagname);
    /// the curVoice will set the temp->pEv to the last event of it's events list
    insertTag( temp );
    if( curTag )
    	pushTag(curTag);
    curTag = temp;
    return temp;
}


/// close a tagrange opend by the parser/user 
void lgSegment::closeTag( void )
{
	if( curTag )
	{
		gd_info(curTag->pos().toString().c_str());
		if( curEvent != NULL )
		{
			if( curTag->pEvent() != curEvent )
			{
				gd_info("close range");
				/// create a endBracket tag
				lgTag *endTag = factory->newTag(- curTag->getId(),
									 curEvent,
									 ")" );
				/// pointers will be cross linked between begin and end
				curTag->setRange(endTag);
				/// append tag in curSequence[->curChordVoice]
				insertTag(endTag);
			}
			else
			{
				gd_info("empty Range");
			}
        } // if
		else
		{

			gd_info("current event is null");
			// this is an open range
		}
		// curTag = NULL;
	}
	else
	{
		// error, tag could not be found
		gd_error("can't find tag for tag range");
	}
}
lgSegment::~lgSegment( void )
{
	// do only ~lgChord
		delete factory;
}


lgEvent* lgSegment::gd_noteInit (const char *id){
	if( curSequence == NULL )
		curSequence = factory->newSequence(0,1);

	curEvent = curSequence->gd_noteInit(factory, gd_noteName2pc(id));
	gd_info("notInit");
	gd_info(id);
	gd_info(curEvent->toString(NULL).c_str());
	return curEvent;
}
void lgSegment::gd_noteAcc (int n){
	if( curEvent != NULL )
		((lgNote*)curEvent)->setAccidentals(n);

}
void lgSegment::gd_noteOct (int n){
	curSequence->setOctave(n);
}
void lgSegment::gd_noteEnum (long int n){
	// // std::cout << "enum" << n << endl;
	curSequence->setEnum(n);
}
void lgSegment::gd_noteDenom (long int n){
	// std::cout << "denom" << n << endl;
	curSequence->setDenom(n);
}
void lgSegment::gd_noteDot(void){
	curSequence->setDots(1);
}
void lgSegment::gd_noteDdot(void){
	curSequence->setDots(2);

}
void lgSegment::gd_noteTdot(void){
	curSequence->setDots(3);

}
void lgSegment::gd_noteAbsDur (long int n){
	gd_info("setAbsDur");
	curSequence->setAbsDur(n);
}

void lgSegment::gd_seqAppendNote(){

	if( curSequence != NULL )
	{
		curEvent = curSequence->gd_seqAppendNote();
	}
}

void lgSegment:: gd_tagRange(){
	// this is range begin

}
void lgSegment:: gd_seqAppendChord(){
	if( curSequence != NULL )
	{
		curEvent = curSequence->gd_seqAppendChord();
	}
	curChordVoice = NULL;
}
void lgSegment:: gd_chordInitNote(){
	if( curSequence != NULL )
	{
		curChordVoice = curSequence->gd_chordInitNote();
	}
}
void lgSegment:: gd_chordAppendNote(){
	if( curSequence != NULL )
	{
		curEvent = curSequence->gd_chordAppendNote();

	}
}
void lgSegment:: gd_chordInit(){
	if( curSequence != NULL )
	{
		curSequence->gd_chordInit();
	}
}

void lgSegment:: gd_addTag(){
	// curTag = popTag();
}

lgTag* lgSegment::popTag()
{
	lgTag* res = NULL;
	if( mTagStack.empty() )
	{
	}
	else
	{
		res = mTagStack.top();
		mTagStack.pop();
	}
	return res;
}
void lgSegment::pushTag(const lgTag *tag)
{
	mTagStack.push(const_cast<lgTag *>(tag));
}

void lgSegment::gd_tagEnd(void){
	gd_info("tag End");
	if( curSequence != NULL )
	{
		{
			closeTag();
			curTag = popTag();
		}
		mCurRangeOpened = !mCurRangeOpened;
	}}




