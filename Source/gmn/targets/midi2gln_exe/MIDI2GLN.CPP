/* midi2gmn */
/* (c) 1998 by Juergen Kilian */
/* MIDI To GUIDO Low level notation converter */

#include <stdio.h>
#include <string>

#include "midi.h"
#include "funcs.h"

char defaultFileName[] = "test.mid";

FILE *STDERR;

int main(int argc, char*argv[]) {
  char *fileName;
  char *inFileName;
  char *outFileName, *s;

  printf("\nMIDI2GLN %s\n\n",VERSION);
  printf("written 1998 by Jürgen Kilian ");
  printf("as part of the GUIDO toolbox\n");
  printf("for more information on GUIDO Music Notation please visit\n");
  printf("http://www.informatik.tu-darmstadt.de/AFS/CM/GUIDO\n\n");

	if(argc <= 1)
	  fileName = defaultFileName;
	else if(strcmp(argv[1],"-h")==0 || strcmp(argv[1],"/h")==0
			 || strcmp(argv[1],"-help")==0 || strcmp(argv[1],"/help")==0
			 || strcmp(argv[1],"-?")==0 || strcmp(argv[1],"/?")==0 ) {
	 printf("description:\n");
	 printf("  converts MIDI files to GLN (guido low-level notation)\n\n");
	 printf("call syntax:\n");
	 printf("  midi2gln <filename>\n");
	 printf("  midi2gln <filename>.mid\n");
	 printf("  -> creates file <filename>.gmn from <filename>.mid\n");
	 printf("  midi2gln -help\n");
	 printf("  -> displays this information\n");
	 printf("\n");
	 return(0);
	 }
	else if ( argc == 2 )
		fileName = argv[1];
	else
	{
		printf("Error: Wrong calling syntax!\n");
		return(0);
	}


  outFileName = new char[strlen(fileName)+4+1];
  inFileName = new char[strlen(fileName)+4+1];
  strcpy(outFileName, fileName);

  if((s=strstr(outFileName, ".mid"))==NULL) {
	 sprintf(outFileName, "%s.gmn", fileName);
	 sprintf(inFileName, "%s.mid", fileName);
	 }
  else {
	 strcpy(s,".gmn");
	 strcpy(inFileName,fileName);
	 }

  printf("converting '%s' to '%s' ... \n",
			inFileName, outFileName);

	TMIDIFILE *Infile;	// Zeiger auf MIDI-Datei

	FILE *TempOut;
	TempOut = fopen( outFileName, "wt" );
	STDERR = fopen( "stderr.txt", "wt");

	Infile = new TMIDIFILE(inFileName );
	if( !Infile )
		ErrorMsg( 2 );
	if( !Infile->Open(TempOut) )
		ErrorMsg( 0 );

	if( !Infile->Read(1, TempOut) )
		ErrorMsg( 1 );
	fclose( TempOut );
	fclose( STDERR );

	delete Infile;
	/* ------------------- */



  delete [] outFileName;
  delete [] inFileName;

  printf("... done.\n\n");
	return(0);
  }

