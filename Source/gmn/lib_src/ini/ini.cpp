/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

 /*----------------------------------------------------------------
	filename: ini.cpp
	author:	  Juergen Kilian
	date:	  1998-2001
	Implementation of TInifile, TInientry
------------------------------------------------------------------*/
#ifdef _Windows
#ifndef  __BCPLUSPLUS__
	#include "..\sp_app.form\stdafx.h"
#endif
#endif

#include <sstream>
using namespace std;

#include "ini.h"
#include <string.h>

#include <stdlib.h>
#include <ctype.h>

#define JK_YES "y"



/* demo for strtod, strtol
#include <stdio.h>
#include <stdlib.h>

int main()
{ 
   double d;
   const char *string = "51.2% are admitted";
   char *stringPtr;
   
   d = strtod( string, &stringPtr );
   printf( "The string \"%s\" is converted to the\n", 
           string );
   printf( "double value %.2f and the string \"%s\"\n", 
           d, stringPtr );

   return 0;
}
*/



void writeInt( int i, FILE *out )
{
    int msb = i / 256;
    int lsb = i % 256;
    fputc( lsb, out);
    fputc( msb, out);
}


void TInifile::FPrintf(FILE *out,
			 const char *valStr,
			 char /* binary */)
{
	fprintf(out, "%s", valStr );
}

void TInifile::FPrintf(FILE *out,
			 const char *formatStr,
			 const char *valStr,
			 char /* binary */)
{
	fprintf(out, formatStr, valStr );
}



// -------------------------------------------------------
#ifdef INTERACTIVE
void _Printf( const char *str )
{
	Printf( str );
}
const char * _YesNoQuestion(const char */*tempStr*/, 
				  const char */*defStr*/,
				  const char */*m*/)
{
	return YesNoQuestion( tempStr, defStr, mf );
}
#else
void _Printf( const char * /*str*/ )
{
	
}
const char * _YesNoQuestion(const char * /*tempStr*/, 
				  const char * /*defStr*/,
				  const char * /*m*/)
{
	return JK_YES;
}

// ------ class TInientry --------------------------------------------
TInientry::TInientry( const char *entry /*! name of entry */,
					  const char *val /* value of entry */,
                      const char *remark,
					  char hide /* if == 1 entry will note be written to file*/)

{
	/// make it save
	if( !entry )
		entry = REMARK_ENTRY;

	// create new entry
	EntryI = new char [strlen(entry)+1];

	// copy name of entry	
	if( EntryI )
		strcpy( EntryI, entry );

	if( !strcmp(EntryI, REMARK_ENTRY ) )
		ValI = NULL;

    // reset ptr
    remarkI = NULL;
	ValI = NULL;
	nextI  = NULL;

	// copy value
	SetVal( val, remark, hide );

	hideFlag = hide;
}

TInientry::~TInientry( void )
{
	if( EntryI )
		delete [] EntryI;
	EntryI = NULL;

	if( ValI )
		delete [] ValI;
	ValI = NULL;

    if( remarkI )
        delete [] remarkI;
    remarkI = NULL;
}




void TInientry::SetVal( const char *val,
                        const char *remark,
                        char hide )
{
	hideFlag = hide;
	// if already exists then delete old Value
	if( ValI )
		delete [] ValI;

	// make it save
	if( !val )
		val = "";	// default

    ValI   = new char [strlen(val)+1+0];
	if( ValI  )
	{
			strcpy( ValI, val );
	}

    if( remark )
    {
        if( remarkI )
            delete [] remarkI;
        remarkI = new char[strlen(remark)+5];
        // strend
        *remarkI = 0;
        // coun't space at begin of remark
        int addSpace = 3;
        unsigned int pos = 0;
        while( pos < strlen(remark) &&
        		remark[pos] == ' ' )
        {
        	addSpace--;
        	pos++;
        }
        while( addSpace )
        {
        	strcat(remarkI, " ");
        	addSpace--;
        }
        strcat( remarkI, remark);
    }

    // remove all \n\r
    if( ValI )
    {
            unsigned int i;
	    for( i = 0; i < strlen( ValI ); i++ )
		{
			if( ValI[i] == '\n' ||
				ValI[i] == '\r' )
			{
				ValI[i] = ' ';
			}
		} // for
    } // if val
    if( remarkI )
    {
    	unsigned int i;
        for( i = 0; i < strlen( remarkI ); i++ )
        {
    	    if( remarkI[i] == '\n' ||
        	    remarkI[i] == '\r' )
        	{
           	 remarkI[i] = ' ';
        	}
    	} // for
    } // if remark
}


