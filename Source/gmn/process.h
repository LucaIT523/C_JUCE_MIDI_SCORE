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

#ifndef __process_h_
#define __process_h_


#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
using namespace std;


#include "./parser/guido.h"
#include "./leanguido/lgfactory.h"
#include "./leanguido/lgsegment.h"
#include "./fermata/H_midi.h"
#include "./fermata/H_track.h"
#include "./fermata/Fragm.h"
#include "./lib_src/ini/ini.h"

std::string processString( std::string *glnString,  const string &iniFile );
std::string processFile( std::string *file,  const string &iniFile );

/*
int processBuffer( std::string *glnString,
					std::string *file,
				stringstream  &gmnOutput,
				const string &iniFile );
				*/
std::string readFile(const string & fileName);

#endif
