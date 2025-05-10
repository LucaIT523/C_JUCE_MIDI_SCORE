#ifndef _midi2gmn_h_
#define _midi2gmn_h_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;


#ifdef WIN32

# ifdef MIDI2GMN_EXPORTS
#  define export		 _declspec (dllexport)
# else
#  define export		 _declspec (dllimport)
# endif

# pragma warning ( disable : 4275 4251 )

#else

# ifdef MIDI2GMN_EXPORTS
#  define export		 __attribute__ ((visibility("default")))
# else
#  define export		
# endif

#endif


typedef string (CBInputQuestion)(const char *prompt, const char *def, const char * /*midiFilename*/);
typedef const char* (CBYesNoQuestion)(const char *prompt, const char *def, const char * /*midiFilename*/);
typedef void (CBShowErrorMsg)(const char *msg);

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_kitcon_midi2gmn_Midi2Gmn
 * Method:    process
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */


#ifdef __cplusplus
	export int registerCallbacks(CBInputQuestion *inputQuestion,
								CBYesNoQuestion *yesNoQuestion,
								CBShowErrorMsg *errorMsg);
	export int readMidiFile(int argc, char*argv[], char *Mode, char*	p_outPath);
	export int processBuffer( std::string *buffer,
					std::string *fileName,
				 stringstream  &gmnOutput, 
				   const string &iniFile);
}
#endif

#endif
