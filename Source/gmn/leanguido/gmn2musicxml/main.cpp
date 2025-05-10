/*
 main.cpp for gmn2musicxml
 GUIDO to MusicXML converter
 Juergen Kilian Tue April 15 2003 during
 *  a never ending train ride from Paris to Lyon
 
 */


#include "gmn2musicxml.h"
const char *version_str = "gmn2musicxml v1.0";
int main( int argc, char *argv[] )
{
    printf("This is %s for GUIDO to MusicXML.\n", version_str);
    printf("written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n");
    printf("Usage: gmn2musicxml filename\n");
    printf("Please report problems to kilian@noteserver.org and\n");
    printf("visit also the GUIDO noteserver: http://www.noteserver.org \n");
    printf("\n");
    FILE *out;
    if( argc != 2 )
    {
        printf("Error: wrong number parameters \n");
        return 1;
    }

    printf("processing %s\n", argv[1]);
    	string outfname;
        outfname = argv[1];
        outfname += ".xml";


        mXMLLgSegment *segment;
        segment = new mXMLLgSegment();
        if( !segment->parseFile(string(argv[1])) )
        {
            out = fopen(outfname.c_str(), "wt" );
            if( !out )
            {
                printf("Can't open outfile %s!\n",outfname.c_str() );
                delete segment;
                return 1;
            }
            printf("writing %s\n", outfname.c_str() );
            fprintf(out,"<!-- File created by by %s\n", version_str );
            fprintf(out,"written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n");
            fprintf(out, "Please report problems to kilian@noteserver.org and\n");
            fprintf(out, "visit also the GUIDO noteserver: http://www.noteserver.org -->");
            fprintf(out, "\n");
            segment->convert( );
            segment->write( out );
            fclose( out );
            printf("successful finished.\n");
        }
        delete segment;
        return 0;
}