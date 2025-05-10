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

#include <stdlib.h>
#include "chunklist.h"

TChunkList::TChunkList( int cs,
						int os )
{
	chunkSize = cs;
	maxID = -1;
	offset = os;
	link = NULL;
	chunk = new PTVoid[chunkSize];
	
	int i;
	for( i = 0; i < chunkSize; i++ )
	{
		chunk[i] = NULL;
	}
};

void TChunkList::clear()
{
	maxID = -1;
}

TChunkList::~TChunkList( void )
{
	delete [] chunk;
	if ( link )
		delete link;
	
}


char TChunkList::setP( int id, PTVoid ptr )
{
	// remember highest child ID
	if( id > maxID )
		maxID = id;

	if( id < offset + chunkSize )
	{
		chunk[id-offset] = ptr;
		return 1;
	}

	/// create link?

	if( !link )
	{
		link = new TChunkList(chunkSize, offset+chunkSize);
	}

	return link->setP( id, ptr );

}


PTVoid TChunkList::getP( int id )
{
	if( id > maxID )
		return NULL;
	if( id < offset + chunkSize )
	{
		return chunk[id-offset];
	}
	if( link ) // should always be true!
	{
		return link->getP( id );
	}

	return NULL; // == error

}
void TChunkList::remove( int id )
{
	int i;

	for( i = id; i < count(); i++ )
	{
		setP(i, getP(i+1));
	}
	maxID--;
}

//---------------------------------------
void TDoubleChunkList::Debug( FILE *out )
{
	char mustClose = 0;
	if( !out )
	{
		out = fopen("_doublechunklist.txt","wt");
		mustClose = 1;
	}
	int i;
	for( i = 0; i < count(); i++ )
	{
		fprintf(out,"%f\n",get(i));
	}

	if( mustClose )
	{
		fclose(out);
	}
}

