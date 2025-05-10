#define MIDI2GMN_EXPORTS
#include "../include/fermata.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "q_funcs.h"
#include "portable.h"
//#include "q_midi.h"
#include "h_midi.h"
#include "hpatternfile.h"
#include "../lib_src/ini/ini.h"
#include "import.h"


#include "../parser/guido.h"
#include "../leanguido/lgfactory.h"
#include "../leanguido/lgsegment.h"
#include "H_midi.h"
#include "H_track.h"
#include "Fragm.h"
#include "../lib_src/ini/ini.h"

#include "meta_tempo.h"
#include "meta_meter.h"

CBInputQuestion *cbInputQuestion = NULL;
CBYesNoQuestion *cbYesNoQuestion = NULL;
CBShowErrorMsg *cbShowErrorMsg = NULL;

int getPitch( lgTag *tag);
lgDuration getOnset( lgTag *tag);
lgDuration getDuration( lgTag *onset, lgTag *offset);
int getIntens( lgTag *tag);

bool isTag(lgTag *tag, const std::string &tagName);
int getBpm(lgTag *tag, int &bpm, int &tactusNum, int &tactusDenom);
int str2int(const string &str);
void tokenize(const std::string& str, vector<std::string>& tokens,
		const std::string& delimiters);




// FILE *STDERR;
// FILE *LOG_FILE; // accessed from all modules

// char *LOG_NAME = NULL;  // accessed from different modules

//#define Inifilename "FERMATA.INI"

#include "funcs.h"
#include <math.h>

#define INTERACTIVE
#include "import.h"

// extern TInifile *Inifile;

const char defaultFileName[] = "test.mid";