void TInientry::SetNext( TInientry *next )
{
	nextI = next;
}

void TInientry::Write( FILE *out,
					   TInifile *inifile,
					   char binary
					   )
{
	// write to a file
	if( !out || hideFlag)
		return;

	if( !strcmp(Entry(),REMARK_ENTRY ) )
	// don't write name and add ; before the value
	{
		if( binary )
		{
			int len = 1+strlen(Val());
			if( remarkI )
				len += 1+strlen(remarkI);
			len++; // \n
            writeInt(len, out );
		}
		inifile->FPrintf(out,";%s",Val(), binary);
		if( remarkI )
			inifile->FPrintf(out,";%s",remarkI,binary);
		inifile->FPrintf(out, "\n",binary);

	}
	else if( strstr( Val()," " ) &&
			!(Val()[0] == '"') &&
			!strstr( Val(),"[" ) &&
			!strstr( Val(),"{") )
	// if the value includes some whitespace put " " around it
	{
		if( binary )
		{
			int len = strlen(Entry()) + 1;
			len += 1+ strlen(Val()) + 1;
			if( remarkI )
				len += 1+strlen(remarkI);
			len++; // \n
            writeInt(len, out );
		}
		// include "
		inifile->FPrintf(out, "%s=", Entry(), binary);
		inifile->FPrintf(out, "\"%s\"", Val(), binary);
        if( remarkI )
            inifile->FPrintf(out,";%s", remarkI,binary);
        inifile->FPrintf(out,"\n", binary);
	}
	else // stad single value
	{
		if( binary )
		{
			int len = strlen(Entry()) + 1;
			len += strlen(Val());
			if( remarkI )
				len += 1+strlen(remarkI);
			len++; // \n
            writeInt(len, out );
		}
		inifile->FPrintf(out, "%s=", Entry(), binary);
		inifile->FPrintf(out, "%s",  Val(), binary);
        if( remarkI )
            inifile->FPrintf(out,";%s", remarkI, binary);
        inifile->FPrintf(out,"\n", binary);
    }

}

//---------class TInifile-----------------------------------
TInifile::TInifile( const char *filename )
{
	Dirty = 0;
	FilenameI = string("");
	EntriesI = NULL;
	createFilename( filename ); // copy filename

	Read(0 /* non binary */);
}

TInifile::~TInifile( void )
{
	// write to file if changes

    if( Dirty )
		Write();
;
	// delete all entries
	TInientry *temp = EntriesI;
	while( temp )
	{
		TInientry *temp2 = temp->Next();
		delete temp;
		temp = temp2;
	}
	EntriesI = NULL;
}

