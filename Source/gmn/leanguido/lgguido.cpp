/*
	leanGUIDO class library
	Copyright (C) 2003  Juergen Kilian, Holger H. Hoos, SALIERI Project

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


/* lgguido.cpp - application specific adaption of naguido.cpp of the GUIDO parser-kit */

#include <stdio.h>
#include <iostream>
#include "../parser/guido.h"

#include "lgsegment.h"
#include "lgchord.h"

extern lgSegment *curSegment;


int gd_parseGMNFileMode(lgSegment *segment, const char* filename, int mode)
{
DEBUG(
  printf("parsing file '%s', mode %i ...\n", filename, mode);
)
  gd_init();
  curSegment = segment;
  int res = gd_parse(filename, mode);
  gd_exit();
  curSegment = NULL;
  return(res);
}

int gd_parseGMNBuffer(lgSegment *segment, const char *buffer)
{
DEBUG(
  printf("parsing file '%s' ...\n", filename);
)
  gd_init();
  curSegment = segment;

  int res = gd_parse_buffer(buffer);
  gd_exit();
  curSegment = NULL;
  return(res);
}

int gd_parseGMNFile(lgSegment *segment, const char* filename)
{
DEBUG(
  printf("parsing file '%s' ...\n", filename);
)
  gd_init();
  curSegment = segment;

  int res = gd_parse(filename, PARSE_MODE_ALL);
  gd_exit();
  curSegment = NULL;
  return(res);
}

/*
char gd_getTagArgType(int n)
{
  return(gd_getTagArgType(n));
}

char* gd_getTagArgName(int n)
{
  return(gd_getTagArgName(n));
}

long int gd_getTagArgInt(int n)
{
  return(gd_getTagArgInt(n));
}

T_REAL gd_getTagArgFloat(int n)
{
  return(gd_getTagArgFloat(n));
}

char* gd_getTagArgUnit(int n)
{
  return(gd_getTagArgUnit(n));
 }

char* gd_getTagArgStr(int n)
{
  return(gd_getTagArgStr(n));
}
*/


/* -------------------------------------------------
	APPLICATION SPECIFIC SECTIONS:

	the following part has to be adapted for the
	application which is to use the GUIDO parser module
	------------------------------------------------- */


/*
extern lgSequence *curSequence;
extern lgChord *curChord;
extern lgEvent *curEvent;
*/
void gd_segmInit()
{
    /// std call
    if( !curSegment )
	{
        curSegment = new lgSegment( new lgFactory() );
	}
    else
    {
        // do nothing, call was from lgSegment itself!
    }
}

void gd_segmExit() {
DEBUG(
	printf("gd_segmExit()\n");
      )
//    curSegment = NULL;
}
void gd_segmAppendSeq(void)
{
	// printf("gd_appendSeq()\n");

}


void gd_seqInit() {
    if( !curSegment )
		gd_segmInit();

    if( curSegment )
        curSegment->appendSequence(); //! create an empty seq
}

void gd_seqExit() {
    DEBUG(
	printf("gd_seqExit()\n");
          )
	if( curSegment )
		curSegment->exitSequence();

//    curSequence = NULL;
//	curEvent = NULL;
}


void gd_noteInit (const char *id){
	curSegment->gd_noteInit(id);
}
void gd_noteAcc (int n){
	curSegment->gd_noteAcc(n);
}
void gd_noteOct (int n){
	curSegment->gd_noteOct(n);
}
void gd_noteEnum (long int n){
	curSegment->gd_noteEnum(n);
}
void gd_noteDenom (long int n){
	curSegment->gd_noteDenom(n);
}
void gd_noteDot(void){
	curSegment->gd_noteDot();
}
void gd_noteDdot(void){
	curSegment->gd_noteDdot();
}
void gd_noteTdot(void){
	curSegment->gd_noteTdot();
}
void gd_noteAbsDur (long int n){
	curSegment->gd_noteAbsDur(n);
}

void gd_seqAppendNote(){
	curSegment->gd_seqAppendNote();
}

void gd_chordInitVoice( void )
{
	curSegment->initChordVoice();
}

/*
 void gd_chordInitNote(void )
{
}
*/

void gd_chordInit(long int durPosN, long int durPosD)
{
	// open a new chord, all following tags, notes, ... will be added to the new chord 
	curSegment->initChord(durPosN, durPosD );
}