int readMidiFile(int argc, char*argv[], char* Mode, char* p_outPath) {
	clock_t start = clock();

	char *fileNameWork;
	char* fileNameMemory = "";

	time_t t;
	srand((unsigned) time(&t));
	char *s;
	char okFlag = 1;

	char *str = NULL; // for input question
	const char *Inifilename = NULL;

	int ex_code = 0;
	{ // fileNameBlock
	const char *fileName = NULL;

	//printf("\nmidi2gmn %s\n\n",getVersion().c_str());
	//printf("written 1998-2004-2012 by Juergen Kilian ");
	//printf("as part of the GUIDO toolbox\n");
	//printf("for more information on GUIDO Music Notation please visit\n");
	//printf("http://www.salieri.org/GUIDO\n\n");

	int relDuration = 0;
	
	 
	
	int cur_arg = 1;
	
	while( cur_arg < argc ) // read options and parameters
	{
		if(strcmp(argv[cur_arg],"-h")==0 || strcmp(argv[cur_arg],"/h")==0
			|| strcmp(argv[cur_arg],"-help")==0 || strcmp(argv[cur_arg],"/help")==0
			|| strcmp(argv[cur_arg],"-?")==0 || strcmp(argv[cur_arg],"/?")==0 ) 
		{
			//printf("description:\n");
			//printf("  converts MIDI files to GMN (guido music notation)\n\n");
			//printf("call syntax:\n");
			//printf("  midi2gmn <filename>\n");
			//printf("  midi2gmn <filename>.mid\n");
			////		printf("  midi2gmn -d<playDuration> <filename>\n");
			//printf("  midi2gmn -c<inifilename> <filename>\n");
			//printf("  midi2gmn -c<inifilename> <filename>.mid\n");
			//printf("  -> creates file <filename>.gmn from <filename>.mid\n");
			//printf("  -> quantise file <filename>.gmn from <filename>.gmn\n");
			//printf("  midi2gmn -help\n");
			//printf("  -> displays this information\n");
			//printf("\n");
			okFlag = 0; // no processing
			ex_code = EXIT_OK;
			cur_arg = argc; // stop 
		}
		else if( strncmp(argv[cur_arg],"-d", 2) == 0 ||
				 strncmp(argv[cur_arg],"-D", 2) == 0 )
		{
			//				fileName = argv[2];
			double temp;
			temp = saveAtof( argv[cur_arg]+2 ); // skip -D
			
			if( temp <= 0 )
			{
				ErrorMsg( 30 );
				temp = 100;
			}
			if( temp <= 2 )
				temp *= 100;
			relDuration = temp;
			relDuration = abs(relDuration);
			cur_arg++;
		}
		else if( strncmp(argv[cur_arg],"-c", 2) == 0 ||
			strncmp(argv[cur_arg],"-C", 2) == 0 )
		{
			Inifilename = argv[cur_arg]+2; // skip "-d"
			cur_arg++;
		}
		else if ( *argv[cur_arg] != '-' ) // no option -> filename
		{
			fileName = argv[cur_arg];
			cur_arg++;
		}
		else // unknown option
		{
			printf("Error: Unknown option %s!\n", argv[cur_arg]);
			okFlag = 0;
			cur_arg = argc; // stop
			ex_code = EXIT_UNKNOWN_OPTION;
		}
		
	} // while
	
	if( !Inifilename )
	{
		Inifilename = "fermata.ini";
	}
	
	TInifile	*Inifile = new TInifile( Inifilename );
	
	*Mode = VERBOSE;
	{ // check mode
		const char *temp;
		temp = Inifile->GetValChar("MODE",
									"INTERACTIVE",
									"[INTERACTIVE|SILENT] use SILENT for batch processing ");
		if( temp &&
			!strcmp( temp, "SILENT") )
		{
			*Mode = SILENT;
		}
	}
	
	if( okFlag )
	{
		if( relDuration == 0 ) // no -d
		{
			double tDuration;
			tDuration = Inifile->GetValFloat("PLAYDURATION","1","relation between played note durations and duration in score");
			if( tDuration <= 2 )
				tDuration *= 100;
			relDuration = tDuration;
			relDuration = abs(relDuration);
		}
		
		string str;
		if( !fileName ) // no filename in commandline
		{
			fileName = Inifile->GetValChar("FILENAME",NULL,NULL);
			if( !fileName )
			{
				str = InputQuestion("Input filename is missing, please enter filename: ","test.mid", NULL);
				fileName = str.c_str();
			}
		}
		fileNameMemory = new char [strlen(fileName)+1];
		strcpy(fileNameMemory, fileName);
	} // fileNameBlock;

	fileNameWork = fileNameMemory;

		char *fileNamePtr = fileNameWork;

		do // maybe multiple filenames
		{
			ex_code = 0;
			char *temp = strstr(fileNamePtr, ","); // search for komma separator
			if( temp )
			{
				fileNameWork = fileNamePtr; // copy filename
				*temp = 0; // str end
				temp++;
				fileNamePtr = temp;
			}
			else
			{
				fileNameWork = fileNamePtr;
				fileNamePtr = NULL;
			}
			if( strlen(fileNameWork) <= 0 )
				break;
				
			// delete whitespace
			sscanf(fileNameWork,"%s",fileNameWork);

			char *outFileName = new char[strlen(fileNameWork)+8+1];
			char *inFileName = new char[strlen(fileNameWork)+4+1];
			strcpy(outFileName, fileNameWork);


			// add ,mid to infilename and .gmn to outfilename
			char *midPostfix,
				 *gmnPostfix;

			/*
			postfix = strstr( outFileName, ".mid");
			if( !postfix )
				postfix = strstr( outFileName, ".MID");

			if( postfix )
				myStrupr(postfix);
//			outFileName = myStrupr(outFileName);
			*/
			midPostfix = const_cast<char *>( getMidPostfix( outFileName ));
			gmnPostfix = const_cast<char *>( getGmnPostfix( outFileName ));

			if(!gmnPostfix && 
			   !midPostfix && 
			   !strstr(outFileName , ".") )	// .mid is default
			{
				sprintf(outFileName, "%s.gmn", fileNameWork);
				// append default postfix
				sprintf(inFileName, "%s.mid", fileNameWork);
			}
			else if( midPostfix ) // s points to ".mid" of outfilename
			{
				s = midPostfix;
				strcpy(s,"gmn");
				strcpy(inFileName,fileNameWork);
			}
			else if( gmnPostfix )
			{
				strcpy(inFileName,fileNameWork);
				sprintf(outFileName, "%s.gmn", fileNameWork);
			}

			sprintf(outFileName, "%s", p_outPath);
			

			char *LOG_NAME = new char[strlen(inFileName)+strlen(DEF_LOG_NAME)+1];
			strcpy(LOG_NAME, inFileName);

			midPostfix = const_cast<char *>(getMidPostfix( LOG_NAME ));
			gmnPostfix = const_cast<char *>(getGmnPostfix( LOG_NAME ));

			if( midPostfix )
				temp = midPostfix;
			else if( gmnPostfix )
				temp = gmnPostfix;
			else
				temp = NULL;

			if( temp )
				strcpy( temp, "log");
			else
				strcpy( LOG_NAME, DEF_LOG_NAME );

			FILE *LOG_FILE = fopen(LOG_NAME,"wt");
			if( !LOG_FILE )
			{
				printf("Warning: can't open \"%s\", output will be written to error.log!\n",LOG_NAME);
				LOG_FILE = fopen("error.log","wt+");
				if(!LOG_FILE)
				{
					Printf("Error: can't open error.log!\n");
					ex_code = EXIT_CANT_OPEN_FILE;
					
				}
			}
			
			/* Process */
			THMIDIFILE *Infile;	// Ptr to MIDI-file
/*			
			FILE *TempOut;
			TempOut = fopen( LOG_NAME, "wt" );
*/			
			

			printf("Converting '%s' to '%s' ... \n",
						inFileName, outFileName);

			Infile = new THMIDIFILE(inFileName );
			Infile->setInifile(Inifile);
			if( !Infile )
			{
				printf("Error: Can't create THMIDIFILE %s\n",inFileName);
				ex_code = EXIT_FILE_NOT_FOUND;
			}
			else if( !Infile->Open(LOG_FILE) )
			{
				printf("Error: Can't open \"%s\"\n",inFileName);
				ex_code = EXIT_CANT_OPEN_FILE;
			}
			else if (ex_code)
			{
				// wait, do nothing
			}
			else
			{
				Infile->SetRelduration( relDuration );
				printf("playDuration: %d%%\n",relDuration );
				if( !Infile->Read(LOG_FILE) )
				{
					ErrorMsg( 1 );
					ex_code = EXIT_CANT_READ_FILE;
				}
				else
				{	
					
					Infile->Close(); // close midifile
					


					/*
					qDurations = readDurationList( DurationFilename );
					qAttacks = readAttackpointList( AttackFilename);
					qDurations->createDistribution();
					qAttacks->createDistribution();
					*/
					
					
					Infile->preProcess();
					Infile->Quantize( );

					/*
					if( qAttacks )
					{
						qAttacks->writeIni( AttackFilename );
						delete  qAttacks;
						qAttacks = NULL;
					}
					if( qDurations )
					{
						qDurations->writeIni( DurationFilename );
						delete  qDurations;
						qDurations = NULL;
					}
					*/
					// COnvert to GUIDO
					ofstream outfile (outFileName);
					string midiFileName = string(inFileName);
					if ( !Infile->Convert( outfile, midiFileName))
					{
						ErrorMsg( 4 );
						ex_code = EXIT_CANT_CONVERT;
					}
					outfile.close();
					// printf("... done.\n\n");
				} // else
			} // else
			
			delete Infile;
			delete [] outFileName;
			delete [] inFileName;
			
			if( LOG_FILE )
			{
				fclose( LOG_FILE);
				LOG_FILE = NULL;
			}
			if( LOG_NAME )
			{
				delete [] LOG_NAME;
				LOG_NAME = NULL;
			}
		} while(fileNamePtr); 
		if( str ) // result from Input Question
			delete [] str;
	} // if okFlag
	delete [] fileNameMemory;
#ifdef _DEBUG
	getc(stdin);
#endif
    clock_t finish = clock();
	double processTime = (double)(finish - start) / (double)CLOCKS_PER_SEC;
	printf("(%fs)\n",processTime);
	return( ex_code );
}



