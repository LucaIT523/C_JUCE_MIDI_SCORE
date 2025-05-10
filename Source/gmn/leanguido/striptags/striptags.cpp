/*
 *  striptags.cpp
 *  striptags
 *
 *  Created by JŸrgen Kilian on Fri Apr 04 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "striptags.h"
int myLgChord::removeTags( char *tName )
{
	int res = 0;


	lgObject *obj;
    myLgChord *chord;
	lgTag *curTag,
		  *nextTag;
    
	string tagName;
	if( tName )
	{
		tagName = "";
		// add \ to tagName if not specified
		if( tName[0] != '\\' )
			tagName = "\\";
		tagName += tName;
	}
	
	lgVoice *temp;
	// check if this is a chord or a segment
    if( dynamic_cast<lgSegment *>(this) )
        temp = dynamic_cast<lgSegment *>(this)->firstSequence();
    else
        temp = firstVoice();
        
	while( temp )
    {

		// remove all tags in voice
		curTag = temp->firstTag();
		while( curTag )
		{
			char toRemark = 0;
			nextTag = dynamic_cast<lgTag *>(curTag->next());
			// skip "*)", it might be deleted !
			if( nextTag &&
				nextTag == curTag->endRange() )
			{
				nextTag = dynamic_cast<lgTag *>(nextTag->next());
			}
			// for keeping to output correct we need to remove 
			// also all content inside \cue and \grace ranges!
			if( strstr(curTag->name().c_str(), "\\cue") ||
				strstr(curTag->name().c_str(), "\\grace") )
			{
				toRemark = 1; // put all notes in range into a remark (* notes ... *)
			}
			if( !tName ) // remove all tags
			{
				if( !toRemark )
					temp->deleteTag( curTag );
				else
					toRemark++;
				res++;
			}
			else if( strstr(curTag->name().c_str(), tagName.c_str()) ) 
			{
				if( toRemark )
					toRemark++;
				else
					temp->deleteTag( curTag );
				res++;
			}
			// erase also notes in range
			if( toRemark == 2 )
			{
					// convert the tagname AND tagname of endrange into a remark
					curTag->setName( "(* hidden notes of cue/grace range ",
						 			" *) \n");
					toRemark = 0;
			}

			curTag = nextTag;
		} // while tag
        
        // chords contain separate tag lists -> look also into all chords
		obj = temp->firstEvent();
        while( obj )
        {
            chord = dynamic_cast<myLgChord *>( obj );
            if( chord )
            {
                res += chord->removeTags(tName);
            }
            obj = obj->next();
        } // while obj
        temp = dynamic_cast<lgVoice *>(temp ->next());
    } // while
	return res;
}