void gd_seqAppendChord(int voices, long int durN, long int durD, int dots,
    long int durPosN, long int durPosD) 
{
    /// close the current chord
    curSegment->appendChord( NULL );
}

void gd_tagStart(const char* id, long int no)
{
	gd_info(id);
	curSegment->appendTag(no, id);
}
void gd_tagIntArg(long int n)
{
	  if( !curSegment->getCurTag() )
	    {
	        printf("Ignored tag arg!\n");
	        return;
	    }
    curSegment->getCurTag()->setArgValue(n);

}
void gd_tagFloatArg(T_REAL r)
{
    if( !curSegment->getCurTag() )
    {
        printf("Ignored tag arg!\n");
        return;
    }
    curSegment->getCurTag()->setArgValue(r);

}

void gd_tagArgUnit(char* unit){
    if( ! curSegment->getCurTag() )
    {
        printf("Ignored tag arg!\n");
        return;
    }
    string arg =  string(unit);
    curSegment->getCurTag()->setUnit(arg);

}

void gd_tagStrArg(char *s){
    if( ! curSegment->getCurTag() )
    {
        printf("Ignored tag arg!\n");
        return;
    }
    string arg =  string(s);
    curSegment->getCurTag()->setArgValue(arg);

}

void gd_tagAdd(void)
{
	curSegment->gd_addTag();
}
void gd_tagAddArg(const char *s)
{
	   if( ! curSegment->getCurTag() )
	    {
	        printf("Ignored tag arg! getCurTag is null\n");
	        printf("%s",s);
	        return;
	    }
	   string arg =  string(s);
	   curSegment->getCurTag()->addArg(arg);

}

void gd_tagEnd(void)
{
	 curSegment->gd_tagEnd( );
}

void gd_tagRange()
{
	curSegment->gd_tagRange();
}
void gd_seqAppendChord()
{
	curSegment->gd_seqAppendChord();
}

void gd_chordInitNote()
{
	curSegment->gd_chordInitNote();
}

void gd_chordAppendNote()
{
	curSegment->gd_chordAppendNote();
}

void gd_chordInit()
{
	curSegment->gd_chordInit();
}

/* ... to here. */

/*
void gd_tagAdd(long int tagno, int nargs)
{
    if( !curTag )
    {
        printf("Ignored tag arg!\n");
        return;
    }
// 	printf("gd_tagAdd(tagno=%li, nargs=%i)\n", tagno, nargs);
  	if (nargs > 0) 
  	{
//    printf("  tag parameters:\n");
  		int n;
	    for (n=1; n<=nargs; n++) 
	    {
		    lgTagArg *tagArg = NULL;
//  	    printf("  #%i: gd_getTagArgName(%i)=\"%s\"  ",
//                        n, n, gd_getTagArgName(n));
      		switch (gd_getTagArgType(n))
      		{
		        case 'i': tagArg = new lgIntTagArg(
		                            gd_getTagArgName(n),
		                            gd_getTagArgInt(n),
		                            gd_getTagArgUnit(n) );
		                  break;
		        case 'f': tagArg = new lgFloatTagArg(
		                            gd_getTagArgName(n),
		                            gd_getTagArgFloat(n),
		                            gd_getTagArgUnit(n) );
		                  break;
		        case 's': tagArg = new lgStrTagArg(
		                            gd_getTagArgName(n),
		                            gd_getTagArgStr(n));
		                  break;
		        default: printf("    invalid tag arg type!! gd_getTagArgType(%i)=%c\n",
		                        n, gd_getTagArgType(n));
        	} // switch
        	if( tagArg )
            	curTag->addArg( tagArg );
      } // for
	} // if
}
*/

int gd_parseError(long int lnr, long int cnr, const char *msg) {
 	printf("\nERROR: %s (line %li, char %li)\n", msg, lnr,cnr);
    return 0;
}





/* --- specific initialization code, to be implemented by applications */
void gd_imp_init(void)
{

}

/* --- specific cleanup code, to be implemented by applications */
void gd_imp_exit(void)
{

}

int gd_error(char const *s)
{
 	std::cout << "ERROR: " << s << "\n";
 	std::cout.flush();
 	return 1;
}
int gd_info(char const *s)
{
#ifdef _DEBUG
 	std::cout << "INFO: " << s << "\n";
 	std::cout.flush();
#endif
 	return 1;
}