/*!
	Read and parse inifile if filename is given
*/
void TInifile::Read( char binaryMode )
{
	if( FilenameI.length() == 0 )
        return;

	FILE *in;
	if( !binaryMode )
		in = fopen( FilenameI.c_str(), "rt");
	else
		in = fopen( FilenameI.c_str(), "rb");

	if( !in ) // create a new file
	{
//		printf("Can't open %s\n",FilenameI);
		if( !binaryMode )
			in = fopen( FilenameI.c_str(), "wt");
		else
			in = fopen( FilenameI.c_str(), "wb");
		if( in )
			fclose(in);
		// re open as read-only
		if( !binaryMode )
			in = fopen( FilenameI.c_str(), "rt");
		else
			in = fopen( FilenameI.c_str(), "rb");
	}
	else
	{
		// ok
	}

	if( in )
	{
		
		TInientry *prev = NULL;
		while( !feof(in) ) // read complete file
		{
			
            // keep position of line begin
			fpos_t lineStart;
			fgetpos(in, &lineStart);
			

			int lineLength = getLineLength( in, binaryMode );
			
			fsetpos(in, &lineStart); // rewind to start of line

			char *entry = readLine( in, 
							  lineLength,
							  binaryMode );

			// create memory space
//			fseek(in, (long)lineStart,  SEEK_SET); // rewind to start of line
//			printf("%s\n",entry);

			char *remark = NULL,
			     *val = NULL;
			// read any chr's in this line?
			if( strcmp(entry,"") )
			{
				// check for remark
				remark = strstr( entry, ";" );
				if(remark) // split string, add remark later to the list
				{
					*remark = 0;
					// points to char after ";"
					remark++;
				}								
			} // if

			if( strcmp(entry,"") ) // is empty if only remark!!
			{
				val = strstr( entry, "=");
				if( val ) // remove "="
				{
					*val = 0; // split entry=val
					val++;

					// if single parameter remove whitespace of val at begin and end
					if(!strstr(val,",") &&	// values separated by ,
						!strstr(val,"[") &&  // gmn sequence
						!strstr(val, "\"") ) 
						sscanf(val,"%s", val ); // remove the whitespace

					// remove \t and " 
					unsigned int i;
 					for( i = 0; i < strlen(val); i++ )
					{
						if( val[i] == '\t' )
						{
							val[i] = ' ';
						}

						// remove "
						if( val[i] == '"' )
						{
							val[i] = ' ';
						}
					}
					
					// remove whitespace of entry name
					sscanf(entry,"%s",entry);
					
					// add new entry to list
					if( Find(entry) )
					{
						_Printf("WARNING: TInifile entry");
						_Printf(entry);
						_Printf("existed twice!\n");
						Dirty = 1;
					}
					else
					{
                        TInientry *temp = new TInientry( entry, val, remark );
						if( prev )
						{
							prev->SetNext(temp);
						}
						else
						{
							EntriesI = temp;
						}
						prev = temp;
                        // don't add again as separate entry
                        remark = NULL;
					}
				} // if val
			}// if entry

			// add any remark entry?
			if( remark &&
				strcmp(remark,"") )
			{
				// append remark entry
				TInientry *temp = new TInientry( REMARK_ENTRY, remark, NULL );
				if( prev )
				{
					prev->SetNext(temp);
				}
				else
				{
					EntriesI = temp;
				}
				prev = temp;
			}
			
			// delete line
			delete [] entry;
			entry = NULL;
		} // while
		fclose(in);
	} // if in
	else
	{
		_Printf("Warning: Could not open ");
		_Printf(FilenameI.c_str());
		_Printf("!\n");
	}
}

/// search for specific entry
TInientry *TInifile::Find( const char *entry )
{
	if( !entry )
		return NULL;
	if( !strcmp(entry, REMARK_ENTRY )  )// don't search for remarks
		return NULL;
	TInientry *temp = EntriesI;
	while( temp &&
		   strcmp(temp->Entry(), entry) )
	{
        temp = temp->Next();
	}
    return temp;
}

/*!
	Add or change an string entry.
	The entry needs not to exist.
*/
void TInifile::SetEntry( const char *entry, 
						const char *val,
                         const char *remark,
                         char hide)
{
		Dirty = 1;
		TInientry *temp = Find( entry ); // does it already exist?
		if( !temp ) // insert new entry
		{
			temp = new TInientry( entry, val, remark, hide );
			temp->SetNext( EntriesI );
			EntriesI = temp;
		}
		else
		{
 			temp->SetVal( val,
                          remark,
                           hide );
		} // else
}

/*!
	 add an double value to the entry list
	 It should be checked if this function works correct for 
	 very long values!
*/
void TInifile::SetEntry( const char *entry, 
						double val, char *remark,
						char hide)
{
#ifndef __BCPLUSPLUS__
  	 ostringstream tmp;
	 tmp.precision(8);
	 tmp << val;
	SetEntry(entry,tmp.str().c_str(),remark, hide);
#else
	SetEntry( entry, val, remark, hide );
#endif

}


/*! 
	returns an integer value of an entry
	If the entry does not exist saveAtoi(*def) will be returned instead.

*/
long   TInifile::GetValLong( const char *entry , 
						  const char *def, 
						  const char *remark,
						  char hide)
{
	const char *res = GetValChar(entry, def, remark,hide); 
	if( res )
		return saveAtol( res );
	else
		return 0;
}

int   TInifile::GetValInt( const char *entry , 
						  const char *def, 
						  const char *remark,
						  char hide)
{
	return GetValLong( entry, def, remark, hide);
}

/*!
	returns a double value of an entry
	If the entry does not exist saveAtoi(*def) will be returned instead
*/
double TInifile::GetValFloat( const  char *entry , 
							 const char *def, 
							 const char *remark,
							 char hide)
{
	const char *res = GetValChar(entry, def, remark,hide); 
	if( res )
		return  saveAtof( res );
    return 0;
}



