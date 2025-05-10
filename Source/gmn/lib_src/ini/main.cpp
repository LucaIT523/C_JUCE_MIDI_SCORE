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

/*
 *  main.cpp
 *  ini
 *
 *  Created by JŸrgen Kilian on Wed May 07 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "main.h"
#include "ini.h"

int main( int, char *[] )
{
    TInifile *ini;

    ini = new TInifile( "test.ini",1);

	/*
    ini->SetEntry("SINGLE_ENTRY","theValue1");
    ini->SetEntry("STR_ENTRY","theValue part 2 part3");
    ini->SetEntry("REMARK_ENTRY","basdjhb the Value part3", "the remark 3 sdfjhk");
    ini->SetEntry("STR_REMARK_ENTRY","the Value part4","the remark 4 sdfjhk");
    ini->SetEntry("SINGLE_REMARK_ENTRY","theValue5","the remark 5 sdkfjhsdkjh");
    ini->SetEntry("DOUBLE_ENTRY",1200.445, "The remark");
    

	ini->Write(1);
*/
	ini->Write(0);

    delete ini;
}
