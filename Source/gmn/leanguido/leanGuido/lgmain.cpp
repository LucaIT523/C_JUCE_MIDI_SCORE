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
	lgmain.cpp - main() for testing the leanGUIDO library 
	reads a .gmn file and writes the output to test_out.gmn
*/

#include "../lgsegment.h"
#include "../lgchord.h"


#include "../../parser/guido.h"

#include <iostream>

/// defined in lgguido.cpp
extern lgSegment *curSegment;



int main( int argc, char *argv[] )
{
    curSegment = NULL;

	char *fname;
	if( argc < 2 )
		fname = "test.gmn";
	else
		fname = argv[1];

	std::cout << "Parsing " << fname << "\n";
    
    /// call the parser-kit, curSegment will be filled from there
    gd_parse(fname, 0);



    FILE *out;
    fname = "test_out.gmn";
    printf("Writing %s\n",fname);
    out= fopen(fname,"wt");

	/// write the data to a new .gmn file
    if( curSegment )
        curSegment->write(out);
		
	fclose(out);

	out = fopen("_test_split.gmn","wt");
    if( curSegment )
	{
		curSegment->splitTagRanges();
        curSegment->write(out);
	}

	fclose( out );

    delete curSegment;
    printf("Complete\n");
    return 0;
}
