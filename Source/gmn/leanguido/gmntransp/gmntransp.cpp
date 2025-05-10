/*
 *  gmntransp.cpp
 *  gmntransp
 *
 *  Created by JŸrgen Kilian on Fri Apr 04 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "gmntransp.h"

void myLgChord::transpose( int deltaPC,
						   int voice )
{
    lgVoice *temp;
    if( dynamic_cast<lgSegment *>(this) )
        temp = dynamic_cast<lgSegment *>(this)->firstSequence();
    else
        temp = firstVoice();
    lgObject *obj;
    myLgChord *chord;
    myLgNote *note;
    int cVoice = 1;
    while( temp )
    {
    	// is this the correct voice?
    	if( voice < 1 ||
    	    cVoice == voice )
    	{
		    obj = temp->firstEvent();
		    // do for all notes inside voice/sequence
	        while( obj )
	        {
	        	// there is no lgOject::transpose, so we need a cast:	        	
	            note = dynamic_cast<myLgNote *>( obj);
	            chord = dynamic_cast<myLgChord *>( obj );
	            if( note )
	            {
	                note->transpose( deltaPC );
	            }
	            else if( chord )
	            {
	                chord->transpose( deltaPC );
	            }
	            obj = obj->next();
	        }
	    } // if
	    cVoice++;
        temp = dynamic_cast<lgVoice *>(temp ->next());
    } // while
}

void myLgNote::transpose( int deltaPC )
{
    int pc,
       oct,
       acc;
    
    if( deltaPC == 0 )
    	return; // do nothing
    	
    pc = pitchClass();
    oct = octave();
    acc = accidentals();


//    printf("pc=%d, acc=%d, ", pc, acc);


    pc += deltaPC;
    // check if octave needs to be changed
    while( pc > 11 )
    {
        pc -=12;
        oct += 1;
    }
    while(pc < 0 )
    {
        pc += 12;
        oct-= 1;
    }

	// check diatonic pc 
    switch( pc )
    {
        case 1 : // cis
        case 3 : // dis
        case 6 : // fis
        case 8 : // gis
        case 10 : // ais
            if(acc <= 0 ) // cis&->c
            {
                pc--;
                acc++;
            }
            else if( acc > 0 ) //cis# ->d
            {
                pc++;
                acc--;
            }
            break;
    }
    // now check octave again!
    while( pc > 11 )
    {
        pc -=12;
        oct += 1;
    }
    while(pc < 0 )
    {
        pc += 12;
        oct-= 1;
    }
    
    // for doing a always "perfect" transpose the key information should be avaluated!
    
//    printf("tranp: pc=%d, acc=%d, \n", pc, acc);

	// store new pitch information
	setAccidentals(acc);
    setPitchClass( pc );
    setOctave( oct );
}

