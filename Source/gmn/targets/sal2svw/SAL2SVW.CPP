//#include <stdlib.h>
#include "../../fermata/portable.h"
#include <stdio.h>
#include <string.h>



void main ( int argc, char *argv[] )
{
	char OutFilename[255];
	char InFilename[255];

	char *Endung,
		  c;

	int state = 0;

	FILE *InFile,
	     *OutFile;


//	printf("\nSalieri to Salview\n");

	if( argc == 0 )
	{
//		printf("\nDateiname fehlt !\n");
		exit( 1 );
	}

	strcpy( InFilename, argv[1] );
	strcpy( OutFilename, argv[1] );
	strupr( OutFilename );

	Endung = strstr( OutFilename, ".SAL");
	if( !Endung )
	{
		printf("\UngÅltiger Dateiname\n");
		exit( 1 );
	}

	strcpy( Endung, ".SVW");

	InFile = fopen( InFilename, "rt");
	if( !InFile )
	{
		printf("\nEingabeDatei kann nicht geîffnet werden\n");
		exit( 1 );
	}
	OutFile = fopen( OutFilename, "wt");
	if( !OutFile )
	{
		printf("\nAusgabeDatei kann nicht geîffnet werden\n");
		exit( 1 );
	}

	printf("Eingabedatei : %s\n", InFilename );
	printf("Ausgabedatei : %s\n", OutFilename );
	while( !feof( InFile ) )
	{
		c = fgetc( InFile );
		switch( state )
		{
			case 0 : if( c == '[' ) // Bezeichner Åberlesen
				 {
					putc( c, OutFile );
					state = 1;
				 }
				 break;
			case 1 : if( c == ']' ) // stimme endet
				 {
					state = 2;
				 }
				 else
				 {
					putc( c, OutFile );
				 }
				 break;
			case 2 : if( c == ',' )
				 {
					state = 3; // neue Stimme
				 }
				 if( c == '[' )   // Stimme geht weiter
				 {
					state = 1;
				 }
				 break;
			case 3 : if( c == '[' )	// neue Stimme beginnt
				 {
					putc( c, OutFile );
					state = 1;
				 }
				 break;

		} // switch

	} //

	putc( ']', OutFile );

	fclose( InFile );
	fclose( OutFile );

} // main

