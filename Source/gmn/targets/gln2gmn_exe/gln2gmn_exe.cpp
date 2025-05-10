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

// jni_dll_exe.cpp : Defines the entry point for the console application.
//


#include "../../fermata/debug.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "../../process.h"



int main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	string ini = "test.ini";
	
	

//	string gln = readFile("N:\\C_src\\sony_exe_Debug\\bch062.gln");
	for( int i = 1; i < argc; i++ )
	{
		string fname =  string(argv[i]);
		string res = processFile(&fname, ini);
		std::cout << res;
	}
/*
		gln = readFile("N:\\C_src\\sony_exe_Debug\\MelodyCrash4.gln");
	 res = processString( &gln, ini);
	std::cout << res;
*/
	return 0;
}