/*!
	returns a char value of an entry
	If the entry does not exist saveAtoi(*def) will be returned instead
*/
const char *TInifile::GetValChar( const char *entry, 
								 const char *def, 
								 const char *remark,
								 char hide)
{
	TInientry *temp = Find( entry );
	if( temp &&
		strlen(temp->Val()) > 0 ) // don't return empty entries
	{
		return temp->Val();
	}
	else if( def ) // entry not found, use/write default?
	{
		char *tempStr = NULL;
		if( FilenameI.length() > 0 )
		{
			tempStr = new char[strlen(entry)+50+FilenameI.length()];
			sprintf(tempStr,"%s is missing/empty in %s, add default?",
				entry, FilenameI.c_str() );
		}

		const char *s = NULL;
        if( !hide )
            s = _YesNoQuestion(tempStr, "y", NULL);

		if( !s ||
			strcmp(s, JK_YES) ) // no yes -> hide
            hide = 1;
        
        {
			Dirty = 1;
			if( !temp ) // create new
			{
				temp = new TInientry( entry, def, remark, hide );
				temp->SetNext( EntriesI );
				EntriesI = temp;
			}
			else
			{
				temp->SetVal( def, remark, hide );
			}
		}
		if( tempStr )
			delete [] tempStr;
	}
    else
        return NULL;
    
    return temp->Val();
}
void TInifile::Write( char binary )
{
	if( FilenameI.length() == 0 )
		return;

	FILE *out = NULL;
	if( !binary )
		out = fopen(FilenameI.c_str(), "wt");
	else
		out = fopen(FilenameI.c_str(), "wb");

	if( out )
	{
		TInientry *temp = EntriesI;
		while( temp )
		{
			temp->Write( out, this,  binary);
			temp = temp->Next();
		} // while
		fflush( out );
		fclose( out );
	} // if out
	Dirty = 0;
}

void TInifile::setFilename( const char *filename )
{
	createFilename( filename );
}



void TInifile::createFilename(const char *filename)
{
	
	if( filename && strlen(filename) == 0 )
	{
		filename = NULL;
	}
	if( filename )
	{
		FilenameI = string(filename);		
	}
	else
	{
		FilenameI = string("");
	}
}


/// get max n of all entries with name = name+n (= integer suffix) 
/// -> name+(n+i) == unused 
int TInifile::getMaxN(const char *name)
{
	if( !name )
		return -1;

	int maxN = -1;
	TInientry *temp = EntriesI;
	while( temp )
	{
		if(!strncmp(temp->Entry(), name, strlen(name) ) )
		{
			int n;
			sscanf(temp->Entry() + strlen(name), "%d", &n );
			if( n > maxN )
				maxN = n;
		}
		temp = temp->Next();
	}
	return maxN;
}

/// entry name = name + n as suffix
void TInifile::SetEntryN( const char *name, int n, 
						 const char *val,
                          const char *remark,
                          char hide)
{
#ifndef __BCPLUSPLUS__
	ostringstream nameN;
	nameN<< name;
	nameN<< n;

	/// set val or cretae if needed
	SetEntry(nameN.str().c_str(), val,remark, hide);
#else
	char *nameN = new char[ strlen(name)+20];
	sprintf("%s%d", name, n);
	SetEntry(nameN, val, remark, hide );
	delete [] nameN;
#endif
}

/// add a new entry with a unique int suffix == retval
int TInifile::addEntryN( const char *name, 
						const char *val,
                         const char *remark,
                         char hide)
{

	int n = getMaxN(name);
	n++;
	SetEntryN(name,n,val,remark, hide);
	return n;
}

//------------------------------------------------------------------------------------
int TintPList::c()
{
	return countI;
}

