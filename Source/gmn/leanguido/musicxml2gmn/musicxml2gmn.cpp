//! MusicXML to GUIDO Parser by Juergen Kilian v.1.0 (2-7/2003)
// based on the XML to GUIDO Parser by Andreas Berndt v.1.0
// Build on Expat 
//
// musicxml2gmn.cpp (main functions to convert musicxml -> guido)
//
// This file contains the implementation of the callback functions,
// wich will be called from the parser. Furthermore
// we have some basic file-operations to handle the output file-stream.
//

#include <sstream>
using namespace std;
#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>
#include "expat.h"
#include "Mystack.h"
#include "bkPosition.h"
#include "bkList.h"
#include "bkAylist.h"
/// use the leanGUIDO data structure
#include "../lgSegment.h"
#define firstParse (filterVoice==1)&&(filterPart==1)


int finaliseChord( void );



/// Globals!
string typeName(lgTag *tag)
{
    string tname;
    tname= tag->name();
    return tname;
}
string typeName(const char *str)
{
    return string(str);
}

bkAylist *bL= new bkAylist();



// globale Variablen fuer zu lesende Stimme
int filterVoice;
int maxFilterVoice;
int filterPart;
int maxFilterPart;
//Mystack<bkList *> bL;

Mystack<const char *> tagNameStack;

Mystack<lgTag *> openedTagStack;

Mystack<char *> bracketTag;

Mystack<lgSequence *> SeqStack;



#define BUFFSIZE	8192

// backup state 1 -> in Backup section -> ignore everything except duration
//jk: not needed anymore, ignore state can be inferred by scorePos < writtenTimePos, voiceID != filter voice, ...
// int  bkstate=0;

// jk: be sure that index can not exceed BUFFSIZE!!!
char Buff[BUFFSIZE];
char check_last_sequence[BUFFSIZE];
/// the parser-object
XML_Parser p; 
/// counter for coloumn-depth, if col is >= 80 then print a linefeed
int col = 0; 
///checks tags like fermata
int is_ambigous = 0; 
///flag for unknown tags
int found_unknown = 0; 
// filepointer for gmn-file
FILE *gmn_file; 
/// filepointer for unknown-tags-file
FILE *unknown_tags; 

char *majorCircleO5[] = {"C&","G&","D&","A&","E&","B&","F","C","G","D","A","E","B","F#","C#"};
char *minorCircleO5[] = {"a&","e&","b&","f","c","g","d","a","e","b","f#","c#","g#","d#","a#"};

/// count voices in current chord
// int chordVoices;

// == 1 if unfinished chord is opened
int openedChord;
// scorePosition of end (start+duration) of current chord (indicated by openedChord=1)
lgDuration chordEndPos;

///global zaehler fuer Nummer von Sequences
int part;

/// desired staff for current element
int curStaffID;
// flag, indicating that \staff must be written before next note starts!
char writeStaff;

// 1 indicates that current note is a grace note
char graceFlag;

/// partID of current note
int partID;
/// voiceID of current Note
int voiceID;
/// current ptr for parser
extern lgSegment *curSegment;
/// current ptr for parser
extern lgTag *curTag;

lgSequence *cSeq = NULL;
lgTag *cTag=NULL;
/// state flag, == 1 -> ignore all global information which should be read only once
char inIgnore;
/// state flag
char inNote = 0;
/// state flag
char collect = 0;
/// state flag, 
//0:no chord, 
//1:add current note to existing chord, 
// 2:current note is first note after chord
// 3:current note should be merged with previous note to a NEW chord
char inChord = 0;
/// gloabl collect variables
char stemup=0;
char stemdown=0;

char * creatortype = NULL;
int division = 0;
int key = 0;
int beamID=0;

int barID=0;
string _timeSig;
string _key;
string _clef;
long int _tagNo = 0;

int cN_pc, cN_oct, cN_acc;
long int cN_durN, cN_durD;
long int cpy_durN, cpy_durD;
int cN_dots;
long int cN_durPosN, cN_durPosD;


string cDataString;
/// key signature string
string keyStr; 

// beam's number 
int bmID;


lgEvent
/// needed for chordHandling
*preNote = NULL,
/// needed for TagHandling
*prePreNote = NULL;

char *movementTitle = NULL;
char *workTitle = NULL;
string composerName;


//bool multiStaves=false;

/// init function for collect variables
void initNoteVals( void )
{
    cDataString = "";
    cN_pc = -1;
    cN_oct = 0;
    cN_acc = 0;
    cN_durN = -1;
    cN_durD = 0;
    cN_dots = 0;
    cN_durPosN = -1;
    cN_durPosD = 0;
    collect = 0;
}

