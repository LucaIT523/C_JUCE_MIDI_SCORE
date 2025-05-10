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
 *  lgtag.cpp
 *
 */

#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif

#include "lgtag.h"
#include <sstream>
using namespace std;
#include <string.h>

#include "lgfactory.h"
#include "lgsequence.h"
#include "../parser/guido.h"

lgTag::lgTag( long int id,
              lgEvent *pEv,
              const char *na)
{
	// set unqiue id given by the parser or by user    
    idI = id;
    prevEvI = pEv;
    argsI = NULL;
    if( !na )
        na = ""; // NULL names should never occur
    if(na)
    {
		nameI = string(na); // tag name
    }

	rangeEnd = NULL;
    
}

lgTag::~lgTag( void )
{
  	// delete the arguments
    lgObject *cur = argsI;
    while( cur )
    {
        lgObject *n = cur->next();
        delete cur;
        cur = n;
    }
}

int lgTag::tagType( void )
{
	if( !strcmp( name().c_str(), ")" ) )
		return endBracketType;
	else if( strstr( name().c_str(), "Begin") )
		return beginType;
	else if( strstr( name().c_str(), "End") )
		return endType;
	else if( strstr( name().c_str(), "(*") )
		return remarkBegin;
	else if( strstr( name().c_str(), "*)") )
		return remarkEnd;
	else if( rangeEnd )
		return rangeType;
	
	return noRangeType;
}

/// split (..) ranges into \begin \end ranges
/// if the lastInRange and prevEv events are located in different
/// voices (should never happen) the \...end should be placed in
/// the voice of lastInRange!
//lgTag *lgTag::splitRange( lgFactory *factory )
int lgTag::splitRange( void  )
{
	// check if this is a \tag( ... ) range
	if( rangeEnd &&
		!strcmp(rangeEnd->name().c_str(), ")") )
    {
        string newName;
        newName = nameI + "End";
        rangeEnd->nameI = newName;
        nameI += "Begin";
        return 1;
    }
    return 0;
}

void lgTag::addArg(lgTagArg *pa)
{
	if( !pa )
		return;
    lgObject *cur = argsI;
    // append new arg at end of list
    while( cur &&
           cur->next() )
    {
        cur = cur->next();
    }
    if( cur )
        cur->setNext( pa );
    else
        argsI = pa;
    
}
    
/// convert tag and arguments into a string, add a range open "(" if needed
string lgTag::toString( lgVoice *v)
{
    ostringstream s;
    int tType = this->tagType();
    // put a \n before a new tag
    if(  tType == noRangeType ||
    	 tType == rangeType ||
    	 tType == beginType ||
    	 tType == remarkBegin )
        s << "\n";
       
    if( tType != remarkBegin &&
    	tType != remarkEnd &&
    	tType != endBracketType)
    {
    	// add \ if missing
    	if( nameI.c_str()[0] != '\\' )
    		s << "\\"; 
    }
	s << nameI;
	if( idI > 0 )
		s << ":" << idI;
    if( argsI ) // add all arguments
    {
		s << "<";

        lgTagArg *cur = argsI;
        int c = 0;
        while( cur )
        {
            if( c )
				s << ", ";
			string temp = cur->toString(v);
			s << temp.c_str();
            c++;
            cur = (lgTagArg *)cur->next();
        } // while arguments
		s << ">";
    } // if args
    
    if( tagType() == rangeType ) // bracket range
		s << "( " ;
    else
		s << " ";

	return s.str();
}
    

void lgTag::setRange( lgTag *ta )
{	
    rangeEnd = ta;
    // cross link in both directions
    if( ta )
    	ta->rangeEnd = this;
}

char lgTag::hasRange( void )
{
    if( (rangeEnd &&
    	tagType() == rangeType) ||
    	tagType() == beginType )
        return 1;
    else
        return 0;
}


/// tag range is empty if prevEvI == endRangeI
char lgTag::emptyRange( void )
{
    if( rangeEnd &&
        rangeEnd->prevEvI == prevEvI )
        return 1;
    
    return 0;
}

// return number of tag arguments
int lgTag::cArgs( void )
{
    int i = 0;
    lgTagArg *cur = argsI;
    while( cur )
    {
        i++;
        cur = (lgTagArg *)cur->next();
    }
    return i;
}


lgTagArg *lgTag::firstArg( void )
{
    return argsI;
}

// return time position of tag
lgFrac lgTag::pos( void )
{
	lgDuration res(0,1);

	//! check an event before  the tag
	if( !dynamic_cast<lgVoice *>(prevEvI) )
	{
		res += prevEvI->pos() + prevEvI->duration();
	}
	else if( prevEvI ) //! the tag is first tag in voice
	{
		res = prevEvI->pos();
	}
	else
	{
		res = 0l;
	}
	return res;
}

