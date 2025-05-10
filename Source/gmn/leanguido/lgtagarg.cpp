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
 *  lgtagarg.cpp
 *
 */

#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif
#include <sstream>
#include "lgtagarg.h"
#include "lgsequence.h"


lgTagArg::lgTagArg(std::string &na,
				std::string &unit)
{
	nameI =  string(na) ;
	unitI = unit;
    nextI = NULL;
}
lgTagArg::lgTagArg(std::string &na )
{
	nameI =  string(na) ;
	unitI = string("");
    nextI = NULL;
}
lgTagArg::lgTagArg( )
{
	nameI =  string("") ;
	unitI = string("");
    nextI = NULL;
}


lgTagArg::~lgTagArg( void )
{
}


string lgTagArg::toString( lgVoice * )
{
	string res;
	if( nameI != "" )
	{
			res = nameI;
			res += "=";
	}

	return res;    
}

//-----------------------------------------------------

lgStrTagArg::lgStrTagArg( string &na,
                          string &v) : lgTagArg(na)
{
	valI = v;
}
lgStrTagArg::lgStrTagArg(  string &v) : lgTagArg()
{
	valI = v;
}


lgStrTagArg::~lgStrTagArg( void )
{
}
string lgStrTagArg::toString( lgVoice *v )
{
	string s = lgTagArg::toString(v);
	s +="\"";
	s += valI;
	s += "\"";
	return s;
}
//-----------------------------------------------------

lgIntTagArg::lgIntTagArg( string &na,
                    long &v,
                    string &unit ) : lgTagArg(na, unit)
{
    valI = v;
}
lgIntTagArg::lgIntTagArg( string &na,
                    long &v) : lgTagArg(na)
{
    valI = v;
}
lgIntTagArg::lgIntTagArg( long &v) : lgTagArg()
{
    valI = v;
}

string lgIntTagArg::toString( lgVoice *v)
{
	ostringstream s;
	s << lgTagArg::toString(v);
	s << valI;
	s << unitI;
	return s.str();
}
//-----------------------------------------------------

lgFloatTagArg::lgFloatTagArg( string &na,
                    double &v,
                    string &unit ) : lgTagArg(na, unit)
{
    valI = v;
}
lgFloatTagArg::lgFloatTagArg( string &na,
                    double &v ) : lgTagArg(na)
{
    valI = v;
}
lgFloatTagArg::lgFloatTagArg(  double &v ) : lgTagArg()
{
    valI = v;
}

string lgFloatTagArg::toString( lgVoice *v)
{
	ostringstream s;
	s.setf(ios::fixed, ios::floatfield);
	s << lgTagArg::toString(v); // the name
	s << valI;
	s << unitI;
	return s.str();
}

//------------------------------------------------------


