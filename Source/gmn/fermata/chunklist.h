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

/*!
	chunklist.h
	by Juergen Kilian
	9/2003
*/

#ifndef __chunkList_h__
#define __chunkList_h__

#include <stdio.h>

/// ptr to void, not clean but usefull
typedef void * PTVoid;
/// generic chunk-list class, inherited classes should define a get and set function to encapsulate getP and setP 
class TChunkList
{
protected:

	/// start id
	int offset;

	/// array size 
	int chunkSize;
	/// the ptr array = [0...maxID]
	PTVoid *chunk;
	TChunkList *link;
	/// get ptr at postion id, inherited classes may do a typecastm, should only be called from inherited classes
	PTVoid getP( /// [0...maxID]
			     int id );
	
	/// set ptr at postion id, return = 1 if ok ,0 else (range error), chould only be called from inherited classes
	char setP( /// [0...maxID]
				int id, 
			  	PTVoid ptr );
	/// end id
	int maxID;

public: 
	void clear( void );
	virtual void remove( int id );
	TChunkList( /// chunk size
				 int cS,
				 /// start id
				 int os = 0 );
	/// destructor, deletes only ptr array(!) id deepCopy == 0 not the ptr-objects!
	virtual ~TChunkList( void );

	int count( void )
	{
		return maxID+1;
	}
};
/// chunklist with "double" entries
class TDoubleChunkList : public TChunkList 
{
public:
	TDoubleChunkList( /// chunk size
				 int cS =10,
				 /// start id
				 int os = 0 ) : TChunkList( cS, os ){};

	/// return ptr at position id or NULL
	double get( int id )
	{
		void *ptr = getP(id);
		if( ptr )
			return *((double *)(ptr));
		else
		{
			fprintf(stderr,"Double chunk list NULL pointer at %d!\n", id);
			return 0;
		}
	}
	/// set ptr at pos id, return = 1 if ok, 0 if range error
	char set( int id, 
			  double val )
	{

		double *ptr = new double;
		*ptr = val;

		return setP(id, ptr );
	}

	virtual void add( double val )
	{
		set(count(), val );
	}
	virtual ~TDoubleChunkList( void )
	{
		// delete the objects modified jk memory leak test
		for( int i = offset; i <= maxID; i++ )
		{
			void *ptr = getP(i);
			if( ptr )
				delete (double *)(ptr);
			else
				fprintf(stderr,"Double chunk list NULL ptr at %d!\n", i);
		}
	}
	virtual void Debug( FILE *out = NULL );

};


/// chunklist with "int" entries
class TIntChunkList : public TChunkList 
{
public:
	TIntChunkList( /// chunk size
				 int cS =10,
				 /// start id
				 int os = 0 ) : TChunkList( cS, os ){};

	/// return ptr at position id or NULL
	int get( int id )
	{
		void *ptr = getP(id);
		if( ptr )
			return *((int *)(ptr));
		else
		{
			fprintf(stderr,"Double chunk list NULL pointer!\n");
			return 0;
		}
	}
	/// set ptr at pos id, return = 1 if ok, 0 if range error
	char set( int id, 
			  int val )
	{

		int *ptr = new int;
		*ptr = val;

		return setP(id, ptr );
	}

	virtual void add( int val )
	{
		set(count(), val );
	}
	virtual ~TIntChunkList( void )
	{
		// delete the objects 
		for( int i = offset; i < maxID; i++ )
		{
			delete (int *)getP( i );
		}
	}
	 double operator[](int i)
	{
		return get(i);
	}
};


#endif
