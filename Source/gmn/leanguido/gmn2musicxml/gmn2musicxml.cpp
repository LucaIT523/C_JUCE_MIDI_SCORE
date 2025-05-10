/*!
 *  gmn2musicxml.cpp
 *  GUIDO to MusicXML converter
 *
 *  Created by JŸrgen Kilian on Tue Apr 15 2003 during
 *  a never ending train ride from Paris to Lyon
 *
 */

#include "gmn2musicxml.h"

/// set this to 1 during chord::toString
/// so an event knows that it's part of a chord
int inChord;

/// current score resolution set by mXMLLgSegment::toString
int measurePPQ;


/// current unique id
long int uidid = 0;
/// create unique id's
long int uid( void )
{
    return ++uidid;
}

/// convert score durations into tick durations
int frac2PPQ( lgDuration dur )
{
    int res;
    lgDuration temp;
    temp = lgDuration(4 * measurePPQ,1);

    temp *= dur;
    if( temp.durD > 1 )
    {
        printf("Warning: illegal resolution %d for duration %s!\n",
               measurePPQ,
               dur.toString().c_str());
    }
    res = temp.toDouble();
    return res;
}

/// convert gmn pitch into MusicXML pitch
void toXMLPitch( int &pc, int &acc, int &oct )
{
    // remove chromatic pitchclasses
    switch( pc )
    {
        case 1 : // cis
        case 3 : // dis
        case 6 : // fis
        case 8 : // gis
        case 10 : // ais
            if( acc > 0 )
            {
                pc++;
                acc--;
            }
            else
            {
                pc--;
                acc++;
            }
    };
    if( pc < 0 )
    {
        oct--;
        pc += 12;
    }
    else if( pc > 11 )
    {
        oct++;
        pc -= 12;    
    }

    // convert to diatonic picth
    switch( pc )
    {
        case 2 : // d
            pc = 1;
            break;
        case 4 : // e
            pc = 2;
            break;
        case 5 : // f
            pc = 3;
            break;
        case 7 : // g
            pc = 4;
            break;
        case 9 : // a
            pc = 5;
            break;
        case 11 : //h
            pc = 6;
            break;
    }
    
    oct += 3;
}

/// get score position where the next bar starts
lgDuration mXMLLgSequence::nextBarStart( lgDuration barStart /*! current bar */,
                                       lgDuration curMeterSig,
                                       lgDuration &nextMeterSig)
{
    /// todo search tag list also for balrines and meter signatures changes!
    return barStart+curMeterSig;
}


string mXMLLgNote::toString( lgVoice *callingSeq )
{
    ostringstream res;
    int oct, acc, pc;

    oct = octave();
    acc = accidentals();
    pc = pitchClass();
    toXMLPitch( pc, acc, oct);
    
    if( inChord )
        res << " <chord/>\n";
    res << " <pitch>\n";
    res << "   <step>";
// get diatonic pitchclass and corresponding accidentals
    res << pc;
    res << "</step>\n";
    res << "   <alter>" << acc << "</alter>\n";
    res << "   <octave>";
    res << oct;
    res << "</octave>\n";
    res << " </pitch>\n";
    res << " <duration>" << frac2PPQ(duration()) << "</duration>\n";
    mXMLLgSequence *seq;
    seq = dynamic_cast<mXMLLgSequence *>(callingSeq);
    if( seq )
    {
        res << " <staff>" << seq->getStaff(this) << "</staff>\n"; 
    }
//    res << " <type>";
//    res << "</type>\n";
    return res.str();
};


string mXMLLgRest::toString( lgVoice *callingSeq )
{
    ostringstream res;
    if( !inChord )
        if( inChord )
            res << " <chord/>\n";
        res << " <rest/>\n";
        res << " <duration>" << frac2PPQ(duration()) << "</duration>\n";
        mXMLLgSequence *seq;
        seq = dynamic_cast<mXMLLgSequence *>(callingSeq);
        if( seq )
        {
            res << " <staff>" << seq->getStaff(this) << "</staff>\n";
        }
        return res.str();
};