//----- TintPList ---- not yet totally implemented
void TintPList::scanStr( const char *strOrg )
{
	if( !strOrg ) 
		return;

	// make copy because of const!! 
	char *str = new char[strlen(strOrg)+1];
	strcpy( str, strOrg );
	char *cp = str;

	// count nr of entries
	countI = 1;
	char *ptr = str;
	
	/// remove {}, []
	char delim = 0;
	
	// search for ","
	int i=0;
	while( ptr[i] != 0 && 
		   delim == 0)
	{
		if( ptr[i] == '[' )
			delim = '}';
		else if( ptr[i] == '{' )
			delim = '}';
		else if( ptr[i] == '\"' )
			delim = '\"';
		else
			i++;
		
		if( delim ) 
			ptr[i] = ' ';	// replace by space		
	} // while in string
	
	/// search for closing delim
	if( delim )
	{
		i = strlen(ptr)-1;
		while( i > -1 )
		{
			if( ptr[i] == delim )
			{
				ptr[i] = ' '; // replace by space
				i = -1; // stop loop
			}
			i--;
		};	
	} // if delim
	
	/// scan for numbers
	while( strstr(ptr,",") )
	{
		countI++;
		ptr = strstr(ptr,",")+1;
	}
	// create entries
	values = new double[countI];
	ptr = str;
	i = 0;
	while( strstr(str,",") )
	{
		ptr = strstr(str,",");
		*ptr = 0; // make end of string
		values[i] = saveAtof( str );
		str = ptr + 1; // skip 0
		i++;
	}
	// add last entry
	values[i] = saveAtof( str );
	delete []  cp;
}

TintPList::TintPList( const char *str )
{
	countI = NULL;
	values = NULL;
	scanStr( str );
}
TintPList::~TintPList( void )
{
	if( values )
		delete [] values;
}

TintPList::TintPList( int count,
			   int *initVals )
{
	countI = count;
	values = new double[countI];
	int i;
	for( i = 0; i < countI; i++ )
	{
		values[i] = initVals[i];
	}
}

int TintPList::val( int id, int &val )
{
	if( id < countI &&
		id > -1 )
	{
		val =  values[id];
		return 1; // succsess
	}
	return 0; // error
}

int TintPList::setVal( int id, int val )
{
	if( id < countI &&
		id > -1 )
	{
		values[id] = val;
		return 1; // succsess
	}
	return 0; // error
}


/// double PList -------
TdoublePList::TdoublePList( const char *str ) : TintPList( str )
{
}

TdoublePList::TdoublePList( int count,
			   double *initVals ) : TintPList( NULL )
{
	countI = count;
	values = new double[countI];
	int i;
	for( i = 0; i < countI; i++ )
	{
		values[i] = initVals[i];
	}
}

TdoublePList::~TdoublePList( void )
{
}

int TdoublePList::val( int id, double &val )
{
	if( id < countI &&
		id > -1 )
	{
		val =  values[id];
		return 1; // succsess
	}
	return 0; // error
}

int TdoublePList::setVal( int id, double val )
{
	if( id < countI &&
		id > -1 )
	{
		values[id] = val;
		return 1; // succsess
	}
	return 0; // error
}


#endif

int TInifile::getLineLength(FILE *in, char binaryMode)
{
	// count length of current line
	int lineLength = 0;
	if( !binaryMode )
	{
		int chr = fgetc( in );
		while( 	!feof( in ) &&
			chr != '\n' &&
			chr != '\r' )
		{
			chr = fgetc( in );
			//				printf("%c",chr);
			lineLength++;				
		} // while
	}
	else
	{
        int msb = fgetc(in);
        int lsb = fgetc(in);
        lineLength = msb*256 + lsb;
	}
	return lineLength;	
}

char TInifile::RemoveEntry(const char *name )
{
	TInientry *prev = NULL;
	TInientry *cur = EntriesI;
	while( cur &&
		   strcmp(cur->Entry(), name ) )
	{
		prev = cur;
		cur = cur->Next();
	}
	if( prev )
	{
		if( cur )
			prev->SetNext( cur->Next() );
		else
			prev->SetNext( NULL );
	}
	else
	{
		if( cur )
			EntriesI = cur->Next();
		else
			EntriesI = NULL;
	}
	if( cur )
	{
		Dirty = 1; 
		delete cur;
		return 1;
	}
	return 0;
}

char * TInifile::readLine(FILE *in, int lineLength, char binaryMode)
{
	if( !binaryMode )
	{
		char *entry = new char[lineLength+1];
		// read single line
		char chr = fgetc( in );
		int c = 0;	
		while( c < lineLength &&
				!feof( in ) &&
				chr != '\n' )
		{
			if( chr != '\r' )
				entry[c] = chr;
			else
				entry[c] = ' ';	
			chr = fgetc( in );
			c++;
		}
		entry[c] = 0; // str end;
		return entry;	
	}
	return NULL; // binary not implemented
}
