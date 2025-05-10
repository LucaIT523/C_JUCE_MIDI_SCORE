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

#include <vector>
#include "process.h"
#include "string.h"
#include "meta_tempo.h"
#include "meta_meter.h"
#include "../include/fermata.h"
using namespace std;

// FILE *LOG_FILE; // accessed from all modules
// char *LOG_NAME = NULL; // accessed from different modules

// extern TInifile *Inifile;
std::string processString( std::string *glnString,
		const string &iniFileName) {
//	LOG_NAME = "jni_midi2gmn.log";
//	LOG_FILE = NULL; // fopen(LOG_NAME,"wt");

	stringstream ss (stringstream::in | stringstream::out);

	processBuffer( glnString, NULL, ss, iniFileName);

//	if( LOG_FILE != NULL )
//		fclose(LOG_FILE);

	return ss.str();

}


std::string processFile( std::string *file,
		const string &iniFileName) {
//	LOG_NAME = "jni_midi2gmn.log";
//	LOG_FILE = NULL; // fopen(LOG_NAME,"wt");

	stringstream ss (stringstream::in | stringstream::out);

	processBuffer( NULL, file, ss, iniFileName);

//	if( LOG_FILE != NULL )
//		fclose(LOG_FILE);

	return ss.str();

}

std::string readFile(const string &fileName) {
	stringstream oss;
	string line;

	ifstream myfile(fileName.c_str());
	if (myfile.is_open()) {
		while (myfile.good()) {
			getline(myfile, line);
			oss << line << endl;
		}
		myfile.close();
	} else {
		oss << "Can't open " << fileName;
	}
	return oss.str();
}



void ShowErrorMsg(const char *str) {
	fprintf(stderr, "Error: ");
	if (str)
		fprintf(stderr, "%s", str);
}

/// result must be deleted
string InputQuestion(const char *prompt, const char *def, const char * /*midiFilename*/) {

	return "";
}

const char *YesNoQuestion(const char *prompt, const char *def, const char * /*midiFilename*/) {

	return "yes";
}
void _stdcall MyPrintf(const char* ptr, int i, int j, int k) {
	std::cout << "implement MyPrintf";
}
void _stdcall MyPrintf(const char* ptr) {
	std::cout << "implement MyPrintf 2";

}