int processBuffer( std::string *buffer,
					std::string *fileName,
				 stringstream  &gmnOutput, 
				   const string &iniFile) {
	
	int res = -1;
	lgFactory *factory = new lgFactory();
	lgSegment *segment = new lgSegment(factory);
	if( fileName != NULL ){
		res = segment->parseFile(*fileName);
	}
	else
		res = segment->parseBuffer(buffer);
	if( res == 0 )
	{
		
		THMIDIFILE *midiFile = new THMIDIFILE(buffer);
		midiFile->setIniFilename(iniFile.c_str());

		lgSequence *sequence = segment->firstSequence();
		bool firstTempo = true;

		while (sequence != NULL) {
			THTRACK *track = new THTRACK(0, midiFile);
			midiFile->addTrack(track);

			printf("next sequence ***\n");
			lgTag *tag = sequence->firstTag();
			map<int, lgTag *> noteMap;
			while (tag != NULL) {
				// std::cout  << tag->name() << tag->id() << " at " << tag->pos().toString() << " ";
				// std::cout.flush();
				lgTagArg *arg = tag->firstArg();
				while (arg != NULL) {
					// std::cout << arg->name() << ":" << arg->valStr() << " / ";
					arg = (lgTagArg *) arg->next();
				}
				// std::cout << "end of tag args" << std::endl;
				// std::cout.flush();
				TAbsTime pos = TAbsTime(tag->pos().durN, tag->pos().durD);

				if (isTag(tag, "noteOn") || isTag(tag, "noteOff")) {
					int pitch = getPitch(tag);
					if (isTag(tag, "noteOn")) {
						noteMap[pitch] = tag;
					} else if (isTag(tag, "noteOff")) {
						lgTag *noteOn = noteMap[pitch];
						if (noteOn) {
							lgDuration onset = getOnset(noteOn);
							lgDuration duration = getDuration(noteOn, tag);
							int id = noteOn->id();

							TFrac onsetMIDI = TFrac(onset);
							TFrac durMIDI = TFrac(duration);
							int intens = getIntens(noteOn);

							noteMap[pitch] = NULL;
							/*
							std::cout << " id" << id << " pitch: " << pitch
									<< " intens: " << intens << " onset: "
									<< onsetMIDI.toString() << " duration: "
									<< durMIDI.toString();
								*/
							TNOTE *note = track->CreateNewNote(onsetMIDI, durMIDI,
									pitch, intens);
							note->setId(id);
							track->Insert(note);

						}
					}
				} else if (isTag(tag, "meter")) {
					// std::cout << "handle tag meter";
					string unit = "";
					string arg = "";
					if( tag->getParamChar("meter",1,arg, unit) > 0 )
					{
						vector<string> tokens;
						tokenize(arg,tokens, "/");
						if( tokens.size() == 2)
						{
							int num = str2int(tokens[0]);
							int denom = str2int(tokens[1]);
							// cout << "meter at " << pos.toString() << "ms " << num<<"/"<<denom << endl;
							TMetaMeter *meter = new TMetaMeter(pos,
									num,
									denom);
							if( meter != NULL)
								midiFile->getControlTrack()->Insert(meter);
						}
						else
						{
							cerr << "invalid meter arg: " << arg << std::endl;
						}
					}
					else
					{
						cerr << "Invalid meter tag\n";
					}

				} else if (isTag(tag, "tempo")) {
					int beatNum = 1;
					int beatDenom = 4;
					int bpm = 120;
					if( getBpm(tag, bpm, beatNum, beatDenom ) > 0 )
					{
						// cout << "tempo at " << pos.toString() << " " << beatNum << "/" << beatDenom << "="<< bpm << endl;
						TMetaTempo *tempo =
								new TMetaTempo(pos, bpm, beatNum, beatDenom);
						if( tempo != NULL )
						{
							midiFile->getControlTrack()->Insert(tempo);
							if( firstTempo && tempo->GetTempo() != 0)
							{
								float recPPQ = (float)60000.0 / (float)tempo->GetTempo();
								midiFile->setRecPPQ((int)recPPQ);
								midiFile->setRecTempo(tempo->GetTempo());
								firstTempo = false;
							}
							else
							{
								cerr << "Error: tempo changes not supported yet!!" << endl;
							}
						}
					}
					else
					{
						cerr << "Invalid tempo tag\n";
					}
				} else {
					std::cout << " unknown tag " << tag->name();
				}

				// std::cout << "\n";
				// std::cout.flush();
				tag = (lgTag*) tag->next();
			}

			sequence = (lgSequence *) sequence->next();
		}
		cout << "--- start processing --- "<< endl;
		if( firstTempo )
		{ // there was no tempo tag
			cout << "forcing tempo detection to on"<< endl;
			midiFile->setRecPPQ(1000);
			midiFile->setRecTempo(60);
			midiFile->SetCalcTempo( 1 );

		}
		midiFile->Debug();
		midiFile->preProcess();
		midiFile->Debug();

		midiFile->Quantize();
		midiFile->Debug();
		string empty = "";
		midiFile->Convert(gmnOutput, empty);
		delete midiFile;
		res = 1;
	}
	else
	{
		cerr << " Aborted " << endl;
		res = -1;
	}
	if( segment )
		delete segment; // this delets also the factory
	
	return res;
}