/// current score position
lgDuration scorePos = 0, 
/// score position of previous note
chordStartPos = 0,
/// score position in output file
writtenTimePos = 0;
/// collect the user data section of the xml file
void cDataHandler(void *userData,
                  const XML_Char *s,
                  int len)
{
    if( collect  )
        cDataString += string(s, len); 
}
/// global indent counter!
int cTab = 0;
/// main parser function for start attributes
void start(void *data, const char *el, const char **attr) 
{
    
#ifdef _DEBUG
    // do indent
    for( int i = 0; i < cTab; i++ )
        printf(" ");
    printf("start %s, %s\n",el, cDataString.c_str());
    // inc indent
    cTab++;
#endif
    
    //jk: todo bitte Sinn und Zweck kommentieren
    // int found_in_for = 0;
    char attr_temp[100][100]; 
    int schleife1; 
    for (schleife1 = 0; schleife1 < 100; schleife1++) { 
        attr_temp[schleife1][0] = '\0';} 
    int attr_count;
    
    
    cDataString = "";
    //segment
    
    if( inIgnore == 2)
    {
		// certain events should be read only once
		// do nothing until close event
	}
	else 
	{
		if (!strcmp(el, "score-partwise") )
		{		
			if(firstParse)  // do only first time
			{
				curSegment = new lgSegment(new lgFactory() );
				cSeq = NULL;
				scorePos = 0;
				//	            chordVoices = 0;
				preNote = NULL;
				prePreNote = NULL;
			}
			else 
			{
				inIgnore = 2;
			}
		}
		else if (!strcmp(el, "part-list"))
		{
			inIgnore = 2;
			// ignore complete partlist
		}
		else if (!strcmp(el, "part") )
		{
			// during a single parse, read for ech part a single voice
			
			/// create an empty sequence
			curStaffID = -1;
			partID++; 
			// jk: todo append only if part == filterPart??
			cSeq = curSegment->appendSequence();
			
			scorePos = 0;
			writtenTimePos = 0;
			preNote = NULL;
			prePreNote = NULL;
			if(firstParse)
			{			
				maxFilterPart=partID;
				if( workTitle )
				{
					lgStrTagArg *arg;
					arg=new lgStrTagArg(string(""), string(workTitle));
					lgTag *curTag=new lgTag(_tagNo++, NULL, "\\title");
					curTag->addArg(arg);
					curSegment->insertTag(curTag);
					
				}
				if( movementTitle )
				{
					lgStrTagArg *arg;
					arg=new lgStrTagArg(string(""), string(movementTitle));
					lgTag *curTag=new lgTag(_tagNo++, NULL, "\\title");
					curTag->addArg(arg);
					curSegment->insertTag(curTag);
				}            
				if(composerName.c_str())
				{
					if( strcmp("", composerName.c_str()) )
					{
						lgStrTagArg *arg;
						arg=new lgStrTagArg(string(""), composerName);
						lgTag *cTag=new lgTag(_tagNo++, NULL, "\\composer");
						cTag->addArg(arg);
						curSegment->insertTag(cTag);
					}
				}
			} // if first parse
			graceFlag = 0;
		} 
		else if (!strcmp(el, "backup"))
		{
			// do nothing here, in end:backup scorePos will be rewinded
		} //end note
		else if (!strcmp(el, "note"))
		{
			initNoteVals();
			inNote = 1;
		} //end note
		else if (!strcmp(el, "chord")  )
		{    	
			if( inChord == 0 )
			{
				// start a new chord
				inChord = 3;
			}
			else
			{
				// chord already exists
				inChord = 1;
			}
			// add new voice to chord
		} // else if chord	
		else if(!strcmp(el, "grace") || 
			!strcmp(el, "accent")|| 
			!strcmp(el, "staccato")||
			!strcmp(el, "tenuto")||
			!strcmp(el, "trill-mark")
			)
		{
			/// ls: stack solution for tag with Range like grace, accent,.. 
			//	        if( !bkstate  )
			// jk: push always, decide in end:note if evaluate or discard
			tagNameStack.push(el);		
			if(!strcmp(el, "grace") )
			{
				graceFlag = 1;
			}
		}
		else if (!strcmp(el, "duration"))
		{
			collect = 1;        
			for (attr_count = 0; attr[attr_count]; attr_count +=2)
			{
				
			}	        
			cN_durN = 1;
			cN_durD = 4;
		} //end note	
		else if (!strcmp(el, "octave"))
		{
			collect = 1;
		}	
		else if (!strcmp(el, "step"))
		{
			collect = 1;
			for (attr_count = 0; attr[attr_count]; attr_count +=2)
			{
				printf("(%s,%s), ",attr[attr_count], attr[attr_count+1]);
			}
			cDataString = "";
		} //end note	
		else if (!strcmp(el, "divisions"))
		{
			collect = 1;
		}    
		else if (!strcmp(el, "alter") ||
			!strcmp(el, "sign") ||
			!strcmp(el, "line") ||
			!strcmp(el, "beats") ||
			!strcmp(el, "beat-type") ||
			!strcmp(el, "fifths") ||
			!strcmp(el, "staff") ||
			!strcmp(el, "mode")||
			!strcmp(el, "directive")||
			!strcmp(el, "accidental")||
			!strcmp(el, "stem")||
			!strcmp(el, "voice")
			)
		{
			// start collecting data
			//        if( !bkstate )
			{
				collect = 1;
				cDataString = "";
			}
		}	
		else if( !strcmp(el, "movement-title")||
			!strcmp(el, "work-title"))
		{
			
			collect = 1;
		}
		
		else if(!strcmp(el, "creator"))
		{
			collect = 1;
			cDataString = "";
			if(!strcmp(attr[1], "composer"))
			{
				creatortype = new char [strlen("composer") + 1];
				strcpy(creatortype, "composer");
			}
		}
		///ls: stack solution for bracketTag like "tieBegin, slurBegin..." 
		else if( // !bkstate &&
			( !strcmp(el, "tie")
			|| !strcmp(el, "slur")) 
			)
		{		
			if(!strcmp(attr[1], "start"))
			{
				char *ElName = new char[strlen(el)+10];
				strcpy(ElName, el);
				strcat(ElName, "Begin");
				// to check, whether the tag with argument "number" or not.
				if(attr[2])
				{
					const char *i=attr[3];
					printf("attr[3]=%s\n", i);
					strcat(ElName, ":");
					strcat(ElName, i);
					printf("ElName is: %s\n", ElName);
				}
				bracketTag.push(ElName);
			}
			if(!strcmp(attr[1], "stop"))
			{
				char *ElName = new char[strlen(el)+10];
				strcpy(ElName, el);
				strcat(ElName, "End");
				if(attr[2])
				{
					const char *j=attr[3];
					printf("attr[3]=%s\n", j);
					strcat(ElName, ":");
					strcat(ElName, j);
				}
				bracketTag.push(ElName);
			}
		}	
		else if(!strcmp(el, "beam") )
		{	
			collect = 1;
			cDataString = "";
			if(attr[1])
				printf("beamNumber is: %s", attr[1]);
			bmID=atoi(attr[1]);
		}	
		else if(!strcmp(el, "measure") )
		{
			if(attr[1])
				barID=atoi(attr[1]);
			// jk: todo this is a hack! here scorePos should always be equal to writtenTimePos
			scorePos = writtenTimePos;
		}	
		else if (!strcmp(el, "rest")  ) 
		{
			inNote = 1;
		}	
    } // if inIgnore == 2
}  /* End of start handler */