string mXMLLgChord::toString( lgVoice *callingSeq  )
{
    string res;

        lgVoice *cur;
        cur = firstVoice();
        int c = 0;
        while( cur )
        {
            // write "," before each real note
            if( c )
            {
                // new line for each sequence
                if( dynamic_cast<lgSequence *>(cur) )
                    res += "\n";

                
                
                // first note and last /note will be writte by callingSeq
                res += "</note>\n";
                res += "<note>\n";
            }
            res += cur->toString(/* from, to*/);
            /// get tags between curEv and next event
            /// == tags starting before note
            string temp;
            temp = cur->tagsToString( cur->firstEvent() );
            if( temp != "" )
            {
                res += " <notations>\n";
                res += temp;
                res += " </notations>\n";
            }
            c++;
            cur = dynamic_cast<lgVoice *>(cur->next());
            inChord = 1;
        }

        res += " ";
        //! write all tags between this and next
        //! callSeq might be NULL for Segements
        /*
        if( callingSeq )
            res += callingSeq->tagsToString( this );
         */
         inChord = 0;
        return res;
};


char mXMLLgSequence::splitNextEvent( lgEvent *ev, 
									lgDuration pos,
									xmlFactory *factory)
{
    lgEvent *nextEv;
    if( ev )
        nextEv = dynamic_cast<lgEvent *>(ev->next());
    else
        nextEv = firstEvent();

    if( !nextEv )
        return 0;
    else if( ! (nextEv->pos() + nextEv->duration()  > pos ) )
        return 0;
    else if( nextEv->pos() >= pos )
        return 0;
    else
    {
        // create new tag
        lgTag *tie;
        if( !ev )
            ev = this;
        
        long int id = uid();
        tie = new mXMLLgTag(id,
                          ev,
                          "\\tieBegin");
        insertTag( tie );
        tie = new mXMLLgTag(id,
                          nextEv,
                          "\\tieEnd");
        insertTag( tie);
        
        // split
        lgEvent *newEv;
        lgDuration oldDur,
                 newDur1,
                 newDur2;
        oldDur = nextEv->duration();
        newDur1 = nextEv->pos() - pos + oldDur;
        newDur2 = oldDur - newDur1;
        
        if( dynamic_cast<lgChord *>(nextEv) )
        {
            lgNote *tNote;
            tNote = dynamic_cast<lgNote *>(nextEv);
            newEv = factory->newNote( tNote->pitchClass(),
                             tNote->octave(),
                             tNote->accidentals(),
                             newDur1.durN,
                             newDur1.durD,
                             0,
                             nextEv->pos().durN,
                             nextEv->pos().durD
                            );
        }
        else if( dynamic_cast<lgNote *>(nextEv) )
        {
            newEv = new mXMLLgRest(newDur1.durN,
                                   newDur1.durD,
                                   0,
                                   nextEv->pos().durN,
                                   nextEv->pos().durD
                                   );
        }
        else if( dynamic_cast<lgRest *>(nextEv) )
        {
            newEv = new mXMLLgRest(newDur1.durN,
                                   newDur1.durD,
                                   0,
                                   nextEv->pos().durN,
                                   nextEv->pos().durD
                                   );
        }
        else if( dynamic_cast<lgEmpty *>(nextEv) )
        {
            newEv = new mXMLLgRest( newDur1.durN,
                                     newDur1.durD,
                                     0,
                                     nextEv->pos().durN,
                                     nextEv->pos().durD
                                     );
        }
        // insert new event
        insertEvent( newEv );
        ev->setPos( pos );
        ev->setDuration( newDur2 );
        return 1;
    }
}