int noteName2pc(const char *c)
{
	if( strcmp(c, "c") == 0 )
	{
		return 0;
	}
	else if( strcmp(c, "cis") == 0 )
	{
		return 1;
	}
	else if( strcmp(c, "d") == 0 )
	{
		return 2;
	}
	else if( strcmp(c, "dis") == 0 )
	{
		return 3;
	}
	else if( strcmp(c, "e") == 0 )
	{
		return 4;
	}
	else if( strcmp(c, "f") == 0 )
	{
		return 5;
	}
	else if( strcmp(c, "fis") == 0 )
	{
		return 6;
	}
	else if( strcmp(c, "g") == 0 )
	{
		return 7;
	}
	else if( strcmp(c, "gis") == 0 )
	{
		return 8;
	}
	else if( strcmp(c, "a") == 0 )
	{
		return 9;
	}
	else if( strcmp(c, "ais") == 0 )
	{
		return 10;
	}
	else if( strcmp(c, "b") == 0 ||
			strcmp(c, "h") == 0)
	{
		return 11;
	}
	else
		return 99;
}
int getPitch(lgTag *tag) {
	int pitch = 0;

	if (tag != NULL) {
		std::string pitchArg = "";
		std::string unit = "";
		if (tag->getParamChar("pitch", 1, pitchArg, unit)) {
			const char *pitchStr = pitchArg.c_str();
			std::string pitchClass = "";
			int i = 0;
			while (isalpha(pitchStr[i])) {
				pitchClass += pitchStr[i];
				i++;
			}
			pitch = noteName2pc(pitchClass.c_str());
			while (pitchStr[i] == '#' || pitchStr[i] == '&') {
				if (pitchStr[i] == '#') {
					pitch++;
				}
				if (pitchStr[i] == '&') {
					pitch--;
				}
				i++;
			}
			int oct = 0;
			int sign = 1;
			if( pitchStr[i] == '-' )
			{
				i++;
				sign = -1;
			}
			while (isdigit(pitchStr[i])) {
				int cur = pitchStr[i] - '0';
				oct *= 10;
				oct += cur;
				i++;
			}
			oct *= sign;
			pitch = (oct + 4) * 12 + pitch;
		}
	}
	return pitch;
}