//--------------------------------------------------------------------------------------
///function end handles all end-tags/attributes
void end(void *data, const char *el)
{
    
#ifdef _DEBUG
    if( cDataString != ""		// hide emtpy el
        )
    {
        for( int i = 0; i < cTab; i++ )
            printf(" ");
        printf("%s\n",cDataString.c_str());
    }
    // decr indent
    cTab--;
    // do indent
    for( int i = 0; i < cTab; i++ )
        printf(" ");
    printf("end: %s\n",el);
#endif
    if (!strcmp(el, "score-partwise"))
    {
    }
    else if (!strcmp(el, "part-list"))
    {
        inIgnore = 0;
    }
    //sequence
    else if (!strcmp(el, "sign") )
    {
		//        if(firstParse)
		// jk: todo create in every parse an array clef[staffid] = clef
		// append clef in end:note if
		// a) current note is first note of a staff
		// b) clef has chenged in staff of current note 
        {
            if(strstr(cDataString.c_str(), "G"))
            {
                _clef = "g";
            }
            else if(strstr(cDataString.c_str(), "F"))
            {
                _clef = "f";
            }
            else
            {
                _clef = "";
                printf("Unknown clef: %s\n",
					cDataString.c_str() );
            }
        } // if firstParse
    }
    else if (!strcmp(el, "line") )
    {
		//        if(firstParse)
		// jk: todo in every parse!! create array staff[i].clef = ....
		//    append clef in end(note) to voice: 
		//   a) before first note of each voice in each staff
		//	 b) if clef in staff of current note in current voice in current staff has chenged!
		/*
        {
		if( _clef != "" )
		{
		_clef += cDataString;
		curTag = curSegment->appendTag(_tagNo++, "\\clef");
		lgStrTagArg *arg;
		arg = new lgStrTagArg("",_clef.c_str());
		curTag->addArg(arg);
		}
        }
        */
        _clef = "";
    }
    else if (!strcmp(el, "beats") )
    {
        _timeSig = cDataString;
    }    
    else if (!strcmp(el, "staff") )
    {
        int staff = atoi(cDataString.c_str() );
        int m=(staff-1)+partID;
		//        if( m!=preStaffID)
		if( m != curStaffID )
        {
            //printf("Warning can't handle multiples staffs!\n");
			/*         	   
            lgIntTagArg *arg = new lgIntTagArg("",m);
            lgTag *curTag=new lgTag(_tagNo++, NULL, "\\staff");
            curTag->addArg(arg);
            curSegment->insertTag(curTag);
			*/
            curStaffID=staff;
            writeStaff = 1;
            // jk:todo in end:note append \staff tag if:
            // a) current note is first note in voice
            // b) is staffID is different between last APPENDED note and current note            
        }        
        else
        {
			writeStaff = 0;
        }
    }
    else if (!strcmp(el, "beat-type")  )
    {
		//        if(firstParse)
		// copy meter into all voices
		//jk: todo do not append here, write to stack and append in end:note after closing pending chord!
        {
            _timeSig += "/";
            _timeSig += cDataString.c_str();
            
            curTag = curSegment->appendTag(_tagNo++, "\\meter");
			curTag->setRange(NULL);
            lgStrTagArg *arg;
            arg = new lgStrTagArg(string(""),_timeSig);
            curTag->addArg(arg);
            _timeSig = "";
        }
    }
    else if(!strcmp(el, "directive"))
    {
		//! ls: text in Guido
		// jk: todo  put here to STACK! and add in end:note
		// add only to note in first voice of each staff OR
		// create separate voice for each staff with only empty notes and directives??
		/*
        if(firstParse)
        {
		lgStrTagArg *arg;
		arg=new lgStrTagArg("", cDataString.c_str());
		lgTag *curTag=new lgTag(_tagNo++, NULL, "\\text");
		curTag->addArg(arg);
		curSegment->insertTag(curTag);            
        } 
        */       
    }// ls: accidental 
    else if(!strcmp(el, "accidental") )
    {
        if(!strcmp(cDataString.c_str(),"natural")) 
            cN_acc++;
        else if(!strcmp(cDataString.c_str(),"flat")) 
            cN_acc=-1;
    }	
    else if(!strcmp(el, "dot"))
    {
        cN_dots++;
    }	
    //ls: title in Guido convert
    else if(!strcmp(el, "work-title") )
    {
        if( workTitle )
            delete [] workTitle;
        workTitle = new char [strlen(cDataString.c_str()) +1 ];
        strcpy( workTitle, cDataString.c_str());
    }
    else if(!strcmp(el, "movement-title") )
    {
        if( movementTitle )
            delete [] movementTitle;
        movementTitle = new char [strlen(cDataString.c_str()) +1 ];
        strcpy( movementTitle, cDataString.c_str());
    }
    else if(!strcmp(el, "creator") )
    {
		//ls: composer only creator with argument type="composer" in Guido convert
        if(creatortype &&
			strstr(creatortype, "composer"))
        {
            composerName = cDataString;
            delete [] creatortype;
            creatortype = NULL;
        }
    }    
    else if (!strcmp(el, "backup") )
    {
        // jk: we skip back in time
        // in end:note every information with scorePos < writtenTimePos will be ignorered
        scorePos -= lgDuration(cN_durN, cN_durD);
        
        //	printf("backup anfang's scorePos is: %s", scorePos);
		// here we need a stack for current sequences + start/end positions!
		// turn parse off, if scorePos == writtenPos 
		printf("Warning: start skipping backup data %s -> %ld, %ld!\n", 
			writtenTimePos.toString().c_str(),
			cN_durN, cN_durD);
		//parseOnly = 1; // waehrend == 1 darf NICHTS eingefuegt werden, nur Zeit weiter zaehlen!
		//            bkstate++;
		//            inIgnore = 1;
        // -> Liste anlegen mit scorePos, Duration und aktueller staff(part) nummer von backup
    }	
    /*	else if(!strcmp(el, "staves") )
    {
	if( atoi(cDataString.c_str()) > 1 )
	{
	//printf("Warning: can't handle multiple staves!\n");
	preStaffID=0;
	multiStaves=true;
	}
    }*/
    else if (!strcmp(el, "divisions") )
    {
        collect = 0;
        division = atoi(cDataString.c_str());
    }
    //sequence
    else if (!strcmp(el, "alter") )
    {
        cN_acc = atoi(cDataString.c_str());
    }
    else if (!strcmp(el, "duration") )
    {
        collect = 0;
        int ticks;
        ticks = atoi(cDataString.c_str() );
        lgDuration dur(ticks,division*4);
        dur.rationalise();
        cN_durN = dur.durN;
        cN_durD = dur.durD;
    }
    else if (!strcmp(el, "octave") )
    {
        collect = 0;
        cN_oct = atoi(cDataString.c_str() )-3;
    }
    else if (!strcmp(el, "step") )
    {
        collect = 0;
        if( strstr(cDataString.c_str(), "C") ) 
            cN_pc = 0;                     
        else if( strstr(cDataString.c_str(), "D") )
            cN_pc = 2;                     
        else if( strstr(cDataString.c_str(), "E") )
            cN_pc = 4;                     
        else if( strstr(cDataString.c_str(), "F") )
            cN_pc = 5;                     
        else if( strstr(cDataString.c_str(), "G") )
            cN_pc = 7;                     
        else if( strstr(cDataString.c_str(), "A") )
            cN_pc = 9;                     
        else if( strstr(cDataString.c_str(), "B") )
            cN_pc = 11;
    }
    //note
    else if (!strcmp(el, "note"))
    {
		// jk: todo put complete if( note ){ ... } block into a separate function
		// function should work according following outline
		// 1. close pending chords
		// 2. append all "end" tags, pushed to tag duraing current note
		// 3. append needed empty notes
		// 4. write \bar and \staff \meter tags if needed
		// 6. write all "start" tags pushed during current note, including \text tags
		// 7. append note
		// 8. write/close all single note range tags
		
		if( inChord == 2 ) // firstNote after chord
		{
			// set scorepos to end of chord
			scorePos = chordEndPos;			
		}
		// check if current note should be added to segment
		if( graceFlag )
		{
			finaliseChord();
			inIgnore = 1;
			cN_durN = 0;
			cN_durD = 0; // don't increment score position!

		}
		else if( scorePos < writtenTimePos ||
			partID != filterPart ||
			(partID == filterPart &&
			voiceID != filterVoice))
		{
			// we must be inside a backup, this note doe not belong to correct voice
			inIgnore = 1;
		}    
		else if( scorePos > writtenTimePos &&
			partID == filterPart &&
			voiceID == filterVoice &&
			inChord != 1 &&
			inChord != 3)
		{
			// first close chord -> then insert empty note!!
			finaliseChord();
			// jk: todo write endXXX tags before appending the empty note and
			// staff, meter, key tags AFTER appending the empty note !!!
			// we need to insert an empty note
			lgDuration delta = scorePos - writtenTimePos;
			preNote = curSegment->appendEmpty( delta.durN, delta.durD, 0,
				scorePos.durN, scorePos.durD );                        
			writtenTimePos = scorePos;
			if( inIgnore != 2 )
				inIgnore = 0;
		}
		else
		{
			inIgnore = 0;
		}
		
		if( inChord == 3 ) // note will become chord with previous note
		{
			// keep current scorePos, first note after chord will start here
			chordEndPos = scorePos;		
			scorePos = chordStartPos;
		}
		if( inChord == 1 )
		{
			// jump back to chord start
			scorePos = chordStartPos;
		}
		
		
        inNote = 0;
        if(inIgnore)
        {
            // neu 23.2.04 hier alle gespeicherten tags löschen (stack) delete variable
            // delete all tags for the current note
            // clear/ignore all tag information 
            openedTagStack.clear();
            tagNameStack.clear();
            bracketTag.clear();
            /*
            if(inChord==1)
            {
			inChord=0;
			//                prePreNote = curSegment->appendChord(NULL);
            }
            */			
            if( inChord == 0 ||
				inChord == 2 )
            {
				// if next note has a <chord/>, the chord will start here
				chordStartPos = scorePos;
            }
        }
        else if( !inIgnore )
        {
			// append note to segment
			
			if( inChord == 3 ) // first note in chord -> must be merged with prev note
			{
				// cur note includes a <chord/> tag
				
				// remove preNote
				// create new chord
				if( !dynamic_cast<lgChord *>(preNote) )
				{
					// start of new chord, previous note must become root of chord
					lgEvent *tChord = NULL;
					lgEvent *tNote = NULL;
					
					tChord = curSegment->initChord(chordStartPos.durN, chordStartPos.durD );
					/// put prenote into chord
					if( dynamic_cast<lgNote *>(preNote) )
					{
						curSegment->initChordVoice();
						tNote = curSegment->appendNote(
							dynamic_cast<lgNote *>(preNote)->pitchClass(),
							dynamic_cast<lgNote *>(preNote)->octave(),
							dynamic_cast<lgNote *>(preNote)->accidentals(),
							preNote->duration().durN,
							preNote->headDuration().durD,
							preNote->cDots(),
							preNote->pos().durN,
							preNote->pos().durD
							);
					}  // if previous note               
					curSegment->replaceRangePtr( preNote, tNote );
					cSeq->deleteEvent( preNote );
					// jk: todo rename prePreNote into chordRoot
					prePreNote = tChord; 
					preNote = tNote;  
				} // if new chord
				openedChord++;	        
				// set correct flags for handling current note
				inChord = 1;
				curSegment->initChordVoice();                        
			} // if new chord
			else if (inChord == 1 )// chord does already exist
			{
				curSegment->initChordVoice();                        
				openedChord++;
			}
			
			
            //neu 23.2.04 hier irgendwo vor append note, gespiechter tags appendTag
            
            // hier sollte zuerst get the current staff, damit setzt die richtige Sequence wieder ein
            // staff--> setzt die Sequence, deren seqID=staff ist, als current Sequence!
            // current note is first note after chord -> finalise chord 
            if( inChord == 2 || 
                (inChord == 0 && openedChord) ) // first note after chord is closing now
            {
				finaliseChord();
            }
            else
            {
				// jk:todo is this correct?
                prePreNote = preNote;
            }
            
            
            /// now a possible chord is closed and we can write ALL tags from the stack
            // ls: insert Tag with Range, for instance "grace"
            //search all entries in openedTagStack to check, whether it is in tagNameStack too, 
            // if it is not in tagNameStack => remove the entry from openedTagStack.
            for( int i = openedTagStack.size()-1; i >=0  ; i-- )
            {
                lgTag *cur = openedTagStack.peek(i);
                const char* tName= cur->name().c_str();
                if(tName)
                {
                    if( !tagNameStack.searchByName(tName))
                    {
                        lgTag *startRange = cur;
                        //endTag=new lgTag(tagid, NULL, ")");
                        lgTag *endTag = new lgTag(-startRange->id(), NULL, ")" );
#ifdef _DEBUG
                        printf("element to be poped is: %s\n ", string(typeName(cur)).c_str());
#endif
						//jk: todo call append tag for new tag
						// don't call for meter!!!
                        endTag->setRange( startRange );
						
                        //jk: check and re implement curSegment->insertTag( endTag );
						
                        openedTagStack.remove(cur);
                    } // if ! tName in stack
                } // if tName
            }//for
            int index=0;
            
			// jk: todo if not inside chord
			// append here empty notes
			// then
			// append \bar tags
			// append \staff, \meter tags
			
            
            
            
            
#ifdef _DEBUG
            for(int j=0; j<tagNameStack.size(); j++)
            {
                const char* t;
                t=tagNameStack.peek(j);
                if(t)
                    printf("all element in tagNameStack is: %s\n ", t);
            }
#endif			
            // search for all entries in tagNameStack, find all the entries,which are not in openedTagStack => new Tags, 
            // push all these new tags into openedNameStack	and insert them into curSequence too!	
            do{ 
                const char *tmpName=tagNameStack.peek(index);
                if(tmpName)
                {
                    if(!(openedTagStack.searchByName(tmpName)))
                    {
						if( 0 ) // jk > todo, re-implement after check!
						{
							if(!strcmp(tmpName, "grace"))
							{
								curTag=new lgTag(_tagNo++, NULL, "grace");
								curSegment->insertTag(curTag);
							}
							
							if(!strcmp(tmpName, "accent"))
							{
								curTag=new lgTag(_tagNo++, NULL, "accent");
								curSegment->insertTag(curTag);
							}
							
							if(!strcmp(tmpName, "staccato"))
							{
								curTag=new lgTag(_tagNo++, NULL, "stacc");
								curSegment->insertTag(curTag);
							}
							
							if(!strcmp(tmpName, "tenuto"))
							{
								curTag=new lgTag(_tagNo++, NULL, "ten");
								curSegment->insertTag(curTag);
							}
							
							if(!strcmp(tmpName, "trill-mark"))
							{
								curTag=new lgTag(_tagNo++, NULL, "trill");
								curSegment->insertTag(curTag);
							}
						}
                        printf("new to be inserted Tag is:%s\n", tmpName);
						// jk: todo make sure that i.e. \meter can not be pushed to stack!
                        // openedTagStack.push( curTag );
                    }
                } // if tmpName
                index++;                
            } while(index<tagNameStack.size() && !tagNameStack.isEmpty()); 		
            
            // now remove all Elemente from tagNameStack, empty tagNameStack is for tags in next Note!					
            tagNameStack.clear();
            // ls: insert bracketTag with "Begin", for instance "tieBegin"
            // to see what's now in bracketTag stack!
#ifdef _DEBUG
            for(int m=0; m<bracketTag.size(); m++)
            {
                char* tt;
                tt=bracketTag.peek(m);
                if(tt)
                    printf("all element in bracketStack is: %s\n ", tt);
            }
#endif			
            
            int start=0;
            while(!bracketTag.isEmpty()&& start<bracketTag.size())
            {
                char* tagName=bracketTag.peek(start);
                if(strstr(tagName, "Begin"))
                {
#ifdef _DEBUG
                    printf("tagName in bracketTag is: %s\n", tagName);
#endif		
                    int i;
                    char* startPos=strstr(tagName, ":");
                    
                    if(startPos)
                        i=atoi(startPos+1);
                    else i=0;
                    
					if( 0 ) // jk: todo re-implement after check!!
					{
						char *tagN = new char[20];
						
						lgTag *temp;
						if(strstr(tagName, "tieBegin"))
						{
							sprintf(tagN,"\\tieBegin:%d", i);
							temp=new lgTag(_tagNo++, NULL, tagN );
							curSegment->insertTag(temp);
						}                    
						else if(strstr(tagName, "slurBegin"))
						{
							sprintf(tagN, "\\slurBegin:%d", i);
							temp=new lgTag(_tagNo++, NULL, tagN);
							curSegment->insertTag(temp);
						}                    
						else if(strstr(tagName, "beamBegin"))
						{
                        /*strcat(tagN, "\\");
							strcat(tagN, tagName);*/
							sprintf(tagN, "\\%s", tagName);
							temp=new lgTag(_tagNo++, NULL, tagN);
							curSegment->insertTag(temp);
						}                    
						else
						{
							start++;
						}
						if(tagN) 
							delete [] tagN;
					}
                    bracketTag.remove(tagName);
                }
                else
                {
                    start++;
                }                
            } // while	
            
            //jk xml grace notes have no duration -> use 1/8 as default
            long graphDurN, graphDurD;
            if( cN_durN <= 0 )
            {
                graphDurN = 1;
                graphDurD = 8;
                // grace notes will not increase the score time!
            }
            else                        
            {
                graphDurN = cN_durN;
                graphDurD = cN_durD;            
            }
            if( cN_pc > -1	 )
            {
                preNote = curSegment->appendNote( cN_pc, cN_oct, cN_acc,
					graphDurN, graphDurD, 
					0, // dots are already included in duration cN_dots,
					scorePos.durN, scorePos.durD );                        
            }        
            else if( !inChord ) 
            {// can't add rests to chords!
                preNote = curSegment->appendRest( graphDurN, graphDurD, 
					0, // dots are already included in duration cN_dots,
					scorePos.durN, scorePos.durD );	            
            }
			if( !inChord  || 
				inChord == 2 )
			{
				writtenTimePos = scorePos + lgDuration(cN_durN, cN_durD);
			}                                            
            //ls: insert bracketTag with "End", for instance "tieEnd"	
            int end=0;
            while((!bracketTag.isEmpty())&& end<bracketTag.size()) 
            {
                char* tagName=bracketTag.peek(end);            
                if(strstr(tagName, "End"))
                {
                    char* startPos=strstr(tagName, ":");
                    int i;                
                    if(startPos)
                        i= atoi(startPos+1);
                    else 
                        i=0;
                    
					if( 0 ) // jk: todo check and re-implement
					{
						char* tagN= new char[20];
						if(strstr(tagName, "tieEnd"))
						{
							sprintf(tagN, "tieEnd:%d", i);
							curTag=new lgTag(_tagNo++, NULL, tagN);
							curSegment->insertTag(curTag);
						}                
						else if(strstr(tagName, "beamEnd"))
						{
                        /*strcat(tagN, "\\"); ---------> fuehrt zur Fehler beim Ausgabe!!?
							strcat(tagN, tagName); */
							sprintf(tagN, "\\%s", tagName);
							curTag=new lgTag(_tagNo++, NULL, tagN);
							curSegment->insertTag(curTag);
						}                
						else if(strstr(tagName, "slurEnd"))
						{
							sprintf(tagN, "slurEnd:%d", i);
							curTag=new lgTag(_tagNo++, NULL, tagN);
							curSegment->insertTag(curTag);
							if(bmID)
							{
								lgTag* tTag;
								tTag=new lgTag(_tagNo++, NULL, "\\beamsOff");
								curSegment->insertTag(tTag);
								bmID=0;                            
							}
						}
						else end++;
						
						if(tagN) 
							delete [] tagN;
					}
                    bracketTag.remove(tagName);
                }// end if(strstr(tagName, "End"))
                else
                    end++;
            }// end while
			
			
			if( inChord != 1 ) 
			{
				chordStartPos = scorePos;
			}
			else
			{
				printf("");
			}
        }// end if(!inIgnore)


        scorePos += lgDuration( cN_durN, cN_durD);
        if( inChord == 0 )
        {
            // if outside chord increment current time
            /*	printf("scorePos is: %d", scorePos.durN);
            printf("/");
            printf(" %d\n", scorePos.durD);*/        
        }
        else if( inChord == 1) /// during this note <chord> was called
        {
            inChord = 2;            
            // keep score time
        }
        else if( inChord == 2 ) // first note after chord
        {
			inChord = 0;
			/// use chord as prePreNote for tag range pointers
			/// close pending chord
        }
        
        cpy_durN=cN_durN;
        cpy_durD=cN_durD;
        cN_durN = 0;
        cN_durD = 0;
		graceFlag = 0;
    } //end note
    else if (!strcmp(el, "rest"))
    {
        // do nothing rests will end with /note
    }    
    else if(!strcmp(el, "stem") )
    {
        if( cDataString == "down" )
        {
            if(stemdown==0) //default stemsDown is 0
            {
			/*jk push to tagstack !!!
			lgTag *keyTag;
			keyTag = new lgTag(_tagNo++, NULL, "\\stemsDown");
			//neu 23.2.04 ->>  hier tag speichern globale Variable oder stack, ....
			// noch kein insertTag!!!
			// analog für alle anderen tags innerhalb von Note
			
			  curSegment->insertTag(keyTag);
			  stemup=0;
			  stemdown=1;
                */
            }
        }
        if( cDataString == "up" )
        {
            if(stemup==0) 
            {
			/* jk push to tagstack !!!
			lgTag *keyTag;
			keyTag = new lgTag(_tagNo++, NULL, "\\stemsUp");
			curSegment->insertTag(keyTag);
			stemdown=0;
			stemup=1;
                */
            }
        }
    } // stem    
    else if(!strcmp(el, "beam") )
    {
        
        if( cDataString=="begin")
        {
            char *tBeam;
            tBeam= new char[strlen("beamBegin")+3];
            sprintf(tBeam, "beamBegin:%d", bmID);
            bracketTag.push(tBeam);            
        }        
        else if( cDataString == "end")
        {
            char *tBeam;
            tBeam= new char[strlen("beamBegin")+3];
            sprintf(tBeam, "beamEnd:%d", bmID);
            bracketTag.push(tBeam);
        }
        
        /*	if(tBeam)
        delete [] tBeam;
        collect=0;*/
        
    }
    else if(!strcmp(el, "measure") )
    {
		// jk: push to stack or indicate by a flag, evaluate/pop in end:note
		// Eventually empty notes must be written BEFORE this, 
		// and chords must also be closed before!!!!!
		/*
        lgTag *barTag = new lgTag(_tagNo++, NULL, "\\bar");
        lgIntTagArg *barNr= new lgIntTagArg(NULL, barID);
        barTag->addArg(barNr);
        curSegment->insertTag(barTag);		
        */
    }    
    else if (!strcmp(el, "part") )
    {        
    }				
    else if(!strcmp(el, "voice") )
    {        
        voiceID=atoi(cDataString.c_str());        
        if(partID==1) 
        {
			//        	part = voiceID;
        }
        else if(partID>1) 
        {
			//        	part = voiceID+partID;
        }
        //fuer jede Part ist maxVoice festzustellen.
        if(filterPart==partID)
        {          
            if( voiceID > filterVoice &&
				voiceID < maxFilterVoice )
            {
				maxFilterVoice = voiceID;
            }            
        }	//end filterPart==partID
        
        
        if( voiceID != filterVoice )
        {
			
			//            if(bkstate)//falls backup auftaucht, speichert die entsprechende ScorePos und Written-
            {			//-Timepos in korrekte List, die nach partID sortiert ist.
                if(firstParse)
                {
                    
                    bkPosition *a=new bkPosition(scorePos, writtenTimePos);
                    bL->appendNote(a, part);
                    
                }
				//                bkstate=0;
            }//end bkstate 
			finaliseChord();
        }
        
        /* jk: funktioniert so nicht 
        if(voiceID==filterVoice)
        { 
		if((filterVoice!=1) || (filterPart!=1))
		{
		bkList curVoiceList=bL->element(part);// falls diese Voice ist nicht durch
		if(!(curVoiceList.isEmpty()))         // backup erzeugt, ist bL->element(part)=NULL!
		{
		bkPosition * tmp=curVoiceList.first();
		
		  lgDuration cd;
		  lgDuration tp;
		  lgDuration pd;
		  
			while((tmp->isAccessed()) && tmp)// 
			{
			tmp=tmp->getNext();
			
			  }
			  if(tmp) // d.h. diese Abstand-Duration zwischen nacheinanderliegende backup ist noch nicht berechnet
			  {
			  
				tp=tmp->getDur(); 
				cd=tmp->getWrtPos();
				scorePos=tmp->getWrtPos();
				if(!(tmp->isAppend()))
				{
				// scorePos
				
				  lgDuration dn;
				  bkPosition *prev=tmp->getPrev();
				  if(prev)
				  {
				  
					pd=prev->getDur();
					dn= cd-pd;
					
					  }
					  
						else dn=cd;
						
						  
                            curSegment->appendEmpty( dn.durN, dn.durD, 
							cN_dots, pd.durN, pd.durD );	
                            
							  //scorePos+=lgDuration(dn.durN, dn.durD);
							  tmp->appended();
							  
								}//
								if((cd+lgDuration(cN_durN, cN_durD)) < tp) 
								{
								cd+=lgDuration(cN_durN, cN_durD);
								tmp->setScorPos(cd);
								}
								else if((cd+lgDuration(cN_durN, cN_durD)) == tp) 
								{
								cd+=lgDuration(cN_durN, cN_durD);
								tmp->setScorPos(cd);
								tmp->accessed();
								}
								else 
								tmp->accessed();
								} // if temp                      
								}//!(curVoiceList.isEmpty())
								} // if filterVoiceID, filterpartID
								} // if filterVoice == voiceID       
        */
    }    
    else if (!strcmp(el, "fifths") )
    {
        key = atoi( cDataString.c_str() );
        key += 7;
    }
    else if (!strcmp(el, "mode") )
    {
        if(firstParse)
        {
		/* push to stack !!!!
		lgTag *keyTag;
		keyTag = new lgTag(_tagNo++, NULL, "\\key");
		lgStrTagArg *keySig;
		if( cDataString == "major" )
		{
		keyStr = majorCircleO5[key];
		keySig = new lgStrTagArg(NULL, keyStr.c_str() );
		keyTag->addArg(keySig);
		curSegment->insertTag(keyTag);
		}
		else if( cDataString == "minor" )
		{
		keyStr = minorCircleO5[key];
		keySig = new lgStrTagArg(NULL, keyStr.c_str() );
		keyTag->addArg(keySig);
		curSegment->insertTag(keyTag);
		}
		else
		{
		printf("Unrcognised mode: %s", cDataString.c_str() );
		}
            */
        }	
    }
    collect = 0;
    cDataString = "";
    /*	if( scorePos >= writtenTimePos )
    {
	inIgnore=0;
	parseOnly = 0;
	
    }*/
    /* might be used for debug
	if( bkstate )
	{
	//            scorePos += lgDuration( cpy_durN, cpy_durD);
	// reached end of backup region?
	if( scorePos >= writtenTimePos )
	{
	bkstate = 0;
	printf("Stop skipping backup data %s\n", scorePos.toString().c_str());
	if( scorePos > writtenTimePos )
	{
	printf("Warning end of backup does not match start of backup " );
	printf(writtenTimePos.toString().c_str());
	printf("\n");
	}
	} // if end of backup range
	}// if backup
    */
    /*
    FILE *out = fopen("_test.gmn","wt");
    curSegment->write(out);
    fclose(out);
    */
}  /* End of end handler */