lgFrac lgTag::endPos( void )
{
	lgDuration res(-1,1);

	if( hasRange() )
	{
        res = rangeEnd->pos();
	}
	else
	{
		res = pos();
	}
	return res;
}

string lgTag::name()
{
	return nameI;
}


// search fo an argument
lgTagArg * lgTag::findArg(const char *name,
						 int defPos)
{
	string name2 = name;
	lgTagArg *temp = firstArg();
	while( temp &&
		   temp->name() != name2 )
	{
		temp = dynamic_cast<lgTagArg *>(temp->next());
	}
	if( !temp && defPos > 0 )
	{
		temp = getArg(defPos);
		if( temp &&
			temp->name() != string( "" ) )
		{
			// if temp was find by defPos it must have no name!
			temp = NULL;
		}
	}
	return temp;
}


/// get tag arg by id 1...n
lgTagArg *lgTag::getArg(int id /*1..n*/)
{

	int i = 1;
	lgTagArg *temp = firstArg();
	while( temp &&
		   i != id )
	{
		i++;
		temp = dynamic_cast<lgTagArg *>(temp->next());
	}
	return temp;
}

lgEvent * lgTag::lastInRange()
{
    if( rangeEnd )
        return rangeEnd->prevEvI;
    
    return NULL;
}

lgEvent * lgTag::firstInRange()
{
	lgVoice *pVoice = dynamic_cast<lgVoice *>(pEvent());
	/// tag range starts at beginning of sequence
	if( pVoice )
	{
		return pVoice->firstEvent();
	}
	
	if( pEvent() )
		return dynamic_cast<lgEvent *>(pEvent()->next());
	return NULL;
}

void lgTag::removeArg( lgTagArg *pa )
{
	lgTagArg *prev = NULL;
	lgTagArg *cur = firstArg();
	// find pa->prev
	while( cur && 
		   cur  != pa )
	{
		prev = cur;
		cur = dynamic_cast<lgTagArg *>(cur->next());
	}
	if( cur )
	{
		if( prev )
		{
			prev->setNext( cur->next() );
		}
		else
		{
			argsI = dynamic_cast<lgTagArg *>(cur->next());
		}
		delete cur;
	} // if cur
}



/// get parameter value as a string, return 0 if parameter does not exist
int lgTag::getParamChar( /// parameter name (might be NULL)
						const char *pname,
						/// default position of paramter
						int defPos,
						/// value
						string &value,
						/// unit if available
						string &unit )
{
	lgTagArg *param = findArg(pname, defPos );
	int res = 0;
	if( param )
	{
		value = param->valStr();
		unit = param->unit();
		res = 1;
	}
	return res;
}
						
	/// get parameter value as an int, return 0 if parameter does not exist
	int lgTag::getParamInt( /// parameter name (might be NULL)
						const char *pname,
						/// default position of paramter
						int defPos,
						/// value
						int &value,
						/// unit if available
						string &unit )
{
	lgTagArg *param = findArg(pname, defPos );
	int res = 0;
	if( param )
	{
		value = param->valInt();
		unit = param->unit();
		res = 1;
	}
	return res;
}
						
/// get parameter value as a double, return 0 if parameter does not exist
int lgTag::getParamFloat( /// parameter name (might be NULL)
						const char *pname,
						/// default position of paramter
						int defPos,
						/// value
						double &value,
						/// unit if available
						string &unit )
{
	lgTagArg *param = findArg(pname, defPos );
	int res = 0;
	if( param )
	{
		value = param->valFloat();
		unit = param->unit();
		res = 1;
	}
	return res;
}
void lgTag::addArg(string &s){
	mCurArg->setName(s);
}

void lgTag::setArgValue(int n){

	long int tmp = n;
	mCurArg = new lgIntTagArg( tmp);
	addArg(mCurArg);
}
  void lgTag::setArgValue(long n){
		mCurArg = new lgIntTagArg( n);
		addArg(mCurArg);
  }
  void lgTag::setArgValue(    double &n)  {
		mCurArg = new lgFloatTagArg( n);
		addArg(mCurArg);
  }

  void  lgTag::setArgValue( string &val ) {

		mCurArg = new lgStrTagArg( val);
		addArg(mCurArg);
}

  void  lgTag::setUnit( string &unit) {
		if( mCurArg != NULL )
		{
			mCurArg->setUnit(unit);
		}
}

