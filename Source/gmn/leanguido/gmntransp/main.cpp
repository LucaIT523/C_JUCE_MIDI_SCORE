/*
 * leanGUIDO demo application
 * gmn tranpose
 */
#include "gmntransp.h"
const char *version_str = "gmntransp v1.0";

/// main function
int main( int argc, char *argv[] )
{
    printf("This is %s for chromatic transpose of GUIDO files.\n", version_str);
    printf("written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n\n");
    printf("Usage: gmntransp deltaPC1 [,deltaPC2..n]* filename\n\n");
    printf("If only deltaPC1 is given, all sequences/voices will be transposed\n");
	printf("by deltaPC semitone steps.\n");
    printf("If deltaPC2..deltaPCn are specified, each voice will transposed separately.\n\n");
	printf("The ouput will be written to _filename.gmn\n\n");
    printf("Please report problems to kilian@noteserver.org and\n");
    printf("visit also the GUIDO noteserver: http://www.noteserver.org \n");
    printf("\n");
    if( argc < 3 ){
    	printf("Error: filename or deltaPC is missing!\n");
    	exit(1);    	
    }
    FILE *out;
    
        printf("processing %s ... \n", argv[argc-1]);
        string outfname;
        outfname = "_";
        outfname += argv[argc-1];
        		/// use a new segment type
        myLgSegment *segment;
        segment = new myLgSegment();
            	/// parse the gmn file
    if( !segment->parseFile(string(argv[argc-1])) )
    {
	    int voice;
		int deltaPC;
        for( voice = 1; voice < argc-1; voice++ )
    	{
    		
            /// call the transpose function
            if( argc > 3 )
            {
            	deltaPC = atoi(argv[voice]);
            	printf(" transposing  voice %d by %d steps\n", voice, deltaPC);
	            segment->transpose( deltaPC, voice );
	        }
	        else // all voices
	        {
            	deltaPC = atoi(argv[1]);
	        	printf(" transposing all voices by %d semitone step\n",deltaPC);
	            segment->transpose( deltaPC );	        
	        }
        } // for
        out = fopen(outfname.c_str(), "wt" );
        if( !out )
        {
            printf(" Error: Can't open outfile %s!\n",outfname.c_str() );
            delete segment;
            return 1;
        }
        printf(" writing output to: %s\n", outfname.c_str() );
        fprintf(out,"%%File transposed by %s\n", version_str );
        fprintf(out,"%%written 2003 by Juergen Kilian as part of the GUIDO toolbox.\n");
        fprintf(out, "%%Please report problems to kilian@noteserver.org and\n");
        fprintf(out, "%%visit also the GUIDO noteserver: http://www.noteserver.org \n");
        fprintf(out, "\n");
        /// write the output, be sure not to call lgChord::write!
        segment->lgSegment::write( out );
        fclose( out );
        printf("done.\n");
 	} // if
     
    delete segment;
    return 0;
}