//--------------------------------------------------------------------------------------

/// the main function, opens all input and output streams and
/// starts the evaluation of the xml-data-stream
int main(int argc, char *argv[]) 
{
	inIgnore = 0;
    
    FILE *xml_file;
    
    char xml_file_name[256], 
        gmn_file_name[256], 
        unknown_tags_name[256];
    
    int write_error;
    
    printf ("XML-to-GUIDO Parser 1.0\n\n");
    if (argc <= 1) {
        printf ("Usage is: MusicXML2guido xml_file.xml\n\n");
        exit(0);
    }
    
    if (!strcmp(argv[1], "?")) {
        printf ("Usage is MusicXML2guido xml_file.xml\n\n");
        exit(0);
    }
    
    char* to_dot;             // pointer for string operations
    
    strcpy(xml_file_name, argv[1]);
    strcpy(gmn_file_name, xml_file_name);
    strcpy(unknown_tags_name, xml_file_name);
    
    // find the first dot of actual xml_file_name and write the xml-ending behind,
    // if there's no dot then append the xml-ending
    if (!(to_dot = strrchr(xml_file_name, '.')))
    {
        strcat (unknown_tags_name, ".uts");
        strcat(gmn_file_name, ".gmn");
    }
    else
    { 
        strncat(gmn_file_name, xml_file_name, to_dot-xml_file_name);       
        strncat(unknown_tags_name, xml_file_name, to_dot-xml_file_name);       
        gmn_file_name[to_dot-xml_file_name] = '\0';
        unknown_tags_name[to_dot-xml_file_name] = '\0';
        strcat(gmn_file_name, ".gmn");
        strcat(unknown_tags_name, ".uts");
    }
    
    unknown_tags = fopen(unknown_tags_name, "w");
    
    printf("\nwriting '%s'.\n", gmn_file_name); fflush(stdout);
    
    if (!(to_dot = strchr(xml_file_name, '.'))) strcat(xml_file_name, ".xml");
    
    
    curSegment = NULL;
    // setzte Filter settings
    filterVoice = 1;
    filterPart = 1;
    maxFilterPart = 1;
    maxFilterVoice = 1000;
    do{
        
        do{	
			openedChord = 0;
			scorePos = 0;
			writtenTimePos = 0;
			inChord = 0;
            if ((xml_file = fopen (xml_file_name, "r")) == NULL)
            {
                printf("File does not exist or is written wrong... program termination!");
                exit(1);
            }
            
            
            gmn_file = fopen (gmn_file_name, "w");
            if ((write_error = fprintf(gmn_file, "%% Parsed by MusicXML-to-GUIDO Parser 1.0 \n\n")) == 0){
                fprintf(stderr, "Error writing the GMN-File\n");
            }
            
            
			printf("Parsing part %d, voice %d -----\n", 
				filterPart,            
				filterVoice );
            
            p = XML_ParserCreate(NULL);
            
            if (! p) {
                fprintf(stderr, "Couldn't allocate memory for parser\n");
                exit(-1);
            }
            /// set start/end handler
            XML_SetElementHandler(p, start, end);
            /// set the data handler
            XML_SetCharacterDataHandler(p, cDataHandler );
            ///call the parser
            
            
            for (;;) {
                
                int bytes_read;
                void *buff = XML_GetBuffer(p, BUFFSIZE);
                if (buff == NULL) {
                    /* handle error */
                }
                bytes_read = fread(buff, 1, BUFFSIZE,xml_file);
                
                if (bytes_read < 0) {
                    /* handle error */
                    fprintf(stderr, "Read error\n");
                    exit(-1);
                }
                
                
                if (! XML_ParseBuffer(p, bytes_read, bytes_read == 0)) {
                    /* handle parse error */
                    fprintf(stderr, "Parse error at line %d:\n%s\n",
						XML_GetCurrentLineNumber(p),
						XML_ErrorString(XML_GetErrorCode(p)));
                    exit(-1);
                }
                
                
                if (bytes_read == 0)
                    break;
                
            } // 
            
            
            XML_ParserFree(p);
            
            
            
            printf("\n");
            printf("Parsed all MusicXML-Data...\n\n");
            fclose(xml_file);
            // maxFilterVoice is now the next voiceID to be read
            // if no more voices > filterVoice have been found, maxFilterVoice = 1000
            filterVoice = maxFilterVoice;
            maxFilterVoice = 1000;
            partID=0;
        }
        while( filterVoice < 1000 );        
        filterVoice=1; // if a part is over, reset the filtervoice to 1, because for a new part the  
		// voiceid of a note must be reset to 1.
        filterPart++;        
    } while( filterPart <= maxFilterPart);
    
    
    
    curSegment->write(gmn_file);
    fclose(gmn_file);
    if( curSegment )
        delete curSegment;
    if (!found_unknown) {
        fprintf(unknown_tags, "No unknown tags found in %s", xml_file_name);
    }
    else { 	printf("\nFound unknown tags, see list in %s.\nAll unknown tags are parsed an a standard way:\n\\tag<param1=value1,...,paramX=valueX>", unknown_tags_name);}
    fclose(unknown_tags);
    
#ifdef _DEBUG
    printf("Press key to return");
#endif
    return 0;
    
}  /* End of main */



int finaliseChord( void )
{
	
	
	inChord = 0;
	if( !openedChord )
		return 0;
	/// use chord as prePreNote for tag range pointers
	/// close pending chord
	// not needed scorePos += lgDuration( cpy_durN, cpy_durD);
	// next chord might start here
	chordStartPos = scorePos;   
	// if chord was started during inIgnore, there will be no chord appended
	lgChord *_prePreNote = curSegment->appendChord(NULL);
	if( _prePreNote )
		prePreNote = _prePreNote;
	openedChord = 0;
	
	return 1; 
}