int getIntens(lgTag *tag) {
	std::string unit = "";
	int value = 0;
	if (tag != NULL) {
		if( tag->getParamInt("intens", 2, value, unit) <= 0)
			tag->getParamInt("vel", 2, value, unit);
	}

	return value;
}

lgDuration getOnset(lgTag *tag) {
	if (tag != NULL) {
		return tag->pos();
	}
	return lgDuration(0, 1);
}
lgDuration getDuration(lgTag *onset, lgTag *offset) {
	if (onset && offset) {
		return offset->pos() - onset->pos();
	}
	return lgDuration(0, 1);
}

bool isTag(lgTag *tag, const std::string &tagName) {
	if (tag == NULL)
		return false;
	std::string name = "\\" + tagName;
	return name.compare(tag->name()) == 0;
}

int getBpm(lgTag *tag, int &bpm, int &tactusNum, int &tactusDenom) {
	int res = -1;
	if (tag == NULL || !isTag(tag, "tempo")) {
		res = -1;
	} else {
		std::string arg = "";
		std::string unit = "";

		if (tag->getParamChar("tempo", 1, arg, unit) > 0) {
			vector<std::string> tokens;
			tokenize(arg, tokens, "/=");
			if (tokens.size() == 3) {
				bpm = str2int(tokens[2]);
				tactusNum = str2int(tokens[0]);
				tactusDenom = str2int(tokens[1]);
				res = 1;
			}
		}

	}
	return res;
}
int str2int(const string &str) {
	stringstream ss(str);
	int num;
	if ((ss >> num).fail()) {
		//ERROR
	}
	return num;
}

void tokenize(const std::string& str, vector<std::string>& tokens,
		const std::string& delimiters = " ") {
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}


int registerCallbacks(CBInputQuestion *inputQuestion,
								CBYesNoQuestion *yesNoQuestion,
								CBShowErrorMsg *errorMsg){
	cbInputQuestion = inputQuestion;
	cbYesNoQuestion = yesNoQuestion;
	cbShowErrorMsg = errorMsg;
	return 1;
}

string InputQuestion(const char *prompt, const char *def, const char * c/*midiFilename*/){
	if( cbInputQuestion != NULL ){
		return (*cbInputQuestion)(prompt, def, c);
	}
	else
	{
		return string(def);
	}
}

void ShowErrorMsg(char const *msg){
	if( cbShowErrorMsg != NULL )
		(*cbShowErrorMsg)(msg);
}
char const *YesNoQuestion(const char *prompt, const char *def, const char * c/*midiFilename*/){
	if( cbYesNoQuestion != NULL )
		return (*cbYesNoQuestion)(prompt, def, c);
	else
		return JK_YES;
}

