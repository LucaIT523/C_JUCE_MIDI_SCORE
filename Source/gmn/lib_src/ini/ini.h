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

/*
	ini.h
	Definition of the classes TInifile and TInientry.
	Users can access all entries by just using the TInifile class

	(c) 2002 by Juergen Kilian kilian@salieri.org
*/

#if !defined( __INI_H__ )
#define __INI_H__

#include <stdio.h>
#ifndef __BCPLUSPLUS__
#include <string>
#include <string>
using namespace std;
#endif


class TInifile;

#define REMARK_ENTRY "REMARK"

/*!
	class TInientry
	- represents a single entry of TInifile
	- the user needs not to access this class directly
*/
/// Inifile entry with entry(=key), value and remark string
class TInientry
{
protected : 
    /// entry name, might be NULL if only remark
	char *EntryI;
    /// entry value,  all value types are internally stored as strings!
	char *ValI;
    /// remark
    char *remarkI;
	/// if hideflag == 1 entry will not be written to file
	char hideFlag;

	/// link to next entry
	TInientry *nextI;
public:
	TInientry(  /// entry name
				const char *entry,
				/// entry value
	            const char *val,
	            /// optional remark value
	            const char *remark,
	            /// if hide == 1 entry will note be written to file
				char hide = 0);

	virtual ~TInientry( void );
    
	void SetVal( const char *val,
                 const char *remark,
              	 char hide );	
	void SetNext( TInientry *next );
	TInientry *Next( void )
		{ return nextI; };
	const char *Val( void  )
		{ return ValI; };
	const char *Entry( void )
		{ return EntryI; };
	/// write to file 
	void Write( FILE *out,
				TInifile *inifile,
				char binary = 0
				);
};


/*!
	class TInifile 

	A user can set and entries by 
	void SetEntry( char *entry, const char *val );
	void SetEntry( char *entry, double val );	// double entry
	if an entry does not it will be created otherwise the old 
	value will be overriden and the Dirty flag will be set

	A user can retrieve entry values by
	char   *GetValChar( char *entry ,	// entry name
						char *def);		// default value, NULL is allowed
	int    GetValInt( char *entry ,		// entry name
					  char *def);		// default value, NULL is allowed
	double GetValFloat( char *entry ,	// entry name
						char *def);		// default value, NULL is allowed
	If the entry does not exist and def!=NULL it will be added 
	to the entry list and *def will be returned.
	If the entry does not exists and def==NULL only *def will be
	returned and nothing will be added to the list.

    If a filename is specified all changes of entries will be
	automatically written to the specified file when deleteing a
	TInifile variable.
*/
/// Inifile class with retrieval functions
class TInifile
{
	friend class TInientry;
protected:
	/// filename including suffix
	std::string FilenameI;
	/// list of entries
	TInientry *EntriesI;
	/// find an entry by name
	TInientry *Find( const char *entry );
	/// read and fill list
	virtual void Read( char binaryMode );
	/// is 1 if modified
	char Dirty;
	/// set FilenameI
	void createFilename( const char *filename);
	/// read a single line
	virtual char *readLine( FILE *in, 
							int lineLength, 
							char binaryMode );
	/// get linelength for current filepos
	virtual int getLineLength( FILE *in, 
								char binaryMode );
	virtual void FPrintf(FILE *out,
						 const char *valStr,
						 char binary );

	virtual void FPrintf(FILE *out,
						 const char *formatStr,
						 const char *valStr,
						 char binary);	
public:
	/// if filename == NULL no file will be read or written,
	/// but information can be shared between modules!
	TInifile( const char *filename);	
	/// includes automatic write if dirty (= commit)
	virtual ~TInifile( void );			

	const char *filename( void ){return FilenameI.c_str(); };
	/// can be used for later filename specification
	void setFilename( const char *filename );	
	/// write entries to file
	virtual void Write( char binary = 0 );				
	
	
	/// str val
	void SetEntry( const char *entry, 
				   const char *val,
                   const char *remark = NULL, 
                   char hide = 0 ); 
	/// entry name = name + n as suffix
    void SetEntryN( const char *name, 
    				int n, 
    				const char *val,
                    const char *remark = NULL,
                    char hide = 0);
	/// add a new entry with a unique int suffix == retval
    int addEntryN( const char *name, 
    			   const char *val,
                   const char *remark = NULL, 
                   char hide = 0);
	/// double val entry
	void SetEntry( const char *entry, 
				   double val, 
				   char *remark = NULL, 
				   char hide = 0 );


	/// read and add if not exist entries by
	const char  *GetValChar( /// entry name
							 const char *entry ,	
							 /// default value, NULL is allowed
							 const char *defValue,
							 const char *remark = NULL,
							 char hide = 0);
	int    GetValInt( /// entry name
					  const char *entry,
					  /// default value, NULL is allowed		
					  const char *defValue,		
					  const char *remark = NULL,
					  char hide = 0
					  );
	long    GetValLong( /// entry name
						const char *entry,
						/// default value, NULL is allowed		
					    const char *defValue,		
					    const char *remark = NULL,
					    char hide = 0);
					    
	double GetValFloat( /// entry name
						const char *entry,
						/// default value, NULL is allowed	
						const char *defValue,		
						const char *remark = NULL,
						char hide = 0);


	/// get max n of all entries with name = name+n (= integer suffix)  -> name+(n+i) == unused 
	int getMaxN(const char *name);

	char RemoveEntry( const char *name );

};


/// integer parameter list (separated by ,)
class TintPList{
protected : 
	/// nr of entries
	int countI;
	/// value list
	double *values;
	/// scan strinf and fille data structure
	void scanStr( const char *str );
public :
	/// # of entries
	int c( void );
	/// call with "," separated string
	TintPList( const char *str ); 
	/// call with array of integers
	TintPList( int count, int *initVals );
	/// retrieve value add pos id, return 1 == ok, 0 == error
	int val( int id, int &value);
	int setVal( int id, int value );
	virtual ~TintPList( void );
};

/// double parameter list (separated by ,)
class TdoublePList : public TintPList
{
public:
	/// call with "," separated string
	TdoublePList( const char *str ); 
	/// call with array of integers
	TdoublePList( int count, double *initVals );
	/// retrieve value add pos id, return 1 == ok, 0 == error
	int val( int id, double &value);
	int setVal( int id, double value );
	virtual ~TdoublePList( void );
	/// get values split to an array
	double *list( int &size )
	{
		size = c();
		return values;
	}
};


//-------------- some tool functions -------------------------------
void _Printf( const char *str ); // should to be defined for output
const char * _YesNoQuestion(const char *tempStr, 
				  const char *defStr,
				  const char *);

#ifdef INTERACTIVE
/*!
 if TInifile should be used in an interactive environment
 this functions needs to be defined somewhre
*/
/// interactive question function
const char * YesNoQuestion(const char *tempStr, 
						  const char *defStr,
						  const char *);
#endif


long saveAtol(const char *str);
double saveAtof(const char *str);
int saveAtoi(const char *str);

#endif
