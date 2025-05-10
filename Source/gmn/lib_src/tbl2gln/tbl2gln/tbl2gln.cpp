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
 *  Copyright © 1997-2002 Metrowerks Corporation.  All Rights Reserved.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */

#include "ini.h"
#include <stdio.h>
#include <iostream>

int convert( string cFilnanme );
int convert( string inFilnanme, string outFilename );
int nextLine( FILE *in, char *buffer, int bufSize );
int convertLine( char *cLine, FILE *out );


double sustainDuration = 0;

int convertLine( char *cLine, FILE *out )
{
	int res = 0;
	if( strstr(cLine, "score") ||
		strstr(cLine, "dur") ||
//		strstr(cLine, "NaN" )||
		strlen(cLine) < 3 )
	{
		// skip
	}	
	else
	{
		res = 1;
		char *nextColumn = NULL;
		int column = 0;
			int scoreTime = 0,
				key = 0,
				velo = 0,
				voice = 0,
				bar = 0,
				beat = 0;
				
			double perfTime = 0,	
				   duration = 0,
				   susDuration = 0;
			// skip leading whitespace
				   
		/*
		if( sscanf(cLine, "%d\t%f\t%d\t%d\t%f\t%d\t%d\t%d",
					scoreTime,
					perfTime, 
					key,
					velo,
					duration,
					voice,
					bar,
					beat) >= 5 ) 
		{
			printf(".");
		}
		*/
		
		while( cLine &&
		       (*cLine) != 0)
		{
			nextColumn = strstr( cLine, "\t" );
			if( nextColumn )
			{
				(*nextColumn) = 0;
				nextColumn++;
			}	   

			double val;
			if( strstr(cLine, "N") )
				val = -1;
			else
				 val = atof( cLine );
			switch( column )
			{
				case 0 : scoreTime = val; break;
				case 1 : perfTime = val; break;
				case 2 : key = val; break;
				case 3 : velo = val; break;
				case 4 : duration = val; break;
				case 5 : voice = val; break;
				case 6 : bar = val; break;
				case 7 : beat = val; break;
			} // switch
			column++;
			cLine = nextColumn;
		}
		
		
		// writw output
		fprintf(out, "\\note<");
		fprintf(out, "st=%d", scoreTime);
		fprintf(out, ", a=%f", perfTime);
		fprintf(out, ", p=%d", key);
		fprintf(out, ", i=%d", velo);
		if( sustainDuration > 0 &&
		    duration > sustainDuration ) 
		{
			double sd;
			sd = duration - sustainDuration;
			
			duration = sustainDuration;
			fprintf(out, ", sd=%f", sd);			
		}
		fprintf(out, ", d=%f", duration);
		fprintf(out, ", voice=%d", voice);
		fprintf(out, ", beat=%d", beat);
		fprintf(out, ", bar=%d", bar);
		fprintf(out, ">\n");
	}
	return( res );
}


int nextLine( FILE *in, char *buffer, int bufSize )
{
	int i = 0;
	int res = 0;
	buffer[0] = 0;
	while( !feof( in ) &&
		   i < bufSize )
	{
		res ++;
		unsigned char c;
		buffer[i] = 0;
		c = fgetc(in);
		if( c == '\n' ||
			c == '\r' )
		{
			 i = bufSize; // stop loop
		}
		else
		{
			buffer[i] = c;
			i++;
		}	
	}
//	printf(":%s:\n", buffer );
	buffer[bufSize-1] = 0;
	return res;
}



int convert( string inFilename, string outFilename )
{
	
	int res = 0;
	FILE *in = NULL, 
		*out = NULL;
	
	in = fopen( inFilename.c_str(), "rt");
	if( !in )
	{
		printf("ERROR: can't open infile %s\n", inFilename.c_str() );
	}
	else
	{
		out = fopen( outFilename.c_str(), "wt" );
		if( !out )
		{
			printf("ERROR: can't open outfile %s\n", outFilename.c_str() );
		}
	}
	if( in && out )
	{
		cout << "converting " << inFilename << " to " << outFilename << "\n";	
	
		char cLine[1024];
		fprintf(out, "[ (* \n");
		fprintf(out,"st=scoretime with 96ppq\n");
		fprintf(out,"a=performace time [ms]\n");
		fprintf(out,"d=performance duration [ms]\n");
		fprintf(out,"sd=sustain duration [ms]\n");
		fprintf(out,"i=MIDI velocity\n");
		fprintf(out,"p=MIDI pitch\n");
		fprintf(out,"v=voice\n");
		fprintf(out,"bar=1 if score note is at beginning of bar\n");
		fprintf(out,"beat=1 if score note is on the beat\n");
		
		fprintf(out, "*)\n");
		int count = 0;
		while( nextLine( in, cLine, 1024 ) )
		{
			if( convertLine( cLine, out ) )
			{
				count++;
			}
		}
		fprintf(out, "]\n");
		res = 1;
		cout << "converted " << count << " lines\n";
		cout << "done.\n"; 
	}
	
	
	if( in )
		fclose( in );
	if( out )
		fclose( out );
	return res;
	
}


int convert( string inFilename )
{
	char *outFilename;
	if( inFilename == "" )
	{
		printf("ERROR: no input files specified!\n");
		return 0;
	}
	outFilename = new char [strlen(inFilename.c_str())+1];
	strcpy( outFilename, inFilename.c_str());
	int res = 0;
	char *extension = strstr( outFilename, ".tbl");
	if( extension )
	{
		strcpy( extension,".gmn");
		res = convert(inFilename, outFilename );
	}
	else
	{	
		printf("ERROR: wrong extension for filename %s!\n", inFilename.c_str() );
	}
	
	delete [] outFilename;
	return res;	
}


int main(const int argc, char **argv  )
{
	printf("This is tbl2gln: convert tabel data into low-level GUIDO data\n");
	printf("usage: tbl2gln [infilename | ini filename] [-defDuration]\n");
	printf("if infialename is omitted tbl2gln.ini will be used\n");
	printf("(c) 2004 Juergen Kilian (kilian@noteserver.org)\n\n");
	

		
	string fileList = "tbl2gln.ini";
	string cFilename;
	for( int i = 1; i < argc; i++ )
	{
		if( strstr(argv[i], ".tbl" ) )
		{
			cFilename = argv[i];
			fileList = "";
		}
		else if( (argv[i])[0] == '-' )
		{
			sustainDuration = atof( &((argv[i])[1]) );
			if( sustainDuration > 0 )
				printf("defDuraion = %f\n", sustainDuration);
			else
				sustainDuration = 0;
		}
		else if( strstr(argv[i], ".ini" ) )
		{
			fileList = argv[i];
		}
	}
	
	if( strlen(fileList.c_str()) > 0 )
	{
			
			
		// use tbl2gln.ini
		FILE *in;
		in = fopen(fileList.c_str(), "rb");
		
		if( in )
		{
			char lineBuffer[1024];	
			
			while( nextLine(in, lineBuffer, 1024 ) )
			{
				if( strlen( lineBuffer ) > 1 ) 
				{
					cFilename = lineBuffer;
					convert(cFilename);
				}
			}		
		}
		else
		{
			printf("ERROR: can't open listfile %s!", fileList.c_str());
		}
		
	}
	else if( strlen(cFilename.c_str()) > 0 ) 
	{
		convert( cFilename );
	}
	else
	{
		printf("ERROR: wrong number of parameters %d!\n", argc);
	}
	
	
	return 0;
}