string mXMLLgSequence::toString( lgVoice *callingSeq  )
{
    ostringstream res;
    res << "\n<part id =\"" << idI << "\"><!---------------------->\n";

    mXMLLgTag *curMeter;
    curMeter  = dynamic_cast<mXMLLgTag *>(firstTag("\\meter"));
    lgDuration curMeterSig(4,4);
    if( !curMeter )
    {
        printf("Warning: no time signature specified, used 4/4 as default!\n");
    }
    else
    {
        curMeterSig = lgDuration(( dynamic_cast<lgStrTagArg *>
                       (curMeter->getArg(1))->valStr().c_str() ))
					   ;
    }
        lgDuration barStart,
                 nextStart,
                 nextMeterSig;
        lgEvent *curEv;
        nextStart = nextBarStart(barStart,
                                 curMeterSig,
                                 nextMeterSig);
        splitNextEvent( NULL /*track start */, 
					    nextStart,
						factory );
        curEv = firstEvent();
        while( curMeterSig > 0)
        {
            nextStart = nextBarStart(barStart,
                                curMeterSig,
                                nextMeterSig);

        // write measure attributes
        res << " <attributes>\n";
        res << "  <division>" << measurePPQ << "</division>\n";
        res << measureAttributes( barStart, nextStart);
        res << " </attributes>\n";
    

        lgEvent *curEv;
        curEv = firstEvent();
        while( curEv &&
           curEv->pos() < nextStart )
        {
            // split the next event
            splitNextEvent( curEv, nextStart, factory);
            
            /// get event
            res << "<note>\n";
            res << curEv->toString( this );

            /// get tags between curEv and next event
            /// == tags starting before note
            string temp;
            temp = tagsToString( curEv );
            if( temp != "" )
            {
                res << "\n <notations>\n";
                res << temp;
                res << "\n </notations>\n";
            }


            /// write closing ")" == tags closing after note!
            /*
            int i;
            for( i = 0; i < curEv->rangeStack(); i++ )
            {
                res << "   <!--attributes>end of tag range</attributes-->\n";
            }
            */
            res << "</note>\n";
            curEv = nextEvent(curEv);
        } // while event
        curMeterSig = nextMeterSig;
        barStart = nextStart;
    }
    //    res << "</measure>\n";
    /// next measure
    res << "\n</part>\n";
    return res.str();
};

/// get all measure tags in [from ... to) as an attribute string
string mXMLLgSequence::measureAttributes( lgDuration from,
                                          lgDuration to )
{
    ostringstream res;
    mXMLLgTag *curTag;
    curTag = dynamic_cast<mXMLLgTag *>(firstTag());
    /// search for first tag in range
    while( curTag &&
           curTag->pos() < from )
    {
        curTag = dynamic_cast<mXMLLgTag *>(curTag->next());
    }
    while( curTag &&
           curTag->pos() >= from &&
           curTag->pos() < to )
    {
        if( curTag->tagType == tMeasureTag )
        {            
            res << curTag->toString();
            
            curTag->tagType = tProcessed;
        }
        curTag = dynamic_cast<mXMLLgTag *>(curTag->next());
    }
    return res.str();        
}

string mXMLLgSegment::toString( lgVoice *callingSeq )
{
    inChord = 0;
    ostringstream res;

    /// todo calculate ppq from existing durations in all voices
    measurePPQ = 196;
    
    res << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    res << "<!DOCTYPE score-partwise PUBLIC\n";
    res << "    \"-//Recordare//DTD MusicXML 0.7 Partwise//EN\"\n";
    res << "    \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
    res <<  "<score-partwise>\n";
    res << "  <part-list>\n";
            lgVoice *cur;
            cur = firstVoice();
            int c = 0;
            while( cur )
            {
                res << "    <score-part id=\"";
                res << (dynamic_cast< mXMLLgSequence * >(cur))->id();
                res << "\">\n";
                res << "    </score-part>\n";                
                cur = dynamic_cast<lgVoice *>(cur->next());
                c++;
            }
            res << "  </part-list>\n";
            cur = firstVoice();
            while( cur )
            {
                if( c )
                {
                    // new line for each sequence
                    if( dynamic_cast<lgSequence *>(cur) )
                        res << "\n";
                }
                res << cur->toString();
                c++;
                cur = dynamic_cast<lgVoice *>(cur->next());
            }
    res << "</score-partwise>\n";
    return res.str();
};



/*
 <note>
  <notations>
   <slur type='start' number='1'/>
   </notations>

</note>
 */

string mXMLLgTag::toString( void  )
{
    ostringstream res;
    if( tagType == tProcessed )
        return res.str();
    
    if( name() == "\\slurBegin" )
    {
        res << "  <slur type='start' number='" << id() << "'/>\n";
    }
    else if( name() == "\\slurEnd" )
    {
        res << "  <slur type='end' number='" << id() << "'/>\n";
    }
    else if( name() == "\\tieBegin" )
    {
        res << "  <tie type='start' number='" << id() << "'/>\n";
    }
    else if( name() == "\\tieEnd" )
    {
        res << "  <tie type='end' number='" << id() << "'/>\n";
    }
    else
    {
        res << name();
    }
    /*
    res << "\n    <attributes>";
    res << name();
    res << "</attributes>\n";
     */

    return res.str();
}




