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
*  lgtagarg.h
*
*/
#ifndef __lgtagarg_h__
#define __lgtagarg_h__

#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define new DEBUG_NEW

#endif

// declartations of all tag related classes

using namespace std; 
#include <string>
#include <sstream>
#include <stdlib.h>
using namespace std; 

#include "lgevent.h"

#include "lgvisitor.h"

//!  generic tag argument without value
class lgTagArg : public lgObject
{
	string nameI;
	    
protected:
	/// unit string, is "" for stringTagArgs
	string unitI;
    
public :
	virtual string toString( lgVoice *seq = NULL );

    lgTagArg( 	/// argument name
    			std::string &na,
    			/// argument unit
    			std::string &un);
    lgTagArg( 	/// argument name
        			std::string &na);
    lgTagArg( 	);
    virtual ~lgTagArg( void );
	
	
    virtual string valStr( void )
    { return string(""); };
    virtual long valInt( void )
    { return 0; };
    virtual double valFloat( void )
    { return 0; };
	
    void setUnit(string &unit)
    { unitI = unit; }
	virtual string unit( void )
	{ return unitI; };
	
	/// argument name, might be "" if unnamed
    const string name( void )
    {return nameI; };
    	
    void setName(string &s)
    {
    	nameI = s;
    }
};

//! string lgTagArg argument
class lgStrTagArg : public lgTagArg
{
    string valI;
	
public :
	virtual string toString( lgVoice *seq = NULL );
    lgStrTagArg( /// argument name
    			 string &na,
    			 /// argument value
				string &v);
    lgStrTagArg( /// argument value
				string &v);
    virtual ~lgStrTagArg( void );
	
    virtual string valStr( void )
    {return valI; };
    
    virtual long valInt( void )
    { return atoi(valI.c_str()); };
    virtual double valFloat( void )
    { return atof(valI.c_str()); };
    
    
	virtual void setValStr( const char * val )
    { valI = val; };

	
};

//! integer lgTagArg argument
class lgIntTagArg : public lgTagArg
{
    long valI;
	
public :
	virtual string toString( lgVoice *seq = NULL );
	lgIntTagArg( /// argument name, might be ""
				 string &na,
				 /// argument value
				long &v,
				string &unit);
	lgIntTagArg( /// argument name, might be ""
				 string &na,
				 /// argument value
				long &v );
	lgIntTagArg( /// argument value
				long &v );
	
    virtual long valInt( void )
    {return valI; };

    virtual double valFloat( void )
    { return valI; };
    
    virtual string valStr( void )
    { ostringstream res;
      res << valI;
      return res.str();	 };
    
    virtual void setValInt( long val )
    { valI = val; };
};

//! double lgTagArg argument
class lgFloatTagArg : public lgTagArg
{
    double valI;
public :
	virtual string toString( lgVoice *seq = NULL);

    lgFloatTagArg( 	/// argument name
    				std::string &na,
    				/// argument value
					double &v,
					std::string &unit);
    lgFloatTagArg( 	/// argument name
    				std::string &na,
    				/// argument value
					double &v);
    lgFloatTagArg( /// argument value
					double &v);
	
    virtual double valFloat( void )
    { return valI; };
    virtual string valStr( void )
    {  ostringstream res;
      res << valI;
      return res.str();	 };
    virtual long valInt( void )
    { return (long)(valI+0.5); };
    
    virtual void setValFloat( double val )
    { valI = val; };
};


#endif
