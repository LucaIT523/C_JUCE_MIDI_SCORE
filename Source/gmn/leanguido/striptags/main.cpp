/*
 * main file for striptags leanGUIDO demo application
 *
 *
 *
 */


#include "striptags.h"
const char *version_str = "striptags v1.0";
int main( int argc, char *argv[] )
{
    printf("This is %s for removing all tags of a GUIDO file.\n", version_str);
    printf("written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n\n");
    printf("Usage: striptags  [tagname] filename\n");
	printf("If tagname is not given all tags will be removed\n");
	printf("otherwhise only only tags named  tagname will be removed.\n\n");
	printf("The ouput will be written to filename.str\n\n");
    printf("Please report problems to kilian@noteserver.org and\n");
    printf("visit also the GUIDO noteserver: http://www.noteserver.org \n");
    printf("\n");
    FILE *out;
    if( argc != 2 &&
        argc != 3 )
    {
        printf("Error: wrong number parameters \n");
        return 1;
    }

    printf("processing %s\n", argv[argc-1]);
    string outfname;

    outfname = argv[argc-1];
	outfname += ".str";

    myLgSegment *segment;
    segment = new myLgSegment();
    int c = 0;
    /// parse .gmn file
	if( !segment->parseFile( string(argv[argc-1])) )
    {
        if(argc == 3) /// remove tags by name
		{
			printf(" removing tag %s\n",argv[1] );
            c = segment->removeTags( argv[1] );

		}
        else /// remove all tags
		{
			printf(" removing all tags\n");
            c = segment->removeTags( NULL );
		}
            
        /// write output file
        out = fopen(outfname.c_str(), "wt" );
        if( !out )
        {
            printf(" Error: Can't open outfile %s!\n",outfname.c_str() );
            delete segment;
            return 1;
        }
        printf(" removed %d tags.\n",c);
        printf(" writing output: %s\n", outfname.c_str() );
        fprintf(out,"%%all tags removed by %s\n", version_str );
        fprintf(out,"%%written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n");
        fprintf(out, "%%Please report problems to kilian@noteserver.org and\n");
        fprintf(out, "%%visit also the GUIDO noteserver: http://www.noteserver.org \n");
        fprintf(out, "\n");
        segment->lgSegment::write( out );
        fclose( out );
        printf("successful finished.\n");
    }
    delete segment;
    return 0;
}